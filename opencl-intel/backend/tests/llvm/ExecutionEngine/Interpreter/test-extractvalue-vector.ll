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

; extract from struct containing scalars and vector {i32, <2 x double>, i8}
define i32 @ext_str_1()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue {i32, <2 x double>, i8} {i32 12, <2 x double> <double 14.5, double -33.3>, i8 1}, 0
    %tmp1_2 = extractvalue {i32, <2 x double>, i8} {i32 12, <2 x double> <double 14.5, double -33.3>, i8 1}, 1
    %tmp1_3 = extractvalue {i32, <2 x double>, i8} {i32 12, <2 x double> <double 14.5, double -33.3>, i8 1}, 2

    %tmp2_1 = extractelement <2 x double> %tmp1_2, i32 0
    %tmp2_2 = extractelement <2 x double> %tmp1_2, i32 1

    %tmp2_3 = sext i8 %tmp1_3 to i32

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 12 14.500000 -33.300000 1

    ret i32 0
}

; extract from struct containing scalars and vector {<4 x i8>, double, <3 x float>}
define i32 @ext_str_2()
{
%ptr_i = getelementptr [14 x i8]* @r_int_4, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue {<4 x i8>, double, <3 x float>} {<4 x i8><i8 0,i8 -1,i8 2,i8 -3>, double 917.5, <3 x float> <float 2.0, float -3.0, float 4.0>}, 0
    %tmp1_2 = extractvalue {<4 x i8>, double, <3 x float>} {<4 x i8><i8 0,i8 -1,i8 2,i8 -3>, double 917.5, <3 x float> <float 2.0, float -3.0, float 4.0>}, 1
    %tmp1_3 = extractvalue {<4 x i8>, double, <3 x float>} {<4 x i8><i8 0,i8 -1,i8 2,i8 -3>, double 917.5, <3 x float> <float 2.0, float -3.0, float 4.0>}, 2

    %tmp2_1 = extractelement <4 x i8> %tmp1_1, i32 0
    %tmp2_2 = extractelement <4 x i8> %tmp1_1, i32 1
    %tmp2_3 = extractelement <4 x i8> %tmp1_1, i32 2
    %tmp2_4 = extractelement <4 x i8> %tmp1_1, i32 3

    %tmp3_1 = sext i8 %tmp2_1 to i32
    %tmp3_2 = sext i8 %tmp2_2 to i32
    %tmp3_3 = sext i8 %tmp2_3 to i32
    %tmp3_4 = sext i8 %tmp2_4 to i32

    %tmp2_5 = extractelement <3 x float> %tmp1_3, i32 0
    %tmp2_6 = extractelement <3 x float> %tmp1_3, i32 0
    %tmp2_7 = extractelement <3 x float> %tmp1_3, i32 0

    %tmp3_5 = fpext float %tmp2_5 to double
    %tmp3_6 = fpext float %tmp2_6 to double
    %tmp3_7 = fpext float %tmp2_7 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp3_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp3_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp3_7)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_1, i32 %tmp3_2, i32 %tmp3_3, i32 %tmp3_4)
; CHECK: 2.000000 2.000000 2.000000 0 -1 2 -3

    ret i32 0
}

