//      Copyright  (C) 2009-2019 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// cvs_id[] = "$Id: iml_accuracy_interface.c 14521 2018-10-04 22:17:06Z rcgilber $"
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
//      18-Feb-2010, Coding style changes. NA
//      06-Jul-2010, Changed powf into IML_ATTR_exp2f to remove external
//                   dependency from the math library. Added support for
//                   c_attribute_precision. NA
//      13-Jul-2010, Changed the way c_attribute_precision attribute value is
//                   treated in case of unsupported input.
//                   Changed c_attribute_relative_error handling in case
//                   c_accuracy_bitwise_reproducible already requested. NA
//      22-Apr-2011, Added may_i_use_inline_implementation interface extenstion
//                   for inlining. Added domain_exclusion attribute - currently
//                   is only supported in the may_i.
//                   Started rework of get_library_function_name. It should be
//                   implemented via may_i_use_inline_implementation to unify
//                   the function selection methods. Functions table should be
//                   reworked in order to store information on every function
//                   variant and to provide support to all attributes not only
//                   for inlined sequences, but also for library functions. NA
//      30-May-2011, Build warnings cleaned up. NA
//      27-Jun-2011, knf/knc configuration names added. Default value for domain
//                   exclusion is changed into -1. NA
//      20-Dec-2011, Integer functions accuracy initialization fixed in
//                   get_library... NA
//      20-Aug-2012, Addition of new IMF attribute - valid_status_bits - which
//                   means following to IEEE in setting FP flags.
//      19-Nov-2012, Disabling accuracy bits comparison in
//                   svmlMatchFuncDescriptions() (cq306561).
//      20-Mar-2013, 1) Intoduced USE_FUNCTION_DESCRIPTION_TABLE to use
//                   library description table in new full format.
//                   Currently disabled. 2) Interfaces of
//                   svmlGetFuncVariantsNum()
//                   and svmlGetFuncVariantsList() changed to take
//                   length of description table as an argument.
//                   3) PrecisionEnum extended to support char, short and
//                   long types.
//      25-Mar-2013, Fix for cq299935: changing "abs-error" to "absolute-error"
//                   to be in sync with driver. Nikita Astafiev
//      26-Nov-2013, New advanced SVML table usage enabled. Andrey Kolesov
//      29-Nov-2013, Check for integer funcs made both for
//                   LIBM and SVML (bugfix). Andrey Kolesov
//      04-Dec-2013, New SVML table disabled both for LRB and GFX. Andrey Kolesov
//      05-Dec-2013, OCL SVML build restored. Andrey Kolesov
//      16-Dec-2013, KNC new format table enabled. Andrey Kolesov
//      05-Dec-2014, Removed 32-64-bit conditional compilation as required
//                   by compiler process DPD200363179. SVML tables do not differ
//                   across 32-64 bits, so there's no point in maintaining two
//                   versions. Nikita Astafiev
//      05-Feb-2016, Fixed compiler warning in string pointer assignment.
//                   Nikita Astafiev
//      31-May-2016, precision=high now correctly translates to max-error=0.6.
//                   This currently only affects inline sequences. DPD200411603
//                   Nikita Astafiev
//      15-Mar-2017, Adding support for new attributes: use-svml, force-dynamic,
//                   isa-set. Making fixups for integer functions.
//                   Nikita Astafiev
//      16-Mar-2017, Moving tables definitions, requires addition of a symbol
//                   into exclusion list in dev/proton/npcg/pcglib_test.sym
//                   Nikita Astafiev
//      22-Mar-2017, _FEATURE_PCLMULQDQ is not needed to detect HSW, and it is not
//                   used in libsvml/libimf; plus the compiler is not
//                   auto-generating it under -xCORE_AVX2. I'm wiping it from
//                   here, otherwise the isa-set features set passed to the
//                   get_library_function_name under -xCORE_AVX2 is not getting
//                   mapped to HSW specific configuration. A similar change shall
//                   be done in LIBM/SVML CPU dispatchers. Nikita Astafiev
//      19-Jun-2017, strcat related KW warning fixed, CMPLRS-43705. Nikita Astafiev
//      19-Mar-2108, Adding recognition of zmm-low=true/false. CMPLRS-47343. NA
//
//--

#include "llvm/Transforms/Intel_MapIntrinToIml/iml_accuracy_interface.h"

#include <string.h>
#include <stdlib.h>
#include <cstdio>

#include "llvm/Transforms/Intel_MapIntrinToIml/search.h"
#include "llvm/Transforms/Intel_MapIntrinToIml/messaging.h"
#include "llvm/Transforms/Intel_MapIntrinToIml/iml_attr_private.h"

//  When USE_FUNCTION_DESCRIPTION_TABLE is defined library description table in full format
//  FunctionDescription_table is used. This table describes math library
//  using FunctionDescriptionType (see definition below)
//  In current version of FunctionDescription_table there is only cpu svml
//  functions description  automatically generated by btgen_SVML.pl
//  Need to be extended to describe the whole library.
//  Also it is needed to get rid of excessive fields in FunctionDescriptionType
//  like ulp_error and accurate_bits which are interchangeable.
//  When USE_FUNCTION_DESCRIPTION_TABLE the following libimlAttrC tests failed:
//  exp_max_error055, sin_max_error055, exp_arch_consistency_true, logf_combined
//
//

// #define USE_FUNCTION_DESCRIPTION_TABLE

