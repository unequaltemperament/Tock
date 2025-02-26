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

// TODO: future things
// #include "battery.h"
// #include "tomato.h"

#endif