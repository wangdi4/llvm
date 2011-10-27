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


define i32 @main()
{
       %ptr_i = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
       %ptr_f = getelementptr [4 x i8]* @r_float_1, i32 0, i32 0 ; to printf
       %ptr_end = getelementptr [2 x i8]* @r_end, i32 0, i32 0 ; to printf

       %ptr_i_4 = getelementptr [14 x i8]* @r_int_4, i32 0, i32 0 ; to printf
       %ptr_f_4 = getelementptr [14 x i8]* @r_float_4, i32 0, i32 0 ; to printf
       %ptr_i_8 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %ptr_f_8 = getelementptr [26 x i8]* @r_float_8, i32 0, i32 0 ; to printf

       %tmp1 = shufflevector <2 x i32> <i32 3, i32 4>, <2 x i32> undef, <4 x i32> < i32 3, i32 2, i32 1, i32 0 >

       %r_tmp1_1 = extractelement <4 x i32> %tmp1, i32 0
       %r_tmp1_2 = extractelement <4 x i32> %tmp1, i32 1
       %r_tmp1_3 = extractelement <4 x i32> %tmp1, i32 2
       %r_tmp1_4 = extractelement <4 x i32> %tmp1, i32 3
       call i32 (i8*, ...)* @printf(i8* %ptr_i_4, i32 %r_tmp1_1, i32 %r_tmp1_2, i32 %r_tmp1_3, i32 %r_tmp1_4)
; CHECK: 0 0 4 3
       
       %tmp2 = shufflevector <2 x i32> <i32 3, i32 4>, <2 x i32> <i32 5, i32 6>, <3 x i32> < i32 2, i32 undef, i32 1 >

       %r_tmp2_1 = extractelement <3 x i32> %tmp2, i32 0
       %r_tmp2_2 = extractelement <3 x i32> %tmp2, i32 1
       %r_tmp2_3 = extractelement <3 x i32> %tmp2, i32 2
       call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r_tmp2_1)
       call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r_tmp2_2)
       call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r_tmp2_3)
       call i32 (i8*, ...)* @printf( i8* getelementptr ([2 x i8]* @r_end, i32 0, i32 0) )
; CHECK: 5 3 4

       %tmp3 = shufflevector <2 x i32> <i32 3, i32 4>, <2 x i32> undef, <5 x i32> < i32 2, i32 1, i32 3, i32 0, i32 undef >

       %r_tmp3_1 = extractelement <5 x i32> %tmp3, i32 0
       %r_tmp3_2 = extractelement <5 x i32> %tmp3, i32 1
       %r_tmp3_3 = extractelement <5 x i32> %tmp3, i32 2
       %r_tmp3_4 = extractelement <5 x i32> %tmp3, i32 3
       %r_tmp3_5 = extractelement <5 x i32> %tmp3, i32 4
       call i32 (i8*, ...)* @printf(i8* %ptr_i, i32 %r_tmp3_1)
       call i32 (i8*, ...)* @printf(i8* %ptr_i_4, i32 %r_tmp3_2, i32 %r_tmp3_3, i32 %r_tmp3_4, i32 %r_tmp3_5)
; CHECK: 0 4 0 3 3

       %tmp4 = shufflevector <2 x double> <double 3.0, double 4.0>, <2 x double> <double 5.0, double 6.0>, <4 x i32> < i32 3, i32 1, i32 2, i32 0 >

       %r_tmp4_1 = extractelement <4 x double> %tmp4, i32 0
       %r_tmp4_2 = extractelement <4 x double> %tmp4, i32 1
       %r_tmp4_3 = extractelement <4 x double> %tmp4, i32 2
       %r_tmp4_4 = extractelement <4 x double> %tmp4, i32 3
       call i32 (i8*, ...)* @printf(i8* %ptr_f_4, double %r_tmp4_1, double %r_tmp4_2, double %r_tmp4_3, double %r_tmp4_4)
; CHECK: 6.000000 4.000000 5.000000 3.000000

       %tmp5 = shufflevector <2 x float> <float 6.0, float 7.0>, <2 x float> <float 8.0, float 9.0>, <4 x i32> < i32 2, i32 3, i32 0, i32 1 >

       %r_tmp5_a = extractelement <4 x float> %tmp5, i32 0
       %r_tmp5_1 = fpext float %r_tmp5_a to double
       %r_tmp5_b = extractelement <4 x float> %tmp5, i32 1
       %r_tmp5_2 = fpext float %r_tmp5_b to double
       %r_tmp5_c = extractelement <4 x float> %tmp5, i32 2
       %r_tmp5_3 = fpext float %r_tmp5_c to double
       %r_tmp5_d = extractelement <4 x float> %tmp5, i32 3
       %r_tmp5_4 = fpext float %r_tmp5_d to double
       call i32 (i8*, ...)* @printf(i8* %ptr_f_4, double %r_tmp5_1, double %r_tmp5_2, double %r_tmp5_3, double %r_tmp5_4)