#if defined __cplusplus
extern "C" {
#endif // __cplusplus

#if __GNUC__ >= 7
// The switch ladders in this file use implicit fallthrough for compactness.
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

#if 0
#if defined GFX
    #if (defined LRB) || (defined AVX3)
        #error "GFX and AVX3 | LRB defined simultaneously, cannot support this currently."
    #endif
#elif defined LRB
    #ifdef AVX3
        #error "LRB and AVX3 defined simultaneously, cannot support this currently."
    #endif // AVX3
#endif // LRB
#endif

// This should be equal to
// SUPPORTED_ACCURACIES_NUMBER*SUPPORTED_CPUS_NUMBER
#define SUPPORTED_VARIANTS_NUMBER             4


// Macros to simplify navigation in libm_description_table
#define LIBM_FUNC_NAME_STR(function_id)             libm_description_table[function_id][0]
#define LIBM_FUNC_PRECISION_STR(function_id)        libm_description_table[function_id][1]
#define LIBM_FUNC_VARIANT_STR(function_id,variant)  libm_description_table[(function_id)][2+(variant)]

#define INLINE_FUNC_NAME_STR(function_id)           inline_description_table[function_id][0]
#define INLINE_FUNC_PRECISION_STR(function_id)      inline_description_table[function_id][1]

//
// Defines precision levels:
//   low                        : not greater than 4 ulps
//   ep - enhanced performance  : not greater than 2^12 for single or 2^26 for double ulps
//   bitwise_reproducible       : not greater than 0.6 ulps for all CPU configurations
//   high                       : not greater than 0.6 ulps
//   cr - correctly rounded     : 0.5 ulps
//
// note: update in sync with the libm_description_table
//
typedef enum
{
    c_accuracy_low         =  0,
    c_accuracy_ep,
    c_accuracy_bitwise_reproducible,
    c_accuracy_high,
    c_accuracy_cr,
    SUPPORTED_ACCURACIES_NUMBER
} AccuracyEnum;

#if 0
static const char* valid_accuracy_names[] =
{
    "la", "ep", "br", "ha", "cr"
};
#endif

// note: here we maintain ordered list of accuracies, for ease of
// compares:
// accuracy_vector[c_accuracy_ep] < accuracy_vector[c_accuracy_high]
// this is done for the sole purpose of not re-arranging the library
// description table. Should be removed in the future.
static const int accuracy_vector[SUPPORTED_ACCURACIES_NUMBER] =
    {1, 0, 3, 2, 4};

typedef enum
{
    c_precision_unsupported     = -1,
    c_precision_single          =  0,
    c_precision_double,
    c_precision_long_double,
    c_precision_quad,
    c_precision_signed_char,
    c_precision_unsigned_char,
    c_precision_signed_short,
    c_precision_unsigned_short,
    c_precision_signed_int,
    c_precision_unsigned_int,
    c_precision_signed_longlong,
    c_precision_unsigned_longlong,
    c_precision_single_complex,
    c_precision_double_complex,
    c_precision_long_double_complex,
    c_precision_quad_complex
} PrecisionEnum;

#if defined IML_DEBUG
static const char* valid_precision_names[] =
{
    "unsupported",
    "single",
    "double",
    "long_double",
    "quad",
    "signed_char",
    "unsigned_char",
    "signed_short",
    "unsigned_short",
    "signed_int",
    "unsigned_int",
    "signed_longlong",
    "unsigned_longlong",
    "single_complex",
    "double_complex",
    "long_double_complex",
    "quad_complex"
};
#endif

// This list contains all currently supported
// configurations in text and enumerations representations.
// Note: this list must be in sync with the valid_configurations_names list
typedef enum
{
    c_cpu_unsupported = -2,
    c_cpu_force_dynamic = -1,
    c_cpu_all = 0, //dispatcher routine
    c_cpu_x87 = 1, // Not to be used with SVML
    c_cpu_sse = 2, // Not to be used with DP SVML
    c_cpu_sse2,
    c_cpu_sse3,
    c_cpu_ssse3,
    c_cpu_sse41,
    c_cpu_sse42,
    c_cpu_avx,
    c_cpu_avx2,
    c_cpu_micavx512,
    c_cpu_coreavx512,
    c_cpu_coreavx512zmmlow,
    SUPPORTED_CPUS_NUMBER
} SupportedCpusEnum;

#if defined IML_DEBUG
// Note: this list is assumed unsorted
static const char* valid_configurations_names[SUPPORTED_CPUS_NUMBER] =
{
    "all", "x87", "sse", "sse2", "sse3", "ssse3", "sse41", "sse42", "avx", "avx2", "micavx512", "coreavx512", "coreavx512zmmlow"
};

static const char* attrGetConfigName(SupportedCpusEnum c)
{
    if(c == c_cpu_unsupported)
        return "unsupported";
    if(c == c_cpu_force_dynamic)
        return valid_configurations_names[c_cpu_all];
    return valid_configurations_names[c];
}
#endif

//TODO: replace bits from immintrin.h into something else
#include <immintrin.h>

#if 0
//TODO: support cpu features from LLVM in this interface instead of using immintrin.h
static SupportedCpusEnum attrLibFeatureFlagToTTABIndex(unsigned long long features)
{
    SupportedCpusEnum new_value;

    #define LIBIML_ATTR_COREAVX512                  (_FEATURE_AVX512F  | \
                                                     _FEATURE_AVX512CD | \
                                                     _FEATURE_AVX512DQ | \
                                                     _FEATURE_AVX512BW | \
                                                     _FEATURE_AVX512VL )

    #define LIBIML_ATTR_MICAVX512                   (_FEATURE_AVX512F  | \
                                                     _FEATURE_AVX512ER | \
                                                     _FEATURE_AVX512PF | \
                                                     _FEATURE_AVX512CD )

    #define LIBIML_ATTR_AVX2                        (_FEATURE_FMA       | \
                                                     _FEATURE_LZCNT     | \
                                                     _FEATURE_BMI       | \
                                                     _FEATURE_AVX2      )

    if ( (LIBIML_ATTR_COREAVX512 & features) == LIBIML_ATTR_COREAVX512) {
        new_value = c_cpu_coreavx512;
    }
    else if ( (LIBIML_ATTR_MICAVX512 & features) == LIBIML_ATTR_MICAVX512) {
        new_value = c_cpu_micavx512;
    }
    else if ( (LIBIML_ATTR_AVX2 & features) == LIBIML_ATTR_AVX2) {
        new_value = c_cpu_avx2;
    }
    else if ( (_FEATURE_AVX & features) == _FEATURE_AVX) {
        new_value = c_cpu_avx;
    }
    else if ( (_FEATURE_SSE4_2 & features) == _FEATURE_SSE4_2) {
        new_value = c_cpu_sse42;
    }
    else if ( (_FEATURE_SSE4_1 & features) == _FEATURE_SSE4_1) {
        new_value = c_cpu_sse41;
    }
    else if ( (_FEATURE_SSSE3 & features) == _FEATURE_SSSE3) {
        new_value = c_cpu_ssse3;
    }
    else if ( (_FEATURE_SSE3 & features) == _FEATURE_SSE3) {
        new_value = c_cpu_sse3;
    }
    else if ( (_FEATURE_SSE2 & features) == _FEATURE_SSE2) {
        new_value = c_cpu_sse2;
    }
    else if ( (_FEATURE_SSE & features) == _FEATURE_SSE) {
        new_value = c_cpu_sse;
    }
    else if ( (_FEATURE_FPU & features) == _FEATURE_FPU) {
        new_value = c_cpu_x87;
    }
    else if (features == 0) {
        new_value = c_cpu_all;
    }
    else
    {
        new_value = c_cpu_unsupported;
    }

    PRN_MSG("%-32s: \tconverting int64 feature flag %08X%08X into integer index %d meaning %s\n","attrLibFeatureFlagToTTABIndex", *(1+(int *)(&features)), *(0+(int *)(&features)), new_value, attrGetConfigName(new_value));

    return new_value;
}
#endif

// This structure contains all possible information
// on the function. It replaces text specification with
// the enumerated descriptors.
typedef struct
{
    const char * parent_function_name;
    // function position in libm_description_table
    int index;

    // Here are the attributes of function itself:
    // e.g. sin is the first function in the library,
    // it has double precision.

    // function precision
    PrecisionEnum  precision;

    // Below are dynamic attributes: the ones that
    // may vary. E.g. one may request a certain function
    // but in different accuracy flavors, or optimized
    // for different processor (e.g. configuration).

    // requested accuracy
    AccuracyEnum  accuracy;
    // requested configuration
    SupportedCpusEnum configuration;
    // requested relative error
    float ulp_error;
    // requested absolute error
    float absolute_error;
    // requested accurate bits
    float accurate_bits;
    // flag 1 if restricted domain requested, 0 otherwise
    int domain_exclusion;
    // flag 1 if arch consistency requested, 0 otherwise
    int arch_consistency;
    // flag 1 if correcly set status bits requested, 0 otherwise
    int valid_status_bits;
    // flag 1 if function may be called under -fimf-use-svml=true
    int use_svml_allowed;
    // flag 1 if -qopt-zmm-usage=low requested, 0 otherwize
    int zmm_low;

    // array of pointers to constant names of cpu-specific functions
    const char * cpu_specific_names[SUPPORTED_CPUS_NUMBER];

    // flag 1 if GLOB_fusa_math is requested, 0 otherwize
    // TODO: populate table with the right values, now they are not
    // given in the initializers lists and such this always gets 0.
    int fusa;

} FunctionDescriptionType;

// This list contains all currently supported
// attributes in text and enumerations representations.
// Note: this list must be in sync with the valid_attributes_names list
typedef enum
{
    c_attribute_absolute_error = 0,
    c_attribute_accurate_bits,
    c_attribute_accurate_bits_128,
    c_attribute_accurate_bits_32,
    c_attribute_accurate_bits_64,
    c_attribute_accurate_bits_80,
    c_attribute_bitwise_consistency,
    c_attribute_domain_exclusion,
    c_attribute_force_dynamic,
    c_attribute_fusa,
    c_attribute_configuration,
    c_attribute_relative_error,
    c_attribute_precision,
    c_attribute_use_svml,
    c_attribute_valid_status_bits,
    c_attribute_zmm_low,
    SUPPORTED_ATTRIBUTES_NUMBER
} FunctionAttributeEnum;

// Note: this list must be !sorted!
static const char* valid_attributes_names[SUPPORTED_ATTRIBUTES_NUMBER] =
{
    "absolute-error",       // 0
    "accuracy-bits",        // 1
    "accuracy-bits-128",    // 2
    "accuracy-bits-32",     // 3
    "accuracy-bits-64",     // 4
    "accuracy-bits-80",     // 5
    "arch-consistency",     // 6
    "domain-exclusion",     // 7
    "force-dynamic",        // 8
    "fusa",                 // 9
    "isa-set",              // 10
    "max-error",            // 11
    "precision",            // 12
    "use-svml",             // 13
    "valid-status-bits",    // 14
    "zmm-low"               // 15
};

typedef union
{
    float  f;
    double d;
    int    i;
    long   l;
    const char* s;
    void* p;
} AllTypesUnion;

