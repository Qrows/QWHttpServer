
#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "MagickWand/MagickWand.h"

int compress_image(char *src, char *dest, int quality);

#endif

