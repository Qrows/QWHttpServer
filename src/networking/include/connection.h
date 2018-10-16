#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/sendfile.h>
#include <semaphore.h>
#include "inet_sockets.h"

enum socket_type {
	ACCEPT,
	CONNECT
};

typedef struct listener_socket LISTENER_CONNECTION;
typedef struct buffered_socket CONNECTION;

struct CONNECTION_attr {
	size_t write_buffer_size;
	size_t read_buffer_size;
	enum socket_type socket_type;
	LISTENER_CONNECTION *listener;
	struct sockaddr *addr;
	socklen_t *addr_len;
	char *service;
	char *host;
	int type;
};

LISTENER_CONNECTION *lcopen(char *service, int backlog, socklen_t *addrlen);
/* lcopen() will open a LISTENER_CONNECTION, that is connection listening
 * for incoming connection.
 * + *service indicate on which port the CONNECTION should listen.
 * + backlog indicate how many request can't have in the waiting queue
 * + addrlen is where to put a socklen_t struct indicating connection
 *   detail.
 * RETURN:
 * in case of success return a valid LISTENER_CONNECTION pointer,
 * otherwise return NULL and set errno.
 */
int lcclose(LISTENER_CONNECTION *connection);
/* lcclose() will close an open LISTENER_CONNECTION created by
 * lcopen(), close a non-opened LISTENER_CONNECTION is
 * undefined behaviour.
 * RETURN:
 * return 0 in case of success
 * return -1 in case of failure and set errno.
 */


CONNECTION *copen(struct CONNECTION_attr *attr);
/* copen() will open a CONNECTION given a set of parameters.
 * to init a connection needed other value put in the CONNECTION_attr
 * value.
 * the value needed are:
 * + attr->write_buffer_size, the size of the buffer for buffering writing
 * + attr->read_buffer_size, the size of the buffer for buffering reading
 * + attr->socket_type, can be two value ACCEPT or CONNECT, indicate
 *   the type of the socket, if should come from an accept(2) or a
 *   connect(2) call.
 * in case the socket_type is ACCEPT
 * + attr->addr, where the call accept(2) put socket info about
 *   who is connecting to our socket.
 * + attr->addr_len, where the call accept(2) put socket info
 *   about the incoming client.
 * in case the socket_type is CONNECT
 * + attr->service, indicate the port or the service to which connect
 * + attr->host, specifiy to which host should we connect
 * + attr->type, indicate the type of the socket.
 * for more detail references the man pages of accept(2) and connect(2).
 * RETURN:
 * in case of success return a valid CONNECTION pointer.
 * otherwise return NULL and set errno.
 * 
*/
int cclose(CONNECTION *connection, int how);
/* cclose() will close the connection using the how
 * parameters as an OR RING specifing how to close
 * the connection.
 * the accepted how value are
 * + O_RDONLY for closing only in reading
 * + O_WRONLY for closing only in writing
 * + O_RDWR for closing both reading and writing
 * calling cclose() on a not-opened connection is undefined
 * behaviour.
 * RETURN:
 * in case of return return 0.
 * in case of failure return -1 and set errno.
*/

ssize_t crecv(CONNECTION *connect, char *buf, size_t n, int timeout);
/* crecv() will read to n bytes from the CONNECTION and store it
 * inside buf, if no data is sent for more than timeout seconds
 * the call will fail and set errno to ETIMER.
 * The read are buffered, however it should be totaly transperent
 * to the caller.
 * RETURN:
 * in case of success return the number of bytes readed.
 * otherwise return -1 and set errno.
 */
ssize_t csend(CONNECTION *connect, char *buf, size_t n);
/* csend() will read n bytes from buf and send it through
 * a connection.
 * The sending is buffered, that means that a call to csend()
 * don't necessairily means that the data will be automatically
 * sent after this call return.
 * for be sure of that call cflush() after csend().
 * RETURN:
 * in case of success return the number of bytes sent/saved to buffer
 * otherwise return -1.
 */

ssize_t csendfile(CONNECTION *connect, char *fpath, size_t file_size);
/* csendfile() will sent file_size bytes of a file whose path is fpath
 * through a open CONNECTION.
 * this call uses sendfile(2), so the file are sent with a zero-copy
 * technique, making it faster.
 * RETURN:
 * in case of success return the number of bytes sent,
 * otherwise return -1 and set errno.
 */
ssize_t crecvline(CONNECTION *connect, char *buf, size_t n, int timeout);
/* crecvline() will read from a connection a \r\n terminated line and store
 * it inside buf.
 * this call will read up to n bytes, if no newline is found after reading
 * n bytes, the call fails.
 * if no data are sent for more than timeout seconds, the call will fail
 * and set errno to ETIME.
 * RETURN:
 * in case of success return the number of bytes readed.
 * otherwise return -1 and set errno.
*/

int cflush(CONNECTION *connect);
/* cflush() will flush the sending buffer of the CONNECTION
 * sending it even if the buffer is not full.
 * csend() call is buffered, so that means that it will be sent
 * when a certain threshold of data is reached.
 * this call permit to send the stored data even if that threashold
 * is not reached.
 * RETURN:
 * in case of success return 0.
 * otherwise return -1 and set errno.
*/

#endif