typedef struct
{
    FunctionAttributeEnum   attribute_name;
    AllTypesUnion           attribute_value;
} FunctionAttributeType;


#define _IML_ATTR_SIZEOF_TABLE(__table)  (int)(sizeof(__table)/sizeof(__table[0]))


//  LIBM library description table in short format
static const char* libm_description_table[][2+SUPPORTED_VARIANTS_NUMBER] =
{
#if (defined IML_OCLSVML_BUILD)
  #include "iml_table_ocl.inc"
#else
  #include "iml_table_libm.inc"
#endif
};
static const int libm_functions_num = _IML_ATTR_SIZEOF_TABLE(libm_description_table);

//  INLINE library description table in short format
static const char* inline_description_table[][2] =
{
#include "iml_table_inline.inc"
};
static const int inline_functions_num = _IML_ATTR_SIZEOF_TABLE(inline_description_table);

// exp2f implementation included here to avoid dependency on math library
#include "llvm/Transforms/Intel_MapIntrinToIml/iml_exp2f.h"


#if 0
static const char* attrGetAccuracyName(AccuracyEnum a)
{
    return valid_accuracy_names[a];
}
#endif

// This function converts a floating point quantity
// representing a number of correct mantissa bits into
// the error estimate measured in ulps. Mantissa width,
// needed to perform this conversion correctly, is derived
// based on precision parameter.
static float attrBits2Ulps(float bits, PrecisionEnum precision)
{
    float p, ulps;
    switch(precision)
    {
        case c_precision_single:
        case c_precision_single_complex:
            p = 24.0f;
            break;
        case c_precision_double:
        case c_precision_double_complex:
            p = 53.0f;
            break;
        case c_precision_long_double:
        case c_precision_long_double_complex:
            p = 64.0f;
            break;
        case c_precision_quad:
        case c_precision_quad_complex:
            p = 113.0f;
            break;
        default: // nonsense
            PRN_MSG("%-32s: [warning] for given precision ulps are meaningless\n", "attrBits2Ulps");
            return 0.0;
    }
    // b = p - 1 - log2(u)
    ulps = IML_ATTR_exp2f(p - 1.0f - bits);
    return ulps;
}


// This function translates the precision string
// into precision enumeration based on the first letter
static PrecisionEnum attrMapPrecisionStr2Enum(const char* string_precision)
{
    if (string_precision[0] == 's')
    {
        return c_precision_single;
    }
    if (string_precision[0] == 'd')
    {
        return c_precision_double;
    }
    if (string_precision[0] == 'c')
    {
        return c_precision_single_complex;
    }
    if (string_precision[0] == 'z')
    {
        return c_precision_double_complex;
    }
    if (string_precision[0] == 'i')
    {
        return c_precision_signed_int;
    }
    if (string_precision[0] == 'u')
    {
        return c_precision_unsigned_int;
    }
    if (string_precision[0] == 'l')
    {
        return c_precision_long_double;
    }
    if (string_precision[0] == 'q')
    {
        return c_precision_quad;
    }
    if (string_precision[0] == 'x')
    {
        return c_precision_long_double_complex;
    }
    if (string_precision[0] == 'w')
    {
        return c_precision_quad_complex;
    }

    return c_precision_unsupported;
}

// This function inits the function_description structure
static int attrInitFuncDescription(
                FunctionDescriptionType* function_description)
{
    int i;
    if (function_description == NULL)
    {
        PRN_MSG("%-32s: [ERROR] in attrInitFuncDescription: passed NULL pointer\n","attrInitFuncDescription");
        return -1;
    }

    function_description->parent_function_name = NULL;
    function_description->index = -1;
    function_description->precision = c_precision_unsupported;
    function_description->accuracy = c_accuracy_cr;
    function_description->configuration = c_cpu_all;
    function_description->ulp_error = -3.14f;
    function_description->accurate_bits  = -1.57f;
    function_description->absolute_error = -6.28f;
    function_description->domain_exclusion = -1;
    function_description->arch_consistency = 0;
    function_description->valid_status_bits = 0;
    function_description->use_svml_allowed = 0;
    function_description->zmm_low = 0;
    function_description->fusa = 0;

    for (i = 0; i < SUPPORTED_CPUS_NUMBER; i++)
    {
        function_description->cpu_specific_names[i] = NULL;
    }

    return 0;
}

// This function re-initializes description structure
// for the integer function. The intention is to
// remove irrelevant FP attributes and let the function
// pass through any check on FP side.
static int attrFixupIntegerFuncDescription(
                FunctionDescriptionType* function_description)
{
    if (function_description == NULL)
    {
        PRN_MSG("%-32s: [ERROR]: received NULL pointer\n","attrFixupIntegerFuncDescription");
        return -1;
    }

    function_description->accuracy = c_accuracy_cr;
    function_description->ulp_error = 0.5;
    function_description->accurate_bits  = 53;
    function_description->absolute_error = 1e30f;
    function_description->domain_exclusion = 0;
    function_description->arch_consistency = 1;

    return 0;
}


// This function converts a pair of strings attribute name-value
// into internal attribute structure representation.
static int attrExternal2InternalAttr(
                FunctionAttributeType* internal_attribute,
                const ImfAttr*         external_attribute)
{
    int flag = 0;
    const char* aname = NULL;
    (void)aname;
    int attr_chosen = 0;
    unsigned long long int tmp;

    // search internal_attribute name within allowed names
    flag = IML_ATTR_get_name_index(external_attribute->name, valid_attributes_names, SUPPORTED_ATTRIBUTES_NUMBER);
    if (flag < 0)
    {
      PRN_MSG("%-32s: [ERROR] internal_attribute name \"%s\" wasn't found \n", "attrExternal2InternalAttr", external_attribute->name);
    }

    internal_attribute->attribute_name = (FunctionAttributeEnum)flag;

    switch(internal_attribute->attribute_name)
    {
        case c_attribute_configuration: aname = "c_attribute_configuration";
    #if (defined LIBIML_ATTR_IN_WIN)
            sscanf(external_attribute->value, "%I64u", &tmp);
    #else
            sscanf(external_attribute->value, "%llu", &tmp);
    #endif
            //flag = attrLibFeatureFlagToTTABIndex(tmp);
            flag = 0; //TODO: support cpu feature lookup so that target specific flags can be set.
            if (flag < 0)
            {
              PRN_MSG("%-32s: [failure] configuration value \"%s\" wasn't properly converted\n", "attrExternal2InternalAttr", external_attribute->value);
            }

            internal_attribute->attribute_value.i = flag;
            PRN_MSG("%-32s: \tattribute %s = %d \n", "attrExternal2InternalAttr", aname, internal_attribute->attribute_value.i);
            break;

        case c_attribute_accurate_bits_32:  aname = "c_attribute_accurate_bits_32"; attr_chosen = 1;
        case c_attribute_accurate_bits_64:  if(!attr_chosen){aname = "c_attribute_accurate_bits_64"; attr_chosen = 1;}
        case c_attribute_accurate_bits_80:  if(!attr_chosen){aname = "c_attribute_accurate_bits_80"; attr_chosen = 1;}
        case c_attribute_accurate_bits_128: if(!attr_chosen){aname = "c_attribute_accurate_bits_128"; attr_chosen = 1;}
        case c_attribute_accurate_bits:     if(!attr_chosen){aname = "c_attribute_accurate_bits"; attr_chosen = 1;}
        case c_attribute_absolute_error:    if(!attr_chosen){aname = "c_attribute_absolute_error"; attr_chosen = 1;}
        case c_attribute_relative_error:    if(!attr_chosen){aname = "c_attribute_relative_error"; attr_chosen = 1;}

            internal_attribute->attribute_value.f = (float)atof(external_attribute->value);
            PRN_MSG("%-32s: \tattribute %s = %g \n", "attrExternal2InternalAttr", aname, internal_attribute->attribute_value.f);
            break;

        case c_attribute_domain_exclusion:   aname = "c_attribute_domain_exclusion";
            internal_attribute->attribute_value.i = atoi(external_attribute->value);
            PRN_MSG("%-32s: \tattribute %s = %d \n", "attrExternal2InternalAttr", aname, internal_attribute->attribute_value.i);
            break;

        case c_attribute_bitwise_consistency:  aname =  "c_attribute_bitwise_consistency"; attr_chosen = 1;
        case c_attribute_force_dynamic:        if(!attr_chosen){aname =  "c_attribute_force_dynamic"; attr_chosen = 1;}
        case c_attribute_use_svml:             if(!attr_chosen){aname =  "c_attribute_use_svml"; attr_chosen = 1;}
        case c_attribute_valid_status_bits:    if(!attr_chosen){aname =  "c_attribute_valid_status_bits"; attr_chosen = 1;}
        case c_attribute_zmm_low:              if(!attr_chosen){aname =  "c_attribute_zmm_low"; attr_chosen = 1;}
        case c_attribute_fusa:                 if(!attr_chosen){aname =  "c_attribute_fusa"; attr_chosen = 1;}
            internal_attribute->attribute_value.i = (!strcmp(external_attribute->value,"true"))?1:0; //assuming false even if garbage input
            PRN_MSG("%-32s: \tattribute %s = %d \n", "attrExternal2InternalAttr", aname, internal_attribute->attribute_value.i);
            break;

        case c_attribute_precision:
            internal_attribute->attribute_value.s = external_attribute->value;
            PRN_MSG("%-32s: \tattribute c_attribute_precision = \"%s\" \n", "attrExternal2InternalAttr", internal_attribute->attribute_value.s);
            break;

        default: // not supported internal_attribute
            break;
    }

    return 0;
}

