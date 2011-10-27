; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32:64"

@r_f_1 = internal constant [5 x i8] c" %f\0A\00"
@r_f_2 = internal constant [8 x i8] c" %f %f\0A\00"
@r_f_4 = internal constant [14 x i8] c" %f %f %f %f\0A\00"
@r_f_8 = internal constant [26 x i8] c" %f %f %f %f %f %f %f %f\0A\00"

@r_e_2 = internal constant [8 x i8] c" %e %e\0A\00"
@r_e_4 = internal constant [14 x i8] c" %e %e %e %e\0A\00"

@r_i_1 = internal constant [5 x i8] c" %d\0A\00"
@r_i_2 = internal constant [8 x i8] c" %d %d\0A\00"
@r_i_4 = internal constant [14 x i8] c" %d %d %d %d\0A\00"
@r_i_8 = internal constant [26 x i8] c" %d %d %d %d %d %d %d %d\0A\00"

declare i32 @printf(i8*, ...)

define i32 @bitcast_vec_vec()
{
    %ptr_i_4 = getelementptr [14 x i8]* @r_i_4, i32 0, i32 0 ; to printf
    %ptr_f_4 = getelementptr [14 x i8]* @r_f_4, i32 0, i32 0 ; to printf
    %ptr_i_8 = getelementptr [26 x i8]* @r_i_8, i32 0, i32 0 ; to printf
    %ptr_f_2 = getelementptr [8 x i8]* @r_f_2, i32 0, i32 0 ; to printf
    %ptr_i_2 = getelementptr [8 x i8]* @r_i_2, i32 0, i32 0 ; to printf
    %ptr_e_4 = getelementptr [14 x i8]* @r_e_4, i32 0, i32 0 ; to printf

    %tmp1 = bitcast <2 x i64> <i64 12, i64 10000> to <4 x i32>
    %r1_1 = extractelement <4 x i32> %tmp1, i32 0
    %r1_2 = extractelement <4 x i32> %tmp1, i32 1
    %r1_3 = extractelement <4 x i32> %tmp1, i32 2
    %r1_4 = extractelement <4 x i32> %tmp1, i32 3

    call i32 (i8*, ...)* @printf(i8* %ptr_i_4, i32 %r1_1, i32 %r1_2, i32 %r1_3, i32 %r1_4)
; CHECK: 12 0 10000 0

    %tmp2 = bitcast <8 x i16> <i16 1, i16 2, i16 3, i16 4, i16 5, i16 6, i16 7, i16 8> to <4 x i32>
    %r2_1 = extractelement <4 x i32> %tmp2, i32 0
    %r2_2 = extractelement <4 x i32> %tmp2, i32 1
    %r2_3 = extractelement <4 x i32> %tmp2, i32 2
    %r2_4 = extractelement <4 x i32> %tmp2, i32 3

    call i32 (i8*, ...)* @printf(i8* %ptr_i_4, i32 %r2_1, i32 %r2_2, i32 %r2_3, i32 %r2_4)
; CHECK: 131073 262147 393221 524295

    %tmp3 = bitcast <4 x float> <float 1.0, float -2.0, float 3.0, float 4.0> to <4 x i32>
    %r3_1 = extractelement <4 x i32> %tmp3, i32 0
    %r3_2 = extractelement <4 x i32> %tmp3, i32 1
    %r3_3 = extractelement <4 x i32> %tmp3, i32 2
    %r3_4 = extractelement <4 x i32> %tmp3, i32 3

    call i32 (i8*, ...)* @printf(i8* %ptr_i_4, i32 %r3_1, i32 %r3_2, i32 %r3_3, i32 %r3_4)
; CHECK: 1065353216 -1073741824 1077936128 1082130432

    %tmp4 = bitcast <2 x double> <double 2.0, double 20.5> to <4 x i32>
    %r4_1 = extractelement <4 x i32> %tmp4, i32 0
    %r4_2 = extractelement <4 x i32> %tmp4, i32 1
    %r4_3 = extractelement <4 x i32> %tmp4, i32 2
    %r4_4 = extractelement <4 x i32> %tmp4, i32 3

    call i32 (i8*, ...)* @printf(i8* %ptr_i_4, i32 %r4_1, i32 %r4_2, i32 %r4_3, i32 %r4_4)
; CHECK: 0 1073741824 0 1077182464

    %tmp5 = bitcast <4 x float> <float 1.0, float -2.0, float 3.0, float 4.0> to <8 x i16>
    %r5_1 = extractelement <8 x i16> %tmp5, i32 0
    %r5_2 = extractelement <8 x i16> %tmp5, i32 1
    %r5_3 = extractelement <8 x i16> %tmp5, i32 2
    %r5_4 = extractelement <8 x i16> %tmp5, i32 3
    %r5_5 = extractelement <8 x i16> %tmp5, i32 4
    %r5_6 = extractelement <8 x i16> %tmp5, i32 5
    %r5_7 = extractelement <8 x i16> %tmp5, i32 6
    %r5_8 = extractelement <8 x i16> %tmp5, i32 7
    %r6_1 = zext i16 %r5_1 to i32
    %r6_2 = zext i16 %r5_2 to i32
    %r6_3 = zext i16 %r5_3 to i32
    %r6_4 = zext i16 %r5_4 to i32
    %r6_5 = zext i16 %r5_5 to i32
    %r6_6 = zext i16 %r5_6 to i32
    %r6_7 = zext i16 %r5_7 to i32
    %r6_8 = zext i16 %r5_8 to i32

    call i32 (i8*, ...)* @printf(i8* %ptr_i_8, i32 %r6_1, i32 %r6_2, i32 %r6_3, i32 %r6_4, i32 %r6_5, i32 %r6_6, i32 %r6_7, i32 %r6_8)
; CHECK: 0 16256 0 49152 0 16448 0 16512

    %tmp6 = bitcast <4 x i32> < i32 1065353216, i32 -1073741824, i32 1077936128, i32 1082130432> to <4 x float>
    %r7_1 = extractelement <4 x float> %tmp6, i32 0
    %r7_2 = extractelement <4 x float> %tmp6, i32 1
    %r7_3 = extractelement <4 x float> %tmp6, i32 2
    %r7_4 = extractelement <4 x float> %tmp6, i32 3
    %r8_1 = fpext float %r7_1 to double
    %r8_2 = fpext float %r7_2 to double
    %r8_3 = fpext float %r7_3 to double
    %r8_4 = fpext float %r7_3 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_f_4, double %r8_1, double %r8_2, double %r8_3, double %r8_4)
