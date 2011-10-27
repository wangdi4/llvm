/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  ImagesALU.h

\*****************************************************************************/

#ifndef _IMAGESALU_H_
#define _IMAGESALU_H_

#include "dxfloat.h"
#include "FloatOperations.h"
#include "Conformance/reference_math.h"
#include "Conformance/test_common/errorHelpers.h"

namespace Conformance
{

    typedef struct
    {
        size_t width;
        size_t height;
        size_t depth;
        size_t rowPitch;
        size_t slicePitch;
        cl_image_format *format;
    } image_descriptor;

    // Definition for our own sampler type, to mirror the cl_sampler internals
    typedef struct {
        cl_addressing_mode addressing_mode;
        cl_filter_mode     filter_mode;
        bool               normalized_coords;
    } image_sampler_data;

    typedef struct
    {
        float p[4];
    }FloatPixel;

    extern int32_t has_alpha(cl_image_format *format);
    extern size_t get_pixel_size( cl_image_format *format );

    // get number of channels
    size_t get_format_channel_count( const cl_image_format *format );

    template <class T> void read_image_pixel( void *imageData, image_descriptor *imageInfo, 
        int x, int y, int z, T *outData )
    {
        if( x < 0 || x >= (int)imageInfo->width || y < 0 || y >= (int)imageInfo->height || ( imageInfo->depth != 0 && ( z < 0 || z >= (int)imageInfo->depth ) ) )
        {
            // Border color
            outData[ 0 ] = outData[ 1 ] = outData[ 2 ] = outData[ 3 ] = 0;
            if (!has_alpha(imageInfo->format))
                outData[3] = 1;
            return;
        }

        cl_image_format *format = imageInfo->format;

        unsigned int i;
        T tempData[ 4 ];

        // Advance to the right spot
        char *ptr = (char *)imageData;
        size_t pixelSize = get_pixel_size( format );

        ptr += z * imageInfo->slicePitch + y * imageInfo->rowPitch + x * pixelSize;

        // OpenCL only supports reading floats from certain formats
        switch( format->image_channel_data_type )
        {
        case CL_SNORM_INT8:
            {
                cl_char *dPtr = (cl_char *)ptr;
                for( i = 0; i < get_format_channel_count( format ); i++ )
                    tempData[ i ] = (T)dPtr[ i ];
                break;			
            }

        case CL_UNORM_INT8:
            {
                cl_uchar *dPtr = (cl_uchar *)ptr;
                for( i = 0; i < get_format_channel_count( format ); i++ )
                    tempData[ i ] = (T)dPtr[ i ];
                break;			
            }

        case CL_SIGNED_INT8:
            {
                cl_char *dPtr = (cl_char *)ptr;
                for( i = 0; i < get_format_channel_count( format ); i++ )
                    tempData[ i ] = (T)dPtr[ i ];
                break;			
            }

        case CL_UNSIGNED_INT8:
            {
                cl_uchar *dPtr = (cl_uchar*)ptr;
                for( i = 0; i < get_format_channel_count( format ); i++ )
                    tempData[ i ] = (T)dPtr[ i ];
                break;			
            }

        case CL_SNORM_INT16:
            {
                cl_short *dPtr = (cl_short *)ptr;
                for( i = 0; i < get_format_channel_count( format ); i++ )
                    tempData[ i ] = (T)dPtr[ i ];
                break;			
            }

        case CL_UNORM_INT16:
            {
                cl_ushort *dPtr = (cl_ushort *)ptr;
                for( i = 0; i < get_format_channel_count( format ); i++ )
                    tempData[ i ] = (T)dPtr[ i ];
                break;			
            }

        case CL_SIGNED_INT16:
            {
                cl_short *dPtr = (cl_short *)ptr;
                for( i = 0; i < get_format_channel_count( format ); i++ )
                    tempData[ i ] = (T)dPtr[ i ];
                break;			
            }

        case CL_UNSIGNED_INT16:
            {
                cl_ushort *dPtr = (cl_ushort *)ptr;
                for( i = 0; i < get_format_channel_count( format ); i++ )
                    tempData[ i ] = (T)dPtr[ i ];
                break;			
            }

        case CL_HALF_FLOAT:
            // AAK!
            ::log_error( "AAK!\n" );
            break;

        case CL_SIGNED_INT32:
            {
                cl_int *dPtr = (cl_int *)ptr;
                for( i = 0; i < get_format_channel_count( format ); i++ )
                    tempData[ i ] = (T)dPtr[ i ];
                break;			
            }

        case CL_UNSIGNED_INT32:
            {
                cl_uint *dPtr = (cl_uint *)ptr;
                for( i = 0; i < get_format_channel_count( format ); i++ )
                    tempData[ i ] = (T)dPtr[ i ];
                break;			
            }

        case CL_UNORM_SHORT_565:
            {
                cl_ushort *dPtr = (cl_ushort*)ptr;
                tempData[ 0 ] = (T)( dPtr[ 0 ] >> 11 );
                tempData[ 1 ] = (T)( ( dPtr[ 0 ] >> 5 ) & 63 );
                tempData[ 2 ] = (T)( dPtr[ 0 ] & 31 );
                break;			
            }

#ifdef OBSOLETE_FORMAT
        case CL_UNORM_SHORT_565_REV:
            {
                unsigned short *dPtr = (unsigned short *)ptr;
                tempData[ 2 ] = (T)( dPtr[ 0 ] >> 11 );
                tempData[ 1 ] = (T)( ( dPtr[ 0 ] >> 5 ) & 63 );
                tempData[ 0 ] = (T)( dPtr[ 0 ] & 31 );
                break;			
            }

        case CL_UNORM_SHORT_555_REV:
            {
                unsigned short *dPtr = (unsigned short *)ptr;
                tempData[ 2 ] = (T)( ( dPtr[ 0 ] >> 10 ) & 31 );
                tempData[ 1 ] = (T)( ( dPtr[ 0 ] >> 5 ) & 31 );
                tempData[ 0 ] = (T)( dPtr[ 0 ] & 31 );
                break;			
            }

        case CL_UNORM_INT_8888:
            {
                unsigned int *dPtr = (unsigned int *)ptr;
                tempData[ 3 ] = (T)( dPtr[ 0 ] >> 24 );
                tempData[ 2 ] = (T)( ( dPtr[ 0 ] >> 16 ) & 0xff );
                tempData[ 1 ] = (T)( ( dPtr[ 0 ] >> 8 ) & 0xff );
                tempData[ 0 ] = (T)( dPtr[ 0 ] & 0xff );
                break;			
            }
        case CL_UNORM_INT_8888_REV:
            {
                unsigned int *dPtr = (unsigned int *)ptr;
                tempData[ 0 ] = (T)( dPtr[ 0 ] >> 24 );
                tempData[ 1 ] = (T)( ( dPtr[ 0 ] >> 16 ) & 0xff );
                tempData[ 2 ] = (T)( ( dPtr[ 0 ] >> 8 ) & 0xff );
                tempData[ 3 ] = (T)( dPtr[ 0 ] & 0xff );
                break;			
            }

        case CL_UNORM_INT_101010_REV:
            {
                unsigned int *dPtr = (unsigned int *)ptr;
                tempData[ 2 ] = (T)( ( dPtr[ 0 ] >> 20 ) & 0x3ff );
                tempData[ 1 ] = (T)( ( dPtr[ 0 ] >> 10 ) & 0x3ff );
                tempData[ 0 ] = (T)( dPtr[ 0 ] & 0x3ff );
                break;			
            }
#endif			
        case CL_UNORM_SHORT_555:
            {
                cl_ushort *dPtr = (cl_ushort *)ptr;
                tempData[ 0 ] = (T)( ( dPtr[ 0 ] >> 10 ) & 31 );
                tempData[ 1 ] = (T)( ( dPtr[ 0 ] >> 5 ) & 31 );
                tempData[ 2 ] = (T)( dPtr[ 0 ] & 31 );
                break;			
            }

        case CL_UNORM_INT_101010:
            {
                cl_uint *dPtr = (cl_uint *)ptr;
                tempData[ 0 ] = (T)( ( dPtr[ 0 ] >> 20 ) & 0x3ff );
                tempData[ 1 ] = (T)( ( dPtr[ 0 ] >> 10 ) & 0x3ff );
                tempData[ 2 ] = (T)( dPtr[ 0 ] & 0x3ff );
                break;			
            }

        case CL_FLOAT:
            {
                cl_float *dPtr = (cl_float *)ptr;
                for( i = 0; i < get_format_channel_count( format ); i++ )
                    tempData[ i ] = (T)dPtr[ i ];
                break;			
            }
        }


        outData[ 0 ] = outData[ 1 ] = outData[ 2 ] = 0;
        outData[ 3 ] = 1;

        if( format->image_channel_order == CL_A )
        {
            outData[ 3 ] = tempData[ 0 ];
        }
        else if( format->image_channel_order == CL_R   )
        {
            outData[ 0 ] = tempData[ 0 ];
        }
        else if( format->image_channel_order == CL_Rx   )
        {
            outData[ 0 ] = tempData[ 0 ];
        }
        else if( format->image_channel_order == CL_RA )
        {
            outData[ 0 ] = tempData[ 0 ];
            outData[ 3 ] = tempData[ 1 ];
        }
        else if( format->image_channel_order == CL_RG  )
        {
            outData[ 0 ] = tempData[ 0 ];
            outData[ 1 ] = tempData[ 1 ];
        }
        else if( format->image_channel_order == CL_RGx  )
        {
            outData[ 0 ] = tempData[ 0 ];
            outData[ 1 ] = tempData[ 1 ];
        }
        else if( format->image_channel_order == CL_RGB  )
        {
            outData[ 0 ] = tempData[ 0 ];
            outData[ 1 ] = tempData[ 1 ];
            outData[ 2 ] = tempData[ 2 ];
        }
        else if( format->image_channel_order == CL_RGBx  )
        {
            outData[ 0 ] = tempData[ 0 ];
            outData[ 1 ] = tempData[ 1 ];
            outData[ 2 ] = tempData[ 2 ];
        }
        else if( format->image_channel_order == CL_RGBA )
        {
            outData[ 0 ] = tempData[ 0 ];
            outData[ 1 ] = tempData[ 1 ];
            outData[ 2 ] = tempData[ 2 ];
            outData[ 3 ] = tempData[ 3 ];
        }
        else if( format->image_channel_order == CL_ARGB )
        {
            outData[ 0 ] = tempData[ 1 ];
            outData[ 1 ] = tempData[ 2 ];
            outData[ 2 ] = tempData[ 3 ];
            outData[ 3 ] = tempData[ 0 ];
        }
        else if( format->image_channel_order == CL_BGRA )
        {
            outData[ 0 ] = tempData[ 2 ];
            outData[ 1 ] = tempData[ 1 ];
            outData[ 2 ] = tempData[ 0 ];
            outData[ 3 ] = tempData[ 3 ];
        }
        else if( format->image_channel_order == CL_INTENSITY )
        {
            outData[ 1 ] = tempData[ 0 ];
            outData[ 2 ] = tempData[ 0 ];
            outData[ 3 ] = tempData[ 0 ];
        }
        else if( format->image_channel_order == CL_LUMINANCE )
        {
            outData[ 1 ] = tempData[ 0 ];
            outData[ 2 ] = tempData[ 0 ];
        }
#ifdef CL_1RGB_APPLE
        else if( format->image_channel_order == CL_1RGB_APPLE )
        {
            outData[ 0 ] = tempData[ 1 ];
            outData[ 1 ] = tempData[ 2 ];
            outData[ 2 ] = tempData[ 3 ];
            outData[ 3 ] = 0xff;
        }
#endif
#ifdef CL_BGR1_APPLE
        else if( format->image_channel_order == CL_BGR1_APPLE )
        {
            outData[ 0 ] = tempData[ 2 ];
            outData[ 1 ] = tempData[ 1 ];
            outData[ 2 ] = tempData[ 0 ];
            outData[ 3 ] = 0xff;
        }
#endif
        else
        {
            ::log_error("Invalid format:");
        }
    }