; extract from struct with embedded structures containing vectors and scalars
; {{i32, <2 x double>},{<3 x i8>, i16}}
%t_v1 = type <2 x double>
%t_v2 = type <3 x i8>
%t_s1 = type {i32, %t_v1}
%t_s2 = type {%t_v2, i16}
define i32 @ext_str_embedded()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue {%t_s1,%t_s2} {%t_s1 {i32 17, %t_v1 <double 18.0, double 19.0>},%t_s2 { %t_v2 <i8 20, i8 21, i8 22>, i16 23}}, 0, 0
    %tmp1_2 = extractvalue {%t_s1,%t_s2} {%t_s1 {i32 17, %t_v1 <double 18.0, double 19.0>},%t_s2 { %t_v2 <i8 20, i8 21, i8 22>, i16 23}}, 0, 1
    %tmp1_3 = extractvalue {%t_s1,%t_s2} {%t_s1 {i32 17, %t_v1 <double 18.0, double 19.0>},%t_s2 { %t_v2 <i8 20, i8 21, i8 22>, i16 23}}, 1, 0
    %tmp1_4 = extractvalue {%t_s1,%t_s2} {%t_s1 {i32 17, %t_v1 <double 18.0, double 19.0>},%t_s2 { %t_v2 <i8 20, i8 21, i8 22>, i16 23}}, 1, 1

    %tmp2_1 = extractelement %t_v1 %tmp1_2, i32 0;
    %tmp2_2 = extractelement %t_v1 %tmp1_2, i32 1;
    %tmp2_3 = extractelement %t_v2 %tmp1_3, i32 0;
    %tmp2_4 = extractelement %t_v2 %tmp1_3, i32 1;
    %tmp2_5 = extractelement %t_v2 %tmp1_3, i32 2;

    %tmp2_6 = sext i16 %tmp1_4 to i32;

    %tmp3_1 = sext i8 %tmp2_3 to i32;
    %tmp3_2 = sext i8 %tmp2_4 to i32;
    %tmp3_3 = sext i8 %tmp2_5 to i32;

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 17 18.000000 19.000000 20 21 22 23

    ret i32 0
}

; extract from struct containing array of vectors and scalar
; {[2 x <2 x i32>], double}
%t_a1 = type [2 x <2 x i32>]
%t_s3 = type {%t_a1, double}
define i32 @ext_str_arr()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue %t_s3 { %t_a1 [<2 x i32> <i32 917, i32 919>, <2 x i32> <i32 -18, i32 -20>], double 19.1}, 0, 0
    %tmp1_2 = extractvalue %t_s3 { %t_a1 [<2 x i32> <i32 917, i32 919>, <2 x i32> <i32 -18, i32 -20>], double 19.1}, 0, 1
    %tmp1_3 = extractvalue %t_s3 { %t_a1 [<2 x i32> <i32 917, i32 919>, <2 x i32> <i32 -18, i32 -20>], double 19.1}, 1

    %tmp2_1 = extractelement <2 x i32> %tmp1_1, i32 0;
    %tmp2_2 = extractelement <2 x i32> %tmp1_1, i32 1;
    %tmp2_3 = extractelement <2 x i32> %tmp1_2, i32 0;
    %tmp2_4 = extractelement <2 x i32> %tmp1_2, i32 1;

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 917 919 -18 -20 19.100000

    ret i32 0
}

; extract from structure containing vector of double values and pointer to vector of int values
define i32 @ext_str_point()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = alloca <3 x i32>
    store <3 x i32> <i32 111, i32 222, i32 333>, <3 x i32>* %tmp1_1
    %tmp1_2 = insertvalue {<3 x i32>*, <3 x double>} {<3 x i32>* undef, <3 x double> <double -1.0, double 2.0, double -3.5>}, <3 x i32>* %tmp1_1, 0
    %tmp1_3 = extractvalue {<3 x i32>*, <3 x double>} %tmp1_2, 0
    %tmp1_4 = extractvalue {<3 x i32>*, <3 x double>} %tmp1_2, 1

    %tmp2_1 = load <3 x i32>* %tmp1_3;
    %tmp3_1 = extractelement <3 x i32> %tmp2_1, i32 0;
    %tmp3_2 = extractelement <3 x i32> %tmp2_1, i32 1;
    %tmp3_3 = extractelement <3 x i32> %tmp2_1, i32 2;
    %tmp3_4 = extractelement <3 x double> %tmp1_4, i32 0;
    %tmp3_5 = extractelement <3 x double> %tmp1_4, i32 1;
    %tmp3_6 = extractelement <3 x double> %tmp1_4, i32 2

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp3_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp3_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp3_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 111 222 333 -1.000000 2.000000 -3.500000

    ret i32 0
}

