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

/**
 * lcopen() - open a LISTENER_CONNECTION, that is connection listening
 * for incoming connection.
 * @service: indicate on which port the CONNECTION should listen.
 * @backlog indicate how many request can't have in the waiting queue
 * @addrlen is where to put a socklen_t struct indicating connection
 * detail.
 * RETURN:
 * in case of success return a valid LISTENER_CONNECTION pointer,
 * otherwise return NULL and set errno.
 */
LISTENER_CONNECTION *lcopen(char *service, int backlog, socklen_t *addrlen);

/**
 * lcclose() - close an open LISTENER_CONNECTION created by lcopen()
 * @connection: open LISTENER_CONNECTION to close
 * close a non-opened LISTENER_CONNECTION is
 * undefined behaviour.
 * RETURN:
 * return 0 in case of success
 * return -1 in case of failure and set errno.
 */
int lcclose(LISTENER_CONNECTION *connection);

/**
 * copen() - open a CONNECTION given a set of parameters.
 * @attr: CONNECTION_attr struct given socket specific opening parameters
 * to init a connection needed other value put in the CONNECTION_attr
 * value.
 * @write_buffer_size: the size of the buffer for buffering writing
 * @read_buffer_size: the size of the buffer for buffering reading
 * @socket_type: can be two value ACCEPT or CONNECT, indicate
 *   the type of the socket, if should come from an accept(2) or a
 *   connect(2) call.
 * in case the socket_type is ACCEPT
 * + @addr, where the call accept(2) put socket info about
 *   who is connecting to our socket.
 * + @addr_len, where the call accept(2) put socket info
 *   about the incoming client.
 * in case the socket_type is CONNECT
 * + @service, indicate the port or the service to which connect
 * + @host, specifiy to which host should we connect
 * + @type, indicate the type of the socket.
 * for more detail references the man pages of accept(2) and connect(2).
 * RETURN:
 * in case of success return a valid CONNECTION pointer.
 * otherwise return NULL and set errno.
 * 
*/
CONNECTION *copen(struct CONNECTION_attr *attr);

/**
 * cclose() - close an open CONNECTION.
 * @connection: open CONNECTION to close
 * @how: OR_RING flag with the following accepted parameters
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
int cclose(CONNECTION *connection, int how);

/**
 * crecv() - read to n bytes from the CONNECTION and store it
 * inside buf, if no data is sent for more than timeout seconds
 * the call will fail and set errno to ETIMER.
 * @connect: open CONNECTION
 * @buf: where to store the data readed
 * @n: size of the data requested
 * @timeout: timer if no data is sent for tot second.
 * The read are buffered, however it should be totaly transperent
 * to the caller.
 * RETURN:
 * in case of success return the number of bytes readed,
 * this number can be less than n.
 * otherwise return -1 and set errno.
 */
ssize_t crecv(CONNECTION *connect, char *buf, size_t n, int timeout);

/** 
 * csend() - read n bytes from buf and send it through
 * a connection.
 * @connect: open CONNECTION.
 * @buf: buffer containing the data to send.
 * @n: size of the data to send inside buf
 * The sending is buffered, that means that a call to csend()
 * don't necessairily means that the data will be automatically
 * sent after this call return.
 * for be sure of that call cflush() after csend().
 * RETURN:
 * in case of success return the number of bytes sent/saved to buffer
 * it should always send n bytes.
 * otherwise return -1.
 */
ssize_t csend(CONNECTION *connect, char *buf, size_t n);

/**
 * csendfile() - sent file_size bytes of a file whose path is fpath
 * through a open CONNECTION.
 * @connect: open CONNECTION.
 * @fpath: path to the file.
 * @file_size: size of the file to send.
 * this call uses sendfile(2), so the file are sent with a zero-copy
 * technique, making it faster.
 * RETURN:
 * in case of success return the number of bytes sent,
 * otherwise return -1 and set errno.
 */
ssize_t csendfile(CONNECTION *connect, char *fpath, size_t file_size);

/**
 * crecvline() - read from a connection a \r\n terminated line and store
 * it inside buf.
 * @connect: open CONNECTION
 * @buf: buffer where to store the line readed
 * @n: size of buf
 * @timeout: timeouts in seconds.
 * this call will read up to n bytes, if no newline is found after reading
 * n bytes, the call fails.
 * if no data are sent for more than timeout seconds, the call will fail
 * and set errno to ETIME.
 * RETURN:
 * in case of success return the number of bytes readed.
 * otherwise return -1 and set errno.
*/
ssize_t crecvline(CONNECTION *connect, char *buf, size_t n, int timeout);

/**
 *  cflush() - flush the sending buffer of the CONNECTION
 * sending it even if the buffer is not full.
 * @connect: open CONNECTION.
 * csend() call is buffered, so that means that it will be sent
 * when a certain threshold of data is reached.
 * this call permit to send the stored data even if that threashold
 * is not reached.
 * RETURN:
 * in case of success return 0.
 * otherwise return -1 and set errno.
 */
int cflush(CONNECTION *connect);

/**
 * cget_ipaddr() - get the ip address of the connection other host
 * @connect: open connection.
 */

/**
 * cgetpeername - return the address of the client
 * @connection: open connection
 * @address: struct sockaddr where the address will be stored
 * @address_len: size of the struct sockaddr
 */
int cgetpeername(const CONNECTION *connection, struct sockaddr *address, socklen_t *address_len);


#endif
