#ifndef THREADWORK_H
#define THREADWORK_H

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "http.h"
#include "server.h"


void *threadwork(void *data);

#endif
