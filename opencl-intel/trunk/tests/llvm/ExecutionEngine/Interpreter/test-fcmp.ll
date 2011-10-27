; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32:64"

@r_int_2 = internal constant [8 x i8] c" %d %d\0A\00"
@r_int_3 = internal constant [11 x i8] c" %d %d %d\0A\00"
@r_int_4 = internal constant [14 x i8] c" %d %d %d %d\0A\00"
@r_int_8 = internal constant [26 x i8] c" %d %d %d %d %d %d %d %d\0A\00"

; Based on http://en.wikipedia.org/wiki/Single_precision_floating-point_format

@g_fnan = constant [3 x i32] [ i32 2143289344, i32 2143831396, i32 2143831398 ]		; <[3 x i32]*> [#uses=1]
@g_fpinf = constant [3 x i32] [ i32 2139095040, i32 0, i32 0]		; <[3 x i32]*> [#uses=1]
@g_fminf = constant [3 x i32] [ i32 4286578688, i32 0, i32 0]		; <[3 x i32]*> [#uses=1]
@g_fpdenorm = constant [3 x i32] [ i32 1, i32 0, i32 0]		; <[3 x i32]*> [#uses=1]
@g_fmdenorm = constant [3 x i32] [ i32 2147483649, i32 0, i32 0]		; <[3 x i32]*> [#uses=1]

@g_dnan = constant [3 x i64] [ i64 9223235251041752696, i64 9223235251041752697, i64 9223235250773317239 ], align 8		; <[3 x i64]*> [#uses=1]
@g_dpinf = constant [3 x i64] [ i64 9218868437227405312, i64 0, i64 0 ], align 8		; <[3 x i64]*> [#uses=1]
@g_dminf = constant [3 x i64] [ i64 -4503599627370496, i64 0, i64 0 ], align 8		; <[3 x i64]*> [#uses=1]
@g_dpdenorm = constant [3 x i64] [ i64 1, i64 0, i64 0 ], align 8		; <[3 x i64]*> [#uses=1]
@g_dmdenorm = constant [3 x i64] [ i64 9223372036854775809, i64 0, i64 0 ], align 8		; <[3 x i64]*> [#uses=1]

define float @get_fnan()
{
       %ptr = getelementptr [3 x i32]* @g_fnan, i32 0, i32 0
       %val = load i32* %ptr
       %fval = bitcast i32 %val to float
       ret float %fval
}

define float @get_fpinf()
{
       %ptr = getelementptr [3 x i32]* @g_fpinf, i32 0, i32 0
       %val = load i32* %ptr
       %fval = bitcast i32 %val to float
       ret float %fval
}

define float @get_fminf()
{
       %ptr = getelementptr [3 x i32]* @g_fminf, i32 0, i32 0
       %val = load i32* %ptr
       %fval = bitcast i32 %val to float
       ret float %fval
}

define float @get_fpdenorm ()
{
       %ptr = getelementptr [3 x i32]* @g_fpdenorm, i32 0, i32 0
       %val = load i32* %ptr
       %fval = bitcast i32 %val to float
       ret float %fval
}

define float @get_fmdenorm ()
{
       %ptr = getelementptr [3 x i32]* @g_fmdenorm, i32 0, i32 0
       %val = load i32* %ptr
       %fval = bitcast i32 %val to float
       ret float %fval
}

define double @get_dnan()
{
       %ptr = getelementptr [3 x i64]* @g_dnan, i32 0, i32 0
       %val = load i64* %ptr
       %dval = bitcast i64 %val to double
       ret double %dval
}

define double @get_dpinf()
{
       %ptr = getelementptr [3 x i64]* @g_dpinf, i32 0, i32 0
       %val = load i64* %ptr
       %dval = bitcast i64 %val to double
       ret double %dval
}

define double @get_dminf()
{
       %ptr = getelementptr [3 x i64]* @g_dminf, i32 0, i32 0
       %val = load i64* %ptr
       %dval = bitcast i64 %val to double
       ret double %dval
}

define double @get_dpdenorm()
{
       %ptr = getelementptr [3 x i64]* @g_dpdenorm, i32 0, i32 0
       %val = load i64* %ptr
       %dval = bitcast i64 %val to double
       ret double %dval
}

define double @get_dmdenorm()
{
       %ptr = getelementptr [3 x i64]* @g_dmdenorm, i32 0, i32 0
       %val = load i64* %ptr
       %dval = bitcast i64 %val to double
       ret double %dval
}

declare i32 @printf(i8*, ...)

define i32 @fcmp_false_float()
{
       %ptr = getelementptr [11 x i8]* @r_int_3, i32 0, i32 0 ; to printf

       %res_float = fcmp false <3 x float> < float 5.0, float 534.0, float -2.0>, < float 5.0, float 534.0, float -245.0>

       %r_i8_1 = extractelement <3 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <3 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <3 x i1> %res_float, i32 2
       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3)
; CHECK: 0 0 0
       ret i32 0
}

define i32 @fcmp_false_double()
{
       %ptr = getelementptr [11 x i8]* @r_int_3, i32 0, i32 0 ; to printf

       %res_double = fcmp false <3 x double> < double 5.0, double 534.0, double -2.0>, < double 5.0, double 534.0, double -245.0>

       %r_i8_1 = extractelement <3 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <3 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <3 x i1> %res_double, i32 2
       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3)
; CHECK: 0 0 0
       ret i32 0
}

define i32 @fcmp_true_float()
{
       %ptr = getelementptr [11 x i8]* @r_int_3, i32 0, i32 0 ; to printf

       %res_float = fcmp true <3 x float> < float 5.0, float 534.0, float -2.0>, < float 5.0, float 534.0, float -245.0>

       %r_i8_1 = extractelement <3 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <3 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <3 x i1> %res_float, i32 2
       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3)
; CHECK: 1 1 1
       ret i32 0
}

define i32 @fcmp_true_double()
{
       %ptr = getelementptr [11 x i8]* @r_int_3, i32 0, i32 0 ; to printf

       %res_double = fcmp true <3 x double> < double 5.0, double 534.0, double -2.0>, < double 5.0, double 534.0, double -245.0>

       %r_i8_1 = extractelement <3 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <3 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <3 x i1> %res_double, i32 2
       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3)
; CHECK: 1 1 1
       ret i32 0
}

define i32 @fcmp_oeq_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float 0.0,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp oeq <8 x float> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 1 0 1 0 1
       ret i32 0
}

define i32 @fcmp_oeq_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double 0.0,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp oeq <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 1 0 1 0 1
       ret i32 0
}

define i32 @fcmp_ogt_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       %fpinf = call float @get_fpinf()
       %fminf = call float @get_fminf()
       %fpdenorm = call float @get_fpdenorm()
       %fmdenorm = call float @get_fmdenorm()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.255e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp ogt <8 x float> %v4f1_8, %v4f2_8

       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 1 0 1 1 0
       ret i32 0
}

define i32 @fcmp_ogt_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       %fpinf = call double @get_dpinf()
       %fminf = call double @get_dminf()
       %fpdenorm = call double @get_dpdenorm()
       %fmdenorm = call double @get_dmdenorm()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.255e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp ogt <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 1 0 1 1 0
       ret i32 0
}

define i32 @fcmp_oge_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       %fpinf = call float @get_fpinf()
       %fminf = call float @get_fminf()
       %fpdenorm = call float @get_fpdenorm()
       %fmdenorm = call float @get_fmdenorm()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp oge <8 x float> %v4f1_8, %v4f2_8

       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 1 0 1 1 1
       ret i32 0
}

define i32 @fcmp_oge_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       %fpinf = call double @get_dpinf()
       %fminf = call double @get_dminf()
       %fpdenorm = call double @get_dpdenorm()
       %fmdenorm = call double @get_dmdenorm()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp oge <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 1 0 1 1 1
       ret i32 0
}

