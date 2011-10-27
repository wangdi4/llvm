; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32:64"

@r_float_1 = internal constant [4 x i8] c" %f\00"
@r_int_1 = internal constant [4 x i8] c" %d\00"
@r_end = internal constant [2 x i8] c"\0A\00"
@r_int_8 = internal constant [26 x i8] c" %d %d %d %d %d %d %d %d\0A\00"
@r_int_4 = internal constant [14 x i8] c" %d %d %d %d\0A\00"

declare i32 @printf(i8*, ...)

; extract from struct containig float, int, double
define i32 @ext_str()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue {float, i32, double} {float -0.5, i32 12, double 14.5}, 0
    %tmp1_2 = extractvalue {float, i32, double} {float -0.5, i32 12, double 14.5}, 1
    %tmp1_3 = extractvalue {float, i32, double} {float -0.5, i32 12, double 14.5}, 2

    %tmp2_1 = fpext float %tmp1_1 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: -0.500000 12 14.500000

    ret i32 0
}

; extract from structure containing int, pointer to int, double, float
define i32 @ext_str_point()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = alloca i32
    store i32 333, i32* %tmp1_1
    %tmp1_2 = insertvalue {i16, i32*, double, float} {i16 123, i32* undef, double -1.0, float 2.0}, i32* %tmp1_1, 1
    %tmp1_3 = extractvalue {i16, i32*, double, float} %tmp1_2, 0
    %tmp1_4 = extractvalue {i16, i32*, double, float} %tmp1_2, 1
    %tmp1_5 = extractvalue {i16, i32*, double, float} %tmp1_2, 2
    %tmp1_6 = extractvalue {i16, i32*, double, float} %tmp1_2, 3

    %tmp2_1 = sext i16 %tmp1_3 to i32;
    %tmp2_2 = load i32* %tmp1_4;
    %tmp2_3 = fpext float %tmp1_6 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 123 333 -1.000000 2.000000

    ret i32 0
}

; extract from struct with embedded structures
define i32 @ext_str_embedded_1()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue {double,{i8, i16}} {double 17.0,{i8, i16} {i8 19, i16 20}}, 0
    %tmp1_2 = extractvalue {double,{i8, i16}} {double 17.0,{i8, i16} {i8 19, i16 20}}, 1, 0
    %tmp1_3 = extractvalue {double,{i8, i16}} {double 17.0,{i8, i16} {i8 19, i16 20}}, 1, 1

    %tmp2_2 = sext i8 %tmp1_2 to i32;
    %tmp2_3 = sext i16 %tmp1_3 to i32;

    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 17.000000 19 20

    ret i32 0
}

define i32 @ext_str_embedded_2()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue {{i32, i16},{{i8, i8}, double}} {{i32, i16} {i32 17, i16 18},{{i8, i8}, double} {{i8, i8}{i8 19, i8 20}, double 21.1}}, 0, 0 ; yields i32
    %tmp1_2 = extractvalue {{i32, i16},{{i8, i8}, double}} {{i32, i16} {i32 17, i16 18},{{i8, i8}, double} {{i8, i8}{i8 19, i8 20}, double 21.1}}, 0, 1 ; yields i16
    %tmp1_3 = extractvalue {{i32, i16},{{i8, i8}, double}} {{i32, i16} {i32 17, i16 18},{{i8, i8}, double} {{i8, i8}{i8 19, i8 20}, double 21.1}}, 1, 0, 0 ; yields i8
    %tmp1_4 = extractvalue {{i32, i16},{{i8, i8}, double}} {{i32, i16} {i32 17, i16 18},{{i8, i8}, double} {{i8, i8}{i8 19, i8 20}, double 21.1}}, 1, 0, 1 ; yields i8
    %tmp1_5 = extractvalue {{i32, i16},{{i8, i8}, double}} {{i32, i16} {i32 17, i16 18},{{i8, i8}, double} {{i8, i8}{i8 19, i8 20}, double 21.1}}, 1, 1 ; yields double

    %tmp2_2 = sext i16 %tmp1_2 to i32;
    %tmp2_3 = sext i8 %tmp1_3 to i32;
    %tmp2_4 = sext i8 %tmp1_4 to i32;

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 17 18 19 20 21.100000

    ret i32 0
}

