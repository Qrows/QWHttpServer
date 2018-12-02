/* to get NI_MAXHOST and NI_MAXSERV definition from <netdb.h>*/
//#define _BSD_SOURCE

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include "inet_sockets.h"

int inetConnect(const char *host, const char *service, int type)
{
	struct addrinfo hints = {0};
	struct addrinfo *result = NULL, *rp = NULL;
	int sfd = 0, s = 0;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;  /* Allow IPv4 or IPv6 */
	hints.ai_socktype = type;

	s = getaddrinfo(host, service, &hints, &result);
	if (s != 0) {
		errno = ENOSYS;
		return -1;
	}
	/* Walk through returned list until we find an address structure
	   that can be used to successfully connect a socket */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1) {
			continue;
		}
		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
			break; /* Success */
		}

		/* Connection failed, close this socket and try another */

		close(sfd);
	}
	freeaddrinfo(result);

	return (rp == NULL) ? -1 : sfd;
}

static int inetPassiveSocket(const char *service,
                             int type,
                             socklen_t *addrlen,
                             bool doListen,
                             int backlog)
{
	/* Public interface: inetListen() and inetBind() */
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, optval, s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;  /* Allow IPv4 or IPv6 */
	hints.ai_socktype = type;
	hints.ai_flags = AI_PASSIVE;  /* Use wildcards IP address */

	s = getaddrinfo(NULL, service, &hints, &result);
	if (s != 0)
		return -1;
	/* Walk through returned list until we find an address structure
	   that can be used to successfully create and bind a socket */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;    /* On error, try next address*/

		if (doListen) {
			optval = 0;
			if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval,
				       sizeof(optval)) == -1) {
				close(sfd);
				freeaddrinfo(result);
				return -1;
			}
		}

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
			break; /* Success */
		}

		/* bind() failed */
		close(sfd);
	}

	if (rp != NULL && doListen) {
		if (listen(sfd, backlog) == -1) {
			freeaddrinfo(result);
			return -1;
		}
	}

	if (rp != NULL && addrlen != NULL) {
		*addrlen = rp->ai_addrlen;  /* Return address return size */
	}
	freeaddrinfo(result);
	return (rp == NULL) ? -1 : sfd;
}

int inetListen(const char *service, int backlog, socklen_t *addrlen)
{
	return inetPassiveSocket(service, SOCK_STREAM, addrlen, true, backlog);
}


char *inetAddressStr(const struct sockaddr *addr, socklen_t addrlen,
                     char *addrStr, int addrStrLen)
{
	char host[NI_MAXHOST], service[NI_MAXSERV];
	if (getnameinfo(addr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV,
			NI_NUMERICSERV) == 0) {
		snprintf(addrStr, addrStrLen, "(%s, %s)", host, service);
	} else {
		snprintf(addrStr, addrStrLen, "(?UNKNOWN?)");
	}
	addrStr[addrStrLen - 1] = '\0'; /* Ensure result is null terminated */
	return addrStr;
}

