/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __SPECIAL_CASE_FUNCS__H__
#define __SPECIAL_CASE_FUNCS__H__

#include <string>

namespace intel {

// Exactly 6 vector widths are supported overall: 1, 2, 3, 4, 8, 16
#define SUPPORTED_WIDTHS 6

#define FAKE_PREFIX "__(!(fake_"
#define DX_WRAPPER_PREFIX   "dx_soa_"
#define FAKE_NAME_STR(name) std::string((char*)(FAKE_PREFIX)).append(name)


// ****************  Special-case SELECT function ********************

#define SELECT_NAME_PREFIX std::string("__select_")
#define SELECT_FAKE_SCALAR_NAME_PREFIX FAKE_NAME_STR("scalar_select")
#define SELECT_FAKE_VECTOR_NAME_PREFIX FAKE_NAME_STR("vector_select")
extern const char * selectFuncsList[5][SUPPORTED_WIDTHS];

// This service function extracts the vector and integer sizes from "select" function. Note: no comparison is done, to make sure the function name exists!!
// Input: string funcName
// Output <retval> - width of select instruction: 8, 16, 32, 64 mean integer XY arguments; 100 means float XY arguments; 0 means function not found
// Output vectWidth - the vector width of the input select function
unsigned getSelectType(std::string &funcName, unsigned * vectorWidth);



// ****************  Special-case Geometric functions ********************
#define MAX_GEOMETRIC_FUNC_ARGS 16
#define GEOMETRIC_WIDTHS 4   /* number of vector widths supported by geometric funcs */
#define VERTICAL_FUNCS 4     /* number of offunctions appearing in every list of scalar+vertical functions */
typedef const char * geometricListType[GEOMETRIC_WIDTHS][VERTICAL_FUNCS];

extern geometricListType geometric_dot;
extern geometricListType geometric_distance;
extern geometricListType geometric_fast_distance;
extern geometricListType geometric_length;
extern geometricListType geometric_fast_length;
extern geometricListType geometric_cross;
extern geometricListType geometric_normalize;
extern geometricListType geometric_fast_normalize;

#define DOT_PRODUCT_NAME "_Z3dot" // this is realy bad

// Dot product
#define DOT_PRODUCT_NAME_PREFIX        std::string("__dotf")
#define DOT_PRODUCT_FAKE_SCALAR_NAME_PREFIX FAKE_NAME_STR("scalar_dotProd")
#define DOT_PRODUCT_FAKE_VECTOR_NAME_PREFIX FAKE_NAME_STR("vector_dotProd")

// Geometric length
#define LENGTH_NAME_PREFIX          std::string("__lengthf")
#define LENGTH_FAKE_SCALAR_NAME_PREFIX    FAKE_NAME_STR("scalar_lengthf")
#define LENGTH_FAKE_VECTOR_NAME_PREFIX    FAKE_NAME_STR("vector_lengthf")

#define FAST_LENGTH_NAME_PREFIX        std::string("__fast_lengthf")
#define FAST_LENGTH_FAKE_SCALAR_NAME_PREFIX FAKE_NAME_STR("scalar_fast_lengthf")
#define FAST_LENGTH_FAKE_VECTOR_NAME_PREFIX FAKE_NAME_STR("vector_fast_lengthf")

// Geometric distance
#define DISTANCE_NAME_PREFIX        std::string("__distancef")
#define DISTANCE_FAKE_SCALAR_NAME_PREFIX  FAKE_NAME_STR("scalar_distancef")
#define DISTANCE_FAKE_VECTOR_NAME_PREFIX  FAKE_NAME_STR("vector_distancef")

#define FAST_DISTANCE_NAME_PREFIX        std::string("__fast_distancef")
#define FAST_DISTANCE_FAKE_SCALAR_NAME_PREFIX  FAKE_NAME_STR("scalar_fast_distancef")
#define FAST_DISTANCE_FAKE_VECTOR_NAME_PREFIX  FAKE_NAME_STR("vector_fast_distancef")

// Cross product
#define CROSS_PRODUCT_NAME_PREFIX        std::string("__crossf")
#define CROSS_PRODUCT_FAKE_SCALAR_NAME_PREFIX  FAKE_NAME_STR("scalar_crossf")
#define CROSS_PRODUCT_FAKE_VECTOR_NAME_PREFIX  FAKE_NAME_STR("vector_crossf")

// Geometric normalize
#define NORMALIZE_NAME_PREFIX        std::string("__normalizef")
#define NORMALIZE_FAKE_SCALAR_NAME_PREFIX  FAKE_NAME_STR("scalar_normalizef")
#define NORMALIZE_FAKE_VECTOR_NAME_PREFIX  FAKE_NAME_STR("vector_normalizef")

#define FAST_NORMALIZE_NAME_PREFIX        std::string("__fast_normalizef")
#define FAST_NORMALIZE_FAKE_SCALAR_NAME_PREFIX  FAKE_NAME_STR("scalar_fast_normalizef")
#define FAST_NORMALIZE_FAKE_VECTOR_NAME_PREFIX  FAKE_NAME_STR("vector_fast_normalizef")


// ****************  Special-case Sampler functions ********************

// Read sampler
#define READ_IMAGEF_2D_NAME "read_2d_ff"
#define READ_IMAGEF_2D_COORD_TYPE  VectorType::get(Type::getFloatTy(context()), 2)  /* <2 x float> */
#define READ_IMAGEF_RET_TYPE    VectorType::get(Type::getFloatTy(context()), 4)  /* <4 x float> */
#define TRANSPOSED_READ_IMAGEF_2D_NAME "__read_transposed_imagef_resample"
#define FAKE_SCALAR_READ_IMAGEF_2D_NAME FAKE_NAME_STR("scalar_read_2d_ff")
#define FAKE_VECTOR_READ_IMAGEF_2D_NAME FAKE_NAME_STR("vector_read_2d_ff")
// Stream read sampler
#define STREAM_READ_IMAGEF_2D_NAME "__async_work_group_stream_private_from_image"

// Read sampler 3D
#define READ_IMAGEF_3D_NAME "read_3d_ff"
#define READ_IMAGEF_3D_COORD_TYPE  VectorType::get(Type::getFloatTy(context()), 4)  /* <4 x float> */
#define TRANSPOSED_READ_IMAGEF_3D_NAME "__read_transposed_3d_imagef_resample"
#define FAKE_SCALAR_READ_IMAGEF_3D_NAME FAKE_NAME_STR("scalar_read_3d_ff")
#define FAKE_VECTOR_READ_IMAGEF_3D_NAME FAKE_NAME_STR("vector_read_3d_ff")

// Write sampler
#define WRITE_IMAGEF_COORD_TYPE    VectorType::get(Type::getInt32Ty(context()), 2)  /* <2 x i32> */
#define WRITE_IMAGEF_COLORS_TYPE   VectorType::get(Type::getFloatTy(context()), 4)  /* <4 x float> */
#define TRANSPOSED_WRITE_IMAGEF_NAME "write_transposed_imagef"
#define TRANSPOSED_WRITE_IMAGEF_COLOR_TYPE    VectorType::get(Type::getFloatTy(context()), 4)  /* <4 x float> */
#define FAKE_SCALAR_WRITE_IMAGEF_NAME FAKE_NAME_STR("scalar_write_imagef")
#define FAKE_VECTOR_WRITE_IMAGEF_NAME FAKE_NAME_STR("vector_write_imagef")
#define WRITE_IMAGEF_NAME "__write_imagef_2d"
// Stream write sampler
#define STREAM_WRITE_IMAGEF_NAME "__async_work_group_stream_private_to_image"


// ****************  Special-case Early-exit ********************
#define EARLY_EXIT_FAKE_NAME_PREFIX FAKE_NAME_STR("early_exit_")



} // Namespace

#endif // __SPECIAL_CASE_FUNCS__H__
