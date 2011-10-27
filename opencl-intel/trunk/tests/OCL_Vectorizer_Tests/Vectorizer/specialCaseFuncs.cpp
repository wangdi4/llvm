/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "specialCaseFuncs.h"


// ****************  Special-case SELECT function ********************
const char * selectFuncsList[5][SUPPORTED_WIDTHS] = {
	{"__select_1i8"  , "__select_2i8"  , "__select_3i8"  , "__select_4i8"  , "__select_8i8"  , "__select_16i8"  },
	{"__select_1i16" , "__select_2i16" , "__select_3i16" , "__select_4i16" , "__select_8i16" , "__select_16i16" },
	{"__select_1i32" , "__select_2i32" , "__select_3i32" , "__select_4i32" , "__select_8i32" , "__select_16i32" },
	{"__select_1i64" , "__select_2i64" , "__select_3i64" , "__select_4i64" , "__select_8i64" , "__select_16i64" },
	{"__select_ffi"  , "__select_ffi2" , "__select_ffi3" , "__select_ffi4" , "__select_ffi8" , "__select_ffi16" }
};


// This service function extracs the vector and integer sizes from "select" function. Note: no comparison is done, to make sure the function name exists!!
// Input: string funcName
// Output <retval> - width of select instruction: 8, 16, 32, 64 mean integer XY arguments; 100 means float XY arguments; 0 means function not found
// Output vectWidth - the vector width of the input select function
unsigned getSelectType(std::string &funcName, unsigned * vectorWidth)
{
	unsigned prefixSize = SELECT_NAME_PREFIX.size();
	
	
	// Parse the function name
	if (funcName.substr(0, prefixSize) != SELECT_NAME_PREFIX) return 0;

	// First check for Float type selects:
	if (funcName.substr(prefixSize, 2) == "ff")
	{
		// Obtain the vector width
		if (funcName.length() == prefixSize + 3)
		{
			// no width number, so 1
			*vectorWidth = 1;
		}
		else
		{
			unsigned vec_width = (unsigned)(funcName.at(prefixSize + 3) - '0');
			if (vec_width == 1) vec_width = 16; // only "16" is 2-digit, and starts with a '1'
			if (vec_width < 1 || (vec_width > 4 && vec_width != 8 && vec_width != 16))
			{
				// unsupported width! 
				return 0;
			}
			*vectorWidth = vec_width;
		}
		return 100; // value indicating floating point
	}
	
	
	// Getting here, the select is integer
	unsigned tokenSize = 1; // most vector widths are 1-digit long. If not - we'll change this later
	if (funcName.substr(prefixSize, 2) == "16")
	{
		*vectorWidth = 16;
		tokenSize = 2; // "16" is the only 2-digit value. All others are 1 digit
	}
	else
	{
		unsigned vec_width = (unsigned)(funcName.at(prefixSize) - '0'); // obtain the value of the char
		if (vec_width == 0 || (vec_width > 4 && vec_width != 8))
		{
			// unsupported values!
			return 0;
		}
		*vectorWidth = vec_width;
	}
	
	unsigned tokenLoc = prefixSize + tokenSize + 1; // skip to type width
	if (funcName.substr(tokenLoc, 1) == "8")
	{
		return 8;	
	}
	else if (funcName.substr(tokenLoc, 2) == "16")
	{
		return 16;	
	}
	else if (funcName.substr(tokenLoc, 2) == "32")
	{
		return 32;	
	}
	else if (funcName.substr(tokenLoc, 2) == "64")
	{
		return 64;	
	}
	else
	{
		// unsupported integer size
		return 0;
	}
}

// ****************  Special-case sampler functions ********************
const char * transposedread2dList[SUPPORTED_WIDTHS] =
{"_"  , "_"  , "_"  , "__read_transposed_imagef_resample"  , "__read_transposed_imagef_resample8"  , "_"  };
const char * transposedread3dList[SUPPORTED_WIDTHS] =
{"_"  , "_"  , "_"  , "__read_transposed_3d_imagef_resample"  , "__read_transposed_3d_imagef_resample8"  , "_"  };
const char * transposedwriteList[SUPPORTED_WIDTHS] =
{"_"  , "_"  , "_"  , "__write_transposed_imagef"  , "__write_transposed_imagef8"  , "_"  };



// ****************  Special-case Geometric functions ********************

geometricListType geometric_dot = {
	{"__dotf"  , "__vertical_dot1f4", "__vertical_dot1f8", "__vertical_dot1f16"},
	{"__dotf2" , "__vertical_dot2f4", "__vertical_dot2f8", "__vertical_dot2f16"},
	{"__dotf3" , "__vertical_dot3f4", "__vertical_dot3f8", "__vertical_dot3f16"},
	{"__dotf4" , "__vertical_dot4f4", "__vertical_dot4f8", "__vertical_dot4f16"}
};