; extract from array of vectors of double type
define i32 @ext_arr_double()
{
%ptr_f = getelementptr [26 x i8]* @r_float_8, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue [2 x <4 x double>] [<4 x double> <double 1.0, double -2.0, double -3.0, double 4.0>, <4 x double> <double 11.0, double -12.0, double -13.0, double 14.0>], 0
    %tmp1_2 = extractvalue [2 x <4 x double>] [<4 x double> <double 1.0, double -2.0, double -3.0, double 4.0>, <4 x double> <double 11.0, double -12.0, double -13.0, double 14.0>], 1

    %tmp2_1 = extractelement <4 x double> %tmp1_1, i32 0
    %tmp2_2 = extractelement <4 x double> %tmp1_1, i32 1
    %tmp2_3 = extractelement <4 x double> %tmp1_1, i32 2
    %tmp2_4 = extractelement <4 x double> %tmp1_1, i32 3

    %tmp2_5 = extractelement <4 x double> %tmp1_2, i32 0
    %tmp2_6 = extractelement <4 x double> %tmp1_2, i32 1
    %tmp2_7 = extractelement <4 x double> %tmp1_2, i32 2
    %tmp2_8 = extractelement <4 x double> %tmp1_2, i32 3

    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp2_1, double %tmp2_2, double %tmp2_3, double %tmp2_4, double %tmp2_5, double %tmp2_6, double %tmp2_7, double %tmp2_8)
; CHECK: 1.000000 -2.000000 -3.000000 4.000000 11.000000 -12.000000 -13.000000 14.000000

    ret i32 0
}

; extract from array of vectors of float type
define i32 @ext_arr_float()
{
%ptr_f = getelementptr [26 x i8]* @r_float_8, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue [2 x <4 x float>] [<4 x float> <float 21.0, float -22.0, float -23.0, float 24.0>, <4 x float> <float 110.0, float -120.0, float -130.0, float 140.0>], 0
    %tmp1_2 = extractvalue [2 x <4 x float>] [<4 x float> <float 21.0, float -22.0, float -23.0, float 24.0>, <4 x float> <float 110.0, float -120.0, float -130.0, float 140.0>], 1

    %tmp2_1 = extractelement <4 x float> %tmp1_1, i32 0
    %tmp2_2 = extractelement <4 x float> %tmp1_1, i32 1
    %tmp2_3 = extractelement <4 x float> %tmp1_1, i32 2
    %tmp2_4 = extractelement <4 x float> %tmp1_1, i32 3

    %tmp2_5 = extractelement <4 x float> %tmp1_2, i32 0
    %tmp2_6 = extractelement <4 x float> %tmp1_2, i32 1
    %tmp2_7 = extractelement <4 x float> %tmp1_2, i32 2
    %tmp2_8 = extractelement <4 x float> %tmp1_2, i32 3

    %tmp3_1 = fpext float %tmp2_1 to double
    %tmp3_2 = fpext float %tmp2_2 to double
    %tmp3_3 = fpext float %tmp2_3 to double
    %tmp3_4 = fpext float %tmp2_4 to double
    %tmp3_5 = fpext float %tmp2_5 to double
    %tmp3_6 = fpext float %tmp2_6 to double
    %tmp3_7 = fpext float %tmp2_7 to double
    %tmp3_8 = fpext float %tmp2_8 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp3_1, double %tmp3_2, double %tmp3_3, double %tmp3_4, double %tmp3_5, double %tmp3_6, double %tmp3_7, double %tmp3_8)
; CHECK: 21.000000 -22.000000 -23.000000 24.000000 110.000000 -120.000000 -130.000000 140.000000

    ret i32 0
}

