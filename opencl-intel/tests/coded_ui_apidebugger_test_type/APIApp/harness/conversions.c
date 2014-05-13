/******************************************************************
//
//  OpenCL Conformance Tests
// 
//  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
//
******************************************************************/

#include "conversions.h"
#include <limits.h>
#include <time.h>
#include <assert.h>
#include "mt19937.h"
#include "compat.h"

#if defined( __SSE__ ) || defined (_MSC_VER)
    #include <xmmintrin.h>
#endif
#if defined( __SSE2__ ) || defined (_MSC_VER)
    #include <emmintrin.h>
#endif

typedef cl_long Long;
typedef cl_ulong ULong;

static ULong sUpperLimits[ kNumExplicitTypes ] = 
	{ 
		0,
		127, 255, 255,
		32767, 65535, 65535,
		0x7fffffffLL, 0xffffffffLL, 0xffffffffLL,
		0x7fffffffffffffffLL, 0xffffffffffffffffLL, 0xffffffffffffffffLL,
		0, 0 };	// Last two values aren't stored here

static Long sLowerLimits[ kNumExplicitTypes ] = 
	{ 
		-1,
		-128, 0, 0,
		-32768, 0, 0,
		0xffffffff80000000LL, 0, 0,
		0x8000000000000000LL, 0, 0,
		0, 0 };	// Last two values aren't stored here