define i32 @fcmp_olt_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       %fpinf = call float @get_fpinf()
       %fminf = call float @get_fminf()
       %fpdenorm = call float @get_fpdenorm()
       %fmdenorm = call float @get_fmdenorm()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp olt <8 x float> %v4f1_8, %v4f2_8

       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 0 1 0 0 0
       ret i32 0
}

define i32 @fcmp_olt_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       %fpinf = call double @get_dpinf()
       %fminf = call double @get_dminf()
       %fpdenorm = call double @get_dpdenorm()
       %fmdenorm = call double @get_dmdenorm()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp olt <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 0 1 0 0 0
       ret i32 0
}

define i32 @fcmp_ole_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       %fpinf = call float @get_fpinf()
       %fminf = call float @get_fminf()
       %fpdenorm = call float @get_fpdenorm()
       %fmdenorm = call float @get_fmdenorm()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp ole <8 x float> %v4f1_8, %v4f2_8

       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 1 1 0 0 1
       ret i32 0
}

define i32 @fcmp_ole_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       %fpinf = call double @get_dpinf()
       %fminf = call double @get_dminf()
       %fpdenorm = call double @get_dpdenorm()
       %fmdenorm = call double @get_dmdenorm()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp ole <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 1 1 0 0 1
       ret i32 0
}