; CHECK: 1.000000 -2.000000 3.000000 3.000000

    %tmp7 = bitcast <4 x i32> < i32 1065353216, i32 -1073741824, i32 1077936128, i32 1082130432> to <2 x double>
    %r9_1 = extractelement <2 x double> %tmp7, i32 0
    %r9_2 = extractelement <2 x double> %tmp7, i32 1
    call i32 (i8*, ...)* @printf(i8* %ptr_f_2, double %r9_1, double %r9_2)
; CHECK: -2.000000 512.000123

    %tmp8 = bitcast <4 x float> <float 5.0, float 6.0, float 7.0, float -8.0> to <2 x double>
    %r10_1 = extractelement <2 x double> %tmp8, i32 0
    %r10_2 = extractelement <2 x double> %tmp8, i32 1
    call i32 (i8*, ...)* @printf(i8* %ptr_f_2, double %r10_1, double %r10_2)
; CHECK: 8192.001972 -131072.031677

    %tmp9 = bitcast <4 x float> <float 5.0, float 6.0, float 7.0, float -8.0> to <2 x i64>
    %r11_1 = extractelement <2 x i64> %tmp9, i32 0
    %r11_2 = extractelement <2 x i64> %tmp9, i32 1
    call i32 (i8*, ...)* @printf(i8* %ptr_i_2, i64 %r11_1, i64 %r11_2)
; CHECK: 1084227584 1088421888

    %tmp10 = bitcast <4 x i64> < i64 100054567678892229900100000, i64 1010101, i64 9, i64 87956> to <4 x double>
    %r12_1 = extractelement <4 x double> %tmp10, i32 0
    %r12_2 = extractelement <4 x double> %tmp10, i32 1
    %r12_3 = extractelement <4 x double> %tmp10, i32 2
    %r12_4 = extractelement <4 x double> %tmp10, i32 3
    call i32 (i8*, ...)* @printf(i8* %ptr_e_4, double %r12_1, double %r12_2, double %r12_3, double %r12_4)
; CHECK:  -4.584987e+286 4.990562e-318 4.446591e-323 4.345604e-319

    %tmp11 = bitcast <4 x double> <double -4.584987e+286, double 4.990562e-318, double 4.446591e-323, double 4.345604e-319> to <4 x i64>
    %r13_1 = extractelement <4 x i64> %tmp11, i32 0
    %r13_2 = extractelement <4 x i64> %tmp11, i32 1
    %r13_3 = extractelement <4 x i64> %tmp11, i32 2
    %r13_4 = extractelement <4 x i64> %tmp11, i32 3
    call i32 (i8*, ...)* @printf(i8* %ptr_i_4, i64 %r13_1, i64 %r13_2, i64 %r13_3, i64 %r13_4)
; CHECK:  1487020594 1010101 9 87956

    ret i32 0
}

define i32 @bitcast_vec_val()
{
    %ptr_i = getelementptr [5 x i8]* @r_i_1, i32 0, i32 0 ; to printf
    %ptr_f = getelementptr [5 x i8]* @r_f_1, i32 0, i32 0 ; to printf

    ; int vector to int
    %tmp1 = bitcast <4 x i8> <i8 1, i8 0, i8 2, i8 0> to i32
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp1)
; CHECK: 131073

    %tmp2 = bitcast <2 x i16> <i16 121, i16 11> to i32
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %tmp2)
; CHECK: 721017

    %tmp3 = bitcast <2 x i32> <i32 2, i32 3> to i64
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i64 %tmp3)
; CHECK: 2

    %tmp4 = bitcast <4 x i16> <i16 121, i16 232, i16 11, i16 4232> to i64
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i64 %tmp4)
; CHECK: 15204473

    ; float vector to int
    %tmp5 = bitcast <2 x float> <float 1.5, float 2.5> to i64
    call i32 (i8*, ...)* @printf(i8* %ptr_i, i64 %tmp5)