    // Stupid template rules
    bool get_integer_coords_offset( float x, float y, float z, float xAddressOffset, float yAddressOffset, float zAddressOffset, 
        size_t width, size_t height, size_t depth, image_sampler_data *imageSampler, int &outX, int &outY, int &outZ );

    template <class T> void sample_image_pixel_offset( void *imageData, image_descriptor *imageInfo, 
        float x, float y, float z, float xAddressOffset, float yAddressOffset, float zAddressOffset,
        image_sampler_data *imageSampler, T *outData )
    {
        int iX, iY, iZ;
        get_integer_coords_offset( x, y, z, xAddressOffset, yAddressOffset, zAddressOffset, imageInfo->width, imageInfo->height, imageInfo->depth, imageSampler, iX, iY, iZ );
        read_image_pixel<T>( imageData, imageInfo, iX, iY, iZ, outData );
    }


    template <class T> void sample_image_pixel( void *imageData, image_descriptor *imageInfo, 
        float x, float y, float z, image_sampler_data *imageSampler, T *outData )
    {
        return sample_image_pixel_offset<T>(imageData, imageInfo, x, y, z, 0.0f, 0.0f, 0.0f, imageSampler, outData);
    }

    template <> void sample_image_pixel<float>( void *imageData, image_descriptor *imageInfo, 
        float x, float y, float z, image_sampler_data *imageSampler, float *outData );

    FloatPixel sample_image_pixel_float( void *imageData, image_descriptor *imageInfo,
        float x, float y, float z, 
        image_sampler_data *imageSampler, 
        float *outData, int verbose, int *containsDenorms );

    void write_image_pixel_float( void *imageData, image_descriptor *imageInfo, 
        const int x, const int y, float* inData );
    void write_image_pixel_int( void *imageData, image_descriptor *imageInfo, 
        const int x, const int y, int* inData );
    void write_image_pixel_uint( void *imageData, image_descriptor *imageInfo, 
        const int x, const int y, unsigned int* inData );

    // get maximum relative error for pixel
    float get_max_relative_error( cl_image_format *format, image_sampler_data *sampler, int is3D, int isLinearFilter );
    // get maximum absolute error for pixel
    float get_max_absolute_error( cl_image_format *format, image_sampler_data *sampler);
}

#endif