// This function checks which attribute is being set and
// what kind of value is being assigned to it.
// Function fills in the function_description structure accordingly.
static int attrUpdateFuncDescription(
                FunctionDescriptionType* function_description,
                FunctionAttributeType*   internal_attribute)
{
    float error_value;
    float HA_THRESHOLD, LA_THRESHOLD, EP_THRESHOLD;

    union u_error_value {
      unsigned u;
      float f;
    } t_error_value;

    // store function precision
    PrecisionEnum working_precision = function_description->precision;

    // store current internal_attribute name
    FunctionAttributeEnum current_attribute_name = internal_attribute->attribute_name;

    // set the relative error thresholds
    // 0.6 is not exactly representable, so to avoid discrepancies
    // we convert it to floating point using exactly the same mechanism
    // as the max-error attribute: i.e. using atof() library routine.
    HA_THRESHOLD = (float)atof("0.6");
    LA_THRESHOLD = 4.0f;
    // c_accuracy_ep gives 11 and 26 correct bits for
    // single/double precisions. Other precisions do not
    // have this mode, so we place 5.0ulp just to have more than 4.0 LA
    switch(working_precision)
    {
        case c_precision_single:
            EP_THRESHOLD = attrBits2Ulps(11, working_precision);
            break;
        case c_precision_double:
            EP_THRESHOLD = attrBits2Ulps(26, working_precision);
            break;
        default:
            EP_THRESHOLD = 5.0f;
    }

    // precision internal_attribute is different and is processed separately
    if (current_attribute_name == c_attribute_precision)
    {
        if (!strcmp(internal_attribute->attribute_value.s,"high"))
        {
            internal_attribute->attribute_value.f = HA_THRESHOLD;
        }
        else if (!strcmp(internal_attribute->attribute_value.s,"medium"))
        {
            internal_attribute->attribute_value.f = LA_THRESHOLD;
        }
        else if (!strcmp(internal_attribute->attribute_value.s,"low"))
        {
            internal_attribute->attribute_value.f = EP_THRESHOLD;
            // low precision setting also must cause restricted domain
            // this is not documented. commenting out.
            // function_description->domain_exclusion = -1;
        }
        else  //unsupported input
        {
            PRN_MSG("%-32s: [warning] internal_attribute c_attribute_precision value: \"%s\" is unsupported and ignored\n", "attrUpdateFuncDescription", internal_attribute->attribute_value.s);
            return 0;
        }

        // proceed with general scheme for relative error processing
        current_attribute_name = c_attribute_relative_error;
    }

    // search internal_attribute value within allowed for a particular attribute_name
    switch(current_attribute_name)
    {
        case c_attribute_force_dynamic:
            if ( (internal_attribute->attribute_value.i == 0) && (function_description->configuration == c_cpu_force_dynamic) )
            {
                // if previously recorder force_dynamic=true is now
                // cancelled by this request, then we reset the configuration
                // to CPU-dispatcher
                function_description->configuration = c_cpu_all;
            }
            else if (internal_attribute->attribute_value.i == 1)
            {
                // record the force_dynamic=true
                function_description->configuration = c_cpu_force_dynamic;
            }
            else
            {
                // if no force_dynamic=true was previously recorded,
                // then this force_dynamic=false has no effect
            }
            break;

        case c_attribute_configuration:
            // Guard against changing target CPU in case
            // c_attribute_force_dynamic was previously
            // set to 1.
            if (function_description->configuration == c_cpu_force_dynamic)
            {
                PRN_MSG("%-32s: CPU change is ignored due to previous force dynamic request\n","attrUpdateFuncDescription");
                break;
            }

            // check if requested configuration is supported
            // and update function_description accordingly.
            if ( (internal_attribute->attribute_value.i >=
                    SUPPORTED_CPUS_NUMBER) ||
                 (internal_attribute->attribute_value.i < c_cpu_all ))
            {
                // if requested configuration is invalid
                // TODO: may want to change the response in case of
                //       invalid compiler input
                // function_description->configuration = c_cpu_unsupported;
                PRN_MSG("%-32s: unsupported CPU request was changed into a dispatcher request\n","attrUpdateFuncDescription");
                function_description->configuration = c_cpu_all;
            }
            else
            {
                function_description->configuration = (SupportedCpusEnum)internal_attribute->attribute_value.i;
            }
            break;

        case c_attribute_accurate_bits_32:
        case c_attribute_accurate_bits_64:
        case c_attribute_accurate_bits_80:
        case c_attribute_accurate_bits_128:
            if (
                // guard against precision mismatch
                ((current_attribute_name == c_attribute_accurate_bits_32) &&
                (working_precision != c_precision_single) &&
                (working_precision != c_precision_single_complex))
                ||
                ((current_attribute_name == c_attribute_accurate_bits_64) &&
                (working_precision != c_precision_double) &&
                (working_precision != c_precision_double_complex))
                ||
                ((current_attribute_name == c_attribute_accurate_bits_80) &&
                (working_precision != c_precision_long_double) &&
                (working_precision != c_precision_long_double_complex))
                ||
                ((current_attribute_name == c_attribute_accurate_bits_128) &&
                (working_precision != c_precision_quad) &&
                (working_precision != c_precision_quad_complex)))
            {
                PRN_MSG("%-32s: [warning] internal_attribute ignored due to function precision mismatch\n","attrUpdateFuncDescription");
                break;
            }


        case c_attribute_accurate_bits: // proceed with general scheme for accurate bits
            // store number of accurate bits for future processing
            function_description->accurate_bits = internal_attribute->attribute_value.f;

            // Note the name change: we convert internal_attribute from
            // bits to relative error.
            internal_attribute->attribute_name = c_attribute_relative_error;
            // convert bits into relative error
            internal_attribute->attribute_value.f = attrBits2Ulps( internal_attribute->attribute_value.f, working_precision );
            PRN_MSG("%-32s: %.1g accurate bits converted to %g ulp value\n","attrUpdateFuncDescription",function_description->accurate_bits,internal_attribute->attribute_value.f);

        case c_attribute_relative_error: // proceed with relative error
            // Guard against changing accuracy in case
            // c_attribute_bitwise_consistency was previously
            // set to 1.
            if (function_description->arch_consistency == 1)
            {
                PRN_MSG("%-32s: requested accuracy change is ignored due to previous bitwise consistency request\n","attrUpdateFuncDescription");
                break;
            }

            // based on requested relative error and function
            // precision we decide which accuracy flavor
            // is appropriate

            error_value = internal_attribute->attribute_value.f;

            t_error_value.f = error_value;
            // isnan check implemented to avoid 3-rd party dependency
            #define IML_ATTR_ISNANF(x)   ( (((*((unsigned int *)&(x))) & 0x7fffffff) > 0x7f800000) ? 1 : 0 )

            if ( IML_ATTR_ISNANF(t_error_value.u) || (error_value < 0.0f) )
            {
                // NaN value or negative
                PRN_MSG("%-32s: [ERROR] requested accuracy is either NaN or negative\n","attrUpdateFuncDescription");
                break;
            }

            // store requested relative error value for possible future
            // processing
            function_description->ulp_error = error_value;

            // classify requested relative error according to available
            // HA/LA/EP variations
            if ( (error_value >= 0.0f) && (error_value < HA_THRESHOLD) )
            {
                // correctly rounded
                PRN_MSG("%-32s: \t[warning] requested "
                        "%f ulp accuracy which may be not possible, "
                        "selecting c_accuracy_cr instead\n", "attrUpdateFuncDescription",
                        error_value);
                function_description->accuracy = c_accuracy_cr;
            }
            else
            if (error_value >= HA_THRESHOLD && error_value < LA_THRESHOLD)
            {
                // high accuracy
                function_description->accuracy = c_accuracy_high;
            }
            else
            if ( (error_value >= LA_THRESHOLD) && (error_value < EP_THRESHOLD) )
            {
                // low accuracy
                function_description->accuracy = c_accuracy_low;
            }
            else
            if ( error_value >= EP_THRESHOLD )
            {
                // very low accuracy
                function_description->accuracy = c_accuracy_ep;
            }
            else
            {
                // should not get here
                PRN_MSG("%-32s: [ERROR] requested %f ulp accuracy not detected\n", "attrUpdateFuncDescription", error_value);
            }
            break;

        case c_attribute_bitwise_consistency:
            function_description->arch_consistency = internal_attribute->attribute_value.i;

            if (internal_attribute->attribute_value.i == 1)
            {
                function_description->accuracy =
                        c_accuracy_bitwise_reproducible;
            }
            break;

        case c_attribute_domain_exclusion:
            function_description->domain_exclusion = internal_attribute->attribute_value.i;
            break;

        case c_attribute_valid_status_bits:
            function_description->valid_status_bits =
                internal_attribute->attribute_value.i;
            break;

        case c_attribute_absolute_error:
            // Guard against changing accuracy in case
            // c_attribute_bitwise_consistency was previously
            // set to 1.
            if (function_description->arch_consistency == 1)
            {
                PRN_MSG("%-32s: requested accuracy change is ignored due to previous bitwise consistency request\n","attrUpdateFuncDescription");
                break;
            }

            error_value = internal_attribute->attribute_value.f;
            // store requested absolute error value for possible future
            // processing. here we ignore this internal_attribute
            function_description->absolute_error = error_value;

        case c_attribute_use_svml:
            function_description->use_svml_allowed = internal_attribute->attribute_value.i;

            break;

        case c_attribute_zmm_low:
            function_description->zmm_low = internal_attribute->attribute_value.i;

            break;

        case c_attribute_fusa:
            function_description->fusa = internal_attribute->attribute_value.i;
            break;

        default: // not supported internal_attribute
            PRN_MSG("%-32s: [ERROR] internal_attribute not supported\n","attrUpdateFuncDescription");
            break;
    }

    return 0;
}



