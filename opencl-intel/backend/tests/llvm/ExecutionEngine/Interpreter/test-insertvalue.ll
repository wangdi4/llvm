; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32:64"

@r_float_1 = internal constant [4 x i8] c" %f\00"
@r_int_1 = internal constant [4 x i8] c" %d\00"
@r_end = internal constant [2 x i8] c"\0A\00"
@r_float_8 = internal constant [26 x i8] c" %f %f %f %f %f %f %f %f\0A\00"
@r_int_8 = internal constant [26 x i8] c" %d %d %d %d %d %d %d %d\0A\00"
@r_float_4 = internal constant [14 x i8] c" %f %f %f %f\0A\00"
@r_int_4 = internal constant [14 x i8] c" %d %d %d %d\0A\00"


declare i32 @printf(i8*, ...)

; insert to the structure containing int, pointer to int, double, float
%ts_1 = type {i16, i32*, double, float}

define i32 @ins_str()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = alloca i32
    store i32 333, i32* %tmp1_1
    ; check for insert pointer
    %tmp1_2 = insertvalue %ts_1 {i16 123, i32* undef, double -1.0, float 2.0}, i32* %tmp1_1, 1

    %r1_3 = extractvalue %ts_1 %tmp1_2, 0
    %r1_4 = extractvalue %ts_1 %tmp1_2, 1
    %r1_5 = extractvalue %ts_1 %tmp1_2, 2
    %r1_6 = extractvalue %ts_1 %tmp1_2, 3

    %r2_1 = sext i16 %r1_3 to i32;
    %r2_2 = load i32* %r1_4;
    %r2_3 = fpext float %r1_6 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r1_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 123 333 -1.000000 2.000000

    ; check for insert int
    %tmp1_3 = insertvalue %ts_1 %tmp1_2, i16 -124, 0

    %r3_3 = extractvalue %ts_1 %tmp1_3, 0
    %r3_4 = extractvalue %ts_1 %tmp1_3, 1
    %r3_5 = extractvalue %ts_1 %tmp1_3, 2
    %r3_6 = extractvalue %ts_1 %tmp1_3, 3

    %r4_1 = sext i16 %r3_3 to i32;
    %r4_2 = load i32* %r3_4;
    %r4_3 = fpext float %r3_6 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r3_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r4_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: -124 333 -1.000000 2.000000

    ; check for insert double
    %tmp1_4 = insertvalue %ts_1 %tmp1_3, double 1.24, 2

    %r5_3 = extractvalue %ts_1 %tmp1_4, 0
    %r5_4 = extractvalue %ts_1 %tmp1_4, 1
    %r5_5 = extractvalue %ts_1 %tmp1_4, 2
    %r5_6 = extractvalue %ts_1 %tmp1_4, 3

    %r6_1 = sext i16 %r5_3 to i32;
    %r6_2 = load i32* %r5_4;
    %r6_3 = fpext float %r5_6 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r6_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r6_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r5_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r6_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: -124 333 1.240000 2.000000

    ; check for insert float
    %tmp1_5 = insertvalue %ts_1 %tmp1_4, float 22.0, 3

    %r7_3 = extractvalue %ts_1 %tmp1_5, 0
    %r7_4 = extractvalue %ts_1 %tmp1_5, 1
    %r7_5 = extractvalue %ts_1 %tmp1_5, 2
    %r7_6 = extractvalue %ts_1 %tmp1_5, 3

    %r8_1 = sext i16 %r7_3 to i32;
    %r8_2 = load i32* %r7_4;
    %r8_3 = fpext float %r7_6 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r8_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r8_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r7_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r8_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: -124 333 1.240000 22.000000

    ret i32 0
}

