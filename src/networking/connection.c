#include "connection.h"

struct listener_socket {
	int lsock;     /* socket listening file descriptor */
};

struct buffered_socket {
	int sock;             /* socket file descriptor */
	char *write_buf;      /* pointer to writer buffer */
	size_t write_buf_size;/* size of allocated writer buffer*/
	size_t buffered_write;/* how much is stored in the writer buffer*/
	sem_t wmutex;         /* mutex to access the writer buffer THREAD-SAFE*/  
	char *read_buf;       /* buffer to reader buffer */
	size_t read_buf_size; /* reader buffer size*/
	char *to_read;        /* where to start reading */
	size_t to_read_size;  /* how much there is to read */
	sem_t rmutex;         /* mutex to access the reader buffer THREAD-SAFE*/
};

static int flush(CONNECTION *connect);

#define MIN(a, b) ((a < b) ? a : b)

LISTENER_CONNECTION *lcopen(char *service, int backlog, socklen_t *addrlen)
{
	LISTENER_CONNECTION *lconn = NULL; 
	lconn = malloc(sizeof(*lconn));
	if (lconn == NULL)
		return NULL;
	lconn->lsock = inetListen(service, backlog, addrlen);
	if (lconn->lsock < 0) {
		free(lconn);
		return NULL;
	}
	return lconn;
}

int lcclose(LISTENER_CONNECTION *connection)
{
	int res = 0;
	res = close(connection->lsock);
	free(connection);
	return res;
}

CONNECTION *copen(struct CONNECTION_attr *attr)
{
	CONNECTION *connection = NULL;
	int err = 0;
	if (attr == NULL) {
		errno = EINVAL;
		return NULL;
	}
	connection = malloc(sizeof(*connection));
	if (connection == NULL)
		return NULL;
	connection->write_buf = calloc(sizeof(char), attr->write_buffer_size);
	if (connection->write_buf == NULL) {
		free(connection);
		return NULL;
	}
	connection->read_buf = calloc(sizeof(char), attr->read_buffer_size);
	if (connection->read_buf == NULL) {
		free(connection->write_buf);
		free(connection);
		return NULL;
	}
	err = sem_init(&(connection->wmutex), 0, 1);
	if (err < 0) {
		free(connection->write_buf);
		free(connection->read_buf);
		free(connection);
	}
	err = sem_init(&(connection->rmutex), 0, 1);
	if (err < 0) {
		free(connection->write_buf);
		free(connection->read_buf);
		free(connection);
	}
	if (attr->socket_type == ACCEPT)
		connection->sock = accept(attr->listener->lsock,
					  attr->addr,
					  attr->addr_len);
	else
		connection->sock = inetConnect(attr->host,
					       attr->service,
					       attr->type);
	if (connection->sock < 0) {
		free(connection->write_buf);
		free(connection->read_buf);
		sem_destroy(&(connection->wmutex));
		sem_destroy(&(connection->rmutex));
		free(connection);
		return NULL;
	}
	connection->write_buf_size = attr->write_buffer_size;
	connection->read_buf_size = attr->read_buffer_size;
	connection->to_read = connection->read_buf;
	connection->to_read_size = 0;
	connection->buffered_write = 0;
	
	return connection;
}
 
int cclose(CONNECTION *connection, int how)
{
	int res = 0;
	if (connection == NULL)
		return 0;
	res = shutdown(connection->sock, how);
	free(connection->write_buf);
	free(connection->read_buf);
	sem_destroy(&(connection->wmutex));
	sem_destroy(&(connection->rmutex));
	free(connection);
	return res;
}

static ssize_t
crecv_notsync(CONNECTION *connect, char *buf, size_t n, int timeout)
{
	int ready = 0;
	struct timeval tm_out;
	fd_set read_set;
	// read first from buffered read
	if (connect->to_read_size != 0) {
		if (connect->to_read_size > n) {
			memcpy(buf, connect->to_read, n);
			connect->to_read += n;
			connect->to_read_size -= n;
			return n;
		} else if (connect->to_read_size == n) {
			memcpy(buf, connect->to_read, n);
			connect->to_read = connect->read_buf;
			connect->to_read_size = 0;
			return n;
		} else {
			memcpy(buf, connect->to_read, connect->to_read_size);
			n -= connect->to_read_size;
			buf += connect->to_read_size;
			connect->to_read = connect->read_buf;
			connect->to_read_size = 0;
		}
	}
	// set fd to watch
	FD_ZERO(&read_set);
	FD_SET(connect->sock, &read_set);

	/* set timeout val*/
	tm_out.tv_sec = timeout;
	tm_out.tv_usec = 0;
	ready = select(connect->sock + 1, &read_set, NULL, NULL, &tm_out);
	if (ready == -1)
		return -1;
	if (ready) {
		/* data available
		 * when data is available if is greater than n
		 * buffer it and return the first n readed bytes
		 * if is less return all the readed bytes
		 * if n is greater than the buffer size
		 * skip it and copy it directly to buf
		 */
		ssize_t rd = 0;
		if (connect->read_buf_size > n) {
			rd = recv(connect->sock,
				  connect->read_buf,
				  connect->read_buf_size,
				  0);
			if (rd <= 0) {
				return rd;
			}
			if ((size_t)rd <= n) {
				memcpy(buf, connect->read_buf, rd);
				return rd;
			} else {
				memcpy(buf, connect->read_buf, n);
				connect->to_read += n;
				connect->to_read_size = rd - n;
				return n;
			}
		} else {
			rd = recv(connect->sock, buf, n, 0);
			return rd;
		}
	} else {
		/* timeout expired*/
		errno = ETIME;
		return -1;
	}
}

