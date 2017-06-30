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

File Name:  ImagesALU.cpp

\*****************************************************************************/
#include <math.h>
#include <limits.h>
#include "ImagesALU.h"
#include "Conformance/test_common/compat.h"
#include "Conformance/test_common/errorHelpers.h"

using namespace Validation;

namespace Conformance
{

    // fwd declaration
    static void read_image_pixel_float( void *imageData, image_descriptor *imageInfo, 
        int x, int y, int z, float *outData );


    // Define the addressing functions
    typedef int (*AddressFn)( int value, size_t maxValue );

    int			NoAddressFn( int value, size_t maxValue )				{ return value; }
    int			RepeatAddressFn( int value, size_t maxValue )			
    {   
        if( value < 0 ) 
            value += (int)maxValue; 
        else if( value >= (int)maxValue ) 
            value -= (int)maxValue; 
        return value; 
    }
    int			MirroredRepeatAddressFn( int value, size_t maxValue )			
    {   
        if( value < 0 ) 
            value  = 0; 
        else if( (size_t) value >= maxValue ) 
            value = (int) (maxValue - 1); 
        return value; 
    }
    int ClampAddressFn( int value, size_t maxValue )			{ return ( value < -1 ) ? -1 : ( ( value > (cl_long) maxValue ) ? (int)maxValue : value ); }
    int ClampToEdgeNearestFn( int value, size_t maxValue )	{ return ( value < 0 ) ? 0 : ( ( (size_t)value > maxValue - 1 ) ? (int)maxValue - 1 : value ); }
    AddressFn ClampToEdgeLinearFn = ClampToEdgeNearestFn;

    // Note: normalized coords get repeated in normalized space, not unnormalized space! hence the special case here
    float RepeatNormalizedAddressFn( float fValue, size_t maxValue ) 
    {
        // General computation for repeat
        return (fValue - floorf( fValue )) * (float) maxValue; // Reduce to [0, 1.f]
    }

    float MirroredRepeatNormalizedAddressFn( float fValue, size_t maxValue ) 
    {
        // Round to nearest multiple of two
        float s_prime = 2.0f * rintf( fValue * 0.5f );        // Note halfway values flip flop here due to rte, but they both end up pointing the same place at the end of the day

        // Reduce to [-1, 1], Apply mirroring -> [0, 1]
        s_prime = fabsf( fValue - s_prime ); 

        // un-normalize
        return s_prime * (float) maxValue; 
    }

    struct AddressingTable
    {
        AddressingTable()
        {
            mTable[ CL_ADDRESS_NONE - CL_ADDRESS_NONE ][ CL_FILTER_NEAREST - CL_FILTER_NEAREST ]			= NoAddressFn;
            mTable[ CL_ADDRESS_NONE - CL_ADDRESS_NONE ][ CL_FILTER_LINEAR - CL_FILTER_NEAREST ]				= NoAddressFn;
            mTable[ CL_ADDRESS_REPEAT - CL_ADDRESS_NONE ][ CL_FILTER_NEAREST - CL_FILTER_NEAREST ]			= RepeatAddressFn;
            mTable[ CL_ADDRESS_REPEAT - CL_ADDRESS_NONE ][ CL_FILTER_LINEAR - CL_FILTER_NEAREST ]			= RepeatAddressFn;
            mTable[ CL_ADDRESS_CLAMP_TO_EDGE - CL_ADDRESS_NONE ][ CL_FILTER_NEAREST - CL_FILTER_NEAREST ]	= ClampToEdgeNearestFn;
            mTable[ CL_ADDRESS_CLAMP_TO_EDGE - CL_ADDRESS_NONE ][ CL_FILTER_LINEAR - CL_FILTER_NEAREST ]	= ClampToEdgeLinearFn;
            mTable[ CL_ADDRESS_CLAMP - CL_ADDRESS_NONE ][ CL_FILTER_NEAREST - CL_FILTER_NEAREST ]			= ClampAddressFn;
            mTable[ CL_ADDRESS_CLAMP - CL_ADDRESS_NONE ][ CL_FILTER_LINEAR - CL_FILTER_NEAREST ]			= ClampAddressFn;
            mTable[ CL_ADDRESS_MIRRORED_REPEAT - CL_ADDRESS_NONE ][ CL_FILTER_NEAREST - CL_FILTER_NEAREST ] = MirroredRepeatAddressFn;
            mTable[ CL_ADDRESS_MIRRORED_REPEAT - CL_ADDRESS_NONE ][ CL_FILTER_LINEAR - CL_FILTER_NEAREST ]  = MirroredRepeatAddressFn;
        }

        AddressFn operator[]( image_sampler_data *sampler )
        {
            return mTable[ (int)sampler->addressing_mode - CL_ADDRESS_NONE ][ (int)sampler->filter_mode - CL_FILTER_NEAREST ];
        }

        AddressFn mTable[ 6 ][ 2 ];
    };

    static AddressingTable	sAddressingTable;

    int32_t has_alpha(cl_image_format *format)
    {
        switch (format->image_channel_order) 
        {
        case CLK_R:
            return 0;
        case CLK_A:
            return 1;
        case CLK_RG:
            return 0;
        case CLK_RA:
            return 1;
        case CLK_RGB:
            return 0;
        case CLK_RGBA:
        case CLK_sRGBA:
        case CLK_sBGRA:
            return 1;
        case CLK_BGRA:
            return 1;
        case CLK_ARGB:
            return 1;
        case CLK_INTENSITY:
            return 1;
        case CLK_LUMINANCE:
        case CLK_DEPTH:
            return 0;
#ifdef CL_BGR1_APPLE
        case CLK_BGR1_APPLE: return 1;
#endif
#ifdef CL_1RGB_APPLE
        case CLK_1RGB_APPLE: return 1;
#endif
        default:
            ::log_error("Invalid image channel order: %d\n", format->image_channel_order);
            return 0;
        } 

    }
    
    // fwd declarations
    size_t get_format_type_size( const cl_image_format *format );
    size_t get_channel_data_type_size( cl_channel_type channelType );
    size_t get_format_channel_count( const cl_image_format *format );
    size_t get_channel_order_channel_count( cl_channel_order order );
    
    inline size_t get_format_type_size( const cl_image_format *format )
    {
        return get_channel_data_type_size( format->image_channel_data_type );
    }

    inline size_t get_channel_data_type_size( cl_channel_type channelType )
    {
        switch( channelType )
        {
        case CLK_SNORM_INT8:
        case CLK_UNORM_INT8:
        case CLK_SIGNED_INT8:
        case CLK_UNSIGNED_INT8:
            return 1;

        case CLK_SNORM_INT16:
        case CLK_UNORM_INT16:
        case CLK_SIGNED_INT16:
        case CLK_UNSIGNED_INT16:
        case CLK_HALF_FLOAT:
            return sizeof( cl_short );

        case CLK_SIGNED_INT32:
        case CLK_UNSIGNED_INT32:
            return sizeof( cl_int );

        case CLK_UNORM_SHORT_565:
        case CLK_UNORM_SHORT_555:
#ifdef OBSOLETE_FORAMT
        case CLK_UNORM_SHORT_565_REV:
        case CLK_UNORM_SHORT_555_REV:
#endif
            return 2;

#ifdef OBSOLETE_FORAMT
        case CLK_UNORM_INT_8888:
        case CLK_UNORM_INT_8888_REV:
            return 4;
#endif

        case CLK_UNORM_INT_101010:
#ifdef OBSOLETE_FORAMT
        case CLK_UNORM_INT_101010_REV:
#endif
            return 4;

        case CLK_FLOAT:
            return sizeof( cl_float );

        default:
            return 0;
        }
    }

    size_t get_format_channel_count( const cl_image_format *format )
    {
        return get_channel_order_channel_count( format->image_channel_order );
    }