; insert to an embedded structure containing int, pointer to int, double, float
%ts_2 = type {{i16, i32*}, {double, float}}
define i32 @ins_str_embedded_1()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = alloca i32
    store i32 555, i32* %tmp1_1
    ; check for insert pointer to an embedded structure
    %tmp1_2 = insertvalue %ts_2 {{i16, i32*}{i16 -331, i32* undef}, {double, float}{double -17.0, float 28.0}}, i32* %tmp1_1, 0, 1

    %r1_3 = extractvalue %ts_2 %tmp1_2, 0, 0
    %r1_4 = extractvalue %ts_2 %tmp1_2, 0, 1
    %r1_5 = extractvalue %ts_2 %tmp1_2, 1, 0
    %r1_6 = extractvalue %ts_2 %tmp1_2, 1, 1

    %r2_1 = sext i16 %r1_3 to i32;
    %r2_2 = load i32* %r1_4;
    %r2_3 = fpext float %r1_6 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r1_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: -331 555 -17.000000 28.000000

    ; check for insert int to an embedded structure
    %tmp1_3 = insertvalue %ts_2 %tmp1_2, i16 421, 0, 0

    %r3_3 = extractvalue %ts_2 %tmp1_3, 0, 0
    %r3_4 = extractvalue %ts_2 %tmp1_3, 0, 1
    %r3_5 = extractvalue %ts_2 %tmp1_3, 1, 0
    %r3_6 = extractvalue %ts_2 %tmp1_3, 1, 1

    %r4_1 = sext i16 %r3_3 to i32;
    %r4_2 = load i32* %r3_4;
    %r4_3 = fpext float %r3_6 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r3_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r4_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 421 555 -17.000000 28.000000

    ; check for insert double to an embedded structure
    %tmp1_4 = insertvalue %ts_2 %tmp1_3, double 24.1, 1, 0

    %r5_3 = extractvalue %ts_2 %tmp1_4, 0, 0
    %r5_4 = extractvalue %ts_2 %tmp1_4, 0, 1
    %r5_5 = extractvalue %ts_2 %tmp1_4, 1, 0
    %r5_6 = extractvalue %ts_2 %tmp1_4, 1, 1

    %r6_1 = sext i16 %r5_3 to i32;
    %r6_2 = load i32* %r5_4;
    %r6_3 = fpext float %r5_6 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r6_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r6_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r5_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r6_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 421 555 24.100000 28.000000

    ; check for insert float to an embedded structure
    %tmp1_5 = insertvalue %ts_2 %tmp1_4, float -104.0, 1, 1

    %r7_3 = extractvalue %ts_2 %tmp1_5, 0, 0
    %r7_4 = extractvalue %ts_2 %tmp1_5, 0, 1
    %r7_5 = extractvalue %ts_2 %tmp1_5, 1, 0
    %r7_6 = extractvalue %ts_2 %tmp1_5, 1, 1

    %r8_1 = sext i16 %r7_3 to i32;
    %r8_2 = load i32* %r7_4;
    %r8_3 = fpext float %r7_6 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r8_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r8_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r7_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r8_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 421 555 24.100000 -104.000000

    ret i32 0
}

; insert structure to the structure
define i32 @ins_str_embedded_2()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = insertvalue {{i16, i8}, {double, float}} undef, {i16, i8} {i16 1, i8 2}, 0
    %tmp1_2 = insertvalue {{i16, i8}, {double, float}} %tmp1_1, {double, float} {double 3.0, float 4.0}, 1

    %r1_1 = extractvalue {{i16, i8}, {double, float}} %tmp1_2, 0, 0
    %r1_2 = extractvalue {{i16, i8}, {double, float}} %tmp1_2, 0, 1
    %r1_3 = extractvalue {{i16, i8}, {double, float}} %tmp1_2, 1, 0
    %r1_4 = extractvalue {{i16, i8}, {double, float}} %tmp1_2, 1, 1

    %r2_1 = sext i16 %r1_1 to i32;
    %r2_2 = sext i8 %r1_2 to i32;
    %r2_3 = fpext float %r1_4 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r1_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 1 2 3.000000 4.000000

    ret i32 0
}