; extract from struct with embedded structures
define i32 @ext_str_embedded_3()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue {double,{i8, i16}} {double 17.0,{i8, i16} {i8 19, i16 20}}, 0
    %tmp1_2 = extractvalue {double,{i8, i16}} {double 17.0,{i8, i16} {i8 19, i16 20}}, 1

    %tmp2_1 = extractvalue {i8, i16} %tmp1_2, 0
    %tmp2_2 = extractvalue {i8, i16} %tmp1_2, 1

    %tmp3_1 = sext i8 %tmp2_1 to i32;
    %tmp3_2 = sext i16 %tmp2_2 to i32;

    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp3_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 17.000000 19 20

    ret i32 0
}

; extract from struct of arrays
define i32 @ext_str_arr()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue {[2 x i32],[2 x double]} {[2 x i32] [i32 917, i32 -18],[2 x double] [double 19.1, double -20.2]}, 0, 0
    %tmp1_2 = extractvalue {[2 x i32],[2 x double]} {[2 x i32] [i32 917, i32 -18],[2 x double] [double 19.1, double -20.2]}, 0, 1
    %tmp1_3 = extractvalue {[2 x i32],[2 x double]} {[2 x i32] [i32 917, i32 -18],[2 x double] [double 19.1, double -20.2]}, 1, 0
    %tmp1_4 = extractvalue {[2 x i32],[2 x double]} {[2 x i32] [i32 917, i32 -18],[2 x double] [double 19.1, double -20.2]}, 1, 1

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 917 -18 19.100000 -20.200000

    ret i32 0
}

; extract from struct pointed by the pointer to struct
define i32 @ext_str_point_point()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = alloca {i32, double}
    store {i32, double} {i32 444, double 56.78}, {i32, double}* %tmp1_1
    %tmp1_2 = insertvalue {{i32, double}*, double, i32} {{i32, double}* undef, double -1.0, i32 2}, {i32, double}* %tmp1_1, 0

    %tmp2_1 = extractvalue {{i32, double}*, double, i32} %tmp1_2, 0
    %tmp2_2 = extractvalue {{i32, double}*, double, i32} %tmp1_2, 1
    %tmp2_3 = extractvalue {{i32, double}*, double, i32} %tmp1_2, 2
    %tmp2_4 = load {i32, double}* %tmp2_1;

    %tmp2_5 = extractvalue {i32, double} %tmp2_4, 0
    %tmp2_6 = extractvalue {i32, double} %tmp2_4, 1

    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_5)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp2_6)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: -1.000000 2 444 56.780000

    ret i32 0
}

; extract from array of integer type
define i32 @ext_arr_int()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_3 = extractvalue [3 x i32] [i32 182, i32 -194, i32 22], 2
    %tmp1_2 = extractvalue [3 x i32] [i32 182, i32 -194, i32 22], 1
    %tmp1_1 = extractvalue [3 x i32] [i32 182, i32 -194, i32 22], 0
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 182 -194 22

    ret i32 0
}

; extract from array of float type
define i32 @ext_arr_float()
{
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue [2 x float] [float 1.8000000e+01, float -3.6700000e+02], 0
    %tmp1_2 = extractvalue [2 x float] [float 1.8000000e+01, float -3.6700000e+02], 1

    %tmp2_1 = fpext float %tmp1_1 to double
    %tmp2_2 = fpext float %tmp1_2 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp2_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 18.000000 -367.000000

    ret i32 0
}

; extract from array of double type
define i32 @ext_arr_double()
{
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue [2 x double] [double 12.7, double -14.8], 0
    %tmp1_2 = extractvalue [2 x double] [double 12.7, double -14.8], 1

    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 12.700000 -14.800000

    ret i32 0
}