#define BOOL_CASE(inType) \
		case kBool:	\
			boolPtr = (bool *)outRaw; \
			*boolPtr = ( *inType##Ptr ) != 0 ? true : false; \
			break;

#define SIMPLE_CAST_CASE(inType,outEnum,outType) \
		case outEnum:								\
			outType##Ptr = (outType *)outRaw;		\
			*outType##Ptr = (outType)(*inType##Ptr);	\
			break;
			
// Sadly, the ULong downcasting cases need a separate #define to get rid of signed/unsigned comparison warnings
#define DOWN_CAST_CASE(inType,outEnum,outType,sat) \
		case outEnum:								\
			outType##Ptr = (outType *)outRaw;		\
			if( sat )								\
			{										\
				if( ( sLowerLimits[outEnum] < 0 && *inType##Ptr > (Long)sUpperLimits[outEnum] ) || ( sLowerLimits[outEnum] == 0 && (ULong)*inType##Ptr > sUpperLimits[outEnum] ) )\
					*outType##Ptr = (outType)sUpperLimits[outEnum];\
				else if( *inType##Ptr < sLowerLimits[outEnum] )\
					*outType##Ptr = (outType)sLowerLimits[outEnum]; \
				else											\
					*outType##Ptr = (outType)*inType##Ptr;	\
			} else {								\
				*outType##Ptr = (outType)( *inType##Ptr & ( 0xffffffffffffffffLL >> ( 64 - ( sizeof( outType ) * 8 ) ) ) ); \
			}										\
			break;
			
#define U_DOWN_CAST_CASE(inType,outEnum,outType,sat) \
		case outEnum:								\
			outType##Ptr = (outType *)outRaw;		\
			if( sat )								\
			{										\
				if( (ULong)*inType##Ptr > sUpperLimits[outEnum] )\
					*outType##Ptr = (outType)sUpperLimits[outEnum];\
				else											\
					*outType##Ptr = (outType)*inType##Ptr;	\
			} else {								\
				*outType##Ptr = (outType)( *inType##Ptr & ( 0xffffffffffffffffLL >> ( 64 - ( sizeof( outType ) * 8 ) ) ) ); \
			}										\
			break;
			
#define TO_FLOAT_CASE(inType)				\
		case kFloat:						\
			floatPtr = (float *)outRaw;		\
			*floatPtr = (float)(*inType##Ptr);	\
			break;
#define TO_DOUBLE_CASE(inType)				\
		case kDouble:						\
			doublePtr = (double *)outRaw;		\
			*doublePtr = (double)(*inType##Ptr);	\
			break;
			
	
/* Note: we use lrintf here to force the rounding instead of whatever the processor's current rounding mode is */
#define FLOAT_ROUND_TO_NEAREST_CASE(outEnum,outType)	\
		case outEnum:									\
			outType##Ptr = (outType *)outRaw;			\
			*outType##Ptr = (outType)lrintf_clamped( *floatPtr );	\
			break;

#define FLOAT_ROUND_CASE(outEnum,outType,rounding,sat)	\
		case outEnum:									\
		{												\
			outType##Ptr = (outType *)outRaw;			\
			/* Get the tens digit */					\
			Long wholeValue = (Long)*floatPtr;\
			float largeRemainder = ( *floatPtr - (float)wholeValue ) * 10.f; \
			/* What do we do based on that? */				\
			if( rounding == kRoundToEven )					\
			{												\
				if( wholeValue & 1LL )	/*between 1 and 1.99 */	\
					wholeValue += 1LL;	/* round up to even */  \
			}												\
			else if( rounding == kRoundToZero )				\
			{												\
				/* Nothing to do, round-to-zero is what C casting does */							\
			}												\
			else if( rounding == kRoundToPosInf )			\
			{												\
				/* Only positive numbers are wrong */		\
				if( largeRemainder != 0.f && wholeValue >= 0 )	\
					wholeValue++;							\
			}												\
			else if( rounding == kRoundToNegInf )			\
			{												\
				/* Only negative numbers are off */			\
				if( largeRemainder != 0.f && wholeValue < 0 ) \
					wholeValue--;							\
			}												\
			else											\
			{   /* Default is round-to-nearest */			\
				wholeValue = (Long)lrintf_clamped( *floatPtr );	\
			}												\
			/* Now apply saturation rules */				\
			if( sat )								\
			{										\
				if( ( sLowerLimits[outEnum] < 0 && wholeValue > (Long)sUpperLimits[outEnum] ) || ( sLowerLimits[outEnum] == 0 && (ULong)wholeValue > sUpperLimits[outEnum] ) )\
					*outType##Ptr = (outType)sUpperLimits[outEnum];\
				else if( wholeValue < sLowerLimits[outEnum] )\
					*outType##Ptr = (outType)sLowerLimits[outEnum]; \
				else											\
					*outType##Ptr = (outType)wholeValue;	\
			} else {								\
				*outType##Ptr = (outType)( wholeValue & ( 0xffffffffffffffffLL >> ( 64 - ( sizeof( outType ) * 8 ) ) ) ); \
			}										\
		}				\
		break;
	
#define DOUBLE_ROUND_CASE(outEnum,outType,rounding,sat)	\
		case outEnum:									\
		{												\
			outType##Ptr = (outType *)outRaw;			\
			/* Get the tens digit */					\
			Long wholeValue = (Long)*doublePtr;\
			double largeRemainder = ( *doublePtr - (double)wholeValue ) * 10.0; \
			/* What do we do based on that? */				\
			if( rounding == kRoundToEven )					\
			{												\
				if( wholeValue & 1LL )	/*between 1 and 1.99 */	\
					wholeValue += 1LL;	/* round up to even */  \
			}												\
			else if( rounding == kRoundToZero )				\
			{												\
				/* Nothing to do, round-to-zero is what C casting does */							\
			}												\
			else if( rounding == kRoundToPosInf )			\
			{												\
				/* Only positive numbers are wrong */		\
				if( largeRemainder != 0.0 && wholeValue >= 0 )	\
					wholeValue++;							\
			}												\
			else if( rounding == kRoundToNegInf )			\
			{												\
				/* Only negative numbers are off */			\
				if( largeRemainder != 0.0 && wholeValue < 0 ) \
					wholeValue--;							\
			}												\
			else											\
			{   /* Default is round-to-nearest */			\
				wholeValue = (Long)lrint_clamped( *doublePtr );	\
			}												\
			/* Now apply saturation rules */				\
			if( sat )								\
			{										\
				if( ( sLowerLimits[outEnum] < 0 && wholeValue > (Long)sUpperLimits[outEnum] ) || ( sLowerLimits[outEnum] == 0 && (ULong)wholeValue > sUpperLimits[outEnum] ) )\
					*outType##Ptr = (outType)sUpperLimits[outEnum];\
				else if( wholeValue < sLowerLimits[outEnum] )\
					*outType##Ptr = (outType)sLowerLimits[outEnum]; \
				else											\
					*outType##Ptr = (outType)wholeValue;	\
			} else {								\
				*outType##Ptr = (outType)( wholeValue & ( 0xffffffffffffffffLL >> ( 64 - ( sizeof( outType ) * 8 ) ) ) ); \
			}										\
		}				\
		break;
		
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

void generate_random_data( ExplicitType type, size_t count, MTdata d, void *outData )
{
	bool *boolPtr;
	cl_char *charPtr;
	cl_uchar *ucharPtr;
	cl_short *shortPtr;
	cl_ushort *ushortPtr;
	cl_int *intPtr;
	cl_uint *uintPtr;
	cl_long *longPtr;
	cl_ulong *ulongPtr;
	cl_float *floatPtr;
    cl_double *doublePtr;
	cl_ushort *halfPtr;
	size_t i;
    cl_uint bits = genrand_int32(d);
    cl_uint bitsLeft = 32;
    
	switch( type )
	{
		case kBool:
			boolPtr = (bool *)outData;
			for( i = 0; i < count; i++ )
			{
                if( 0 == bitsLeft) 
                { 
                    bits = genrand_int32(d);   
                    bitsLeft = 32; 
                }
				boolPtr[i] = ( bits & 1 ) ? true : false;
                bits >>= 1; bitsLeft -= 1;
			}
			break;
			
		case kChar:
			charPtr = (cl_char *)outData;
			for( i = 0; i < count; i++ )
			{
                if( 0 == bitsLeft) 
                { 
                    bits = genrand_int32(d);   
                    bitsLeft = 32; 
                }
				charPtr[i] = (cl_char)( (cl_int)(bits & 255 ) - 127 );
                bits >>= 8; bitsLeft -= 8;
			}
			break;
			
		case kUChar:
		case kUnsignedChar:
			ucharPtr = (cl_uchar *)outData;
			for( i = 0; i < count; i++ )
			{
                if( 0 == bitsLeft) 
                { 
                    bits = genrand_int32(d);   
                    bitsLeft = 32; 
                }
				ucharPtr[i] = (cl_uchar)( bits & 255 );
                bits >>= 8; bitsLeft -= 8;
			}
			break;
			
		case kShort:
			shortPtr = (cl_short *)outData;
			for( i = 0; i < count; i++ )
			{
                if( 0 == bitsLeft) 
                { 
                    bits = genrand_int32(d);   
                    bitsLeft = 32; 
                }
				shortPtr[i] = (cl_short)( (cl_int)( bits & 65535 ) - 32767 );
                bits >>= 16; bitsLeft -= 16;
			}
			break;
			
		case kUShort:
		case kUnsignedShort:
			ushortPtr = (cl_ushort *)outData;
			for( i = 0; i < count; i++ )
			{
                if( 0 == bitsLeft) 
                { 
                    bits = genrand_int32(d);   
                    bitsLeft = 32; 
                }
				ushortPtr[i] = (cl_ushort)( (cl_int)( bits & 65535 ) );
                bits >>= 16; bitsLeft -= 16;
			}
			break;
			
		case kInt:
			intPtr = (cl_int *)outData;
			for( i = 0; i < count; i++ )
			{
				intPtr[i] = (cl_int)genrand_int32(d);
			}
			break;
			
		case kUInt:
		case kUnsignedInt:
			uintPtr = (cl_uint *)outData;
			for( i = 0; i < count; i++ )
			{
				uintPtr[i] = (unsigned int)genrand_int32(d);
			}
			break;
			
		case kLong:
			longPtr = (cl_long *)outData;
			for( i = 0; i < count; i++ )
			{
				longPtr[i] = (cl_long)genrand_int32(d) | ( (cl_long)genrand_int32(d) << 32 );
			}
			break;
			
		case kULong:
		case kUnsignedLong:
			ulongPtr = (cl_ulong *)outData;
			for( i = 0; i < count; i++ )
			{
				ulongPtr[i] = (cl_ulong)genrand_int32(d) | ( (cl_ulong)genrand_int32(d) << 32 );
			}
			break;
			
		case kFloat:
			floatPtr = (cl_float *)outData;
			for( i = 0; i < count; i++ )
			{
                // [ -(double) 0x7fffffff, (double) 0x7fffffff ]
				double t = genrand_real1(d);
				floatPtr[i] = (float) ((1.0 - t) * -(double) 0x7fffffff + t * (double) 0x7fffffff);      
			}
			break;
      
        case kDouble:
            doublePtr = (cl_double *)outData;
            for( i = 0; i < count; i++ )
            {
                cl_long u = (cl_long)genrand_int32(d) | ( (cl_long)genrand_int32(d) << 32 );
                double t = (double) u;
                t *= MAKE_HEX_DOUBLE( 0x1.0p-32, 0x1, -32 );        // scale [-2**63, 2**63] to [-2**31, 2**31]
                doublePtr[i] = t;
            }
            break;
			
		case kHalf:
			halfPtr = (ushort *)outData;
			for( i = 0; i < count; i++ )
			{
                if( 0 == bitsLeft) 
                { 
                    bits = genrand_int32(d);   
                    bitsLeft = 32; 
                }
				halfPtr[i] = bits & 65535;	 /* Kindly generates random bits for us */
                bits >>= 16; bitsLeft -= 16;
			}
			break;
			
		default:
			log_error( "ERROR: Invalid type passed in to generate_random_data!\n" );
			break;
	}
}

float get_random_float(float low, float high, MTdata d)
{
	float t = (float)((double)genrand_int32(d) / (double)0xFFFFFFFF);
	return (1.0f - t) * low + t * high;
}

size_t get_random_size_t(size_t low, size_t high, MTdata d)
{
  enum { N = sizeof(size_t)/sizeof(int) };
  
  union {
    int word[N];
    size_t size;
  } u;
  
  for (unsigned i=0; i != N; ++i) {
    u.word[i] = genrand_int32(d);
  }

  assert(low <= high && "Invalid random number range specified");
  size_t range = high - low;
  
  return (range) ? low + ((u.size - low) % range) : low;
}