; insert to an structure containing arrais of ints, pointer to ints, doubles, floats
%ts_3 = type {[2 x i16], [2 x i32*], [2 x double], [2 x float]}
define i32 @ins_str_arr()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_0 = alloca i32
    store i32 555, i32* %tmp1_0
    %tmp1_1 = alloca i32
    store i32 -333, i32* %tmp1_1
    ; check for insert pointer to an array embedded structure
    %tmp1_2 = insertvalue %ts_3 {[2 x i16][i16 -331, i16 332],[2 x i32*][i32* undef, i32* undef], [2 x double][double -17.0, double 18.0],[2 x float][float 28.0, float -10.0]}, i32* %tmp1_0, 1, 0
    %tmp1_3 = insertvalue %ts_3 %tmp1_2, i32* %tmp1_0, 1, 1

    %r1_1 = extractvalue %ts_3 %tmp1_3, 0, 0
    %r1_2 = extractvalue %ts_3 %tmp1_3, 0, 1
    %r1_3 = extractvalue %ts_3 %tmp1_3, 1, 0
    %r1_4 = extractvalue %ts_3 %tmp1_3, 1, 1
    %r1_5 = extractvalue %ts_3 %tmp1_3, 2, 0
    %r1_6 = extractvalue %ts_3 %tmp1_3, 2, 1
    %r1_7 = extractvalue %ts_3 %tmp1_3, 3, 0
    %r1_8 = extractvalue %ts_3 %tmp1_3, 3, 1

    %r2_1 = sext i16 %r1_1 to i32;
    %r2_2 = sext i16 %r1_2 to i32;
    %r2_3 = load i32* %r1_3;
    %r2_4 = load i32* %r1_4;
    %r2_5 = fpext float %r1_7 to double
    %r2_6 = fpext float %r1_8 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r1_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r1_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: -331 332 555 555 -17.000000 18.000000 28.000000 -10.000000

    ; check for insert int to an array embedded to structure
    %tmp1_4 = insertvalue %ts_3 %tmp1_3, i16 4, 0, 0

    %r3_1 = extractvalue %ts_3 %tmp1_4, 0, 0
    %r3_2 = extractvalue %ts_3 %tmp1_4, 0, 1
    %r3_3 = extractvalue %ts_3 %tmp1_4, 1, 0
    %r3_4 = extractvalue %ts_3 %tmp1_4, 1, 1
    %r3_5 = extractvalue %ts_3 %tmp1_4, 2, 0
    %r3_6 = extractvalue %ts_3 %tmp1_4, 2, 1
    %r3_7 = extractvalue %ts_3 %tmp1_4, 3, 0
    %r3_8 = extractvalue %ts_3 %tmp1_4, 3, 1

    %r4_1 = sext i16 %r3_1 to i32;
    %r4_2 = sext i16 %r3_2 to i32;
    %r4_3 = load i32* %r3_3;
    %r4_4 = load i32* %r3_4;
    %r4_5 = fpext float %r3_7 to double
    %r4_6 = fpext float %r3_8 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r3_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r3_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r4_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r4_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 4 332 555 555 -17.000000 18.000000 28.000000 -10.000000

    ; check for insert double to an array embedded to structure
    %tmp1_5 = insertvalue %ts_3 %tmp1_4, double 777.7, 2, 1

    %r5_1 = extractvalue %ts_3 %tmp1_5, 0, 0
    %r5_2 = extractvalue %ts_3 %tmp1_5, 0, 1
    %r5_3 = extractvalue %ts_3 %tmp1_5, 1, 0
    %r5_4 = extractvalue %ts_3 %tmp1_5, 1, 1
    %r5_5 = extractvalue %ts_3 %tmp1_5, 2, 0
    %r5_6 = extractvalue %ts_3 %tmp1_5, 2, 1
    %r5_7 = extractvalue %ts_3 %tmp1_5, 3, 0
    %r5_8 = extractvalue %ts_3 %tmp1_5, 3, 1

    %r6_1 = sext i16 %r5_1 to i32;
    %r6_2 = sext i16 %r5_2 to i32;
    %r6_3 = load i32* %r5_3;
    %r6_4 = load i32* %r5_4;
    %r6_5 = fpext float %r5_7 to double
    %r6_6 = fpext float %r5_8 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r6_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r6_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r6_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r6_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r5_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r5_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r6_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r6_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 4 332 555 555 -17.000000 777.700000 28.000000 -10.000000

    ; check for insert float to an array embedded to structure
    %tmp1_6 = insertvalue %ts_3 %tmp1_5, float 3.12e+2, 3, 1

    %r7_1 = extractvalue %ts_3 %tmp1_6, 0, 0
    %r7_2 = extractvalue %ts_3 %tmp1_6, 0, 1
    %r7_3 = extractvalue %ts_3 %tmp1_6, 1, 0
    %r7_4 = extractvalue %ts_3 %tmp1_6, 1, 1
    %r7_5 = extractvalue %ts_3 %tmp1_6, 2, 0
    %r7_6 = extractvalue %ts_3 %tmp1_6, 2, 1
    %r7_7 = extractvalue %ts_3 %tmp1_6, 3, 0
    %r7_8 = extractvalue %ts_3 %tmp1_6, 3, 1

    %r8_1 = sext i16 %r7_1 to i32;
    %r8_2 = sext i16 %r7_2 to i32;
    %r8_3 = load i32* %r7_3;
    %r8_4 = load i32* %r7_4;
    %r8_5 = fpext float %r7_7 to double
    %r8_6 = fpext float %r7_8 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r8_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r8_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r8_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r8_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r7_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r7_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r8_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r8_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 4 332 555 555 -17.000000 777.700000 28.000000 312.000000

    ret i32 0
}

