#include <stdio.h>
#include "images.h"
#include <memory.h>
#pragma pack(push,1)
typedef struct
{
    unsigned char x;
    unsigned char y;
    unsigned char z;
    unsigned char w;
} uchar4;

/**
 * ColorPelette of type uchar4
 */
typedef uchar4 ColorPalette;

/**
 * struct Bitmap header
 */
typedef struct {
    short id;
    int size;
    short reserved1;
    short reserved2;
    int offset;
} BitMapHeader;


/**
 * struct Bitmap info header
 */
typedef struct {
    int sizeInfo;
    int width;
    int height;
    short planes;
    short bitsPerPixel;
    unsigned compression;
    unsigned imageSize;
    int xPelsPerMeter;
    int yPelsPerMeter;
    int clrUsed;
    int clrImportant;
} BitMapInfoHeader;

static const short bitMapID = 19778;

cl_uchar4* read_bmp_image(const char * filename, cl_uint * width, cl_uint *height)
{
    // Open BMP file
    FILE * fd = fopen(filename, "rb");
	BitMapHeader header;
	BitMapInfoHeader info_header;
	int numColors_;	
	ColorPalette * colors_;	
	uchar4 * pixels_;
    // Opened OK
    if (fd != NULL) {
        // Read header
        fread(&header, sizeof(BitMapHeader), 1, fd);

        // Failed to read header
        if (ferror(fd)) {
            fclose(fd);
            return NULL;
        }

        // Confirm that we have a bitmap file
        if (header.id != bitMapID) {
            fclose(fd);
            return NULL;
        }

        // Read map info header
        fread(&info_header, sizeof(BitMapInfoHeader), 1, fd);

        // Failed to read map info header
        if (ferror(fd)) {
            fclose(fd);
            return NULL;
        }

		*width=info_header.width;
		*height=info_header.height;

        // No support for compressed images
        if (info_header.compression) {
            fclose(fd);
            return NULL;
        }

        // Support only 8 or 24 bits images
        if (info_header.bitsPerPixel < 8) {
            fclose(fd);
            return NULL;
        }

        // Store number of colors
        numColors_ = 1 << info_header.bitsPerPixel;

        //load the palate for 8 bits per pixel
        if(info_header.bitsPerPixel == 8) {
            colors_ = new ColorPalette[numColors_];
            if (colors_ == NULL) {
                fclose(fd);
                return NULL;
            }
            fread(
                (char *)colors_,
                numColors_ * sizeof(ColorPalette),
                1,
                fd);

            // Failed to read colors
            if (ferror(fd)) {
                fclose(fd);
                return NULL;
            }
        }

        // Allocate buffer to hold all pixels
        unsigned int sizeBuffer = header.size - header.offset;
        unsigned char * tmpPixels = new unsigned char[sizeBuffer];

        if (tmpPixels == NULL) {
            delete colors_;
            colors_ = NULL;
            fclose(fd);
            return NULL;
        }

        // Read pixels from file, including any padding
        fread(tmpPixels, sizeBuffer * sizeof(unsigned char), 1, fd);

        // Failed to read pixel data
        if (ferror(fd)) {
            delete colors_;
            colors_ = NULL;
            delete tmpPixels;
            fclose(fd);
            return NULL;
        }

        // Allocate image
        pixels_ = new uchar4[info_header.width * info_header.height];
        if (pixels_ == NULL) {
            delete colors_;
            colors_ = NULL;
            delete tmpPixels;
            fclose(fd);
            return NULL;
        }
        // Set image, including w component (white)
        memset(pixels_, 0xff, info_header.width * info_header.height * sizeof(uchar4));

        unsigned int index = 0;
        for(int y = 0; y < info_header.height; y++) {
            for(int x = 0; x < info_header.width; x++) {
                // Read RGB values
                if (info_header.bitsPerPixel == 8) {
                    pixels_[(y * info_header.width + x)] = colors_[tmpPixels[index++]];
                }
                else { // 24 bit
                    pixels_[(y * info_header.width + x)].z = tmpPixels[index++];
                    pixels_[(y * info_header.width + x)].y = tmpPixels[index++];
                    pixels_[(y * info_header.width + x)].x = tmpPixels[index++];
                }
            }

            // Handle padding
            for(int x = 0; x < (4 - (3 * info_header.width) % 4) % 4; x++) {
                index++;
            }
        }

        // Loaded file so we can close the file.
        fclose(fd);
        delete[] tmpPixels;
    }
	return (cl_uchar4*)pixels_;
}
int colorIndex(uchar4 color,int numColors_,ColorPalette * colors_)
{
    for (int i = 0; i < numColors_; i++) {
        if (colors_[i].x == color.x &&
            colors_[i].y == color.y &&
            colors_[i].z == color.z &&
            colors_[i].w == color.w) {
            return i;
        }
    }

    return true;
}

bool write_bmp_image(const char * filename,cl_uchar4 * pixels)
{
	uchar4 *pixels_=(uchar4*)pixels;
	BitMapHeader header;
	BitMapInfoHeader info_header;
	int numColors_;
	ColorPalette * colors_;	
    // Open BMP file
    FILE * fd = fopen(filename, "wb");

    // Opened OK
    if (fd != NULL) {
        // Write header
        fwrite(&header, sizeof(BitMapHeader), 1, fd);

        // Failed to write header
        if (ferror(fd)) {
            fclose(fd);
            return false;
        }

        // Write map info header
        fwrite(&info_header, sizeof(BitMapInfoHeader), 1, fd);

        // Failed to write map info header
        if (ferror(fd)) {
            fclose(fd);
            return false;
        }

        // Write palate for 8 bits per pixel
        if(info_header.bitsPerPixel == 8) {
            fwrite(
                (char *)colors_,
                numColors_ * sizeof(ColorPalette),
                1,
                fd);

            // Failed to write colors
            if (ferror(fd)) {
                fclose(fd);
                return false;
            }
        }

        for(int y = 0; y < info_header.height; y++) {
            for(int x = 0; x < info_header.width; x++) {
                // Read RGB values
                if (info_header.bitsPerPixel == 8) {
                    fputc(
                        colorIndex(
                            pixels_[(y * info_header.width + x)], numColors_,colors_),
                            fd);
                }
                else { // 24 bit
                    fputc(pixels_[(y * info_header.width + x)].z, fd);
                    fputc(pixels_[(y * info_header.width + x)].y, fd);
                    fputc(pixels_[(y * info_header.width + x)].x, fd);

                    if (ferror(fd)) {
                        fclose(fd);
                        return false;
                    }
                }
            }

            // Add padding
            for(int x = 0; x < (4 - (3 * info_header.width) % 4) % 4; x++) {
                fputc(0, fd);
            }
        }

        return true;
    }

    return false;
}