; CHECK: 1069547520

    ret i32 0
}

define i32 @bitcast_val_vec()
{
    %ptr_i_4 = getelementptr [14 x i8]* @r_i_4, i32 0, i32 0 ; to printf
    %ptr_e_2 = getelementptr [8 x i8]* @r_e_2, i32 0, i32 0 ; to printf
    %ptr_i_2 = getelementptr [8 x i8]* @r_i_2, i32 0, i32 0 ; to printf

    ; int to int vector
    %tmp1 = bitcast i32 1234567 to <4 x i8>
    %r1_1 = extractelement <4 x i8> %tmp1, i32 0
    %r1_2 = extractelement <4 x i8> %tmp1, i32 1
    %r1_3 = extractelement <4 x i8> %tmp1, i32 2
    %r1_4 = extractelement <4 x i8> %tmp1, i32 3
    %r2_1 = zext i8 %r1_1 to i32
    %r2_2 = zext i8 %r1_2 to i32
    %r2_3 = zext i8 %r1_3 to i32
    %r2_4 = zext i8 %r1_4 to i32
    call i32 (i8*, ...)* @printf(i8* %ptr_i_4, i32 %r2_1, i32 %r2_2, i32 %r2_3, i32 %r2_4)
; CHECK: 135 214 18 0

    %tmp2 = bitcast i64 12345671234567 to <4 x i16>
    %r3_1 = extractelement <4 x i16> %tmp2, i32 0
    %r3_2 = extractelement <4 x i16> %tmp2, i32 1
    %r3_3 = extractelement <4 x i16> %tmp2, i32 2
    %r3_4 = extractelement <4 x i16> %tmp2, i32 3
    %r4_1 = zext i16 %r3_1 to i32
    %r4_2 = zext i16 %r3_2 to i32
    %r4_3 = zext i16 %r3_3 to i32
    %r4_4 = zext i16 %r3_4 to i32
    call i32 (i8*, ...)* @printf(i8* %ptr_i_4, i32 %r4_1, i32 %r4_2, i32 %r4_3, i32 %r4_4)
; CHECK: 13319 29529 2874 0

    ; float to int vector
    %tmp3 = bitcast float 1234.5 to <4 x i8>
    %r5_1 = extractelement <4 x i8> %tmp3, i32 0
    %r5_2 = extractelement <4 x i8> %tmp3, i32 1
    %r5_3 = extractelement <4 x i8> %tmp3, i32 2
    %r5_4 = extractelement <4 x i8> %tmp3, i32 3
    %r6_1 = zext i8 %r5_1 to i32
    %r6_2 = zext i8 %r5_2 to i32
    %r6_3 = zext i8 %r5_3 to i32
    %r6_4 = zext i8 %r5_4 to i32
    call i32 (i8*, ...)* @printf(i8* %ptr_i_4, i32 %r6_1, i32 %r6_2, i32 %r6_3, i32 %r6_4)
; CHECK: 0 80 154 68

    ; int to float vector
    %tmp4 = bitcast i64 12345671234567 to <2 x float>
    %r7_1 = extractelement <2 x float> %tmp4, i32 0
    %r7_2 = extractelement <2 x float> %tmp4, i32 1
    %r8_1 = fpext float %r7_1 to double
    %r8_2 = fpext float %r7_2 to double
    call i32 (i8*, ...)* @printf(i8* %ptr_e_2, double %r8_1, double %r8_2)
; CHECK: 1.720861e+{{[0]*}}31 4.027332e-{{[0]*}}42

    ; double to int vector
    %tmp5 = bitcast double 12346789456.5 to <2 x i32>
    %r9_1 = extractelement <2 x i32> %tmp5, i32 0
    %r9_2 = extractelement <2 x i32> %tmp5, i32 1
    call i32 (i8*, ...)* @printf(i8* %ptr_i_2, i32 %r9_1, i32 %r9_2)
; CHECK: 1921253376 1107754856

    ; double to float vector
    %tmp6 = bitcast double 12346789456.5 to <2 x float>
    %r10_1 = extractelement <2 x float> %tmp6, i32 0
    %r10_2 = extractelement <2 x float> %tmp6, i32 1
    %r11_1 = fpext float %r10_1 to double
    %r11_2 = fpext float %r10_2 to double
    call i32 (i8*, ...)* @printf(i8* %ptr_e_2, double %r11_1, double %r11_2)
; CHECK:  5.229059e+{{[0]*}}30 3.374942e+{{[0]*}}1

    ret i32 0
}

define i32 @main()
{
    call i32 @bitcast_vec_vec()
    call i32 @bitcast_vec_val()
    call i32 @bitcast_val_vec()

    ret i32 0
}


