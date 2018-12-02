#include "image_processing.h"

int compress_image(char *src, char *dest, int quality)
{
	MagickWand *mw = NULL;
	MagickBooleanType res = MagickFalse;

	if (src == NULL || dest == NULL|| quality < 0 || quality > 100) {
		errno = EINVAL;
		return -1;
	}
	MagickWandGenesis();
	/* Create a wand */
	mw = NewMagickWand();
	if (mw == NULL)
		return -1;
	/* Read the input image */
	res = MagickReadImage(mw, src);
	if (res == MagickFalse)
		return -1;
	res = MagickSetImageCompressionQuality(mw, quality);
	if (res == MagickFalse)
		return -1;
	/* write it */
	res = MagickWriteImage(mw, dest);
	if (res == MagickFalse)
		return -1;
	/* Tidy up */
	mw = DestroyMagickWand(mw);
	MagickWandTerminus();
	return 0;
}