define i32 @fcmp_one_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float 0.0,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp one <8 x float> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 0 1 0 1 0

       ret i32 0
}

define i32 @fcmp_one_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double 0.0,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp one <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 0 1 0 1 0
       ret i32 0
}

define i32 @fcmp_ord_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float 0.0,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp ord <8 x float> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 1 1 1 1 1

       ret i32 0
}

define i32 @fcmp_ord_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double 0.0,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp ord <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 0 0 0 1 1 1 1 1
       ret i32 0
}

define i32 @fcmp_ueq_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float 0.0,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp ueq <8 x float> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 1 0 1 0 1
       ret i32 0
}

define i32 @fcmp_ueq_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double 0.0,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp ueq <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 1 0 1 0 1
       ret i32 0
}

define i32 @fcmp_ugt_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       %fpinf = call float @get_fpinf()
       %fminf = call float @get_fminf()
       %fpdenorm = call float @get_fpdenorm()
       %fmdenorm = call float @get_fmdenorm()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.255e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp ugt <8 x float> %v4f1_8, %v4f2_8

       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 1 0 1 1 0
       ret i32 0
}

define i32 @fcmp_ugt_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       %fpinf = call double @get_dpinf()
       %fminf = call double @get_dminf()
       %fpdenorm = call double @get_dpdenorm()
       %fmdenorm = call double @get_dmdenorm()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.255e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp ugt <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 1 0 1 1 0
       ret i32 0
}

define i32 @fcmp_uge_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       %fpinf = call float @get_fpinf()
       %fminf = call float @get_fminf()
       %fpdenorm = call float @get_fpdenorm()
       %fmdenorm = call float @get_fmdenorm()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp uge <8 x float> %v4f1_8, %v4f2_8

       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 1 0 1 1 1
       ret i32 0
}

define i32 @fcmp_uge_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       %fpinf = call double @get_dpinf()
       %fminf = call double @get_dminf()
       %fpdenorm = call double @get_dpdenorm()
       %fmdenorm = call double @get_dmdenorm()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp uge <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 1 0 1 1 1
       ret i32 0
}

define i32 @fcmp_ult_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       %fpinf = call float @get_fpinf()
       %fminf = call float @get_fminf()
       %fpdenorm = call float @get_fpdenorm()
       %fmdenorm = call float @get_fmdenorm()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp ult <8 x float> %v4f1_8, %v4f2_8

       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 0 1 0 0 0
       ret i32 0
}

define i32 @fcmp_ult_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       %fpinf = call double @get_dpinf()
       %fminf = call double @get_dminf()
       %fpdenorm = call double @get_dpdenorm()
       %fmdenorm = call double @get_dmdenorm()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp ult <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 0 1 0 0 0
       ret i32 0
}

define i32 @fcmp_ule_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       %fpinf = call float @get_fpinf()
       %fminf = call float @get_fminf()
       %fpdenorm = call float @get_fpdenorm()
       %fmdenorm = call float @get_fmdenorm()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp ule <8 x float> %v4f1_8, %v4f2_8

       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 1 1 0 0 1
       ret i32 0
}

define i32 @fcmp_ule_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       %fpinf = call double @get_dpinf()
       %fminf = call double @get_dminf()
       %fpdenorm = call double @get_dpdenorm()
       %fmdenorm = call double @get_dmdenorm()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double %fpinf,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double %fpdenorm,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double %fminf,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double %fmdenorm,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp ule <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 1 1 0 0 1
       ret i32 0
}

define i32 @fcmp_une_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float 0.0,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp une <8 x float> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 0 1 0 1 0

       ret i32 0
}

define i32 @fcmp_une_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double 0.0,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp une <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 0 1 0 1 0
       ret i32 0
}


define i32 @fcmp_uno_float()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       
       %v4f1_1 = insertelement <8 x float> <float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %fnan, i32 0
       %v4f1_2 = insertelement <8 x float> %v4f1_1 , float %fnan,               i32 1
       %v4f1_3 = insertelement <8 x float> %v4f1_2 , float 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x float> %v4f1_3 , float 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x float> %v4f1_4 , float 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x float> %v4f1_5 , float 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x float> %v4f1_6 , float 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x float> %v4f1_7 , float 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>, float %fnan, i32 0
       %v4f2_2 = insertelement <8 x float> %v4f2_1 , float 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x float> %v4f2_2 , float %fnan,               i32 2
       %v4f2_4 = insertelement <8 x float> %v4f2_3 , float 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x float> %v4f2_4 , float 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x float> %v4f2_5 , float 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x float> %v4f2_6 , float 0.0,              i32 6
       %v4f2_8 = insertelement <8 x float> %v4f2_7 , float 5.0,        i32 7

       %res_float = fcmp uno <8 x float> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_float, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_float, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_float, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_float, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_float, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_float, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_float, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_float, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 0 0 0 0 0

       ret i32 0
}