; CHECK: 8.000000 9.000000 6.000000 7.000000

       %tmp6 = shufflevector <2 x double> <double 63.0, double 49.0>, <2 x double> <double 95.0, double 68.0>, <8 x i32> < i32 3, i32 2, i32 1, i32 0, i32 0, i32 1, i32 2, i32 3 >

       %r_tmp6_1 = extractelement <8 x double> %tmp6, i32 0
       %r_tmp6_2 = extractelement <8 x double> %tmp6, i32 1
       %r_tmp6_3 = extractelement <8 x double> %tmp6, i32 2
       %r_tmp6_4 = extractelement <8 x double> %tmp6, i32 3
       %r_tmp6_5 = extractelement <8 x double> %tmp6, i32 4
       %r_tmp6_6 = extractelement <8 x double> %tmp6, i32 5
       %r_tmp6_7 = extractelement <8 x double> %tmp6, i32 6
       %r_tmp6_8 = extractelement <8 x double> %tmp6, i32 7
       call i32 (i8*, ...)* @printf(i8* %ptr_f_8, double %r_tmp6_1, double %r_tmp6_2, double %r_tmp6_3, double %r_tmp6_4, double %r_tmp6_5, double %r_tmp6_6, double %r_tmp6_7, double %r_tmp6_8)
; CHECK: 68.000000 95.000000 49.000000 63.000000 63.000000 49.000000 95.000000 68.000000

       %tmp7 = shufflevector <4 x float> <float 16.0, float 17.0, float 18.0, float 19.0>,<4 x float> <float 216.0, float 217.0, float 218.0, float 219.0 >, <8 x i32> < i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3 >

       %r_tmp7_a = extractelement <8 x float> %tmp7, i32 0
       %r_tmp7_1 = fpext float %r_tmp7_a to double
       %r_tmp7_b = extractelement <8 x float> %tmp7, i32 1
       %r_tmp7_2 = fpext float %r_tmp7_b to double
       %r_tmp7_c = extractelement <8 x float> %tmp7, i32 2
       %r_tmp7_3 = fpext float %r_tmp7_c to double
       %r_tmp7_d = extractelement <8 x float> %tmp7, i32 3
       %r_tmp7_4 = fpext float %r_tmp7_d to double
       %r_tmp7_e = extractelement <8 x float> %tmp7, i32 4
       %r_tmp7_5 = fpext float %r_tmp7_e to double
       %r_tmp7_f = extractelement <8 x float> %tmp7, i32 5
       %r_tmp7_6 = fpext float %r_tmp7_f to double
       %r_tmp7_g = extractelement <8 x float> %tmp7, i32 6
       %r_tmp7_7 = fpext float %r_tmp7_g to double
       %r_tmp7_h = extractelement <8 x float> %tmp7, i32 7
       %r_tmp7_8 = fpext float %r_tmp7_h to double
       call i32 (i8*, ...)* @printf(i8* %ptr_f_8, double %r_tmp7_1, double %r_tmp7_2, double %r_tmp7_3, double %r_tmp7_4, double %r_tmp7_5, double %r_tmp7_6, double %r_tmp7_7, double %r_tmp7_8)
; CHECK: 16.000000 16.000000 17.000000 17.000000 18.000000 18.000000 19.000000 19.000000

       %tmp8 = shufflevector <3 x i8> <i8 56, i8 67, i8 78>, <3 x i8> <i8 89, i8 90, i8 1>, <8 x i32> < i32 5, i32 1, i32 4, i32 2, i32 3, i32 undef, i32 undef, i32 0 >

       %r_tmp8_a = extractelement <8 x i8> %tmp8, i32 0
       %r_tmp8_1 = sext i8 %r_tmp8_a to i32
       %r_tmp8_b = extractelement <8 x i8> %tmp8, i32 1
       %r_tmp8_2 = sext i8 %r_tmp8_b to i32
       %r_tmp8_c = extractelement <8 x i8> %tmp8, i32 2
       %r_tmp8_3 = sext i8 %r_tmp8_c to i32
       %r_tmp8_d = extractelement <8 x i8> %tmp8, i32 3
       %r_tmp8_4 = sext i8 %r_tmp8_d to i32
       %r_tmp8_e = extractelement <8 x i8> %tmp8, i32 4
       %r_tmp8_5 = sext i8 %r_tmp8_e to i32
       %r_tmp8_f = extractelement <8 x i8> %tmp8, i32 5
       %r_tmp8_6 = sext i8 %r_tmp8_f to i32
       %r_tmp8_g = extractelement <8 x i8> %tmp8, i32 6
       %r_tmp8_7 = sext i8 %r_tmp8_g to i32
       %r_tmp8_h = extractelement <8 x i8> %tmp8, i32 7
       %r_tmp8_8 = sext i8 %r_tmp8_h to i32
       call i32 (i8*, ...)* @printf(i8* %ptr_i_8, i32 %r_tmp8_1, i32 %r_tmp8_2, i32 %r_tmp8_3, i32 %r_tmp8_4, i32 %r_tmp8_5, i32 %r_tmp8_6, i32 %r_tmp8_7, i32 %r_tmp8_8)
