//      Copyright  (C) 2009-2021 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
/* Begin of libiml_attr table */
// IA32 not yet supported by compile-time dispatcher
{
"empty_function", //const char * parent_function_name;
0, //int index;
c_precision_double, // PrecisionEnum  precision;
c_accuracy_reference, // AccuracyEnum  accuracy;
c_cpu_all, //SupportedCpusEnum  configuration;
0.6f, //float ulp_error;
1e30f, //float absolute_error;
52, //float accurate_bits;
0, //int domain_exclusion;
0, //int arch_consistency;
0, //int valid_status_bits;
0, //int use_svml_allowed;
0, //int zmm_low;
1, //int fusa;
    {
    "empty_function", //const char * cpu_specific_names[generic];
    "", //const char * cpu_specific_names[x87];
    "", //const char * cpu_specific_names[sse];
    "", //const char * cpu_specific_names[sse2];
    "", //const char * cpu_specific_names[sse3];
    "", //const char * cpu_specific_names[ssse3];
    "", //const char * cpu_specific_names[sse41];
    "", //const char * cpu_specific_names[sse42];
    "", //const char * cpu_specific_names[avx];
    "", //const char * cpu_specific_names[avx2];
    "", //const char * cpu_specific_names[avx512mic];
    "", //const char * cpu_specific_names[avx512core];
    "", //const char * cpu_specific_names[avx512zmmlow];
    },
},
/* End of libiml_attr table */