// This function accesses libm_description_table and based on input
// function ID returns pointer to a string containing function precision
static const char* inlineGetFuncPrecisionString(int function_id)
{
    return INLINE_FUNC_PRECISION_STR(function_id);
}

// This function looks up the function precision in the
// libm_description_table and translates it into enumeration
static PrecisionEnum inlineGetFuncPrecision(int function_id)
{
    const char* string_precision = inlineGetFuncPrecisionString(function_id);
    return attrMapPrecisionStr2Enum(string_precision);
}


// this function performs a binary search over the column of strings
// and returns the index if the value is found
static int inlineGetNameIndex(const char* name)
{
    int l, r, s, direction;
    if (!strcmp(name, ""))
    {
        PRN_MSG("%-32s: [ERROR] empty search token\n","inlineGetNameIndex");
        return -1;
    }
    l = 0;
    r = inline_functions_num - 1;
    while ( (r - l) > 1 )
    {
        s = (l + r) / 2;
        direction = strcmp(name, INLINE_FUNC_NAME_STR(s));
        if (direction < 0){r = s; }
        else if (direction > 0) { l = s; }
        else if (direction == 0){ return s; }
    }
    if (!strcmp(name, INLINE_FUNC_NAME_STR(l))) { return l; }
    if (!strcmp(name, INLINE_FUNC_NAME_STR(r))) { return r; }
    return -3;
}




// This function accesses libm_description_table and based on input
// function ID returns pointer to a string containing function precision
static const char* libmGetFuncPrecisionString(int function_id)
{
    return LIBM_FUNC_PRECISION_STR(function_id);
}

// this function performs a binary search over the !sorted! column of strings
// and returns the index if the value is found
static int libmGetNameIndex(const char* name)
{
    int l, r, s, direction;

    if (!strcmp(name, ""))
    {
        PRN_MSG("%-32s: [ERROR] empty search token\n","libmGetNameIndex");
        return -1;
    }

#if defined IML_DEBUG
    {
        // user should not suffer the penalty of this extra check
        for (s = 0; s < libm_functions_num - 1; s++)
        {
            if (strcmp(LIBM_FUNC_NAME_STR(s), LIBM_FUNC_NAME_STR(s+1)) >= 0)
            {
                PRN_MSG("%-32s: [ERROR] search space unsorted\n","libmGetNameIndex");
                PRN_MSG("%-32s: [ERROR] %s >= %s\n",  "libmGetNameIndex", LIBM_FUNC_NAME_STR(s), LIBM_FUNC_NAME_STR(s+1));
                return -2;
            }
        }
    }
#endif

    l = 0; r = libm_functions_num - 1;
    while ( (r - l) > 1 )
    {
        s = (l + r) / 2;
        direction = strcmp(name, LIBM_FUNC_NAME_STR(s));
        if (direction < 0) { r = s; }
        else if (direction > 0) { l = s; }
        else if (direction == 0) { return s; }
    }

    if (!strcmp(name, LIBM_FUNC_NAME_STR(l))) { return l; }
    if (!strcmp(name, LIBM_FUNC_NAME_STR(r))) { return r; }

    //TODO: maybe insert unsorted search: just in case...
    PRN_MSG( "%-32s: [failure] token \"%s\" not found\n", "libmGetNameIndex", name );
    return -3;
}


// This function accesses libm_description_table and based on input
// function ID and function variant number returns pointer to a string
// containing appropriate function variant name
static const char* libmGetVariantName(int function_id, int variant)
{
    const char* pstr = NULL;

    // Guard against wrong queries
    if ( (function_id < 0 || function_id >= libm_functions_num) || (variant < 0 || variant >= SUPPORTED_VARIANTS_NUMBER) )
    {
        PRN_MSG("%-32s: [ERROR] invalid query (id = %d, variant = %d)\n","libmGetVariantName",function_id,variant);
        return NULL;
    }

    pstr = LIBM_FUNC_VARIANT_STR(function_id, variant);

    // If the variant string is empty, return NULL pointer
    if (!strcmp(pstr, ""))
    {
        return NULL;
    }

    return pstr;
}


// This function looks up the function precision in the
// libm_description_table and translates it into enumeration
static PrecisionEnum libmGetFuncPrecision(int function_id)
{
    const char* string_precision = libmGetFuncPrecisionString(function_id);
    return attrMapPrecisionStr2Enum(string_precision);
}



static int svmlGetFuncVariantsNum(
                            const char* func_base_name,
                            const FunctionDescriptionType* functions_list,
                            int functions_list_length,
                            int* length)
{
    int i, j;
    *length = -1;

    if (0 == strcmp(func_base_name, ""))
    {
        PRN_MSG("%-32s: [ERROR] empty search token\n","svmlGetFuncVariantsNum");
        return -1;
    }

    for (i = 0; i < functions_list_length; i++)
    {
        if (0 == strcmp(func_base_name, functions_list[i].parent_function_name))
        {
            for (j = i+1; j < functions_list_length; j++)
            {
                if (0 != strcmp(func_base_name, functions_list[j].parent_function_name))
                { // functions_list[j] has different base name
                    break;
                }
            }
            *length = j - i;
            PRN_MSG("%-32s: Base name found at position = %d, number of variants = %d\n", "svmlGetFuncVariantsNum", i, *length);
            return i;
        }
    }

    PRN_MSG("%-32s: [failure] token \"%s\" not found\n", "svmlGetFuncVariantsNum", func_base_name);
    return -2;
}