    inline size_t get_channel_order_channel_count( cl_channel_order order )
    {
        switch( order )
        {
        case CLK_R:
        case CLK_A:
        case CLK_INTENSITY:
        case CLK_LUMINANCE:
        case CLK_DEPTH:
            return 1;

        case CLK_RG:
        case CLK_RA:
            return 2;

        case CLK_RGB:
            return 3;

        case CLK_RGBA:
        case CLK_sRGBA:
        case CLK_sBGRA:
        case CLK_ARGB:
        case CLK_BGRA:
#ifdef CL_1RGB_APPLE
        case CLK_1RGB_APPLE:
#endif
#ifdef CL_BGR1_APPLE
        case CLK_BGR1_APPLE:
#endif
            return 4;

        default:
            return 0;
        }
    }

    // Format helpers
    size_t get_pixel_size( cl_image_format *format )
    {
        switch( format->image_channel_data_type )
        {
        case CLK_SNORM_INT8:
        case CLK_UNORM_INT8:
        case CLK_SIGNED_INT8:
        case CLK_UNSIGNED_INT8:
            return get_format_channel_count( format );

        case CLK_SNORM_INT16:
        case CLK_UNORM_INT16:
        case CLK_SIGNED_INT16:
        case CLK_UNSIGNED_INT16:
        case CLK_HALF_FLOAT:
            return get_format_channel_count( format ) * sizeof( cl_short );

        case CLK_SIGNED_INT32:
        case CLK_UNSIGNED_INT32:
            return get_format_channel_count( format ) * sizeof( cl_int );

        case CLK_UNORM_SHORT_565:
        case CLK_UNORM_SHORT_555:
#ifdef OBSOLETE_FORAMT
        case CLK_UNORM_SHORT_565_REV:
        case CLK_UNORM_SHORT_555_REV:
#endif
            return 2;

#ifdef OBSOLETE_FORAMT
        case CLK_UNORM_INT_8888:
        case CLK_UNORM_INT_8888_REV:
            return 4;
#endif

        case CLK_UNORM_INT_101010:
#ifdef OBSOLETE_FORAMT
        case CLK_UNORM_INT_101010_REV:
#endif
            return 4;

        case CLK_FLOAT:
            return get_format_channel_count( format ) * sizeof( cl_float );

        default:
            return 0;
        }
    }

    static float convert_half_to_float( unsigned short halfValue )
    {
        // We have to take care of a few special cases, but in general, we just extract
        // the same components from the half that exist in the float and re-stuff them
        // For a description of the actual half format, see http://en.wikipedia.org/wiki/Half_precision
        // Note: we store these in 32-bit ints to make the bit manipulations easier later
        int sign =     ( halfValue >> 15 ) & 0x0001;
        int exponent = ( halfValue >> 10 ) & 0x001f;
        int mantissa = ( halfValue )       & 0x03ff;

        // Note: we use a union here to be able to access the bits of a float directly
        union 
        {
            unsigned int bits;
            float floatValue;
        } outFloat;

        // Special cases first
        if( exponent == 0 )
        {
            if( mantissa == 0 )
            {
                // If both exponent and mantissa are 0, the number is +/- 0
                outFloat.bits  = sign << 31;
                return outFloat.floatValue; // Already done!
            }

            // If exponent is 0, it's a denormalized number, so we renormalize it
            // Note: this is not terribly efficient, but oh well
            while( ( mantissa & 0x00000400 ) == 0 )
            {
                mantissa <<= 1;
                exponent--;
            }

            // The first bit is implicit, so we take it off and inc the exponent accordingly
            exponent++;
            mantissa &= ~(0x00000400);
        }
        else if( exponent == 31 ) // Special-case "numbers"
        {
            // If the exponent is 31, it's a special case number (+/- infinity or NAN).
            // If the mantissa is 0, it's infinity, else it's NAN, but in either case, the packing 
            // method is the same
            outFloat.bits = ( sign << 31 ) | 0x7f800000 | ( mantissa << 13 );
            return outFloat.floatValue;
        }

        // Plain ol' normalized number, so adjust to the ranges a 32-bit float expects and repack
        exponent += ( 127 - 15 );
        mantissa <<= 13;

        outFloat.bits = ( sign << 31 ) | ( exponent << 23 ) | mantissa;
        return outFloat.floatValue;
    }


#define CLAMP_FLOAT( v ) (fmaxf(fminf( v, 1.f ), -1.f ) )