; extract from array of vectors of int type
define i32 @ext_arr_int()
{
%ptr_i = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue [4 x <2 x i8>] [<2 x i8> <i8 1, i8 -2>, <2 x i8> <i8 11, i8 -12>, <2 x i8> <i8 21, i8 -22>, <2 x i8> <i8 31, i8 -32>], 0
    %tmp1_2 = extractvalue [4 x <2 x i8>] [<2 x i8> <i8 1, i8 -2>, <2 x i8> <i8 11, i8 -12>, <2 x i8> <i8 21, i8 -22>, <2 x i8> <i8 31, i8 -32>], 1
    %tmp1_3 = extractvalue [4 x <2 x i8>] [<2 x i8> <i8 1, i8 -2>, <2 x i8> <i8 11, i8 -12>, <2 x i8> <i8 21, i8 -22>, <2 x i8> <i8 31, i8 -32>], 2
    %tmp1_4 = extractvalue [4 x <2 x i8>] [<2 x i8> <i8 1, i8 -2>, <2 x i8> <i8 11, i8 -12>, <2 x i8> <i8 21, i8 -22>, <2 x i8> <i8 31, i8 -32>], 3

    %tmp2_1 = extractelement <2 x i8> %tmp1_1, i32 0
    %tmp2_2 = extractelement <2 x i8> %tmp1_1, i32 1
    %tmp2_3 = extractelement <2 x i8> %tmp1_2, i32 0
    %tmp2_4 = extractelement <2 x i8> %tmp1_2, i32 1

    %tmp2_5 = extractelement <2 x i8> %tmp1_3, i32 0
    %tmp2_6 = extractelement <2 x i8> %tmp1_3, i32 1
    %tmp2_7 = extractelement <2 x i8> %tmp1_4, i32 0
    %tmp2_8 = extractelement <2 x i8> %tmp1_4, i32 1

    %tmp3_1 = sext i8 %tmp2_1 to i32
    %tmp3_2 = sext i8 %tmp2_2 to i32
    %tmp3_3 = sext i8 %tmp2_3 to i32
    %tmp3_4 = sext i8 %tmp2_4 to i32
    %tmp3_5 = sext i8 %tmp2_5 to i32
    %tmp3_6 = sext i8 %tmp2_6 to i32
    %tmp3_7 = sext i8 %tmp2_7 to i32
    %tmp3_8 = sext i8 %tmp2_8 to i32

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_1, i32 %tmp3_2, i32 %tmp3_3, i32 %tmp3_4, i32 %tmp3_5, i32 %tmp3_6, i32 %tmp3_7, i32 %tmp3_8)
; CHECK: 1 -2 11 -12 21 -22 31 -32

    ret i32 0
}

; extract from array of structs
%t_v3 = type <4 x i32>
%t_v4 = type <4 x double>
%t_s4 = type {%t_v3, %t_v4}
define i32 @ext_arr_str()
{
%ptr_i = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [26 x i8]* @r_float_8, i32 0, i32 0 ; to printf

    %tmp2_1 = extractvalue [2 x %t_s4] [%t_s4 {%t_v3 <i32 1, i32 -2, i32 3, i32 -4>, %t_v4 <double -1.9, double -2.8, double -3.7, double -4.6>}, %t_s4{%t_v3 <i32 0, i32 11, i32 22, i32 33>, %t_v4 <double 18.9, double -4.7, double 9.4, double -10.0>}], 0, 0
    %tmp2_2 = extractvalue [2 x %t_s4] [%t_s4 {%t_v3 <i32 1, i32 -2, i32 3, i32 -4>, %t_v4 <double -1.9, double -2.8, double -3.7, double -4.6>}, %t_s4{%t_v3 <i32 0, i32 11, i32 22, i32 33>, %t_v4 <double 18.9, double -4.7, double 9.4, double -10.0>}], 0, 1
    %tmp2_3 = extractvalue [2 x %t_s4] [%t_s4 {%t_v3 <i32 1, i32 -2, i32 3, i32 -4>, %t_v4 <double -1.9, double -2.8, double -3.7, double -4.6>}, %t_s4{%t_v3 <i32 0, i32 11, i32 22, i32 33>, %t_v4 <double 18.9, double -4.7, double 9.4, double -10.0>}], 1, 0
    %tmp2_4 = extractvalue [2 x %t_s4] [%t_s4 {%t_v3 <i32 1, i32 -2, i32 3, i32 -4>, %t_v4 <double -1.9, double -2.8, double -3.7, double -4.6>}, %t_s4{%t_v3 <i32 0, i32 11, i32 22, i32 33>, %t_v4 <double 18.9, double -4.7, double 9.4, double -10.0>}], 1, 1

    %tmp3_1 = extractelement %t_v3 %tmp2_1, i32 0
    %tmp3_2 = extractelement %t_v3 %tmp2_1, i32 1
    %tmp3_3 = extractelement %t_v3 %tmp2_1, i32 2
    %tmp3_4 = extractelement %t_v3 %tmp2_1, i32 3

    %tmp3_5 = extractelement %t_v3 %tmp2_3, i32 0
    %tmp3_6 = extractelement %t_v3 %tmp2_3, i32 1
    %tmp3_7 = extractelement %t_v3 %tmp2_3, i32 2
    %tmp3_8 = extractelement %t_v3 %tmp2_3, i32 3

    %tmp4_1 = extractelement %t_v4 %tmp2_2, i32 0
    %tmp4_2 = extractelement %t_v4 %tmp2_2, i32 1
    %tmp4_3 = extractelement %t_v4 %tmp2_2, i32 2
    %tmp4_4 = extractelement %t_v4 %tmp2_2, i32 3

    %tmp4_5 = extractelement %t_v4 %tmp2_4, i32 0
    %tmp4_6 = extractelement %t_v4 %tmp2_4, i32 1
    %tmp4_7 = extractelement %t_v4 %tmp2_4, i32 2
    %tmp4_8 = extractelement %t_v4 %tmp2_4, i32 3

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_1, i32 %tmp3_2, i32 %tmp3_3, i32 %tmp3_4, i32 %tmp3_5, i32 %tmp3_6, i32 %tmp3_7, i32 %tmp3_8)
; CHECK: 1 -2 3 -4 0 11 22 33
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp4_1, double %tmp4_2, double %tmp4_3, double %tmp4_4, double %tmp4_5, double %tmp4_6, double %tmp4_7, double %tmp4_8)
; CHECK: -1.900000 -2.800000 -3.700000 -4.600000 18.900000 -4.700000 9.400000 -10.000000

    ret i32 0
}