static int svmlGetFuncVariantsList(FunctionDescriptionType** functions_list,
                                        const char* func_base_name,
                                        const FunctionDescriptionType* functions_table,
                                        int functions_table_length)
{
    int start = -1;
    int length = -1;

    start = svmlGetFuncVariantsNum(func_base_name, functions_table, functions_table_length, &length);
    if ((start >=0) && (length >0))
    {
        *functions_list = const_cast<FunctionDescriptionType*>(functions_table) + start;
        return length;
    }

    PRN_MSG("%-32s: [failure] token \"%s\" not found\n", "svmlGetFuncVariantsList", func_base_name);
    return -1;
}



// This function returns 1 or 0 meaning "yes" or "no". It answers the question:
// whether the compiler may use an instructions sequence with certain
// properties described by function_description_compiler
// under conditions specified by function_description_user.
static int svmlMatchFuncDescriptions(
                    const FunctionDescriptionType function_description_user,
                    const FunctionDescriptionType function_description_compiler)
{
    // Here we will walk over all function description fields and
    // do the matching.

    // this is dummy for now
    PRN_MSG("%-32s: \tconfiguration check : requested \"%s\", suggested \"%s\": ","svmlMatchFuncDescriptions",\
            attrGetConfigName(function_description_user.configuration), attrGetConfigName(function_description_compiler.configuration));

    if ( (function_description_user.configuration <= c_cpu_unsupported)     ||
         (function_description_user.configuration >= SUPPORTED_CPUS_NUMBER) ||
         (function_description_compiler.configuration <= c_cpu_unsupported) ||
         (function_description_compiler.configuration >= SUPPORTED_CPUS_NUMBER)
        )
    {
        PRN_MSG("[failure]\n");
        return 0;
    }
    else
    {
        PRN_MSG("[success]\n");
    }


#if 0
    PRN_MSG("%-32s: \taccuracy check : requested \"%s\" (%d), suggested \"%s\" (%d): ","svmlMatchFuncDescriptions",\
            attrGetAccuracyName(function_description_user.accuracy), accuracy_vector[function_description_user.accuracy], \
            attrGetAccuracyName(function_description_compiler.accuracy), accuracy_vector[function_description_compiler.accuracy]);

    if ( accuracy_vector[function_description_user.accuracy] > accuracy_vector[function_description_compiler.accuracy] )
    {
        PRN_MSG("[failure]\n");
        return 0;
    }
    else
    {
        PRN_MSG("[success]\n");
    }
#endif

#if 0
    //
    // Temporarily disable this part due to cq306561: we don't initialize this
    // variable if only a "max-error" attribute comes (no bits attributes).
    //
    PRN_MSG("%-32s: \taccurate bits check : requested %f, suggested %f: ","svmlMatchFuncDescriptions",\
            function_description_user.accurate_bits, function_description_compiler.accurate_bits);

    if ( function_description_user.accurate_bits > function_description_compiler.accurate_bits )
    {
        PRN_MSG("[failure]\n");
        return 0;
    }
    else
    {
        PRN_MSG("[success]\n");
    }
#endif

    // relative and absolute error
    PRN_MSG("%-32s: \trelative error check : requested %f, suggested %f: ","svmlMatchFuncDescriptions",\
            function_description_user.ulp_error, function_description_compiler.ulp_error);
    if (function_description_user.ulp_error <  function_description_compiler.ulp_error)
    {
        PRN_MSG("[failure]\n");
#if 0

        if (function_description_compiler.absolute_error < 0.0f)
        {
            PRN_MSG("\t\t\t\t<negative absolute error for suggested argument,>\n");
            PRN_MSG("\t\t\t\t<this is either error or non-initialized value,>\n");
            PRN_MSG("\t\t\t\t<so working as if it were huge and declaring a mismatch>\n");
            return 0;
        }

        PRN_MSG("%-32s: \tadditional absolute error check : requested %f, suggested %f: ","svmlMatchFuncDescriptions",\
                function_description_user.absolute_error, function_description_compiler.absolute_error);

        if (function_description_user.absolute_error <  function_description_compiler.absolute_error)
        {
            PRN_MSG("[failure]\n");
            return 0;
        }
        else
        {
            PRN_MSG("[success]\n");
        }
#else
            return 0;
#endif
    }
    {
        PRN_MSG("[success]\n");
    }

    PRN_MSG("%-32s: \trestricted domain requirement check : requested %d, suggested %d: ","svmlMatchFuncDescriptions",\
            function_description_user.domain_exclusion, function_description_compiler.domain_exclusion);

    if ((function_description_user.domain_exclusion | function_description_compiler.domain_exclusion) != function_description_user.domain_exclusion)
    {
        PRN_MSG("[failure]\n");
        return 0;
    }
    else
    {
        PRN_MSG("[success]\n");
    }

    PRN_MSG("%-32s: \tarch consistency requirement check : requested %d, suggested %d: ","svmlMatchFuncDescriptions",\
            function_description_user.arch_consistency, function_description_compiler.arch_consistency);
    if (function_description_user.arch_consistency > function_description_compiler.arch_consistency)
    {
        PRN_MSG("[failure]\n");
        return 0;
    }
    else
    {
        PRN_MSG("[success]\n");
    }

    PRN_MSG("%-32s: \tcorrectly set status bits  requirement check : requested %d, suggested %d: ", "svmlMatchFuncDescriptions",\
            function_description_user.valid_status_bits,  function_description_compiler.valid_status_bits);
    if (function_description_user.valid_status_bits >  function_description_compiler.valid_status_bits)
    {
        PRN_MSG("[failure]\n");
        return 0;
    }
    else
    {
        PRN_MSG("[success]\n");
    }

    PRN_MSG("%-32s: \tuse-svml requirement check : requested %d, suggested %d: ", "svmlMatchFuncDescriptions",\
            function_description_user.use_svml_allowed,  function_description_compiler.use_svml_allowed);
    if (function_description_user.use_svml_allowed >  function_description_compiler.use_svml_allowed)
    {
        PRN_MSG("[failure]: either we are trying to inline a sequence (which has no use_svml_allowed field or it is set to zero) or we are trying to call some complex LIBM function (which is not handled here in this function yet anyway)\n");
        return 0;
    }
    else
    {
        PRN_MSG("[success]\n");
    }

    PRN_MSG("%-32s: ***** ALL REQUIREMENTS MATCH [success]\n","svmlMatchFuncDescriptions");
    return 1;
}


static int svmlGetListForBaseName(const char * func_base_name, FunctionDescriptionType ** functions_list_to_fill)
{
    const FunctionDescriptionType * FunctionDescription_table_ptr;
    int svml_functions_num;

//  SVML library description table in full format  automatically generated by btgen_SVML.pl
#if !defined LRB
    static const FunctionDescriptionType FunctionDescription_table32[] = {
        #include "iml_table_svml_ia32.inc"
        //#pragma message "IA32 LIBIMLATTR TABLE LOADED"
    };
#endif
#if 0
    static const FunctionDescriptionType FunctionDescription_table64[] = {
#if defined LRB
        #include "iml_table_svml_knc.inc"
        //#pragma message "KNC LIBIMLATTR TABLE LOADED"
#else
        #include "iml_table_svml_em64t.inc"
        //#pragma message "EFI2 LIBIMLATTR TABLE LOADED"
#endif
    };
#endif

#if !defined LRB
//TODO: here we are using some compiler internal functionality
//      to keep our code standalone we should not be doing this.
//      The only other way around is to receive target arch as yet
//      another attribute...
//TODO for xmain: cpu_info keeps track of target architecture specific features in icc.
//This needs to be changed to use an appropriate LLVM mechanism such as the Target Triple.
//See MapIntrinToIml.cpp for usage.
/*
    if (cpu_info::em64t_target())
    {
#endif
        FunctionDescription_table_ptr = FunctionDescription_table64;
        svml_functions_num = sizeof(FunctionDescription_table64) / sizeof(FunctionDescriptionType);
#if !defined LRB
    }
    else
    {
*/
        FunctionDescription_table_ptr = FunctionDescription_table32;
        svml_functions_num = sizeof(FunctionDescription_table32) / sizeof(FunctionDescriptionType);
//    }
#endif

    // read the table and locate a sub-array of function descriptions
    int variants_num = svmlGetFuncVariantsList(  functions_list_to_fill,
                                                   func_base_name,
                                                   FunctionDescription_table_ptr,
                                                   svml_functions_num );
    return variants_num;
}