    void read_image_pixel_float( void *imageData, image_descriptor *imageInfo, 
        int x, int y, int z, float *outData )
    {
            if ( x < 0 || y < 0 || z < 0 || x >= (int)imageInfo->width 
               || ( imageInfo->height != 0 && y >= (int)imageInfo->height )
               || ( imageInfo->depth != 0 && z >= (int)imageInfo->depth )
               || ( imageInfo->arraySize != 0 && z >= (int)imageInfo->arraySize ) )
        {
            // Border color
            outData[ 0 ] = outData[ 1 ] = outData[ 2 ] = outData[ 3 ] = 0;
            if (!has_alpha(imageInfo->format))
                outData[3] = 1;
            return;
        }

        cl_image_format *format = imageInfo->format;

        unsigned int i;
        float tempData[ 4 ];

        // Advance to the right spot
        char *ptr = (char *)imageData;
        size_t pixelSize = get_pixel_size( format );

        ptr += z * imageInfo->slicePitch + y * imageInfo->rowPitch + x * pixelSize;

        // OpenCL only supports reading floats from certain formats
        size_t channelCount = get_format_channel_count( format );
        switch( format->image_channel_data_type )
        {
        case CLK_SNORM_INT8:
            {
                char *dPtr = (char *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] = CLAMP_FLOAT( (float)dPtr[ i ] / 127.0f );
                break;			
            }

        case CLK_UNORM_INT8:
            {
                unsigned char *dPtr = (unsigned char *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] = (float)dPtr[ i ] / 255.0f;
                break;			
            }

        case CLK_SIGNED_INT8:
            {
                cl_char *dPtr = (cl_char *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] =  (float)dPtr[ i ];
                break;			
            }

        case CLK_UNSIGNED_INT8:
            {
                cl_uchar *dPtr = (cl_uchar *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] = (float) dPtr[ i ];
                break;			
            }

        case CLK_SNORM_INT16:
            {
                cl_short *dPtr = (cl_short *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] = CLAMP_FLOAT( (float)dPtr[ i ] / 32767.0f );
                break;			
            }

        case CLK_UNORM_INT16:
            {
                cl_ushort *dPtr = (cl_ushort *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] = (float)dPtr[ i ] / 65535.0f;
                break;			
            }

        case CLK_SIGNED_INT16:
            {
                cl_short *dPtr = (cl_short *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] = (float)dPtr[ i ];
                break;			
            }

        case CLK_UNSIGNED_INT16:
            {
                cl_ushort *dPtr = (cl_ushort *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] = (float) dPtr[ i ];
                break;			
            }

        case CLK_HALF_FLOAT:
            {
                cl_ushort *dPtr = (cl_ushort *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] = convert_half_to_float( dPtr[ i ] );
                break;
            }

        case CLK_SIGNED_INT32:
            {
                cl_int *dPtr = (cl_int *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] = (float)dPtr[ i ];        
                break;			
            }

        case CLK_UNSIGNED_INT32:
            {
                cl_uint *dPtr = (cl_uint *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] = (float)dPtr[ i ];                     
                break;			
            }

        case CLK_UNORM_SHORT_565:
            {
                cl_ushort *dPtr = (cl_ushort *)ptr;
                tempData[ 0 ] = (float)( dPtr[ 0 ] >> 11 ) / (float)31;
                tempData[ 1 ] = (float)( ( dPtr[ 0 ] >> 5 ) & 63 ) / (float)63;
                tempData[ 2 ] = (float)( dPtr[ 0 ] & 31 ) / (float)31;
                break;			
            }

        case CLK_UNORM_SHORT_555:
            {
                cl_ushort *dPtr = (cl_ushort *)ptr;
                tempData[ 0 ] = (float)( ( dPtr[ 0 ] >> 10 ) & 31 ) / (float)31;
                tempData[ 1 ] = (float)( ( dPtr[ 0 ] >> 5 ) & 31 ) / (float)31;
                tempData[ 2 ] = (float)( dPtr[ 0 ] & 31 ) / (float)31;
                break;			
            }

        case CLK_UNORM_INT_101010:
            {
                cl_uint *dPtr = (cl_uint *)ptr;
                tempData[ 0 ] = (float)( ( dPtr[ 0 ] >> 20 ) & 0x3ff ) / (float)1023;
                tempData[ 1 ] = (float)( ( dPtr[ 0 ] >> 10 ) & 0x3ff ) / (float)1023;
                tempData[ 2 ] = (float)( dPtr[ 0 ] & 0x3ff ) / (float)1023;
                break;			
            }

        case CLK_FLOAT:
            {
                float *dPtr = (float *)ptr;
                for( i = 0; i < channelCount; i++ )
                    tempData[ i ] = (float)dPtr[ i ];
                break;			
            }
        }


        outData[ 0 ] = outData[ 1 ] = outData[ 2 ] = 0;
        outData[ 3 ] = 1;

        switch( format->image_channel_order )
        {
        case CLK_A:
            outData[ 3 ] = tempData[ 0 ];
            break;
        case CLK_R:
            outData[ 0 ] = tempData[ 0 ];
            break;
        case CLK_RA:
            outData[ 0 ] = tempData[ 0 ];
            outData[ 3 ] = tempData[ 1 ];
            break;
        case CLK_RG:
            outData[ 0 ] = tempData[ 0 ];
            outData[ 1 ] = tempData[ 1 ];
            break;
        case CLK_RGB:
            outData[ 0 ] = tempData[ 0 ];
            outData[ 1 ] = tempData[ 1 ];
            outData[ 2 ] = tempData[ 2 ];
            break;
        case CLK_RGBA:
            outData[ 0 ] = tempData[ 0 ];
            outData[ 1 ] = tempData[ 1 ];
            outData[ 2 ] = tempData[ 2 ];
            outData[ 3 ] = tempData[ 3 ];
            break;
        case CLK_ARGB:
            outData[ 0 ] = tempData[ 1 ];
            outData[ 1 ] = tempData[ 2 ];
            outData[ 2 ] = tempData[ 3 ];
            outData[ 3 ] = tempData[ 0 ];
            break;
        case CLK_BGRA:
            outData[ 0 ] = tempData[ 2 ];
            outData[ 1 ] = tempData[ 1 ];
            outData[ 2 ] = tempData[ 0 ];
            outData[ 3 ] = tempData[ 3 ];
            break;
        case CLK_INTENSITY:
            outData[ 0 ] = tempData[ 0 ];
            outData[ 1 ] = tempData[ 0 ];
            outData[ 2 ] = tempData[ 0 ];
            outData[ 3 ] = tempData[ 0 ];
            break;
        case CLK_LUMINANCE:
            outData[ 0 ] = tempData[ 0 ];
            outData[ 1 ] = tempData[ 0 ];
            outData[ 2 ] = tempData[ 0 ];
            break;
        case CLK_DEPTH:
            outData[ 0 ] = tempData[ 0 ];
            break;
        case CLK_sRGBA:
            outData[ 0 ] = (tempData[ 0 ] <= 0.04045 ) ? tempData[ 0 ]/12.92 :
                            pow((tempData[ 0 ]+0.055)/1.055,2.4);
            outData[ 1 ] = (tempData[ 1 ] <= 0.04045 ) ? tempData[ 1 ]/12.92 :
                            pow((tempData[ 1 ]+0.055)/1.055,2.4);
            outData[ 2 ] = (tempData[ 2 ] <= 0.04045 ) ? tempData[ 2 ]/12.92 :
                            pow((tempData[ 2 ]+0.055)/1.055,2.4);
            outData[ 3 ] = tempData[ 3 ];
            break;
            
        case CLK_sBGRA:
            outData[ 0 ] = (tempData[ 2 ] <= 0.04045 ) ? tempData[ 2 ]/12.92 :
                            pow((tempData[ 2 ]+0.055)/1.055,2.4);
            outData[ 1 ] = (tempData[ 1 ] <= 0.04045 ) ? tempData[ 1 ]/12.92 :
                            pow((tempData[ 1 ]+0.055)/1.055,2.4);
            outData[ 2 ] = (tempData[ 0 ] <= 0.04045 ) ? tempData[ 0 ]/12.92 :
                            pow((tempData[ 0 ]+0.055)/1.055,2.4);
            outData[ 3 ] = tempData[ 3 ];
            break;
#ifdef CL_1RGB_APPLE
        case CLK_1RGB_APPLE:
            outData[ 0 ] = tempData[ 1 ];
            outData[ 1 ] = tempData[ 2 ];
            outData[ 2 ] = tempData[ 3 ];
            outData[ 3 ] = 1.0f; 
            break;
#endif
#ifdef CL_BGR1_APPLE
        case CLK_BGR1_APPLE:
            outData[ 0 ] = tempData[ 2 ];
            outData[ 1 ] = tempData[ 1 ];
            outData[ 2 ] = tempData[ 0 ];
            outData[ 3 ] = 1.0f;
            break;
#endif
        default:
            throw Exception::InvalidArgument("ImagesALU::read_image_pixel_float Invalid format");
            break;
        }
    }

    // If containsDenorms is NULL, flush denorms to zero
    // if containsDenorms is not NULL, record whether there are any denorms
    static inline void  check_for_denorms(float a[4], int *containsDenorms );
    static inline void  check_for_denorms(float a[4], int *containsDenorms )
    {
        if( NULL == containsDenorms )
        {
            for( int i = 0; i < 4; i++ )
            {
                if( fabsf(a[i]) < FLT_MIN )
                    a[i] = Conformance::copysignf( 0.0f, a[i] );
            }
        }
        else
        {
            for( int i = 0; i < 4; i++ )
            {
                if( fabs(a[i]) < FLT_MIN )
                {
                    *containsDenorms = 1;
                    break;
                }
            }
        }
    }
    static float frac(float a) {
        return a - floorf(a); 
    }

    #define errMax( _x , _y )       ( (_x) != (_x) ? (_x) : (_x) > (_y) ? (_x) : (_y) )

    static inline void pixelMax( const float a[4], const float b[4], float *results );
    static inline void pixelMax( const float a[4], const float b[4], float *results )
    {
        for( int i = 0; i < 4; i++ )
            results[i] = errMax( fabsf(a[i]), fabsf(b[i]) );
    }



