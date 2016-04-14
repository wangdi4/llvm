//      Copyright  (C) 2009-2016 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// cvs_id[] = "$Id$"
//

//++
//  Compiler-library interface that is supposed to pass queries to the
//  library and return functions names appropriate for the query.
//
//  AUTHORS: Nikita Astafiev
//
//  CREATION DATE: 23-Sep-2009
//
//  MODIFICATION HISTORY:
//      22-Apr-2011, Added may_i_use_inline_implementation interface extenstion
//                   for inlining. NA
//
//--


#ifndef LIBIML_ATTR_IML_ACCURACY_INTERFACE_H_INCLUDED
#define LIBIML_ATTR_IML_ACCURACY_INTERFACE_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif // __cplusplus


// String pair which defines name and value for a single attribute. The
// first string in a pair contains the attribute name. The second defines
// the attribute value. There is also a pointer to the next attribute
// structure.
typedef struct ImfAttr {
    const char*     name;
    const char*     value;
    struct ImfAttr* next;
} ImfAttr;

// This function returns the name of the library function to call
// given the specified constraints.
//
// base_name
//    is the name of the math function of interest, e.g. sin, expf
//
// attributes
//    define desired constrains for the function. Attributes array is
//    terminated by an attribute with NULL next pointer.
extern const char* get_library_function_name(const char* base_name,
                                             const ImfAttr* attributes);

// This function returns 1 or 0 meaning "yes" or "no". It answers the question:
// whether the compiler may use an instructions sequence with certain
// properties described by inline_implementation_attributes list given the
// conditions specified by user_specified_attributes list.
//
// function_name
//    is the name of the math function of interest, e.g. sin, expf.
//
// user_specified_attributes
//    define desired constrains for the function. Attributes array is
//    terminated by a pair with the NULL attribute name.
//
// inline_implementation_attributes
//    define properties of some function implementation (presumably known to
//    the compiler). Attributes array is terminated by a pair with the NULL
//    attribute name.
int may_i_use_inline_implementation(
                            const char* function_name,
                            const ImfAttr* user_specified_attributes,
                            const ImfAttr* inline_implementation_attributes);

#if defined __cplusplus
}
#endif // __cplusplus

#endif// LIBIML_ATTR_IML_ACCURACY_INTERFACE_H_INCLUDED
