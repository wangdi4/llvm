//      Copyright  (C) 2010-2016 Intel Corporation.
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
//  Common use macros.
//
//  AUTHORS: Nikita Astafiev
//
//  CREATION DATE: 16-Feb-2010
//
//  MODIFICATION HISTORY:
//      10-Sep-2010, mc.h included. NA
//
//--


#ifndef LIBIML_ATTR_IML_ATTR_PRIVATE_H_INCLUDED
#define LIBIML_ATTR_IML_ATTR_PRIVATE_H_INCLUDED

#ifndef _WIN32
// Linux/MacOS/QNX
// Hidden visibility for any internal symbol which is shared inside the library
#define IMLATTR_INTERNAL_FUNC_VISIBILITY  __attribute__((visibility("hidden")))
#else
// Windows
#define IMLATTR_INTERNAL_FUNC_VISIBILITY
#endif

// In ICC, mc.h defines compile-time macros like LRB, AVX3, etc. However, we
// don't have this in LLVM and we must do run-time target checking instead.
// Since run-time checking is required, we cannot build a single table and
// reference it generically, but must select from one of several tables
// based on target.
//#include "mc.h"

#endif //LIBIML_ATTR_IML_ATTR_PRIVATE_H_INCLUDED