define i32 @ins_arr()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_f_4 = getelementptr [14 x i8]* @r_float_4, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    ; check for insert int to array
    %tmp1_1 = insertvalue [3 x i32] [i32 -331, i32 332, i32 12], i32 1, 1

    %r1_1 = extractvalue [3 x i32] %tmp1_1, 0
    %r1_2 = extractvalue [3 x i32] %tmp1_1, 1
    %r1_3 = extractvalue [3 x i32] %tmp1_1, 2

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r1_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r1_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r1_3)

    %tmp1_2 = insertvalue [3 x i32] %tmp1_1, i32 0, 0
    %tmp1_3 = insertvalue [3 x i32] %tmp1_2, i32 2, 2

    %r2_1 = extractvalue [3 x i32] %tmp1_3, 0
    %r2_2 = extractvalue [3 x i32] %tmp1_3, 1
    %r2_3 = extractvalue [3 x i32] %tmp1_3, 2

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: -331 1 12 0 1 2

    %tmp2_1 = insertvalue [4 x double] [double -3.31, double 33.2, double 1.2, double 0.7 ], double 3.0, 3

    %r3_1 = extractvalue [4 x double] %tmp2_1, 0
    %r3_2 = extractvalue [4 x double] %tmp2_1, 1
    %r3_3 = extractvalue [4 x double] %tmp2_1, 2
    %r3_4 = extractvalue [4 x double] %tmp2_1, 3
    call i32 (i8*, ...)* @printf(i8* %ptr_f_4, double %r3_1, double %r3_2, double %r3_3, double %r3_4)
; CHECK: -3.310000 33.200000 1.200000 3.000000

    ; check for insert double to array
    %tmp2_2 = insertvalue [4 x double] %tmp2_1, double 0.0, 0
    %tmp2_3 = insertvalue [4 x double] %tmp2_2, double 1.0, 1
    %tmp2_4 = insertvalue [4 x double] %tmp2_3, double 2.0, 2

    %r4_1 = extractvalue [4 x double] %tmp2_4, 0
    %r4_2 = extractvalue [4 x double] %tmp2_4, 1
    %r4_3 = extractvalue [4 x double] %tmp2_4, 2
    %r4_4 = extractvalue [4 x double] %tmp2_4, 3
    call i32 (i8*, ...)* @printf(i8* %ptr_f_4, double %r4_1, double %r4_2, double %r4_3, double %r4_4)