geometricListType geometric_distance = {
	{"__distancef"  , "__vertical_distance1f4", "__vertical_distance1f8", "__vertical_distance1f16"},
	{"__distancef2" , "__vertical_distance2f4", "__vertical_distance2f8", "__vertical_distance2f16"},
	{"__distancef3" , "__vertical_distance3f4", "__vertical_distance3f8", "__vertical_distance3f16"},
	{"__distancef4" , "__vertical_distance4f4", "__vertical_distance4f8", "__vertical_distance4f16"},
};
geometricListType geometric_fast_distance = {
	{"__fast_distancef"  , "__vertical_fast_distance1f4", "__vertical_fast_distance1f8", "__vertical_fast_distance1f16"},
	{"__fast_distancef2" , "__vertical_fast_distance2f4", "__vertical_fast_distance2f8", "__vertical_fast_distance2f16"},
	{"__fast_distancef3" , "__vertical_fast_distance3f4", "__vertical_fast_distance3f8", "__vertical_fast_distance3f16"},
	{"__fast_distancef4" , "__vertical_fast_distance4f4", "__vertical_fast_distance4f8", "__vertical_fast_distance4f16"},
};

geometricListType geometric_length = {
	{"__lengthf"  , "__vertical_length1f4", "__vertical_length1f8", "__vertical_length1f16"},
	{"__lengthf2" , "__vertical_length2f4", "__vertical_length2f8", "__vertical_length2f16"},
	{"__lengthf3" , "__vertical_length3f4", "__vertical_length3f8", "__vertical_length3f16"},
	{"__lengthf4" , "__vertical_length4f4", "__vertical_length4f8", "__vertical_length4f16"},
};
geometricListType geometric_fast_length = {
	{"__fast_lengthf"  , "__vertical_fast_length1f4", "__vertical_fast_length1f8", "__vertical_fast_length1f16"},
	{"__fast_lengthf2" , "__vertical_fast_length2f4", "__vertical_fast_length2f8", "__vertical_fast_length2f16"},
	{"__fast_lengthf3" , "__vertical_fast_length3f4", "__vertical_fast_length3f8", "__vertical_fast_length3f16"},
	{"__fast_lengthf4" , "__vertical_fast_length4f4", "__vertical_fast_length4f8", "__vertical_fast_length4f16"},
};
 
geometricListType geometric_cross = {
	{"_", "_", "_", "_"},
	{"_", "_", "_", "_"},
	{"__crossf3" , "__vertical_cross3f4", "__vertical_cross3f8", "__vertical_cross3f16"},
	{"__crossf4" , "__vertical_cross4f4", "__vertical_cross4f8", "__vertical_cross4f16"},
};
geometricListType geometric_normalize = {
	{"__normalizef"  , "__vertical_normalize1f4", "__vertical_normalize1f8", "__vertical_normalize1f16"},
	{"__normalizef2" , "__vertical_normalize2f4", "__vertical_normalize2f8", "__vertical_normalize2f16"},
	{"__normalizef3" , "__vertical_normalize3f4", "__vertical_normalize3f8", "__vertical_normalize3f16"},
	{"__normalizef4" , "__vertical_normalize4f4", "__vertical_normalize4f8", "__vertical_normalize4f16"},
};
geometricListType geometric_fast_normalize = {
	{"__fast_normalizef"  , "__vertical_fast_normalize1f4", "__vertical_fast_normalize1f8", "__vertical_fast_normalize1f16"},
	{"__fast_normalizef2" , "__vertical_fast_normalize2f4", "__vertical_fast_normalize2f8", "__vertical_fast_normalize2f16"},
	{"__fast_normalizef3" , "__vertical_fast_normalize3f4", "__vertical_fast_normalize3f8", "__vertical_fast_normalize3f16"},
	{"__fast_normalizef4" , "__vertical_fast_normalize4f4", "__vertical_fast_normalize4f8", "__vertical_fast_normalize4f16"}
};




// ****************  Special-case fract function ********************
const char * fractFuncsList[SUPPORTED_WIDTHS] =
{"__fractf"  , "__fractf2"  , "__fractf3"  , "__fractf4"  , "__fractf8"  , "__fractf16"  };




// "Safe" functions list: These are functions which appear in the runtime module, receive pointer inputs, but are known to have no side effect
// The list does not include all the functions which are already in the function-hash, and are therefore also known to have no side effects
// To save wasted checks - only the prefix of the function's names appears
const char * noSideEffectFuncs[] = {
	"read_2d_",
	"read_3d_",
	"__get_image_width_",
	"__get_image_height_",
	"get_image_depth",
	"__get_image_channel_",
	"__get_image_dim_",
	"__write_imagef_",
	"__write_imagei_",
	"__write_imageui_",
	NULL 
};