; extract from embedded array of integer type
%t_a2 = type [2 x <2 x i32>]
%t_a3 = type [2 x %t_a2]
%t_a4 = type [2 x %t_a3]
%t_v5 = type <2 x i32>
define i32 @ext_arr_embedded()
{
%ptr_i = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue %t_a4 [%t_a3[%t_a2[%t_v5 <i32 12, i32 -13>, %t_v5 <i32 14, i32 -15>],%t_a2[%t_v5 <i32 16, i32 17>, %t_v5 <i32 18, i32 19>]],%t_a3[%t_a2[%t_v5 <i32 212, i32 -213>, %t_v5 <i32 214, i32 215>],%t_a2[%t_v5 <i32 216, i32 217>, %t_v5 <i32 218, i32 219>]]], 0, 0, 0
    %tmp1_2 = extractvalue %t_a4 [%t_a3[%t_a2[%t_v5 <i32 12, i32 -13>, %t_v5 <i32 14, i32 -15>],%t_a2[%t_v5 <i32 16, i32 17>, %t_v5 <i32 18, i32 19>]],%t_a3[%t_a2[%t_v5 <i32 212, i32 -213>, %t_v5 <i32 214, i32 215>],%t_a2[%t_v5 <i32 216, i32 217>, %t_v5 <i32 218, i32 219>]]], 0, 0, 1
    %tmp1_3 = extractvalue %t_a4 [%t_a3[%t_a2[%t_v5 <i32 12, i32 -13>, %t_v5 <i32 14, i32 -15>],%t_a2[%t_v5 <i32 16, i32 17>, %t_v5 <i32 18, i32 19>]],%t_a3[%t_a2[%t_v5 <i32 212, i32 -213>, %t_v5 <i32 214, i32 215>],%t_a2[%t_v5 <i32 216, i32 217>, %t_v5 <i32 218, i32 219>]]], 0, 1, 0
    %tmp1_4 = extractvalue %t_a4 [%t_a3[%t_a2[%t_v5 <i32 12, i32 -13>, %t_v5 <i32 14, i32 -15>],%t_a2[%t_v5 <i32 16, i32 17>, %t_v5 <i32 18, i32 19>]],%t_a3[%t_a2[%t_v5 <i32 212, i32 -213>, %t_v5 <i32 214, i32 215>],%t_a2[%t_v5 <i32 216, i32 217>, %t_v5 <i32 218, i32 219>]]], 0, 1, 1
    %tmp1_5 = extractvalue %t_a4 [%t_a3[%t_a2[%t_v5 <i32 12, i32 -13>, %t_v5 <i32 14, i32 -15>],%t_a2[%t_v5 <i32 16, i32 17>, %t_v5 <i32 18, i32 19>]],%t_a3[%t_a2[%t_v5 <i32 212, i32 -213>, %t_v5 <i32 214, i32 215>],%t_a2[%t_v5 <i32 216, i32 217>, %t_v5 <i32 218, i32 219>]]], 1, 0, 0
    %tmp1_6 = extractvalue %t_a4 [%t_a3[%t_a2[%t_v5 <i32 12, i32 -13>, %t_v5 <i32 14, i32 -15>],%t_a2[%t_v5 <i32 16, i32 17>, %t_v5 <i32 18, i32 19>]],%t_a3[%t_a2[%t_v5 <i32 212, i32 -213>, %t_v5 <i32 214, i32 215>],%t_a2[%t_v5 <i32 216, i32 217>, %t_v5 <i32 218, i32 219>]]], 1, 0, 1
    %tmp1_7 = extractvalue %t_a4 [%t_a3[%t_a2[%t_v5 <i32 12, i32 -13>, %t_v5 <i32 14, i32 -15>],%t_a2[%t_v5 <i32 16, i32 17>, %t_v5 <i32 18, i32 19>]],%t_a3[%t_a2[%t_v5 <i32 212, i32 -213>, %t_v5 <i32 214, i32 215>],%t_a2[%t_v5 <i32 216, i32 217>, %t_v5 <i32 218, i32 219>]]], 1, 1, 0
    %tmp1_8 = extractvalue %t_a4 [%t_a3[%t_a2[%t_v5 <i32 12, i32 -13>, %t_v5 <i32 14, i32 -15>],%t_a2[%t_v5 <i32 16, i32 17>, %t_v5 <i32 18, i32 19>]],%t_a3[%t_a2[%t_v5 <i32 212, i32 -213>, %t_v5 <i32 214, i32 215>],%t_a2[%t_v5 <i32 216, i32 217>, %t_v5 <i32 218, i32 219>]]], 1, 1, 1

    %tmp2_1 = extractelement %t_v5 %tmp1_1, i32 0
    %tmp2_2 = extractelement %t_v5 %tmp1_1, i32 1
    %tmp2_3 = extractelement %t_v5 %tmp1_2, i32 0
    %tmp2_4 = extractelement %t_v5 %tmp1_2, i32 1
    %tmp2_5 = extractelement %t_v5 %tmp1_3, i32 0
    %tmp2_6 = extractelement %t_v5 %tmp1_3, i32 1
    %tmp2_7 = extractelement %t_v5 %tmp1_4, i32 0
    %tmp2_8 = extractelement %t_v5 %tmp1_4, i32 1

    %tmp3_1 = extractelement %t_v5 %tmp1_5, i32 0
    %tmp3_2 = extractelement %t_v5 %tmp1_5, i32 1
    %tmp3_3 = extractelement %t_v5 %tmp1_6, i32 0
    %tmp3_4 = extractelement %t_v5 %tmp1_6, i32 1
    %tmp3_5 = extractelement %t_v5 %tmp1_7, i32 0
    %tmp3_6 = extractelement %t_v5 %tmp1_7, i32 1
    %tmp3_7 = extractelement %t_v5 %tmp1_8, i32 0
    %tmp3_8 = extractelement %t_v5 %tmp1_8, i32 1

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_1, i32 %tmp2_2, i32 %tmp2_3, i32 %tmp2_4, i32 %tmp2_5, i32 %tmp2_6, i32 %tmp2_7, i32 %tmp2_8)
; CHECK: 12 -13 14 -15 16 17 18 19
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_1, i32 %tmp3_2, i32 %tmp3_3, i32 %tmp3_4, i32 %tmp3_5, i32 %tmp3_6, i32 %tmp3_7, i32 %tmp3_8)
; CHECK: 212 -213 214 215 216 217 218 219
    ret i32 0
}

define i32 @main()
{
    call i32 @ext_str_1()
    call i32 @ext_str_2()
    call i32 @ext_str_embedded()
    call i32 @ext_str_arr()
    call i32 @ext_str_point()

    call i32 @ext_arr_double()
    call i32 @ext_arr_float()
    call i32 @ext_arr_int()
    call i32 @ext_arr_str()
    call i32 @ext_arr_embedded()

    ret i32 0
}

