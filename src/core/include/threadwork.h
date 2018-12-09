#ifndef THREADWORK_H
#define THREADWORK_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "http.h"
#include "server.h"

/**
 * threadwork - server work for a single thread
 * @data: struct global_server_data containing the data to start the server work
 * this function will:
 * 1. listen for incoming connection
 * 2. execute an http exchange
 * 3. close the connection when the exchange is over
 * 4. repeat 
 */
void *threadwork(void *data);

#endif