; CHECK: 0.000000 1.000000 2.000000 3.000000

    ; check for insert float to array
    %tmp3_1 = insertvalue [3 x float] [float 3.12e+2, float 3.12e+3, float 3.12e+4], float 0.0, 0

    %r5_1 = extractvalue [3 x float] %tmp3_1, 0
    %r5_2 = extractvalue [3 x float] %tmp3_1, 1
    %r5_3 = extractvalue [3 x float] %tmp3_1, 2
    %r5_1d = fpext float %r5_1 to double
    %r5_2d = fpext float %r5_2 to double
    %r5_3d = fpext float %r5_3 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r5_1d)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r5_2d)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r5_3d)

    %tmp3_2 = insertvalue [3 x float] %tmp3_1, float 1.0, 1
    %tmp3_3 = insertvalue [3 x float] %tmp3_2, float 2.0, 2

    %r6_1 = extractvalue [3 x float] %tmp3_3, 0
    %r6_2 = extractvalue [3 x float] %tmp3_3, 1
    %r6_3 = extractvalue [3 x float] %tmp3_3, 2
    %r6_1d = fpext float %r6_1 to double
    %r6_2d = fpext float %r6_2 to double
    %r6_3d = fpext float %r6_3 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r6_1d)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r6_2d)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r6_3d)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 0.000000 3120.000000 31200.000000 0.000000 1.000000 2.000000

    ; check for insert pointer to array
    %tmp_p_1 = alloca i8
    store i8 -2, i8* %tmp_p_1
    %tmp_p_2 = alloca i8
    store i8 3, i8* %tmp_p_2
    %tmp_p_3 = alloca i8
    store i8 -4, i8* %tmp_p_3
    %tmp4_1 = insertvalue [2 x i8*] undef, i8* %tmp_p_1, 0
    %tmp4_2 = insertvalue [2 x i8*] %tmp4_1, i8* %tmp_p_2, 1
    %r7_1 = extractvalue [2 x i8*] %tmp4_2, 0
    %r7_2 = extractvalue [2 x i8*] %tmp4_2, 1
    %r8_1 = load i8* %r7_1;
    %r8_2 = load i8* %r7_2;
    %r9_1 = sext i8 %r8_1 to i32;
    %r9_2 = sext i8 %r8_2 to i32;
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r9_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r9_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: -2 3

    %tmp4_3 = insertvalue [2 x i8*] %tmp4_2, i8* %tmp_p_3, 1
    %r10_1 = extractvalue [2 x i8*] %tmp4_3, 0
    %r10_2 = extractvalue [2 x i8*] %tmp4_3, 1
    %r11_1 = load i8* %r10_1;
    %r11_2 = load i8* %r10_2;
    %r12_1 = sext i8 %r11_1 to i32;
    %r12_2 = sext i8 %r11_2 to i32;
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r12_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r12_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: -2 -4

    ret i32 0
}

define i32 @ins_arr_embedded_1()
{
%ptr_i = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

    %tmp1_1 = insertvalue [2 x [2 x [2 x i32]]] [[2 x [2 x i32]][[2 x i32][i32 12, i32 14],[2 x i32][i32 13, i32 15]],[2 x [2 x i32]][[2 x i32][i32 212, i32 214],[2 x i32][i32 213, i32 215]]], i32 -401, 1, 0, 1
    %r1_1 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_1, 0, 0, 0
    %r1_2 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_1, 0, 0, 1
    %r1_3 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_1, 0, 1, 0
    %r1_4 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_1, 0, 1, 1
    %r1_5 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_1, 1, 0, 0
    %r1_6 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_1, 1, 0, 1
    %r1_7 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_1, 1, 1, 0
    %r1_8 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_1, 1, 1, 1
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r1_1, i32 %r1_2, i32 %r1_3, i32 %r1_4, i32 %r1_5, i32 %r1_6, i32 %r1_7, i32 %r1_8)
; CHECK: 12 14 13 15 212 -401 213 215

    %tmp1_2 = insertvalue [2 x [2 x [2 x i32]]] %tmp1_1, i32 -402, 0, 0, 0
    %tmp1_3 = insertvalue [2 x [2 x [2 x i32]]] %tmp1_2, i32 -403, 0, 0, 1
    %tmp1_4 = insertvalue [2 x [2 x [2 x i32]]] %tmp1_3, i32 -404, 0, 1, 0
    %tmp1_5 = insertvalue [2 x [2 x [2 x i32]]] %tmp1_4, i32 -405, 0, 1, 1
    %tmp1_6 = insertvalue [2 x [2 x [2 x i32]]] %tmp1_5, i32 -406, 1, 0, 0
    %tmp1_7 = insertvalue [2 x [2 x [2 x i32]]] %tmp1_6, i32 -407, 1, 1, 0
    %tmp1_8 = insertvalue [2 x [2 x [2 x i32]]] %tmp1_7, i32 -408, 1, 1, 1

    %r2_1 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_8, 0, 0, 0
    %r2_2 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_8, 0, 0, 1
    %r2_3 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_8, 0, 1, 0
    %r2_4 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_8, 0, 1, 1
    %r2_5 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_8, 1, 0, 0
    %r2_6 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_8, 1, 0, 1
    %r2_7 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_8, 1, 1, 0
    %r2_8 = extractvalue [2 x [2 x [2 x i32]]] %tmp1_8, 1, 1, 1

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_1, i32 %r2_2, i32 %r2_3, i32 %r2_4, i32 %r2_5, i32 %r2_6, i32 %r2_7, i32 %r2_8)
; CHECK: -402 -403 -404 -405 -406 -401 -407 -408

    ret i32 0
}