//******************************************************************************************/
//******************************************************************************************/
//******************************************************************************************/
// This function returns the name of the library function to call
// given the specified constraints.
//
// func_base_name
//    is the name of the math function of interest, e.g. sin, expf
//
// external_attributes
//    define desired constrains for the function. Attributes array is
//    terminated by a pair with the NULL attribute name.
//******************************************************************************************/
//******************************************************************************************/
//******************************************************************************************/
const char* get_library_function_name(const char* func_base_name,
                                      const ImfAttr* external_attributes)
{
    int i;
    int is_svml = 0;
    int function_description_compiler_variants_num = 0;
    const char* result_string_pointer = NULL;
    PrecisionEnum function_precision = c_precision_unsupported;
    FunctionAttributeType       internal_attribute; // Storage for internal representation of current attribute
    FunctionDescriptionType     function_description_user; // Storage for current function information
    FunctionDescriptionType *   function_description_compiler_variant;


    PRN_MSG("\n=================================\n");
    PRN_MSG("%-32s: [START] entered function\n", "get_library_function_name");

    if (func_base_name == NULL) // Bad argument check
    {
        PRN_MSG("%-32s: [ERROR] func_base_name is NULL\n", "get_library_function_name");
        return NULL;
    }
    else
    {
        PRN_MSG("%-32s: Func_base_name = \"%s\"\n", "get_library_function_name",func_base_name);
    }

/* Currently new table is only for non-LRB configs */
// TODO: replace GFX macro with run-time target check, which can be inside the
// #if (!defined IML_OCLSVML_BUILD) check.
#if (!defined GFX) && (!defined IML_OCLSVML_BUILD)
    if  (strstr(func_base_name, "svml"))
    {
      is_svml = 1;
    }
#endif

    // fill in the default values into the function description structure
    attrInitFuncDescription(&function_description_user);

    if ( is_svml == 0)
    {
        // Index in libm_description_table and find out the function number
        function_description_user.index = libmGetNameIndex(func_base_name);
        if (function_description_user.index < 0)
        {
            PRN_MSG( "%-32s: [ERROR] function name is invalid - not found in table (id = %d)\n", "get_library_function_name", function_description_user.index );
            return NULL;
        }
        else
        {
            PRN_MSG("%-32s: Function index = %d\n", "get_library_function_name", function_description_user.index );
        }

        // Read the function precision property from the libm_description_table
        function_precision = libmGetFuncPrecision( function_description_user.index );
    }
    else
    {
        // read the table and locate an array of function descriptions
        function_description_compiler_variants_num = svmlGetListForBaseName(func_base_name, &function_description_compiler_variant);

        if (function_description_compiler_variants_num < 1)
        {
          PRN_MSG("%-32s: [FAILURE] function wasn't chosen\n", "get_library_function_name");
          PRN_MSG("=================================\n\n");
          return NULL;
        }

        function_precision = function_description_compiler_variant[0].precision;
    }

    function_description_user.precision = function_precision;
    PRN_MSG("%-32s: Func precision = %s\n", "get_library_function_name", valid_precision_names[function_description_user.precision+1] );

    PRN_MSG("%-32s: User requested attributes:\n", "get_library_function_name");
    // =========================================================================
    // Loop over external_attributes, convert external representation into
    // internal enumerations, update the requested function description
    // based on incoming attribute.
    // =========================================================================
    while (external_attributes != NULL)
    {
        attrExternal2InternalAttr(&internal_attribute, external_attributes);
        attrUpdateFuncDescription(&function_description_user, &internal_attribute);
        external_attributes = external_attributes->next; // get next external attribute pointer
    }

    // check if user requested use svml on a libm function
    if ( (is_svml == 0) && (function_description_user.use_svml_allowed == 1) )
    {
        // hack the base name and try to locate the function in the SVML table
        // NOTE: this strcat is supposedly secure enough as the func_base_name has been
        //       already found in the LIBM table, thus adding prefix/suffix would not
        //       make it longer than 1024 symbols as LIBM names are all short.
        char new_name[1024];
        strcpy(new_name, "__svml_");
        // doing strncat here to assure the compiler we have enough space in destination
        strncat(new_name, func_base_name, 100);
        strcat(new_name, "1");
        PRN_MSG("%-32s: old basename: %s, looking for changed basename %s in SVML table\n", "get_library_function_name", func_base_name, new_name);
        function_description_compiler_variants_num = svmlGetListForBaseName((const char *)new_name, &function_description_compiler_variant);

        if (function_description_compiler_variants_num < 1)
        {
            PRN_MSG("%-32s: [FAILURE] function wasn't chosen\n", "get_library_function_name");
            PRN_MSG("=================================\n\n");
            return NULL;
        }

        // check previously initialized function_precision for a match
        if ( function_precision != function_description_compiler_variant[0].precision )
        {
            PRN_MSG("%-32s: [FAILURE] new function precision doesn't match the old one\n", "get_library_function_name");
            PRN_MSG("=================================\n\n");
            return NULL;
        }

        // pretend we are in SVML now
        is_svml = 1;
        PRN_MSG("%-32s: we found the name in SVML table, now working with its variants\n", "get_library_function_name");
    }

    // =========================================================================
    // If the function is integer, processing FP attributes doesn't make sense.
    // =========================================================================
    if ( (function_description_user.precision == c_precision_signed_int) || (function_description_user.precision == c_precision_unsigned_int) )
    {
        attrFixupIntegerFuncDescription(&function_description_user);
        PRN_MSG("%-32s: Function is integer, resetting its FP attributes to good ones\n", "get_library_function_name");
    }

    // Handle FuSa
    if ( function_description_user.fusa == 1 )
    {
            PRN_MSG("%-32s: [FAILURE] no FuSa functions yet\n", "get_library_function_name");
            PRN_MSG("=================================\n\n");
            return NULL;
    }

    if ( is_svml == 0)
    {
        // =========================================================================
        // Here we presumably finished collecting user requests and classified them
        // into the appropriate fields of the structure. Now we are ready to compute
        // the index in T-tab and lookup the target function name.
        // =========================================================================
        result_string_pointer = libmGetVariantName(
                                    function_description_user.index,
                                    function_description_user.accuracy);

        PRN_MSG("%-32s: [SUCCESS] return \"%s\" function name\n", "get_library_function_name", result_string_pointer);
        PRN_MSG("=================================\n\n");
        return result_string_pointer;
    }
    else
    {
        // loop over function descriptions and choose the fastest acceptable implementation
        for (i = 0; i < function_description_compiler_variants_num; i++)
        {
            PRN_MSG("%-32s: Looking at \"%s\" variant:\n", "get_library_function_name", function_description_compiler_variant[i].cpu_specific_names[c_cpu_all]);
            if (svmlMatchFuncDescriptions(function_description_user, function_description_compiler_variant[i]))
            {   //Assumption: the first matching implementation is the fastest
                result_string_pointer = function_description_compiler_variant[i].cpu_specific_names[c_cpu_all];
                PRN_MSG("%-32s: \"%s\" Generic interface function name\n", "get_library_function_name", result_string_pointer);
                // Find available CPU variant
                // Unless dynamic dispatching was enforced
                if (function_description_user.configuration == c_cpu_force_dynamic)
                {
                    PRN_MSG("%-32s: configuration number %d will be treated further as generic %d\n", "get_library_function_name", c_cpu_force_dynamic, c_cpu_all);
                    function_description_user.configuration = c_cpu_all;
                }
                // Handle zmm-low
                if ( (function_description_user.configuration == c_cpu_coreavx512) && (function_description_user.zmm_low == 1) )
                {
                    PRN_MSG("%-32s: configuration \"%s\" will be treated further as zmm-low \"%s\"\n", "get_library_function_name", \
                             attrGetConfigName(c_cpu_coreavx512), attrGetConfigName(c_cpu_coreavx512zmmlow));
                    function_description_user.configuration = c_cpu_coreavx512zmmlow;
                }
                // Handle invalid input
                if ( (function_description_user.configuration < c_cpu_all) || (function_description_user.configuration >= SUPPORTED_CPUS_NUMBER))
                {
                    PRN_MSG("%-32s: [ERROR] unsupported configuration attribute %d\n", "get_library_function_name", function_description_user.configuration);
                    break;
                }
                // Choose cpu-specific variant
                result_string_pointer = function_description_compiler_variant[i].cpu_specific_names[function_description_user.configuration];
                // Handle data corruption in the table
                if (result_string_pointer == NULL)
                {
                    PRN_MSG("%-32s: [ERROR] configuration number %d unexpectedly led to NULL pointer name\n", "get_library_function_name", function_description_user.configuration);
                    break;
                }
                // Handle missing cpu-specific entries
                if (result_string_pointer[0] == '\0')
                {
                    PRN_MSG("%-32s: configuration number %d led to an empty function name, we may be not exposing these names in SVML or maybe being asked for DP on SSE1?\n", "get_library_function_name", function_description_user.configuration);

                    PRN_MSG("%-32s: resorting to dispatcher entry\n", "get_library_function_name");
                    result_string_pointer = function_description_compiler_variant[i].cpu_specific_names[c_cpu_all];
                }
                // Success
                PRN_MSG("%-32s: [SUCCESS] return \"%s\" function name\n", "get_library_function_name", result_string_pointer);
                PRN_MSG("=================================\n\n");
                return result_string_pointer;
            }
        }

        PRN_MSG("%-32s: [ERROR] function wasn't chosen\n", "get_library_function_name");
        PRN_MSG("=================================\n\n");

        return NULL;
    }
}

