/*
    Copyright 2005-2013 Intel Corporation.  All Rights Reserved.

    The source code contained or described herein and all documents related
    to the source code ("Material") are owned by Intel Corporation or its
    suppliers or licensors.  Title to the Material remains with Intel
    Corporation or its suppliers and licensors.  The Material is protected
    by worldwide copyright laws and treaty provisions.  No part of the
    Material may be used, copied, reproduced, modified, published, uploaded,
    posted, transmitted, distributed, or disclosed in any way without
    Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other
    intellectual property right is granted to or conferred upon you by
    disclosure or delivery of the Materials, either expressly, by
    implication, inducement, estoppel or otherwise.  Any license under such
    intellectual property rights must be express and approved by Intel in
    writing.
*/

#ifndef __TBB_uneven_split_H
#define __TBB_uneven_split_H

#ifdef __TBB_tbb_stddef_H
//#error Uneven_split.h must be included before any other tbb header. E.g. use -include compiler option
#endif

#define __TBB_UNEVEN 1

//#define split even_split
//#include <tbb/tbb_stddef.h>
//#undef split
namespace tbb {
namespace uneven {
class split { // redefine
public:
    int left, right;
    split(int l = 1, int r = 1) : left(l), right(r) {}
    float ratio() const { return float(left)/float(left+right); }
};

}}

//#ifndef __USE_TSCG__
//#include "blocked_range.h"
//#include "blocked_range2d.h"
//#include "blocked_range3d.h"
//#include "partitioner.h"
//#include "parallel_for.h"
//#endif
#endif //__TBB_uneven_split_H
