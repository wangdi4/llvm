/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  oclbuiltin.cl


\*****************************************************************************/

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

/*
* oclbuiltin
This test verifyes that builtin is called and OpenCL Reference and NEAT does not fail.
Tests checking OCL builtins NEAT functions are in unittests for NEAT ALU

This test checks NEAT plugin can call builtins from NEATALU for double scalar
and vector datatypes. Vector datatype now is float4

Kernel argument currently plays no role and are formal.


* @param input  input buffer
* @param output output buffer
* @param buffer_size  buffer size
*/
/*

#define CALL_BI_ONEARG(_f) a_out = _f(a_in); a2_out = _f(a2_in); \
                           a3_out = _f(a3_in); a4_out = _f(a4_in); \
                           a8_out = _f(a8_in); a16_out = _f(a16_in);

#define CALL_BI_TWOARG(_f) a_out = _f(a_in, b_in); a2_out = _f(a2_in, b2_in); \
                           a3_out = _f(a3_in, b3_in); a4_out = _f(a4_in, b4_in); \
                           a8_out = _f(a8_in, b8_in); a16_out = _f(a16_in, b16_in);

#define CALL_BI_THREEARG(_f) a_out = _f(a_in, b_in, c_in); a2_out = _f(a2_in, b2_in, c2_in); \
                           a3_out = _f(a3_in, b3_in, c3_in); a4_out = _f(a4_in, b4_in, c4_in); \
                           a8_out = _f(a8_in, b8_in, c8_in); a16_out = _f(a16_in, b16_in, c16_in);

#define CALL_BI_TWOOUTARG(_f) a_out = _f(a_in, &b_out); a2_out = _f(a2_in, &b2_out); \
                           a3_out = _f(a3_in, &b3_out); a4_out = _f(a4_in, &b4_out); \
                           a8_out = _f(a8_in, &b8_out); a16_out = _f(a16_in, &b16_out);

#define CALL_CLAMP(_f)     a_out = _f(a_in, b_in, c_in); a2_out = _f(a2_in, b_in, c_in); \
                           a3_out = _f(a3_in, b_in, c_in); a4_out = _f(a4_in, b_in, c_in); \
                           a8_out = _f(a8_in, b_in, c_in); a16_out = _f(a16_in, b_in, c_in);

#define CALL_STEP(_f) a_out = _f(a_in, b_in); a2_out = _f(a_in, b2_in); \
                           a3_out = _f(a_in, b3_in); a4_out = _f(a_in, b4_in); \
                           a8_out = _f(a_in, b8_in); a16_out = _f(a_in, b16_in);

#define CALL_VLOAD(_f)     a2_out = _f##2(0, &b_in); \
                           a3_out = _f##3(0, &b_in); a4_out = _f##4(0, &b_in); \
                           a8_out = _f##8(0, &b_in); a16_out = _f##16(0, &b_in);

#define CALL_VSTORE(_f)    _f##2(a2_in, 0, (double*)&a2_out); _f##3(a3_in, 0, (double*)&a3_out); \
                           _f##4(a4_in, 0, (double*)&a4_out); _f##8(a8_in, 0, (double*)&a8_out); \
                           _f##16(a16_in, 0, (double*)&a16_out);

#define CALL_CONVERT(_f,type)   a_out = _f((type)tid); a2_out = _f##2((type##2)tid); \
                           a3_out = _f##3((type##3)tid); a4_out = _f##4((type##4)tid); \
                           a8_out = _f##8((type##8)tid); a16_out = _f##16((type##16)tid);

// for calling min or max functions
#define CALL_MINMAX(_f)    a2_out = _f(a2_in, b_in);\
                           a3_out = _f(a3_in, b_in); a4_out = _f(a4_in, b_in); \
                           a8_out = _f(a8_in, b_in);\
                           a16_out = _f(a16_in, b_in);

#define CALL_DOT(_f) a_out = _f(a_in, b_in); a_out = _f(a2_in, b2_in); \
                     a_out = _f(a3_in, b3_in); a_out = _f(a4_in, b4_in);

#define CALL_NORMALIZE(_f) a_out = _f(a_in); a2_out = _f(a2_in); \
                           a3_out = _f(a3_in); a4_out = _f(a4_in);

#define CALL_MIX(_f) a_out = _f(a_in, b_in, c_in); a2_out = _f(a2_in, b2_in, c2_in);\
                     a3_out = _f(a3_in, b3_in, c3_in); a4_out = _f(a4_in, b4_in, c4_in);\
                     a2_out = _f(a2_in, b2_in, c_in); a3_out = _f(a3_in, b3_in, c_in);\
                     a4_out = _f(a4_in, b4_in, c_in); a8_out = _f(a8_in, b8_in, c_in);\
                     a16_out = _f(a16_in, b16_in, c_in);  a8_out = _f(a8_in, b8_in, c8_in); \
                     a16_out = _f(a16_in, b16_in, c16_in);

#define CALL_FOUT_FIN_IIN(_f) a_out = _f(a_in, i_in); a2_out = _f(a2_in, i2_in); \
                       a3_out = _f(a3_in, i3_in); a4_out = _f(a4_in, i4_in); \
                       a8_out = _f(a8_in, i8_in); a16_out = _f(a16_in, i16_in);

#define CALL_SINGLE_POW(_f) a2_out = _f(a2_in, i_in); \
                       a3_out = _f(a3_in, i_in); a4_out = _f(a4_in, i_in); \
                       a8_out = _f(a8_in, i_in); a16_out = _f(a16_in, i_in);

#define CALL_FREXP(_f) a_out = _f(a_in, &i_out); a2_out = _f(a2_in, &i2_out); \
                       a3_out = _f(a3_in, &i3_out); a4_out = _f(a4_in, &i4_out); \
                       a8_out = _f(a8_in, &i8_out); a16_out = _f(a16_in, &i16_out);
*/