; insert array to the array
define i32 @ins_arr_embedded_2()
{
%ptr_i = getelementptr [14 x i8]* @r_int_4, i32 0, i32 0 ; to printf

    %tmp1_1 = insertvalue [2 x [2 x i32]][[2 x i32][i32 1, i32 2], [2 x i32][i32 3, i32 4]], [2 x i32][i32 5, i32 6],  0
    %tmp1_2 = insertvalue [2 x [2 x i32]] %tmp1_1, [2 x i32][i32 7, i32 8],  1

    %r1_1 = extractvalue [2 x [2 x i32]] %tmp1_2, 0, 0
    %r1_2 = extractvalue [2 x [2 x i32]] %tmp1_2, 0, 1
    %r1_3 = extractvalue [2 x [2 x i32]] %tmp1_2, 1, 0
    %r1_4 = extractvalue [2 x [2 x i32]] %tmp1_2, 1, 1

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r1_1, i32 %r1_2, i32 %r1_3, i32 %r1_4)
; CHECK: 5 6 7 8

    ret i32 0
}

; insert to the structure containing int, pointer to int, double, float
%ts_4 = type {i16, i32*, double, float}

define i32 @ins_arr_str()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp_p_1 = alloca i32
    store i32 33, i32* %tmp_p_1
    %tmp_p_2 = alloca i32
    store i32 -44, i32* %tmp_p_2
    %tmp_p_3 = alloca i32
    store i32 55, i32* %tmp_p_3
    %tmp_p_4 = alloca i32
    store i32 -66, i32* %tmp_p_4

    %tmp1_1 = insertvalue [2 x %ts_4][%ts_4 {i16 1, i32* undef, double -2.0, float 3.0}, %ts_4 {i16 4, i32* undef, double -5.0, float 6.0}], i32* %tmp_p_1, 0, 1
    %tmp1_2 = insertvalue [2 x %ts_4] %tmp1_1, i32* %tmp_p_2, 1, 1

    %tmp1_3 = insertvalue [2 x %ts_4] %tmp1_2, i16 7, 0, 0
    %tmp1_4 = insertvalue [2 x %ts_4] %tmp1_3, i32* %tmp_p_3, 1, 1
    %tmp1_5 = insertvalue [2 x %ts_4] %tmp1_4, double -8.0, 0, 2
    %tmp1_6 = insertvalue [2 x %ts_4] %tmp1_5, float 9.0, 1, 3

    %r1_1 = extractvalue [2 x %ts_4] %tmp1_6, 0, 0
    %r1_2 = extractvalue [2 x %ts_4] %tmp1_6, 0, 1
    %r1_3 = extractvalue [2 x %ts_4] %tmp1_6, 0, 2
    %r1_4 = extractvalue [2 x %ts_4] %tmp1_6, 0, 3
    %r1_5 = extractvalue [2 x %ts_4] %tmp1_6, 1, 0
    %r1_6 = extractvalue [2 x %ts_4] %tmp1_6, 1, 1
    %r1_7 = extractvalue [2 x %ts_4] %tmp1_6, 1, 2
    %r1_8 = extractvalue [2 x %ts_4] %tmp1_6, 1, 3
    %r2_1 = sext i16 %r1_1 to i32
    %r2_2 = load i32* %r1_2;
    %r2_4 = fpext float %r1_4 to double
    %r2_5 = sext i16 %r1_5 to i32
    %r2_6 = load i32* %r1_6;
    %r2_8 = fpext float %r1_8 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r1_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r1_7)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_8)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 7 33 -8.000000 3.000000 4 55 -5.000000 9.000000

    %tmp1_7 = insertvalue [2 x %ts_4] %tmp1_6, i16 10, 1, 0
    %tmp1_8 = insertvalue [2 x %ts_4] %tmp1_7, i32* %tmp_p_4, 0, 1
    %tmp1_9 = insertvalue [2 x %ts_4] %tmp1_8, double -11.0, 1, 2
    %tmp1_10 = insertvalue [2 x %ts_4] %tmp1_9, float 12.0, 0, 3

    %r3_1 = extractvalue [2 x %ts_4] %tmp1_10, 0, 0
    %r3_2 = extractvalue [2 x %ts_4] %tmp1_10, 0, 1
    %r3_3 = extractvalue [2 x %ts_4] %tmp1_10, 0, 2
    %r3_4 = extractvalue [2 x %ts_4] %tmp1_10, 0, 3
    %r3_5 = extractvalue [2 x %ts_4] %tmp1_10, 1, 0
    %r3_6 = extractvalue [2 x %ts_4] %tmp1_10, 1, 1
    %r3_7 = extractvalue [2 x %ts_4] %tmp1_10, 1, 2
    %r3_8 = extractvalue [2 x %ts_4] %tmp1_10, 1, 3
    %r4_1 = sext i16 %r3_1 to i32
    %r4_2 = load i32* %r3_2;
    %r4_4 = fpext float %r3_4 to double
    %r4_5 = sext i16 %r3_5 to i32
    %r4_6 = load i32* %r3_6;
    %r4_8 = fpext float %r3_8 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r3_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r4_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r4_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r3_7)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r4_8)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 7 -66 -8.000000 12.000000 10 55 -11.000000 9.000000
    ret i32 0
}

