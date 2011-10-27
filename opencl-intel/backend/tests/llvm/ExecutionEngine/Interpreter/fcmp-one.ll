; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32:64"

@r_int_1 = internal constant [6 x i8] c"  %d\0A\00"
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

declare i32 @printf(i8*, ...)


define i32 @fcmp_one_float()
{
       %ptr = getelementptr [6 x i8]* @r_int_1, i32 0, i32 0 ; to printf
       %fnan = call float @get_fnan()
       
       %res_float = fcmp one float %fnan, %fnan
       %r_i32_i8_1 = zext i1 %res_float to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1)
; CHECK: 0

       ret i32 0
}


define i32 @main()
{
      call i32 @fcmp_one_float()
      ret i32 0
}