#define CALL_BI_ONEARG(_f) a_out = _f(a_in); a4_out = _f(a4_in); \
                           a8_out = _f(a8_in); a16_out = _f(a16_in);

#define CALL_BI_TWOARG(_f) a_out = _f(a_in, b_in); a4_out = _f(a4_in, b4_in); \
                           a8_out = _f(a8_in, b8_in); a16_out = _f(a16_in, b16_in);

#define CALL_BI_THREEARG(_f) a_out = _f(a_in, b_in, c_in); a4_out = _f(a4_in, b4_in, c4_in); \
                           a8_out = _f(a8_in, b8_in, c8_in); a16_out = _f(a16_in, b16_in, c16_in);

#define CALL_BI_TWOOUTARG(_f) a_out = _f(a_in, &b_out); a4_out = _f(a4_in, &b4_out); \
                           a8_out = _f(a8_in, &b8_out); a16_out = _f(a16_in, &b16_out);

#define CALL_CLAMP(_f)     a_out = _f(a_in, b_in, c_in); a4_out = _f(a4_in, b_in, c_in); \
                           a8_out = _f(a8_in, b_in, c_in); a16_out = _f(a16_in, b_in, c_in);

#define CALL_STEP(_f) a_out = _f(a_in, b_in); a4_out = _f(a_in, b4_in); \
                           a8_out = _f(a_in, b8_in); a16_out = _f(a_in, b16_in);

#define CALL_SMOOTHSTEP(_f) a_out = _f(a_in, b_in, c_in); a4_out = _f(a_in, b_in, c4_in); \
                           a8_out = _f(a_in, b_in, c8_in); a16_out = _f(a_in, b_in, c16_in);

#define CALL_VLOAD(_f)     a4_out = _f##4(0, &b_in); \
                           a8_out = _f##8(0, &b_in); a16_out = _f##16(0, &b_in);

#define CALL_VSTORE(_f)    _f##4(a4_in, 0, (double*)&a4_out); _f##8(a8_in, 0, (double*)&a8_out); \
                           _f##16(a16_in, 0, (double*)&a16_out);

#define CALL_CONVERT(_f,type)   a_out = _f((type)tid); a4_out = _f##4((type##4)tid); \
                           a8_out = _f##8((type##8)tid); a16_out = _f##16((type##16)tid);

// for calling min or max functions
#define CALL_MINMAX(_f)    a4_out = _f(a4_in, b_in); \
                           a8_out = _f(a8_in, b_in);\
                           a16_out = _f(a16_in, b_in);

#define CALL_DOT(_f) a_out = _f(a_in, b_in); a_out = _f(a4_in, b4_in);