%ts_11 = type {i16, <2 x double>, float}
define i32 @ins_str_vector()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1 = insertvalue %ts_11 {i16 123, <2 x double> <double 1.0, double 2.0>, float 3.0}, <2 x double> <double 3.0, double -4.0>, 1

    %r1_1 = extractvalue %ts_11 %tmp1, 0
    %r1_2 = extractvalue %ts_11 %tmp1, 1
    %r1_3 = extractvalue %ts_11 %tmp1, 2

    %r2_1 = sext i16 %r1_1 to i32;
    %r2_2 = extractelement <2 x double> %r1_2, i32 0
    %r2_3 = extractelement <2 x double> %r1_2, i32 1
    %r2_4 = fpext float %r1_3 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 123 3.000000 -4.000000 3.000000

    ret i32 0
}

%ts_12 = type {<2 x double>, float}
%ts_13 = type {i16, %ts_12}
define i32 @ins_str_embedded_vector()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1 = insertvalue %ts_13 {i16 234, %ts_12 {<2 x double> <double 10.0, double 20.0>, float 30.0}}, <2 x double> <double 30.0, double -40.0>, 1, 0

    %r1_1 = extractvalue %ts_13 %tmp1, 0
    %r1_2 = extractvalue %ts_13 %tmp1, 1, 0
    %r1_3 = extractvalue %ts_13 %tmp1, 1, 1

    %r2_1 = sext i16 %r1_1 to i32;
    %r2_2 = extractelement <2 x double> %r1_2, i32 0
    %r2_3 = extractelement <2 x double> %r1_2, i32 1
    %r2_4 = fpext float %r1_3 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r2_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 234 30.000000 -40.000000 30.000000

    ret i32 0
}

define i32 @main()
{
    call i32 @ins_str()
    call i32 @ins_str_embedded_1()
    call i32 @ins_str_embedded_2()
    call i32 @ins_str_arr()

    call i32 @ins_arr()
    call i32 @ins_arr_embedded_1()
    call i32 @ins_arr_embedded_2()
    call i32 @ins_arr_str()

    call i32 @ins_str_vector()
    call i32 @ins_str_embedded_vector()

    ret i32 0
}