ssize_t crecv(CONNECTION *connect, char *buf, size_t n, int timeout)
{
	ssize_t res = 0;
	sem_wait(&(connect->rmutex));
	res = crecv_notsync(connect, buf, n, timeout);
	sem_post(&(connect->rmutex));
	return res;
}

static ssize_t csend_notsync(CONNECTION *connect, char *buf, size_t n)
{
	ssize_t wr = 0;
	if (connect == NULL || buf == NULL) {
		errno = EINVAL;
		return -1;
	}
	/* if n is greater than the buffer 
	 * flush what's inside and send the buffer
	 * directly without buffering.
	 */
	if (n >= connect->write_buf_size) {
		int res = 0;
		res = flush(connect);
		if (res < 0)
			return -1;
		wr = send(connect->sock,
			  buf,
			  n,
			  MSG_NOSIGNAL);
		return wr;
	} else {
		/* if n is less than the buffer
		 * send through buffered IO
		 */
		if (connect->buffered_write + n <= connect->write_buf_size) {
			memcpy(connect->write_buf + connect->buffered_write,
			       buf,
			       n);
			connect->buffered_write += n;
		} else {
			flush(connect);
			memcpy(connect->write_buf, buf, n);
			connect->buffered_write += n;
		}
		return n;
	}	
}

ssize_t csend(CONNECTION *connect, char *buf, size_t n)
{
	ssize_t res = 0;
	sem_wait(&(connect->wmutex));
	res = csend_notsync(connect, buf, n);
	sem_post(&(connect->wmutex));
	return res;
}

static int flush(CONNECTION *connect)
{
	ssize_t wr = 0;
	if (connect->buffered_write == 0)
		return 0;
	wr = send(connect->sock,
		  connect->write_buf,
		  connect->buffered_write,
		  MSG_NOSIGNAL);
	if (wr < 0)
		return -1;
	connect->buffered_write = 0;
	return 0;
}

int cflush(CONNECTION *connect)
{
	int res = 0;
	if (connect == NULL) {
		errno = EINVAL;
		return -1;
	}
	sem_wait(&(connect->wmutex));
	res = flush(connect);
	sem_post(&(connect->wmutex));
	return res;
}

ssize_t csendfile(CONNECTION *connect, char *fpath, size_t file_size)
{
	int fd = 0;
	ssize_t wr = 0;
	off_t pos = 0;
	if (connect == NULL || fpath == NULL) {
		errno = EINVAL;
		return -1;
	}
	fd = open(fpath, O_RDONLY);
	if (fd < 0)
		return -1;
	while (file_size > 0) {
		wr = sendfile(connect->sock, fd, &pos,file_size);
		if (wr < 0)
			return wr;
		file_size -= wr;
	}
	close(fd);
	return file_size;
}

ssize_t crecvline(CONNECTION *connect, char *buf, size_t n, int timeout)
{
    size_t totRead = 0;
    ssize_t numRead = 0;
    char ch = 0;

    if (buf == NULL || n <= 0) {
        errno = EINVAL;
        return -1;
    }
    totRead = 0;
    ch = 0;
    while (ch != '\n') {
        /* read() one bytes */
	    numRead = crecv(connect, &ch, 1, timeout);
        if (numRead == -1) {
            if (errno == EINTR) {
                /* read() interrupted by signal*/
                continue;
            } else {
                /* error */
                return -1;
            }
        } else if (numRead == 0) {
            if (totRead == 0) {
                /* 0 bytes readed */
                return 0;
            } else {
                /* EOF reached*/
                break;
            }
        } else {
            /* discard any bytes over n - 1 bytes */
            if (totRead < n - 1) {
                *buf = ch;
                buf++;
                totRead++;
            }
        }
    }
    *buf = '\0';
    /* return number of read bytes*/
    return totRead;

}