#define CALL_NORMALIZE(_f) a_out = _f(a_in); a4_out = _f(a4_in);

#define CALL_CROSS(_f) a4_out = _f(a4_in, b4_in);

#define CALL_GEOM_ONEARG(_f) a_out = _f(a_in); a_out = _f(a2_in);\
                           a_out = _f(a4_in);

#define CALL_GEOM_TWOARG(_f) a_out = _f(a_in,b_in); a_out = _f(a2_in,b2_in);\
                           a_out = _f(a4_in,b4_in);

#define CALL_MIX(_f) a_out = _f(a_in, b_in, c_in); a4_out = _f(a4_in, b4_in, c4_in);\
                     a4_out = _f(a4_in, b4_in, c_in); a8_out = _f(a8_in, b8_in, c_in);\
                     a16_out = _f(a16_in, b16_in, c_in);  a8_out = _f(a8_in, b8_in, c8_in); \
                     a16_out = _f(a16_in, b16_in, c16_in);

#define CALL_FOUT_FIN_IIN(_f) a_out = _f(a_in, i_in); a4_out = _f(a4_in, i4_in); \
                       a8_out = _f(a8_in, i8_in); a16_out = _f(a16_in, i16_in);

#define CALL_SINGLE_POW(_f) a4_out = _f(a4_in, i_in); \
                       a8_out = _f(a8_in, i_in); a16_out = _f(a16_in, i_in);

#define CALL_FREXP(_f) a_out = _f(a_in, &i_out); a4_out = _f(a4_in, &i4_out); \
                       a8_out = _f(a8_in, &i8_out); a16_out = _f(a16_in, &i16_out);

#define CALL_ILOGB(_f) i_out = _f(a_in); i4_out = _f(a4_in); \
                       i8_out = _f(a8_in); i16_out = _f(a16_in);

#define CALL_NAN(_f) a_out = _f(ul_in); a4_out = _f(ul4_in); \
                       a8_out = _f(ul8_in); a16_out = _f(ul16_in);

#define CALL_SELECT(_f) a_out =  _f(a_in,  b_in,  l_in);    a4_out =  _f(a4_in,  b4_in,  l4_in);   \
                        a8_out = _f(a8_in, b8_in, l8_in);   a16_out = _f(a16_in, b16_in, l16_in);  \
                        a_out =  _f(a_in,  b_in,  ul_in);   a4_out =  _f(a4_in,  b4_in,  ul4_in);  \
                        a8_out = _f(a8_in, b8_in, ul8_in);  a16_out = _f(a16_in, b16_in, ul16_in);
#define CALL_REMQUO(_f) a_out = _f(a_in, b_in, &i_out); a2_out = _f(a2_in, b2_in, &i2_out); \
                        a3_out = _f(a3_in, b3_in, &i3_out); a4_out = _f(a4_in, b4_in, &i4_out); \
                        a8_out = _f(a8_in, b8_in, &i8_out); a16_out = _f(a16_in, b16_in, &i16_out);

#define CALL_SHUFFLE(_f) a2_out = _f(a2_in, ul2_in); a2_out = _f(a4_in, ul2_in); a2_out = _f(a8_in, ul2_in); a2_out = _f(a16_in, ul2_in);\
                         a4_out = _f(a2_in, ul4_in); a4_out = _f(a4_in, ul4_in); a4_out = _f(a8_in, ul4_in); a4_out = _f(a16_in, ul4_in);\
                         a8_out = _f(a2_in, ul8_in); a8_out = _f(a4_in, ul8_in); a8_out = _f(a8_in, ul8_in); a8_out = _f(a16_in, ul8_in);\
                         a16_out = _f(a2_in, ul16_in); a16_out = _f(a4_in, ul16_in); a16_out = _f(a8_in, ul16_in); a16_out = _f(a16_in, ul16_in);

