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
//  Helper macros to provide debug printout.
//
//  AUTHORS: Nikita Astafiev
//
//  CREATION DATE: 23-Sep-2009
//
//  MODIFICATION HISTORY:
//
//--

#ifndef LIBIML_ATTR_MESSAGING_H_INCLUDED
#define LIBIML_ATTR_MESSAGING_H_INCLUDED

#if defined IML_DEBUG
#include <stdio.h>
#define PRN_MSG(...)      printf(__VA_ARGS__)
#else
#define PRN_MSG(...)
#endif

#endif //LIBIML_ATTR_MESSAGING_H_INCLUDED
