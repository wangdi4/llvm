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
//  Helper routines to perform searches in various arrays. Header file.
//
//  AUTHORS: Nikita Astafiev
//
//  CREATION DATE: 23-Sep-2009
//
//  MODIFICATION HISTORY:
//
//--

#ifndef LIBIML_ATTR_SEARCH_H_INCLUDED
#define LIBIML_ATTR_SEARCH_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif // __cplusplus

int IML_ATTR_get_name_index(const char* name,
                const char* where_to_search[],
                int max_size);

int IML_ATTR_get_name_index_unsorted(const char* name,
                const char* where_to_search[],
                int max_size);

int IML_ATTR_get_int_index(const int name,
                const int* where_to_search,
                int max_size);

#if defined __cplusplus
}
#endif // __cplusplus

#endif //LIBIML_ATTR_SEARCH_H_INCLUDED