; extract from embedded array of integer type
define i32 @ext_arr_embedded_1()
{
%ptr_i = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue [2 x [2 x [2 x i32]]] [[2 x [2 x i32]][[2 x i32][i32 12, i32 14],[2 x i32][i32 13, i32 15]],[2 x [2 x i32]][[2 x i32][i32 212, i32 214],[2 x i32][i32 213, i32 215]]], 0, 0, 0
    %tmp1_2 = extractvalue [2 x [2 x [2 x i32]]] [[2 x [2 x i32]][[2 x i32][i32 12, i32 14],[2 x i32][i32 13, i32 15]],[2 x [2 x i32]][[2 x i32][i32 212, i32 214],[2 x i32][i32 213, i32 215]]], 0, 0, 1
    %tmp1_3 = extractvalue [2 x [2 x [2 x i32]]] [[2 x [2 x i32]][[2 x i32][i32 12, i32 14],[2 x i32][i32 13, i32 15]],[2 x [2 x i32]][[2 x i32][i32 212, i32 214],[2 x i32][i32 213, i32 215]]], 0, 1, 0
    %tmp1_4 = extractvalue [2 x [2 x [2 x i32]]] [[2 x [2 x i32]][[2 x i32][i32 12, i32 14],[2 x i32][i32 13, i32 15]],[2 x [2 x i32]][[2 x i32][i32 212, i32 214],[2 x i32][i32 213, i32 215]]], 0, 1, 1
    %tmp1_5 = extractvalue [2 x [2 x [2 x i32]]] [[2 x [2 x i32]][[2 x i32][i32 12, i32 14],[2 x i32][i32 13, i32 15]],[2 x [2 x i32]][[2 x i32][i32 212, i32 214],[2 x i32][i32 213, i32 215]]], 1, 0, 0
    %tmp1_6 = extractvalue [2 x [2 x [2 x i32]]] [[2 x [2 x i32]][[2 x i32][i32 12, i32 14],[2 x i32][i32 13, i32 15]],[2 x [2 x i32]][[2 x i32][i32 212, i32 214],[2 x i32][i32 213, i32 215]]], 1, 0, 1
    %tmp1_7 = extractvalue [2 x [2 x [2 x i32]]] [[2 x [2 x i32]][[2 x i32][i32 12, i32 14],[2 x i32][i32 13, i32 15]],[2 x [2 x i32]][[2 x i32][i32 212, i32 214],[2 x i32][i32 213, i32 215]]], 1, 1, 0
    %tmp1_8 = extractvalue [2 x [2 x [2 x i32]]] [[2 x [2 x i32]][[2 x i32][i32 12, i32 14],[2 x i32][i32 13, i32 15]],[2 x [2 x i32]][[2 x i32][i32 212, i32 214],[2 x i32][i32 213, i32 215]]], 1, 1, 1

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_1, i32 %tmp1_2, i32 %tmp1_3, i32 %tmp1_4, i32 %tmp1_5, i32 %tmp1_6, i32 %tmp1_7, i32 %tmp1_8)
; CHECK: 12 14 13 15 212 214 213 215

    ret i32 0
}

define i32 @ext_arr_embedded_2()
{
%ptr_i = getelementptr [14 x i8]* @r_int_4, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue [2 x [2 x [2 x i32]]] [[2 x [2 x i32]][[2 x i32][i32 12, i32 14],[2 x i32][i32 13, i32 15]],[2 x [2 x i32]][[2 x i32][i32 212, i32 214],[2 x i32][i32 213, i32 215]]], 0, 0
    %tmp1_2 = extractvalue [2 x [2 x [2 x i32]]] [[2 x [2 x i32]][[2 x i32][i32 12, i32 14],[2 x i32][i32 13, i32 15]],[2 x [2 x i32]][[2 x i32][i32 212, i32 214],[2 x i32][i32 213, i32 215]]], 1, 1

    %tmp2_1 = extractvalue [2 x i32] %tmp1_1, 0
    %tmp2_2 = extractvalue [2 x i32] %tmp1_1, 1

    %tmp3_1 = extractvalue [2 x i32] %tmp1_2, 0
    %tmp3_2 = extractvalue [2 x i32] %tmp1_2, 1

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2_1, i32 %tmp2_2, i32 %tmp3_1, i32 %tmp3_2)
; CHECK: 12 14 213 215

    ret i32 0
}