; CHECK: 1 67 90 78 89 56 56 56

       %tmp9 = shufflevector <8 x i16> <i16 3, i16 4, i16 5, i16 6, i16 7, i16 8, i16 9, i16 10>, <8 x i16> <i16 93, i16 94, i16 95, i16 96, i16 97, i16 98, i16 99, i16 100>, <4 x i32> < i32 15, i32 0, i32 14, i32 1>
       %r_tmp9_a = extractelement <4 x i16> %tmp9, i32 0
       %r_tmp9_1 = sext i16 %r_tmp9_a to i32
       %r_tmp9_b = extractelement <4 x i16> %tmp9, i32 1
       %r_tmp9_2 = sext i16 %r_tmp9_b to i32
       %r_tmp9_c = extractelement <4 x i16> %tmp9, i32 2
       %r_tmp9_3 = sext i16 %r_tmp9_c to i32
       %r_tmp9_d = extractelement <4 x i16> %tmp9, i32 3
       %r_tmp9_4 = sext i16 %r_tmp9_d to i32

       call i32 (i8*, ...)* @printf(i8* %ptr_i_4, i32 %r_tmp9_1, i32 %r_tmp9_2, i32 %r_tmp9_3, i32 %r_tmp9_4)
; CHECK: 100 3 99 4

       %tmp10 = shufflevector <4 x i64> < i64 5788215, i64 -67212, i64 -8325, i64 80422>, <4 x i64> <i64 -7, i64 56689632, i64 111645, i64 2742>, <8 x i32> < i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
       
       %r_tmp10_1 = extractelement <8 x i64> %tmp10, i32 0
       %r_tmp10_2 = extractelement <8 x i64> %tmp10, i32 1
       %r_tmp10_3 = extractelement <8 x i64> %tmp10, i32 2
       %r_tmp10_4 = extractelement <8 x i64> %tmp10, i32 3
       %r_tmp10_5 = extractelement <8 x i64> %tmp10, i32 4
       %r_tmp10_6 = extractelement <8 x i64> %tmp10, i32 5
       %r_tmp10_7 = extractelement <8 x i64> %tmp10, i32 6
       %r_tmp10_8 = extractelement <8 x i64> %tmp10, i32 7

       call i32 (i8*, ...)* @printf(i8* %ptr_i_8, i64 %r_tmp10_1, i64 %r_tmp10_2, i64 %r_tmp10_3, i64 %r_tmp10_4, i64 %r_tmp10_5, i64 %r_tmp10_6, i64 %r_tmp10_7, i64 %r_tmp10_8)
; CHECK: 5788215 -67212 -8325 80422 -7 56689632 111645 2742

       %a1 = alloca < 3 x i32 >
       store <3 x i32> <i32 10, i32 11, i32 12>, <3 x i32>* %a1
       %a3 = alloca < 3 x i32 >
       store <3 x i32> <i32 110, i32 111, i32 112>, <3 x i32>* %a3
       %a2 = load <3 x i32>* %a1
       %a4 = load <3 x i32>* %a3
       
       %tmp11 = shufflevector <3 x i32> %a2, <3 x i32> %a4, <8 x i32> < i32 5, i32 4, i32 3, i32 2, i32 1, i32 0, i32 5, i32 4 >

       %r_tmp11_1 = extractelement <8 x i32> %tmp11, i32 0
       %r_tmp11_2 = extractelement <8 x i32> %tmp11, i32 1
       %r_tmp11_3 = extractelement <8 x i32> %tmp11, i32 2
       %r_tmp11_4 = extractelement <8 x i32> %tmp11, i32 3
       %r_tmp11_5 = extractelement <8 x i32> %tmp11, i32 4
       %r_tmp11_6 = extractelement <8 x i32> %tmp11, i32 5
       %r_tmp11_7 = extractelement <8 x i32> %tmp11, i32 6
       %r_tmp11_8 = extractelement <8 x i32> %tmp11, i32 7
       call i32 (i8*, ...)* @printf(i8* %ptr_i_8, i32 %r_tmp11_1, i32 %r_tmp11_2, i32 %r_tmp11_3, i32 %r_tmp11_4, i32 %r_tmp11_5, i32 %r_tmp11_6, i32 %r_tmp11_7, i32 %r_tmp11_8)
; CHECK: 112 111 110 12 11 10 112 111

       %tmp12 = shufflevector <3 x double> <double -10.4, double 11.0, double -12.5>, <3 x double> <double 110.7, double -111.0, double 112.8>, <4 x i32> < i32 5, i32 4, i32 1, i32 0 >

       %r_tmp12_1 = extractelement <4 x double> %tmp12, i32 0
       %r_tmp12_2 = extractelement <4 x double> %tmp12, i32 1
       %r_tmp12_3 = extractelement <4 x double> %tmp12, i32 2
       %r_tmp12_4 = extractelement <4 x double> %tmp12, i32 3

       call i32 (i8*, ...)* @printf(i8* %ptr_f_4, double %r_tmp12_1, double %r_tmp12_2, double %r_tmp12_3, double %r_tmp12_4)
; CHECK: 112.800000 -111.000000 11.000000 -10.400000
       
       ret i32 0
}

