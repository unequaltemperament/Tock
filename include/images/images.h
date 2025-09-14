#ifndef IMAGES_HEADER
#define IMAGES_HEADER

enum class BitmapEncoding
{
    none,
    bit4,
    bit8
};

struct Bitmap
{
    BitmapEncoding encoding;
    int width;
    int height;
    int dataLength;
    const byte *data;
};

#include "splash.h"
#include "power.h"


// TODO: future things
// #include "tomato.h"

#endif