bool is_libm_function(const char *name) {
  return libmGetNameIndex(name) >= 0 ? true : false;
}

//******************************************************************************************/
//******************************************************************************************/
//******************************************************************************************/
// This function returns 1 or 0 meaning "yes" or "no". It answers the question:
// whether the compiler may use an instructions sequence with certain
// properties described by external_attributes_compiler list given the
// conditions specified by external_attributes_user list.
//
// func_base_name
//    is the name of the math function of interest, e.g. sin, expf. Actually
//    it shall be used for the only purpose - determine precision property.
//
// external_attributes_user
//    define desired constrains for the function. Attributes array is
//    terminated by a pair with the NULL attribute name.
//
// external_attributes_compiler
//    define properties of some function implementation (presumably known to
//    the compiler). Attributes array is terminated by a pair with the NULL
//    attribute name.
//******************************************************************************************/
//******************************************************************************************/
//******************************************************************************************/
int may_i_use_inline_implementation(
                                const char* func_base_name,
                                const ImfAttr* external_attributes_user,
                                const ImfAttr* external_attributes_compiler)
{
    // =========================================================================
    // =========================================================================

    int i;
    int flag = 0;
    int function_description_compiler_variants_num = 0;

    // Storage for current function information
    FunctionDescriptionType     function_description_user;
    FunctionDescriptionType     function_description_compiler;
    FunctionDescriptionType*    function_description = NULL;

    // Storage for internal representation of attributes
    FunctionAttributeType       internal_attributes_user;
    FunctionAttributeType       internal_attributes_compiler;
    FunctionAttributeType*      internal_attribute;

    FunctionDescriptionType *   function_description_compiler_variant;

    // Pointer to walk over attributes list
    const ImfAttr*              external_attributes;


    PRN_MSG("\n=================================\n");
    PRN_MSG("%-32s: [START] entered function\n","may_i_use_inline_implementation");

    if (func_base_name == NULL) // Bad arguments check
    {
        PRN_MSG("%-32s: [ERROR] func_base_name is NULL\n", "may_i_use_inline_implementation");
        return 0;
    }
    else
    {
        PRN_MSG("%-32s: Func_base_name = \"%s\"\n", "may_i_use_inline_implementation",func_base_name);
    }

    // fill in the default values into the function description structures
    attrInitFuncDescription(&function_description_user);
    attrInitFuncDescription(&function_description_compiler);

    /* Search in LIBM table */
    flag = libmGetNameIndex(func_base_name);
    if (flag >= 0)
    {
        function_description_user.index = flag;
        function_description_compiler.index = flag;
        function_description_user.precision = libmGetFuncPrecision(function_description_user.index);
        function_description_compiler.precision = libmGetFuncPrecision(function_description_compiler.index);
        PRN_MSG("%-32s: Function \"%s\" found in LIBM table [success]\n", "may_i_use_inline_implementation", func_base_name);
    }
    else
    {
        /* Search in INLINE table */
        flag = inlineGetNameIndex(func_base_name);
        if (flag >= 0)
        {
            function_description_user.index = flag;
            function_description_compiler.index = flag;
            function_description_user.precision = inlineGetFuncPrecision(function_description_user.index);
            function_description_compiler.precision = inlineGetFuncPrecision(function_description_compiler.index);
            PRN_MSG("%-32s: Function \"%s\" found in INLINE table [success]\n", "may_i_use_inline_implementation", func_base_name);
        }
        else
        {
            // read the table and locate an array of function descriptions
            function_description_compiler_variants_num = svmlGetListForBaseName(func_base_name, &function_description_compiler_variant);

            if(function_description_compiler_variants_num > 0)
            {
                function_description_user.precision = function_description_compiler_variant[0].precision;
                function_description_compiler.precision = function_description_compiler_variant[0].precision;
                PRN_MSG("%-32s: Function \"%s\" found in SVML table [success]\n", "may_i_use_inline_implementation", func_base_name);
            }
            else
            {
                PRN_MSG("%-32s: [ERROR] \"%s\" wasn't found in INLINE/LIBM/SVML tables\n", "may_i_use_inline_implementation", func_base_name);
                PRN_MSG("=================================\n\n");
                return 0;
            }
        }
    }

    PRN_MSG("%-32s: User's     func precision = %s\n", "may_i_use_inline_implementation", valid_precision_names[function_description_user.precision+1] );
    PRN_MSG("%-32s: Compiler's func precision = %s\n", "may_i_use_inline_implementation", valid_precision_names[function_description_compiler.precision+1] );

    // =========================================================================
    // Walk over "compiler" and "user" attributes sets.
    // We organize the two passes in a loop of 2 iterations to not duplicate the
    // code. We use pointers to substitute "compiler" and "user" entities.
    // =========================================================================

    // start with "compiler" attributes
    internal_attribute   = &internal_attributes_compiler;
    external_attributes  = external_attributes_compiler;
    function_description = &function_description_compiler;

    for (i = 0; i < 2; i++)
    {
        // =====================================================================
        // Loop over external_attributes, convert external representation into
        // internal enumerations, update the requested function description
        // based on incoming attribute.
        // =====================================================================
        if(i == 0)
        {
          PRN_MSG("%-32s: Compiler suggested attributes:\n", "may_i_use_inline_implementation");
        }
        else
        {
          PRN_MSG("%-32s: User requested attributes:\n", "may_i_use_inline_implementation");
        }

        while (external_attributes != NULL)
        {
            flag = attrExternal2InternalAttr(internal_attribute, external_attributes);
            flag = attrUpdateFuncDescription(function_description, internal_attribute);
            external_attributes = external_attributes->next; // get next external attribute pointer
        }

        // move pointers to "user" attributes
        internal_attribute   = &internal_attributes_user;
        external_attributes  = external_attributes_user;
        function_description = &function_description_user;
    }

    PRN_MSG("%-32s: Comparing user and compiler attributes:\n", "may_i_use_inline_implementation");
    // =========================================================================
    // Here we have prepared two function descriptions based on user and
    // compiler attributes. Now we need to match them up and decide whether
    // the compiler request fits into user defined requirements.
    // =========================================================================
    flag = svmlMatchFuncDescriptions(function_description_user, function_description_compiler);

    if(flag)
    {
      PRN_MSG("%-32s: [SUCCESS] \"%s\" function may be inlined\n", "may_i_use_inline_implementation", func_base_name);
      PRN_MSG("=================================\n\n");
    }
    else
    {
      PRN_MSG("%-32s: [FAILURE] \"%s\" function may not be inlined\n", "may_i_use_inline_implementation", func_base_name);
      PRN_MSG("=================================\n\n");
    }

    return flag;
}

#if defined __cplusplus
}
#endif // __cplusplus
