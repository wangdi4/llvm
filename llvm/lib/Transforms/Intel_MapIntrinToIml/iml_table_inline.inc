//      Copyright  (C) 2009 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// cvs_id[] = "$Id: iml_library_description.c 242914 2013-11-05 06:47:55Z akolesov $"
//

//++
//  LIBM library description table.
//
//  AUTHORS: Nikita Astafiev, Andrey Kolesov
//
//  CREATION DATE: 26-Nov-2013
//
//  MODIFICATION HISTORY:
//      26-Nov-2013, initial version. AK
//      17-Apr-2014, Added __rcpf and __rcpd. SC
//      21-Dec-2018, New fusa attribute introduced. AK

    { "__cdivd",            "z", "false" },
    { "__cdivf",            "c", "false" },
    { "__divd",             "d", "false" },
    { "__divf",             "s", "false" },
    { "__divl",             "l", "false" },
    { "__rcpd",             "d", "false" },
    { "__rcpf",             "s", "false" },
    { "__svml_div2",        "d", "false" },
    { "__svml_div4",        "d", "false" },
    { "__svml_div8",        "d", "false" },
    { "__svml_div8_mask",   "d", "false" },
    { "__svml_divf16",      "s", "false" },
    { "__svml_divf16_mask", "s", "false" },
    { "__svml_rcp8",        "d", "false" },
    { "__svml_rcp8_mask",   "d", "false" },
    { "__svml_rcpf16",      "s", "false" },
    { "__svml_rcpf16_mask", "s", "false" },