    inline float calculate_array_index( float coord, float extent ) {
        // from Section 8.4 of the 1.2 Spec 'Selecting an Image from an Image Array'
        //
        // given coordinate 'w' that represents an index:
        // layer_index = clamp( floor(w + 0.5f), 0.0f, max_value_for_w )
        
        float ret = floorf( coord + 0.5f );
        ret = ret > extent ? extent : ret;
        ret = ret < 0.0f ? 0.0f : ret;
        
        return ret;
    }

/*
 * Utility function to unnormalized a coordinate given a particular sampler.
 *
 * name     - the name of the coordinate, used for verbose debugging only
 * coord    - the coordinate requiring unnormalization
 * offset   - an addressing offset to be added to the coordinate
 * extent   - the max value for this coordinate (e.g. width for x)
 */
    static float unnormalize_coordinate( const char* name, float coord, 
       float offset, float extent, cl_addressing_mode addressing_mode, int verbose ) 
    {
        float ret = 0.0f;
        
        switch (addressing_mode) {
            case CL_ADDRESS_REPEAT:
                ret = RepeatNormalizedAddressFn( coord, extent );
                
                if ( verbose ) {
                    log_info( "\tRepeat filter denormalizes %s (%f) to %f\n", 
                        name, coord, ret );
                }
                
                if (offset != 0.0) {
                    // Add in the offset, and handle wrapping.
                    ret += offset;
                    if (ret > extent) ret -= extent;
                    if (ret < 0.0) ret += extent;
                }
                   
                if (verbose && offset != 0.0f) {
                    log_info( "\tAddress offset of %f added to get %f\n", offset, ret );
                }        
                break;
                
            case CL_ADDRESS_MIRRORED_REPEAT:
                ret = MirroredRepeatNormalizedAddressFn( coord, extent );     
                
                if ( verbose ) {
                    log_info( "\tMirrored repeat filter denormalizes %s (%f) to %f\n", 
                        name, coord, ret );
                }
    
                if (offset != 0.0) {
                    float temp = ret + offset;
                    if( temp > extent )
                        temp = extent - (temp - extent );
                    ret = fabsf( temp );
                }
    
                if (verbose && offset != 0.0f) {
                    log_info( "\tAddress offset of %f added to get %f\n", offset, ret );
                }        
                break;
                
            default:
    
                ret = coord * extent;
    
                if ( verbose ) {
                    log_info( "\tFilter denormalizes %s (%f) to %f\n", 
                        name, coord, ret );
                }
                
                ret += offset;
                
                if (verbose && offset != 0.0f) {
                    log_info( "\tAddress offset of %f added to get %f\n", offset, ret );
                }
        }
        
        return ret;
    }

FloatPixel sample_image_pixel_float( void *imageData, image_descriptor *imageInfo, 
                                    float x, float y, float z,
                                    image_sampler_data *imageSampler, float *outData, int verbose, int *containsDenorms ) {
    return sample_image_pixel_float_offset(imageData, imageInfo, x, y, z, 0.0f, 0.0f, 0.0f, imageSampler, outData, verbose, containsDenorms);
}


FloatPixel sample_image_pixel_float_offset( void *imageData, image_descriptor *imageInfo, 
                                           float x, float y, float z, float xAddressOffset, float yAddressOffset, float zAddressOffset,
                                           image_sampler_data *imageSampler, float *outData, int verbose, int *containsDenorms )
{
    AddressFn adFn = sAddressingTable[ imageSampler ];
    FloatPixel returnVal;
    
    if( containsDenorms )
        *containsDenorms = 0;
    
    if( imageSampler->normalized_coords ) {
        
        // We need to unnormalize our coordinates differently depending on
        // the image type, but 'x' is always processed the same way.
        
        x = unnormalize_coordinate("x", x, xAddressOffset, (float)imageInfo->width, 
            imageSampler->addressing_mode, verbose);

        switch (imageInfo->type) {
        
            // The image array types require special care:
            
            case CL_MEM_OBJECT_IMAGE1D_ARRAY:                
                z = 0; // don't care -- unused for 1D arrays
                break;
                
            case CL_MEM_OBJECT_IMAGE2D_ARRAY:
                y = unnormalize_coordinate("y", y, yAddressOffset, (float)imageInfo->height, 
                    imageSampler->addressing_mode, verbose);
                break;
            
            // Everybody else:
            
            default: 
                y = unnormalize_coordinate("y", y, yAddressOffset, (float)imageInfo->height, 
                    imageSampler->addressing_mode, verbose);
                z = unnormalize_coordinate("z", z, zAddressOffset, (float)imageInfo->depth, 
                    imageSampler->addressing_mode, verbose);
        }
        
    } else if ( verbose ) {
        
        switch (imageInfo->type) {
            case CL_MEM_OBJECT_IMAGE1D_ARRAY:
                log_info("Starting coordinate: %f, array index %f\n", x, y);
                break;
            case CL_MEM_OBJECT_IMAGE2D_ARRAY:
                log_info("Starting coordinate: %f, %f, array index %f\n", x, y, z);
                break;
            case CL_MEM_OBJECT_IMAGE1D:
            case CL_MEM_OBJECT_IMAGE1D_BUFFER:
                log_info("Starting coordinate: %f\b", x);
                break;
            case CL_MEM_OBJECT_IMAGE2D:
                log_info("Starting coordinate: %f, %f\n", x, y);
                break;
            case CL_MEM_OBJECT_IMAGE3D:
            default:
                log_info("Starting coordinate: %f, %f, %f\n", x, y, z); 
        }
    }
    
    // At this point, we have unnormalized coordinates.
    
    if( imageSampler->filter_mode == CL_FILTER_NEAREST )
    {
        int ix, iy, iz;
        
        // We apply the addressing function to the now-unnormalized
        // coordinates.  Note that the array cases again require special
        // care, per section 8.4 in the OpenCL 1.2 Specification.
        
        ix = adFn( floorf( x ), imageInfo->width );
        
        switch (imageInfo->type) {
            case CL_MEM_OBJECT_IMAGE1D_ARRAY:
                iy = calculate_array_index( y, (float)(imageInfo->arraySize - 1) );
                iz = 0;
                break;
            case CL_MEM_OBJECT_IMAGE2D_ARRAY:
                iy = adFn( floorf( y ), imageInfo->height );
                iz = calculate_array_index( z, (float)(imageInfo->arraySize - 1) );
                break;
            default:
                if( imageInfo->height != 0 )
                    iy = adFn( floorf( y ), imageInfo->height );
                else
                    iy = 0;
                if( imageInfo->depth != 0 )
                    iz = adFn( floorf( z ), imageInfo->depth );
                else
                    iz = 0;
        }
        
        if( verbose ) {
            if( iz )
                log_info( "\tActual integer coords used (i = floor(x)): { %d, %d, %d }\n", ix, iy, iz );
            else
                log_info( "\tActual integer coords used (i = floor(x)): { %d, %d }\n", ix, iy );
        }
        
        read_image_pixel_float( imageData, imageInfo, ix, iy, iz, outData );
        check_for_denorms( outData, containsDenorms );
        for( int i = 0; i < 4; i++ )
            returnVal.p[i] = fabsf( outData[i] );
        return returnVal;
    }
    else
    {
        // Linear filtering cases.
    
        size_t width = imageInfo->width, height = imageInfo->height, depth = imageInfo->depth;
        
        // Image arrays can use 2D filtering, but require us to walk into the
        // image a certain number of slices before reading.
        
        if( depth == 0 || imageInfo->type == CL_MEM_OBJECT_IMAGE2D_ARRAY || 
                          imageInfo->type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
        {
            size_t layer_offset = 0;
            
            if (imageInfo->type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
                layer_offset = imageInfo->slicePitch * (size_t)calculate_array_index( 
                    z, (float)(imageInfo->arraySize - 1) 
                );
            }
            else if (imageInfo->type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
                layer_offset = imageInfo->slicePitch * (size_t)calculate_array_index( 
                    y, (float)(imageInfo->arraySize - 1) 
                );
                
                // Set up y and height so that the filtering below is correct
                // 1D filtering on a single slice.
                height = 1;
            }
            
            int x1 = adFn( floorf( x - 0.5f ), width );
            int y1 = 0;
            int x2 = adFn( floorf( x - 0.5f ) + 1, width );
            int y2 = 0;
            if ((imageInfo->type != CL_MEM_OBJECT_IMAGE1D) &&
                (imageInfo->type != CL_MEM_OBJECT_IMAGE1D_ARRAY) &&
                (imageInfo->type != CL_MEM_OBJECT_IMAGE1D_BUFFER)) {
                y1 = adFn( floorf( y - 0.5f ), height );
                y2 = adFn( floorf( y - 0.5f ) + 1, height );
            } else {
              y = 0.5f;
            }
          
            if( verbose )
                log_info( "\tActual integer coords used (i = floor(x-.5)): i0:{%d, %d } and i1:{%d, %d }\n", x1, y1, x2, y2 );
            
            // Walk to beginning of the 'correct' slice, if needed.
            char* imgPtr = ((char*)imageData) + layer_offset;
            
            float upLeft[ 4 ], upRight[ 4 ], lowLeft[ 4 ], lowRight[ 4 ];
            float maxUp[4], maxLow[4];
            read_image_pixel_float( imgPtr, imageInfo, x1, y1, 0, upLeft );
            read_image_pixel_float( imgPtr, imageInfo, x2, y1, 0, upRight );
            check_for_denorms( upLeft, containsDenorms );
            check_for_denorms( upRight, containsDenorms );
            pixelMax( upLeft, upRight, maxUp );
            read_image_pixel_float( imgPtr, imageInfo, x1, y2, 0, lowLeft );
            read_image_pixel_float( imgPtr, imageInfo, x2, y2, 0, lowRight );
            check_for_denorms( lowLeft, containsDenorms );
            check_for_denorms( lowRight, containsDenorms );
            pixelMax( lowLeft, lowRight, maxLow );
            pixelMax( maxUp, maxLow, returnVal.p );
            
            if( verbose )
            {
                if( NULL == containsDenorms )
                    log_info( "\tSampled pixels (rgba order, denorms flushed to zero):\n" );
                else
                    log_info( "\tSampled pixels (rgba order):\n" );
                log_info( "\t\tp00: %f, %f, %f, %f\n", upLeft[0], upLeft[1], upLeft[2], upLeft[3] ); 
                log_info( "\t\tp01: %f, %f, %f, %f\n", upRight[0], upRight[1], upRight[2], upRight[3] ); 
                log_info( "\t\tp10: %f, %f, %f, %f\n", lowLeft[0], lowLeft[1], lowLeft[2], lowLeft[3] ); 
                log_info( "\t\tp11: %f, %f, %f, %f\n", lowRight[0], lowRight[1], lowRight[2], lowRight[3] ); 
            }
            
            bool printMe = false;
            if( x1 <= 0 || x2 <= 0 || x1 >= (int)width-1 || x2 >= (int)width-1 )
                printMe = true;
            if( y1 <= 0 || y2 <= 0 || y1 >= (int)height-1 || y2 >= (int)height-1 )
                printMe = true;
            
            double weights[ 2 ][ 2 ];
            
            weights[ 0 ][ 0 ] = weights[ 0 ][ 1 ] = 1.0 - frac( x - 0.5f );
            weights[ 1 ][ 0 ] = weights[ 1 ][ 1 ] = frac( x - 0.5f );
            weights[ 0 ][ 0 ] *= 1.0 - frac( y - 0.5f );
            weights[ 1 ][ 0 ] *= 1.0 - frac( y - 0.5f );
            weights[ 0 ][ 1 ] *= frac( y - 0.5f );
            weights[ 1 ][ 1 ] *= frac( y - 0.5f );
            
            if( verbose )
                log_info( "\tfrac( x - 0.5f ) = %f,  frac( y - 0.5f ) = %f\n",  frac( x - 0.5f ), frac( y - 0.5f ) );
            
            for( int i = 0; i < 4; i++ )
            {
                outData[ i ] = (float)( ( upLeft[ i ] * weights[ 0 ][ 0 ] ) +
                                       ( upRight[ i ] * weights[ 1 ][ 0 ] ) +
                                       ( lowLeft[ i ] * weights[ 0 ][ 1 ] ) +
                                       ( lowRight[ i ] * weights[ 1 ][ 1 ] ));
                
                // flush subnormal results to zero if necessary
                if( NULL == containsDenorms && fabs(outData[i]) < FLT_MIN )
                    outData[i] = copysignf( 0.0f, outData[i] );
            }
        }
        else
        {    
            // 3D linear filtering
            int x1 = adFn( floorf( x - 0.5f ), width );
            int y1 = adFn( floorf( y - 0.5f ), height );
            int z1 = adFn( floorf( z - 0.5f ), depth );
            int x2 = adFn( floorf( x - 0.5f ) + 1, width );
            int y2 = adFn( floorf( y - 0.5f ) + 1, height );
            int z2 = adFn( floorf( z - 0.5f ) + 1, depth );
            
            if( verbose )
                log_info( "\tActual integer coords used (i = floor(x-.5)): i0:{%d, %d, %d} and i1:{%d, %d, %d}\n", x1, y1, z1, x2, y2, z2 );
            
            float upLeftA[ 4 ], upRightA[ 4 ], lowLeftA[ 4 ], lowRightA[ 4 ];
            float upLeftB[ 4 ], upRightB[ 4 ], lowLeftB[ 4 ], lowRightB[ 4 ];
            float pixelMaxA[4], pixelMaxB[4];
            read_image_pixel_float( imageData, imageInfo, x1, y1, z1, upLeftA );
            read_image_pixel_float( imageData, imageInfo, x2, y1, z1, upRightA );
            check_for_denorms( upLeftA, containsDenorms );
            check_for_denorms( upRightA, containsDenorms );
            pixelMax( upLeftA, upRightA, pixelMaxA );
            read_image_pixel_float( imageData, imageInfo, x1, y2, z1, lowLeftA );
            read_image_pixel_float( imageData, imageInfo, x2, y2, z1, lowRightA );
            check_for_denorms( lowLeftA, containsDenorms );
            check_for_denorms( lowRightA, containsDenorms );
            pixelMax( lowLeftA, lowRightA, pixelMaxB );
            pixelMax( pixelMaxA, pixelMaxB, returnVal.p);
            read_image_pixel_float( imageData, imageInfo, x1, y1, z2, upLeftB );
            read_image_pixel_float( imageData, imageInfo, x2, y1, z2, upRightB );
            check_for_denorms( upLeftB, containsDenorms );
            check_for_denorms( upRightB, containsDenorms );
            pixelMax( upLeftB, upRightB, pixelMaxA );
            read_image_pixel_float( imageData, imageInfo, x1, y2, z2, lowLeftB );
            read_image_pixel_float( imageData, imageInfo, x2, y2, z2, lowRightB );
            check_for_denorms( lowLeftB, containsDenorms );
            check_for_denorms( lowRightB, containsDenorms );
            pixelMax( lowLeftB, lowRightB, pixelMaxB );
            pixelMax( pixelMaxA, pixelMaxB, pixelMaxA);
            pixelMax( pixelMaxA, returnVal.p, returnVal.p );
                        
            if( verbose )
            {
                if( NULL == containsDenorms )
                    log_info( "\tSampled pixels (rgba order, denorms flushed to zero):\n" );
                else
                    log_info( "\tSampled pixels (rgba order):\n" );
                log_info( "\t\tp000: %f, %f, %f, %f\n", upLeftA[0], upLeftA[1], upLeftA[2], upLeftA[3] ); 
                log_info( "\t\tp001: %f, %f, %f, %f\n", upRightA[0], upRightA[1], upRightA[2], upRightA[3] ); 
                log_info( "\t\tp010: %f, %f, %f, %f\n", lowLeftA[0], lowLeftA[1], lowLeftA[2], lowLeftA[3] ); 
                log_info( "\t\tp011: %f, %f, %f, %f\n\n", lowRightA[0], lowRightA[1], lowRightA[2], lowRightA[3] ); 
                log_info( "\t\tp100: %f, %f, %f, %f\n", upLeftB[0], upLeftB[1], upLeftB[2], upLeftB[3] ); 
                log_info( "\t\tp101: %f, %f, %f, %f\n", upRightB[0], upRightB[1], upRightB[2], upRightB[3] ); 
                log_info( "\t\tp110: %f, %f, %f, %f\n", lowLeftB[0], lowLeftB[1], lowLeftB[2], lowLeftB[3] ); 
                log_info( "\t\tp111: %f, %f, %f, %f\n", lowRightB[0], lowRightB[1], lowRightB[2], lowRightB[3] ); 
            }
            
            double weights[ 2 ][ 2 ][ 2 ];
            
            float a = frac( x - 0.5f ), b = frac( y - 0.5f ), c = frac( z - 0.5f );
            weights[ 0 ][ 0 ][ 0 ] = weights[ 0 ][ 1 ][ 0 ] = weights[ 0 ][ 0 ][ 1 ] = weights[ 0 ][ 1 ][ 1 ] = 1.f - a;
            weights[ 1 ][ 0 ][ 0 ] = weights[ 1 ][ 1 ][ 0 ] = weights[ 1 ][ 0 ][ 1 ] = weights[ 1 ][ 1 ][ 1 ] = a; 
            weights[ 0 ][ 0 ][ 0 ] *= 1.f - b; 
            weights[ 1 ][ 0 ][ 0 ] *= 1.f - b; 
            weights[ 0 ][ 0 ][ 1 ] *= 1.f - b; 
            weights[ 1 ][ 0 ][ 1 ] *= 1.f - b; 
            weights[ 0 ][ 1 ][ 0 ] *= b; 
            weights[ 1 ][ 1 ][ 0 ] *= b; 
            weights[ 0 ][ 1 ][ 1 ] *= b; 
            weights[ 1 ][ 1 ][ 1 ] *= b; 
            weights[ 0 ][ 0 ][ 0 ] *= 1.f - c; 
            weights[ 0 ][ 1 ][ 0 ] *= 1.f - c; 
            weights[ 1 ][ 0 ][ 0 ] *= 1.f - c; 
            weights[ 1 ][ 1 ][ 0 ] *= 1.f - c; 
            weights[ 0 ][ 0 ][ 1 ] *= c; 
            weights[ 0 ][ 1 ][ 1 ] *= c; 
            weights[ 1 ][ 0 ][ 1 ] *= c; 
            weights[ 1 ][ 1 ][ 1 ] *= c; 
            
            if( verbose )
                log_info( "\tfrac( x - 0.5f ) = %f,  frac( y - 0.5f ) = %f, frac( z - 0.5f ) = %f\n",  
                         frac( x - 0.5f ), frac( y - 0.5f ), frac( z - 0.5f )  );
            
            for( int i = 0; i < 4; i++ )
            {
                outData[ i ] = (float)( ( upLeftA[ i ] * weights[ 0 ][ 0 ][ 0 ] ) +
                                       ( upRightA[ i ] * weights[ 1 ][ 0 ][ 0 ] ) +
                                       ( lowLeftA[ i ] * weights[ 0 ][ 1 ][ 0 ] ) +
                                       ( lowRightA[ i ] * weights[ 1 ][ 1 ][ 0 ] ) +
                                       ( upLeftB[ i ] * weights[ 0 ][ 0 ][ 1 ] ) +
                                       ( upRightB[ i ] * weights[ 1 ][ 0 ][ 1 ] ) +
                                       ( lowLeftB[ i ] * weights[ 0 ][ 1 ][ 1 ] ) +
                                       ( lowRightB[ i ] * weights[ 1 ][ 1 ][ 1 ] ));
                
                // flush subnormal results to zero if necessary
                if( NULL == containsDenorms && fabs(outData[i]) < FLT_MIN )
                    outData[i] = copysignf( 0.0f, outData[i] );
            }
        }
        
        return returnVal;
    }
}


    template <class T> void swizzle_vector_for_image( T *srcVector, const cl_image_format *imageFormat )
    {
        T temp;
        switch( imageFormat->image_channel_order )
        {
        case CLK_A:
            srcVector[ 0 ] = srcVector[ 3 ];
            break;
        case CLK_R:
        case CLK_RG:
        case CLK_RGB:
        case CLK_RGBA:
        case CLK_DEPTH:
            break;
        case CLK_RA:
            srcVector[ 1 ] = srcVector[ 3 ];
            break;
        case CLK_ARGB:
            temp = srcVector[ 3 ];
            srcVector[ 3 ] = srcVector[ 2 ];
            srcVector[ 2 ] = srcVector[ 1 ];
            srcVector[ 1 ] = srcVector[ 0 ];
            srcVector[ 0 ] = temp;
            break;
        case CLK_BGRA:
            temp = srcVector[ 0 ];
            srcVector[ 0 ] = srcVector[ 2 ];
            srcVector[ 2 ] = temp;
            break;
        case CLK_INTENSITY:
            srcVector[ 3 ] = srcVector[ 0 ];
            srcVector[ 2 ] = srcVector[ 0 ];
            srcVector[ 1 ] = srcVector[ 0 ];
            break;
        case CLK_LUMINANCE:
            srcVector[ 2 ] = srcVector[ 0 ];
            srcVector[ 1 ] = srcVector[ 0 ];
            break;
#ifdef CL_1RGB_APPLE
        case CLK_1RGB_APPLE:
            temp = srcVector[ 3 ];
            srcVector[ 3 ] = srcVector[ 2 ];
            srcVector[ 2 ] = srcVector[ 1 ];
            srcVector[ 1 ] = srcVector[ 0 ];
            srcVector[ 0 ] = temp;
            break;
#endif
#ifdef CL_BGR1_APPLE
        case CLK_BGR1_APPLE:
            temp = srcVector[ 0 ];
            srcVector[ 0 ] = srcVector[ 2 ];
            srcVector[ 2 ] = temp;
            break;
#endif
        }
    }

#define SATURATE( v, min, max ) ( v < min ? min : ( v > max ? max : v ) )
#define SATURATE_MAX( v, max ) ( v > max ? max : v )

    void pack_image_pixel( unsigned int *srcVector, const cl_image_format *imageFormat, void *outData )
    {
        swizzle_vector_for_image<unsigned int>( srcVector, imageFormat );
        size_t channelCount = get_format_channel_count( imageFormat );

        switch( imageFormat->image_channel_data_type )
        {
        case CLK_UNSIGNED_INT8:
            {
                unsigned char *ptr = (unsigned char *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (unsigned char)SATURATE_MAX( srcVector[ i ], 255 );
                break;
            }
        case CLK_UNSIGNED_INT16:
            {
                unsigned short *ptr = (unsigned short *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (unsigned short)SATURATE_MAX( srcVector[ i ], 65535 );
                break;
            }
        case CLK_UNSIGNED_INT32:
            {
                unsigned int *ptr = (unsigned int *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (unsigned int)srcVector[ i ];
                break;
            }
        default:
            break;
        }
    }

    void pack_image_pixel( int *srcVector, const cl_image_format *imageFormat, void *outData )
    {
        swizzle_vector_for_image<int>( srcVector, imageFormat );
        size_t chanelCount = get_format_channel_count( imageFormat );

        switch( imageFormat->image_channel_data_type )
        {
        case CLK_SIGNED_INT8:
            {
                char *ptr = (char *)outData;
                for( unsigned int i = 0; i < chanelCount; i++ )
                    ptr[ i ] = (char)SATURATE( srcVector[ i ], -128, 127 );
                break;
            }
        case CLK_SIGNED_INT16:
            {
                short *ptr = (short *)outData;
                for( unsigned int i = 0; i < chanelCount; i++ )
                    ptr[ i ] = (short)SATURATE( srcVector[ i ], -32768, 32767 );
                break;
            }
        case CLK_SIGNED_INT32:
            {
                int *ptr = (int *)outData;
                for( unsigned int i = 0; i < chanelCount; i++ )
                    ptr[ i ] = (int)srcVector[ i ];
                break;
            }
        default:
            break;
        }
    }

    int round_to_even( float v )
    {
        // clamp overflow
        if( v >= - (float) INT_MIN )
            return INT_MAX;
        if( v <= (float) INT_MIN )
            return INT_MIN;

        // round fractional values to integer value
        if( fabsf(v) < MAKE_HEX_FLOAT(0x1.0p23f, 0x1L, 23) )
        {
            static const float magic[2] = { MAKE_HEX_FLOAT(0x1.0p23f, 0x1L, 23), MAKE_HEX_FLOAT(-0x1.0p23f, -0x1L, 23) };
            float magicVal = magic[ v < 0.0f ];
            v += magicVal;
            v -= magicVal;
        }

        return (int) v;
    }

#define NORMALIZE( v, max ) ( v < 0 ? 0 : ( v > 1.f ? max : round_to_even( v * max ) ) )
#define NORMALIZE_UNROUNDED( v, max ) ( v < 0 ? 0 : ( v > 1.f ? max :  v * max ) )
#define NORMALIZE_SIGNED( v, min, max ) ( v  < -1.0f ? min : ( v > 1.f ? max : round_to_even( v * max ) ) )
#define NORMALIZE_SIGNED_UNROUNDED( v, min, max ) ( v  < -1.0f ? min : ( v > 1.f ? max : v * max ) )
#define CONVERT_INT( v, min, max, max_val)  ( v < min ? min : ( v > max ? max_val : round_to_even( v ) ) )
#define CONVERT_UINT( v, max, max_val)  ( v < 0 ? 0 : ( v > max ? max_val : round_to_even( v ) ) )

    void pack_image_pixel( float *srcVector, const cl_image_format *imageFormat, void *outData )
    {
        swizzle_vector_for_image<float>( srcVector, imageFormat );
        size_t channelCount = get_format_channel_count( imageFormat );
        switch( imageFormat->image_channel_data_type )
        {
        case CLK_HALF_FLOAT:
            {
                // todo: implement for half float
                throw Exception::NotImplemented("pack_image_pixel::Implement Float to Half");
#if 0
                cl_ushort *ptr = (cl_ushort *)outData;
                switch( gFloatToHalfRoundingMode )
                {
                case kRoundToNearestEven:
                    for( unsigned int i = 0; i < channelCount; i++ )
                        ptr[ i ] = float2half_rte( srcVector[ i ] );
                    break;
                case kRoundTowardZero:
                    for( unsigned int i = 0; i < channelCount; i++ )
                        ptr[ i ] = float2half_rtz( srcVector[ i ] );
                    break;
                default:
                    log_error( "ERROR: Test internal error -- unhandled or unknown float->half rounding mode.\n" );
                    exit(-1);
                    break;
                }
#endif
                break;
            }

        case CLK_FLOAT:
            {
                cl_float *ptr = (cl_float *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = srcVector[ i ];
                break;
            }

        case CLK_SNORM_INT8:
            {
                cl_char *ptr = (cl_char *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (char)NORMALIZE_SIGNED( srcVector[ i ], -127.0f, 127.f );
                break;
            }
        case CLK_SNORM_INT16:
            {
                cl_short *ptr = (cl_short *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (short)NORMALIZE_SIGNED( srcVector[ i ], -32767.f, 32767.f  );
                break;
            }
        case CLK_UNORM_INT8:
            {
                cl_uchar *ptr = (cl_uchar *)outData;
                //convert from RGBA to sRGBA
                if(imageFormat->image_channel_order == CLK_sRGBA || imageFormat->image_channel_order == CLK_sBGRA)
                {
                    for(uint64_t i = 0; i<channelCount-1;++i)
                    {
                        srcVector[i] = srcVector[i] <= 0.0031308f ?
                            12.92f*srcVector[i] : 1.055 * pow(srcVector[i], 1.0f/2.4f) - 0.055;
                    }
                }
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (unsigned char)NORMALIZE( srcVector[ i ], 255.f );
#ifdef CL_1RGB_APPLE
                if( imageFormat->image_channel_order == CLK_1RGB_APPLE )
                    ptr[0] = 255.0f;
#endif
#ifdef CL_BGR1_APPLE
                if( imageFormat->image_channel_order == CLK_BGR1_APPLE )
                    ptr[3] = 255.0f;
#endif
                break;
            }
        case CLK_UNORM_INT16:
            {
                cl_ushort *ptr = (cl_ushort *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (unsigned short)NORMALIZE( srcVector[ i ], 65535.f );
                break;
            }
        case CLK_UNORM_SHORT_555:
            {
                cl_ushort *ptr = (cl_ushort *)outData;
                ptr[ 0 ] = ( ( (unsigned short)NORMALIZE( srcVector[ 0 ], 31.f ) & 31 ) << 10 ) |
                    ( ( (unsigned short)NORMALIZE( srcVector[ 1 ], 31.f ) & 31 ) << 5 ) |	
                    ( ( (unsigned short)NORMALIZE( srcVector[ 2 ], 31.f ) & 31 ) << 0 );
                break;
            }
        case CLK_UNORM_SHORT_565:
            {
                cl_ushort *ptr = (cl_ushort *)outData;
                ptr[ 0 ] = ( ( (unsigned short)NORMALIZE( srcVector[ 0 ], 31.f ) & 31 ) << 11 ) |
                    ( ( (unsigned short)NORMALIZE( srcVector[ 1 ], 63.f ) & 63 ) << 5 ) |	
                    ( ( (unsigned short)NORMALIZE( srcVector[ 2 ], 31.f ) & 31 ) << 0 );
                break;
            }
        case CLK_UNORM_INT_101010:
            {
                cl_uint *ptr = (cl_uint *)outData;
                ptr[ 0 ] = ( ( (unsigned int)NORMALIZE( srcVector[ 0 ], 1023.f ) & 1023 ) << 20 ) |
                    ( ( (unsigned int)NORMALIZE( srcVector[ 1 ], 1023.f ) & 1023 ) << 10 ) |	
                    ( ( (unsigned int)NORMALIZE( srcVector[ 2 ], 1023.f ) & 1023 ) << 0 );
                break;
            }
        case CLK_SIGNED_INT8:
            {
                cl_char *ptr = (cl_char *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (char)CONVERT_INT( srcVector[ i ], -127.0f, 127.f, 127 );
                break;
            }
        case CLK_SIGNED_INT16:
            {
                cl_short *ptr = (cl_short *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (short)CONVERT_INT( srcVector[ i ], -32767.f, 32767.f, 32767  );
                break;
            }
        case CLK_SIGNED_INT32:
            {
                cl_int *ptr = (cl_int *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (int)CONVERT_INT( srcVector[ i ], MAKE_HEX_FLOAT( -0x1.0p31f, -1, 31), MAKE_HEX_FLOAT( 0x1.fffffep30f, 0x1fffffe, 30-23), CL_INT_MAX  );
                break;
            }
        case CLK_UNSIGNED_INT8:
            {
                cl_uchar *ptr = (cl_uchar *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (cl_uchar)CONVERT_UINT( srcVector[ i ], 255.f, CL_UCHAR_MAX );
                break;
            }
        case CLK_UNSIGNED_INT16:
            {
                cl_ushort *ptr = (cl_ushort *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (cl_ushort)CONVERT_UINT( srcVector[ i ], 32767.f, CL_USHRT_MAX );
                break;
            }
        case CLK_UNSIGNED_INT32:
            {
                cl_uint *ptr = (cl_uint *)outData;
                for( unsigned int i = 0; i < channelCount; i++ )
                    ptr[ i ] = (cl_uint)CONVERT_UINT( srcVector[ i ], MAKE_HEX_FLOAT( 0x1.fffffep31f, 0x1fffffe, 31-23), CL_UINT_MAX  );
                break;
            }
        default:
            log_error( "INTERNAL ERROR: unknown format (%d)\n", imageFormat->image_channel_data_type);
            exit(-1);
            break;
        }
    }


    float get_max_relative_error( cl_image_format *format, image_sampler_data *sampler, int is3D, int isLinearFilter )
    {
        float maxError = 0.0f;
        float sampleCount = 1.0f;
        if( isLinearFilter )
            sampleCount =  is3D ? 8.0f : 4.0f;

        // Note that the ULP is defined here as the unit in the last place of the maximum 
        // magnitude sample used for filtering.

        // Section 8.3
        switch( format->image_channel_data_type )
        {
            // The spec allows 2 ulps of error for normalized formats 
        case CLK_SNORM_INT8:
        case CLK_UNORM_INT8:
        case CLK_SNORM_INT16:
        case CLK_UNORM_INT16:
        case CLK_UNORM_SHORT_565:
        case CLK_UNORM_SHORT_555:
        case CLK_UNORM_INT_101010:
            maxError = 2*FLT_EPSILON*sampleCount;       // Maximum sampling error for round to zero normalization based on multiplication 
            // by reciprocal (using reciprocal generated in round to +inf mode, so that 1.0 matches spec)
            break;

            // If the implementation supports these formats then it will have to allow rounding error here too, 
            // because not all 32-bit ints are exactly representable in float
        case CLK_SIGNED_INT32:
        case CLK_UNSIGNED_INT32:
            maxError = 1*FLT_EPSILON;
            break;
        }


        // Section 8.2
        if( sampler->addressing_mode == CL_ADDRESS_REPEAT || sampler->addressing_mode == CL_ADDRESS_MIRRORED_REPEAT || sampler->filter_mode != CL_FILTER_NEAREST || sampler->normalized_coords )
        {
#if !defined(_WIN32)
//#warning Implementations will likely wish to pick a max allowable sampling error policy here that is better than the spec
#endif
            // The spec allows linear filters to return any result most of the time. 
            // That's fine for implementations but a problem for testing. After all
            // users aren't going to like garbage images.  We have "picked a number"
            // here that we are going to attempt to conform to. Implementations are 
            // free to pick another number, like infinity, if they like.
            // We picked a number for you, to provide /some/ sanity
            maxError = MAKE_HEX_FLOAT(0x1.0p-7f, 0x1L, -7);
            // ...but this is what the spec allows:
            // maxError = INFINITY;
            // Please feel free to pick any positive number. (NaN wont work.)
        }

        // The error calculation itself can introduce error
        maxError += FLT_EPSILON * 2;

        return maxError;
    }

    float get_max_absolute_error( cl_image_format *format, image_sampler_data *sampler) {
        if (sampler->filter_mode == CL_FILTER_NEAREST)
            return 0.0f;

        switch (format->image_channel_data_type) {
        case CLK_SNORM_INT8:
            return 1.0f/127.0f;
        case CLK_UNORM_INT8:
            return 1.0f/255.0f;
        case CLK_UNORM_INT16:
            return 1.0f/65535.0f;
        case CLK_SNORM_INT16:
            return 1.0f/32767.0f;
        case CLK_FLOAT:
            // disabled CL constant because of gcc  error: use of C99 hexadecimal floating constant
            // return CL_FLT_MIN
            return FLT_MIN; 
        default:
            return 0.0f;
        }
    }

bool get_integer_coords_offset( float x, float y, float z, float xAddressOffset, float yAddressOffset, float zAddressOffset,
                               size_t width, size_t height, size_t depth, image_sampler_data *imageSampler, image_descriptor *imageInfo, int &outX, int &outY, int &outZ )
{
    AddressFn adFn = sAddressingTable[ imageSampler ];
    
    float refX = floorf( x ), refY = floorf( y ), refZ = floorf( z );

    if( imageSampler->normalized_coords )
    {
        switch (imageSampler->addressing_mode)
        {
            case CL_ADDRESS_REPEAT:
                x = RepeatNormalizedAddressFn( x, width );
                if (height != 0) {
                    if (imageInfo->type != CL_MEM_OBJECT_IMAGE1D_ARRAY)
                        y = RepeatNormalizedAddressFn( y, height );
                }
                if (depth != 0) {
                    if (imageInfo->type != CL_MEM_OBJECT_IMAGE2D_ARRAY)
                        z = RepeatNormalizedAddressFn( z, depth );
                }
                
                if (xAddressOffset != 0.0) {
                    // Add in the offset
                    x += xAddressOffset;
                    // Handle wrapping
                    if (x > width)
                        x -= (float)width;
                    if (x < 0)
                        x += (float)width;
                }
                if ( (yAddressOffset != 0.0) && (imageInfo->type != CL_MEM_OBJECT_IMAGE1D_ARRAY) ) {
                    // Add in the offset
                    y += yAddressOffset;
                    // Handle wrapping
                    if (y > height)
                        y -= (float)height;
                    if (y < 0)
                        y += (float)height;
                }
                if ( (zAddressOffset != 0.0) && (imageInfo->type != CL_MEM_OBJECT_IMAGE2D_ARRAY) )  {
                    // Add in the offset
                    z += zAddressOffset;
                    // Handle wrapping
                    if (z > depth)
                        z -= (float)depth;
                    if (z < 0)
                        z += (float)depth;
                }               
                break;
                
            case CL_ADDRESS_MIRRORED_REPEAT:
                x = MirroredRepeatNormalizedAddressFn( x, width );
                if (height != 0) {
                    if (imageInfo->type != CL_MEM_OBJECT_IMAGE1D_ARRAY)
                        y = MirroredRepeatNormalizedAddressFn( y, height );
                }
                if (depth != 0) {
                    if (imageInfo->type != CL_MEM_OBJECT_IMAGE2D_ARRAY)
                        z = MirroredRepeatNormalizedAddressFn( z, depth );
                }
                
                if (xAddressOffset != 0.0) 
                {
                    float temp = x + xAddressOffset;
                    if( temp > (float) width )
                        temp = (float) width - (temp - (float) width );
                    x = fabsf( temp );
                }
                if ( (yAddressOffset != 0.0) && (imageInfo->type != CL_MEM_OBJECT_IMAGE1D_ARRAY) ) {
                    float temp = y + yAddressOffset;
                    if( temp > (float) height )
                        temp = (float) height - (temp - (float) height );
                    y = fabsf( temp );
                }
                if ( (zAddressOffset != 0.0) && (imageInfo->type != CL_MEM_OBJECT_IMAGE2D_ARRAY) )  {
                    float temp = z + zAddressOffset;
                    if( temp > (float) depth )
                        temp = (float) depth - (temp - (float) depth );
                    z = fabsf( temp );
                }               
                break;
                
            default:
                // Also, remultiply to the original coords. This simulates any truncation in
                // the pass to OpenCL
                x *= (float)width+xAddressOffset;
                if (imageInfo->type != CL_MEM_OBJECT_IMAGE1D_ARRAY)
                  y *= (float)height+yAddressOffset;
                if (imageInfo->type != CL_MEM_OBJECT_IMAGE2D_ARRAY)
                  z *= (float)depth+zAddressOffset;
                break;
        }
    }
    
    // At this point, we're dealing with non-normalized coordinates.
    
    outX = adFn( floorf( x ), width );
    
    // 1D and 2D arrays require special care for the index coordinate:
    
    switch (imageInfo->type) {
        case CL_MEM_OBJECT_IMAGE1D_ARRAY:
            outY = calculate_array_index(y, (float)imageInfo->arraySize - 1.0f);
            outZ = 0.0f; /* don't care! */
            break;
        case CL_MEM_OBJECT_IMAGE2D_ARRAY:
            outY = adFn( floorf( y ), height );
            outZ = calculate_array_index(z, (float)imageInfo->arraySize - 1.0f);
            break;
        default:
            // legacy path:
            if (height != 0)
                outY = adFn( floorf( y ), height );
            else
                outY = 0;
            if( depth != 0 )
                outZ = adFn( floorf( z ), depth );
            else
                outZ = 0;
    }
    

    
    return !( (int)refX == outX && (int)refY == outY && (int)refZ == outZ );
}


    template <> void sample_image_pixel<float>( void *imageData, image_descriptor *imageInfo, 
        float x, float y, float z, image_sampler_data *imageSampler, float *outData )
    {
        int containsDenorms = 0;
        sample_image_pixel_float( imageData, imageInfo, x, y, z, imageSampler, outData, 0, &containsDenorms );
    }

} // end namespace validation