;extract from array of pointers
define i32 @ext_arr_point() {

%ptr_i = getelementptr [14 x i8]* @r_int_4, i32 0, i32 0 ; to printf

    %tmp1_1 = alloca i32
    %tmp1_2 = alloca i32
    %tmp1_3 = alloca i32
    %tmp1_4 = alloca i32
    store i32 123, i32* %tmp1_1
    store i32 234, i32* %tmp1_2
    store i32 345, i32* %tmp1_3
    store i32 456, i32* %tmp1_4

    %tmp2_1 = insertvalue [4 x i32*] undef, i32* %tmp1_1, 0
    %tmp2_2 = insertvalue [4 x i32*] %tmp2_1, i32* %tmp1_2, 1
    %tmp2_3 = insertvalue [4 x i32*] %tmp2_2, i32* %tmp1_3, 2
    %tmp2_4 = insertvalue [4 x i32*] %tmp2_3, i32* %tmp1_4, 3

    %tmp3_1 = extractvalue [4 x i32*] %tmp2_4, 0
    %tmp3_2 = extractvalue [4 x i32*] %tmp2_4, 1
    %tmp3_3 = extractvalue [4 x i32*] %tmp2_4, 2
    %tmp3_4 = extractvalue [4 x i32*] %tmp2_4, 3

    %tmp4_1 = load i32* %tmp3_1;
    %tmp4_2 = load i32* %tmp3_2;
    %tmp4_3 = load i32* %tmp3_3;
    %tmp4_4 = load i32* %tmp3_4;

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp4_1, i32 %tmp4_2, i32 %tmp4_3, i32 %tmp4_4)
; CHECK: 123 234 345 456

    ret i32 0
}

; extract from array of structs
define i32 @ext_arr_str()
{
%ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
%ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
%ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

    %tmp1_1 = extractvalue [2 x {i32, double}] [{i32, double} {i32 897, double -108.9}, {i32, double}{i32 -979, double 118.9}], 0, 0
    %tmp1_2 = extractvalue [2 x {i32, double}] [{i32, double} {i32 897, double -108.9}, {i32, double}{i32 -979, double 118.9}], 0, 1
    %tmp1_3 = extractvalue [2 x {i32, double}] [{i32, double} {i32 897, double -108.9}, {i32, double}{i32 -979, double 118.9}], 1, 0
    %tmp1_4 = extractvalue [2 x {i32, double}] [{i32, double} {i32 897, double -108.9}, {i32, double}{i32 -979, double 118.9}], 1, 1

    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_2)
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1_3)
    call i32 (i8*, ...)* @printf(i8* %ptr_f, double %tmp1_4)
    call i32 (i8*, ...)* @printf(i8* %ptr_end)
; CHECK: 897 -108.900000 -979 118.900000
    ret i32 0
}

define i32 @main()
{
    call i32 @ext_str()
    call i32 @ext_str_point()
    call i32 @ext_str_embedded_1()
    call i32 @ext_str_embedded_2()
    call i32 @ext_str_embedded_3()
    call i32 @ext_str_arr()
    call i32 @ext_str_point_point()

    call i32 @ext_arr_int()
    call i32 @ext_arr_float()
    call i32 @ext_arr_double()
    call i32 @ext_arr_embedded_1()
    call i32 @ext_arr_embedded_2()
    call i32 @ext_arr_point()
    call i32 @ext_arr_str()

    ret i32 0
}


