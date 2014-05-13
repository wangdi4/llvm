/******************************************************************
//
//  OpenCL Conformance Tests
// 
//  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
//
******************************************************************/

#ifndef _conversions_h
#define _conversions_h

#include "errorHelpers.h"
#include "mt19937.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <sys/types.h>
#include "compat.h"

#if defined(__cplusplus)
extern "C" {
#endif 

/* Note: the next three all have to match in size and order!! */

enum ExplicitTypes
{
	kBool		= 0,
	kChar,
	kUChar,
	kUnsignedChar,
	kShort,
	kUShort,
	kUnsignedShort,
	kInt,
	kUInt,
	kUnsignedInt,
	kLong,
	kULong,
	kUnsignedLong,
	kFloat,
	kHalf,
	kDouble,
	kNumExplicitTypes
};

typedef enum ExplicitTypes	ExplicitType;

enum RoundingTypes
{
	kRoundToEven = 0,
	kRoundToZero,
	kRoundToPosInf,
	kRoundToNegInf,
	kRoundToNearest,
	
	kNumRoundingTypes,
	
	kDefaultRoundingType = kRoundToNearest
};

typedef enum RoundingTypes	RoundingType;

extern void             print_type_to_string(ExplicitType type, void *data, char* string);
extern size_t           get_explicit_type_size( ExplicitType type );
extern const char *     get_explicit_type_name( ExplicitType type );
extern void             convert_explicit_value( void *inRaw, void *outRaw, ExplicitType inType, bool saturate, RoundingType roundType, ExplicitType outType );

extern void             generate_random_data( ExplicitType type, size_t count, MTdata d, void *outData );
extern void	*         create_random_data( ExplicitType type, MTdata d, size_t count );

extern cl_long          read_upscale_signed( void *inRaw, ExplicitType inType );
extern cl_ulong         read_upscale_unsigned( void *inRaw, ExplicitType inType );
extern float            read_as_float( void *inRaw, ExplicitType inType );

extern float            get_random_float(float low, float high, MTdata d);
extern double           get_random_double(double low, double high, MTdata d);
extern float            any_float( MTdata d );
extern double           any_double( MTdata d );

extern int              random_in_range( int minV, int maxV, MTdata d );

size_t get_random_size_t(size_t low, size_t high, MTdata d);

#if defined(__cplusplus)
}
#endif

#endif // _conversions_h


