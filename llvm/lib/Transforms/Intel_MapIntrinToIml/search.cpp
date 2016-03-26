//      Copyright  (C) 2009-2010 Intel Corporation.
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
//  Helper routines to perform searches in various arrays.
//
//  AUTHORS: Nikita Astafiev
//
//  CREATION DATE: 23-Sep-2009
//
//  MODIFICATION HISTORY:
//
//--


#include "llvm/Transforms/Intel_MapIntrinToIml/search.h"

#include <string.h>
#include "llvm/Transforms/Intel_MapIntrinToIml/messaging.h"
#include "llvm/Transforms/Intel_MapIntrinToIml/iml_attr_private.h"

#if defined __cplusplus
extern "C" {
#endif // __cplusplus

// this function performs a binary search over the !sorted! array of strings
// and returns the index if the value is found
IMLATTR_INTERNAL_FUNC_VISIBILITY
int IML_ATTR_get_name_index(const char* name,
                   const char* where_to_search[],
                   int max_size)
{
    int l, r, s, direction;

    if (!strcmp(name, ""))
    {
        PRN_MSG("error: in IML_ATTR_get_name_index: empty search token\n");
        return -1;
    }

#if defined IML_DEBUG
    // user should not suffer the penalty of this extra check
    {
        int i;

        for (i = 0; i < max_size-1; i++)
        {
            if (strcmp(where_to_search[i], where_to_search[i+1]) >= 0)
            {
                PRN_MSG("error: in IML_ATTR_get_name_index: search space unsorted\n");
                PRN_MSG("error: in IML_ATTR_get_name_index: %s >= %s\n", \
                        where_to_search[i], where_to_search[i + 1]);
                return -2;
            }
        }
    }
#endif

    l = 0;
    r = max_size-1;

    while ( (r - l) > 1 )
    {
        s = (l + r) / 2;

        direction = strcmp(name, where_to_search[s]);

        if (direction < 0)
        {
            r = s;
        }
        else if (direction > 0)
        {
            l = s;
        }
        else if (direction == 0)
        {
            return s;
        }
    }

    if (!strcmp(name, where_to_search[l]))
    {
        return l;
    }
    if (!strcmp(name, where_to_search[r]))
    {
        return r;
    }

    PRN_MSG("error: in IML_ATTR_get_name_index: token \"%s\" not found\n",
            name);
    return -3;
}

// this function performs a search over the !unsorted! array of strings
// and returns the index if the value is found
IMLATTR_INTERNAL_FUNC_VISIBILITY
int IML_ATTR_get_name_index_unsorted(const char* name,
                            const char* where_to_search[],
                            int max_size)
{
    int i;

    if (!strcmp(name, ""))
    {
        PRN_MSG("error: in IML_ATTR_get_name_index_unsorted: empty search token\n");
        return -1;
    }

    for ( i = 0; i < max_size; i++)
    {
        if (!strcmp(name, where_to_search[i]))
        {
            return i;
        }
    }

    PRN_MSG("error: in IML_ATTR_get_name_index_unsorted: token \"%s\" not found\n", \
            name);
    return -2;
}

// this function performs a binary search over the !sorted! array of integers
// returns the index of the value which is less than or equal
// return zero if searching number is less than all present in array
IMLATTR_INTERNAL_FUNC_VISIBILITY
int IML_ATTR_get_int_index(const int name,
                           const int* where_to_search,
                           int max_size)
{
    int l, r, s;

#if defined IML_DEBUG
    // user should not suffer the penalty of this extra check
    {
        int i;

        for (i = 0; i < max_size-1; i++)
        {
            if (where_to_search[i] >= where_to_search[i+1])
            {
                PRN_MSG("error: in IML_ATTR_get_int_index: search space unsorted\n");
                PRN_MSG("error: in IML_ATTR_get_int_index: a[%d] >= a[%d]\n", \
                        i, i + 1);
                return 0;
            }
        }
    }
#endif

    l = 0;
    r = max_size-1;

    while ( (r - l) > 1 )
    {
        s = (l + r) / 2;

        if (name < where_to_search[s])
        {
            r = s;
        }
        else if (name > where_to_search[s])
        {
            l = s;
        }
        else
        {
            return s;
        }
    }

    if (name <= where_to_search[l])
    {
        return l;
    }
    if (name >= where_to_search[r])
    {
        return r;
    }

    PRN_MSG("error: in IML_ATTR_get_int_index: token \"%d\" not found, should not get here\n", \
            name);
    return 0;
}

#if defined __cplusplus
}
#endif // __cplusplus