#define CALL_SHUFFLE2(_f) a2_out = _f(a2_in, b2_in, ul2_in); a2_out = _f(a4_in, b4_in, ul2_in); a2_out = _f(a8_in, b8_in, ul2_in); a2_out = _f(a16_in, b16_in, ul2_in);\
                          a4_out = _f(a2_in, b2_in, ul4_in); a4_out = _f(a4_in, b4_in, ul4_in); a4_out = _f(a8_in, b8_in, ul4_in); a4_out = _f(a16_in, b16_in, ul4_in);\
                          a8_out = _f(a2_in, b2_in, ul8_in); a8_out = _f(a4_in, b4_in, ul8_in); a8_out = _f(a8_in, b8_in, ul8_in); a8_out = _f(a16_in, b16_in, ul8_in);\
                          a16_out = _f(a2_in, b2_in, ul16_in); a16_out = _f(a4_in, b4_in, ul16_in); a16_out = _f(a8_in, b8_in, ul16_in); a16_out = _f(a16_in, b16_in, ul16_in);

__kernel
void oclbuiltin(__global double * input,
                __global double * output,
                __global int * inputInt,
                __global int * outputInt,
                __global long * inputUlong,
                const uint  buffer_size)
{
    uint tid = 0;
    double a_in = input[tid];
    double2 a2_in = input[tid];
    double3 a3_in = input[tid];
    double4 a4_in = input[tid];
    double8 a8_in = input[tid];
    double16 a16_in = input[tid];

    double b_in = input[tid+1];
    double2 b2_in = input[tid];
    double3 b3_in = input[tid];
    double4 b4_in = input[tid];
    double8 b8_in = input[tid];
    double16 b16_in = input[tid];

    double c_in = input[tid+1];
    double2 c2_in = input[tid];
    double3 c3_in = input[tid];
    double4 c4_in = input[tid];
    double8 c8_in = input[tid];
    double16 c16_in = input[tid];

    double a_out = output[tid];
    double2 a2_out = output[tid];
    double3 a3_out = output[tid];
    double4 a4_out = output[tid];
    double8 a8_out = output[tid];
    double16 a16_out = output[tid];

    double b_out = output[tid];
    double2 b2_out = output[tid];
    double3 b3_out = output[tid];
    double4 b4_out = output[tid];
    double8 b8_out = output[tid];
    double16 b16_out = output[tid];

    double c_out = output[tid];
    double2 c2_out = output[tid];
    double3 c3_out = output[tid];
    double4 c4_out = output[tid];
    double8 c8_out = output[tid];
    double16 c16_out = output[tid];

    int i_in = inputInt[tid];
    int2 i2_in = inputInt[tid];
    int3 i3_in = inputInt[tid];
    int4 i4_in = inputInt[tid];
    int8 i8_in = inputInt[tid];
    int16 i16_in = inputInt[tid];

    int i_out = outputInt[tid];
    int2 i2_out = outputInt[tid];
    int3 i3_out = outputInt[tid];
    int4 i4_out = outputInt[tid];
    int8 i8_out = outputInt[tid];
    int16 i16_out = outputInt[tid];

    uint ui_in;
    uint2 ui2_in;
    uint3 ui3_in;
    uint4 ui4_in;
    uint8 ui8_in;
    uint16 ui16_in;

    char ch_in;
    char2 ch2_in;
    char3 ch3_in;
    char4 ch4_in;
    char8 ch8_in;
    char16 ch16_in;

    uchar uch_in;
    uchar2 uch2_in;
    uchar3 uch3_in;
    uchar4 uch4_in;
    uchar8 uch8_in;
    uchar16 uch16_in;

    short s_in;
    short2 s2_in;
    short3 s3_in;
    short4 s4_in;
    short8 s8_in;
    short16 s16_in;

    ushort us_in;
    ushort2 us2_in;
    ushort3 us3_in;
    ushort4 us4_in;
    ushort8 us8_in;
    ushort16 us16_in;

    long l_in;
    long2 l2_in;
    long3 l3_in;
    long4 l4_in;
    long8 l8_in;
    long16 l16_in;

    ulong ul_in = inputUlong[tid];
    ulong2 ul2_in = inputUlong[tid];
    ulong3 ul3_in = inputUlong[tid];
    ulong4 ul4_in = inputUlong[tid];
    ulong8 ul8_in = inputUlong[tid];
    ulong16 ul16_in = inputUlong[tid];

    CALL_BI_ONEARG(acos);
    CALL_BI_ONEARG(acospi);
    CALL_BI_ONEARG(asin);
    CALL_BI_ONEARG(asinpi);
    CALL_BI_ONEARG(atan);
    CALL_BI_TWOARG(atan2);
    CALL_BI_TWOARG(atan2pi);
    CALL_BI_ONEARG(atanpi);
    CALL_BI_ONEARG(cos);
    CALL_BI_ONEARG(cosh);
    CALL_BI_ONEARG(cospi);
    CALL_BI_ONEARG(exp);
    CALL_BI_ONEARG(exp2);
    CALL_BI_ONEARG(exp10);
    CALL_BI_ONEARG(expm1);
    CALL_BI_ONEARG(log);
    CALL_BI_ONEARG(log2);
    CALL_BI_ONEARG(log10);
    CALL_BI_ONEARG(log1p);
    CALL_BI_ONEARG(logb);

    CALL_BI_ONEARG(ceil);
    CALL_BI_TWOARG(pow);

    CALL_BI_THREEARG(clamp);
    CALL_CLAMP(clamp);
    CALL_BI_ONEARG(sinh);
    CALL_BI_ONEARG(sin);
    CALL_BI_ONEARG(sinpi);
    CALL_BI_ONEARG(sqrt);
    CALL_BI_ONEARG(rsqrt);
    CALL_BI_ONEARG(tan);
    CALL_BI_ONEARG(tanh);
    CALL_BI_ONEARG(tanpi);
    CALL_BI_ONEARG(fabs);

    CALL_BI_ONEARG(asinh);
    CALL_BI_ONEARG(acosh);
    CALL_BI_ONEARG(atanh);

    CALL_VLOAD(vload);
    CALL_VSTORE(vstore);

    CALL_BI_TWOARG(min);
    CALL_MINMAX(min);
    CALL_BI_TWOARG(max);
    CALL_MINMAX(max);
    CALL_BI_TWOARG(hypot);

    CALL_BI_TWOARG(step);
    CALL_STEP(step);
    CALL_BI_THREEARG(smoothstep);
    CALL_SMOOTHSTEP(smoothstep);
    CALL_BI_ONEARG(radians);
    CALL_BI_ONEARG(degrees);
    CALL_BI_ONEARG(sign);
    CALL_BI_ONEARG(floor);
    CALL_DOT(dot);
    CALL_MIX(mix);
    CALL_NORMALIZE(normalize);
    CALL_CROSS(cross);
    CALL_GEOM_ONEARG(length);
    CALL_GEOM_TWOARG(distance);

    //CALL_CONVERT(convert_double,long);
    //CALL_CONVERT(convert_double,ulong);

    CALL_FOUT_FIN_IIN(rootn);
    CALL_FOUT_FIN_IIN(ldexp);

    CALL_SINGLE_POW(ldexp);

    CALL_BI_TWOOUTARG(modf);
    CALL_FREXP(frexp);
    
    CALL_BI_TWOARG(maxmag);
    CALL_BI_TWOARG(minmag);

    CALL_BI_TWOARG(copysign);
    CALL_BI_TWOARG(nextafter);
    CALL_BI_TWOARG(fdim);
    CALL_BI_THREEARG(fma);
    CALL_BI_THREEARG(mad);
    CALL_BI_ONEARG(rint);
    CALL_BI_ONEARG(round);
    CALL_BI_ONEARG(trunc);
    CALL_BI_ONEARG(cbrt);
    CALL_BI_TWOARG(powr);
    CALL_BI_TWOARG(fmod);
    CALL_BI_TWOARG(fmin);  // gentype fmin (gentype x, gentype y)
    CALL_BI_TWOARG(fmax);  // gentype fmax (gentype x, gentype y)
    CALL_MINMAX(fmin);     // gentype fmax (gentype x, double y)
    CALL_MINMAX(fmax);     // gentype fmax (gentype x, double y)
    CALL_SINGLE_POW(pown);
    CALL_ILOGB(ilogb);
    CALL_NAN(nan);
    CALL_BI_TWOOUTARG(fract);
    CALL_BI_ONEARG(lgamma);
    CALL_FREXP(lgamma_r);

    CALL_BI_THREEARG(bitselect);
    CALL_SELECT(select);
    CALL_BI_TWOARG(remainder);
    CALL_REMQUO(remquo);

    CALL_SHUFFLE(shuffle);
    CALL_SHUFFLE2(shuffle2);

    // TODO: add here tests for other builtins when they are implemented
    // TODO: in NEAT ALU
}