define i32 @fcmp_uno_double()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %fnan = call double @get_dnan()
       
       %v4f1_1 = insertelement <8 x double> <double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0, double 0.0>, double %fnan, i32 0
       %v4f1_2 = insertelement <8 x double> %v4f1_1 , double %fnan,               i32 1
       %v4f1_3 = insertelement <8 x double> %v4f1_2 , double 3.456e+03,        i32 2
       %v4f1_4 = insertelement <8 x double> %v4f1_3 , double 3.256e+03,        i32 3
       %v4f1_5 = insertelement <8 x double> %v4f1_4 , double 5.0,                 i32 4
       %v4f1_6 = insertelement <8 x double> %v4f1_5 , double 5.0,                 i32 5
       %v4f1_7 = insertelement <8 x double> %v4f1_6 , double 1.0,                 i32 6
       %v4f1_8 = insertelement <8 x double> %v4f1_7 , double 5.0,        i32 7
       
       %v4f2_1 = insertelement <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, double %fnan, i32 0
       %v4f2_2 = insertelement <8 x double> %v4f2_1 , double 0.0,                 i32 1
       %v4f2_3 = insertelement <8 x double> %v4f2_2 , double %fnan,               i32 2
       %v4f2_4 = insertelement <8 x double> %v4f2_3 , double 3.256e+03,           i32 3
       %v4f2_5 = insertelement <8 x double> %v4f2_4 , double 6.0,                 i32 4
       %v4f2_6 = insertelement <8 x double> %v4f2_5 , double 5.0,                 i32 5
       %v4f2_7 = insertelement <8 x double> %v4f2_6 , double 0.0,              i32 6
       %v4f2_8 = insertelement <8 x double> %v4f2_7 , double 5.0,        i32 7

       %res_double = fcmp uno <8 x double> %v4f1_8, %v4f2_8
       
       %r_i8_1 = extractelement <8 x i1> %res_double, i32 0
       %r_i8_2 = extractelement <8 x i1> %res_double, i32 1
       %r_i8_3 = extractelement <8 x i1> %res_double, i32 2
       %r_i8_4 = extractelement <8 x i1> %res_double, i32 3
       %r_i8_5 = extractelement <8 x i1> %res_double, i32 4
       %r_i8_6 = extractelement <8 x i1> %res_double, i32 5
       %r_i8_7 = extractelement <8 x i1> %res_double, i32 6
       %r_i8_8 = extractelement <8 x i1> %res_double, i32 7

       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       %r_i32_i8_3 = zext i1 %r_i8_3 to i32
       %r_i32_i8_4 = zext i1 %r_i8_4 to i32
       %r_i32_i8_5 = zext i1 %r_i8_5 to i32
       %r_i32_i8_6 = zext i1 %r_i8_6 to i32
       %r_i32_i8_7 = zext i1 %r_i8_7 to i32
       %r_i32_i8_8 = zext i1 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 1 1 1 0 0 0 0 0
       ret i32 0
}

define i32 @main()
{
      call i32 @fcmp_false_float()
      call i32 @fcmp_false_double()
      call i32 @fcmp_true_float()
      call i32 @fcmp_true_double()
      call i32 @fcmp_oeq_float()
      call i32 @fcmp_oeq_double()
      call i32 @fcmp_ogt_float()
      call i32 @fcmp_ogt_double()
      call i32 @fcmp_oge_float()
      call i32 @fcmp_oge_double()
      call i32 @fcmp_olt_float()
      call i32 @fcmp_olt_double()
      call i32 @fcmp_ole_float()
      call i32 @fcmp_ole_double()
      call i32 @fcmp_one_float()
      call i32 @fcmp_one_double()
      call i32 @fcmp_ord_float()
      call i32 @fcmp_ord_double()
      call i32 @fcmp_ueq_float()
      call i32 @fcmp_ueq_double()
      call i32 @fcmp_ugt_float()
      call i32 @fcmp_ugt_double()
      call i32 @fcmp_uge_float()
      call i32 @fcmp_uge_double()
      call i32 @fcmp_ult_float()
      call i32 @fcmp_ult_double()
      call i32 @fcmp_ule_float()
      call i32 @fcmp_ule_double()
      call i32 @fcmp_une_float()
      call i32 @fcmp_une_double()
      call i32 @fcmp_uno_float()
      call i32 @fcmp_uno_double()
      
      ret i32 0
}

