/* To prevent accidental double inclusion */
#ifndef INET_SOCKETS_H
#define INET_SOCKETS_H

#include <sys/socket.h>
#include <netdb.h>

/*
  DESCRIPTION:
  The inetConnect() creates a socket with the given socket _type_ and
  connect it to the address specified in host and service.
  This function is designed for TCP and UDP client that need to connect their
  socket to a server socket.

  RETURN VALUE:
  Return the file descriptor of the created socket on success, or -1 on error.
*/
int inetConnect(const char *host, const char *service, int type);

/*
  DESCRIPTION:
  The inetListen() creates a listening stream (SOCK_STREAM) socket bound to the
  wildcard IP address on the TCP port specified by _service_.
  This function is designed to use by the TCP server.

  The _backlog_ value specifies the permitted backlog of pending connection.

  If _addrlen_ is specified as a non-NULL pointer, then the location it points
  to is used to return the size of the socket address structure corresponding
  to the returned file descriptor.
  This value(_addrlen_) allows us to allocate a socket address buffer of the
  appropriate size to be passed to a later accept() call if we want to obtain
  the address of a connecting client.

  RETURN VALUE:
  Return the file descriptor of the created socket on success, or -1 on error.
*/
int inetListen(const char *service, int backlog, socklen_t *addrlen);

/*
  The inetAddressStr() function converts an Internet socket address to
  printable form.
  Given a socket address structure in addr, whose length is specified in addrlen,
  inetAddressStr() returns a null-terminated string containing the corresponding host-
  name and port number in the following form:
  (hostname, port-number)

  The string is returned in the buffer pointed to by addrStr. The caller must specify the
  size of this buffer in addrStrLen. If the returned string would exceed (addrStrLen – 1)
  bytes, it is truncated. The constant IS_ADDR_STR_LEN defines a suggested size for the
  addrStr buffer that should be large enough to handle all possible return strings. As
  its function result, inetAddressStr() returns addrStr.

  RETURN VALUE:
  Returns pointer to addrStr, a string containing host and service name
*/
char *inetAddressStr(const struct sockaddr *addr, socklen_t addrlen,
                     char *addrStr, int addrStrLen);

#define IS_ADDR_STR_LEN 4096
#endif
