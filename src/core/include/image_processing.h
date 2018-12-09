
#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "wand/MagickWand.h"

/**
 * compress_image - compress an image given is source and destination file-path
 * @src: path to the image  to compress
 * @dest: path of the compressed image
 * @quality: quality of the new compression
 */
int compress_image(char *src, char *dest, int quality);

#endif

