; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

@r_int_2 = internal constant [8 x i8] c" %d %d\0A\00"
@r_int_8 = internal constant [26 x i8] c" %d %d %d %d %d %d %d %d\0A\00"
@r_float_2 = internal constant [8 x i8] c" %f %f\0A\00"
@r_float_8 = internal constant [26 x i8] c" %f %f %f %f %f %f %f %f\0A\00"

declare i32 @printf(i8*, ...)

; <result> = insertelement <n x <ty>> <val>, <ty> <elt>, i32 <idx>    ; yields <n x <ty>>

define i32 @insertelement_2()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf
       %ptr_f = getelementptr [8 x i8]* @r_float_2, i32 0, i32 0 ; to printf

       %res_i8_start = insertelement <2 x i8> <i8 21, i8 31>, i8 51, i32 0    ; yields i8
       %res_i8 = insertelement <2 x i8> %res_i8_start, i8 41, i32 1    ; yields i8

       %r_i8_1 = extractelement <2 x i8> %res_i8, i32 0
       %r_i8_2 = extractelement <2 x i8> %res_i8, i32 1
       %r_i32_i8_1 = sext i8 %r_i8_1 to i32
       %r_i32_i8_2 = sext i8 %r_i8_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2)
; CHECK: 51 41

       %res_i16_start = insertelement <2 x i16> <i16 17, i16 14>, i16 5, i32 1    ; yields i16
       %res_i16 = insertelement <2 x i16> %res_i16_start, i16 12, i32 0    ; yields i16       

       %r_i16_1 = extractelement <2 x i16> %res_i16, i32 0    ; yields i16
       %r_i16_2 = extractelement <2 x i16> %res_i16, i32 1    ; yields i16
       %r_i32_i16_1 = sext i16 %r_i16_1 to i32
       %r_i32_i16_2 = sext i16 %r_i16_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i16_1, i32 %r_i32_i16_2)
; CHECK: 12 5

       %res_i32_start = insertelement <2 x i32> <i32 1023, i32 -1024>, i32 345, i32 0    ; yields i32
       %res_i32 = insertelement <2 x i32> %res_i32_start, i32 9, i32 1    ; yields i32

       %r_i32_1 = extractelement <2 x i32> %res_i32, i32 0
       %r_i32_2 = extractelement <2 x i32> %res_i32, i32 1
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_1, i32 %r_i32_2)
; CHECK: 345 9

       %res_i64_start = insertelement <2 x i64> <i64 10, i64 100451>, i64 9999, i32 1    ; yields i64
       %res_i64 = insertelement <2 x i64> %res_i64_start, i64 12000, i32 0    ; yields i64

       %r_i64_1 = extractelement <2 x i64> %res_i64, i32 0
       %r_i64_2 = extractelement <2 x i64> %res_i64, i32 1
       call i32 (i8*, ...)* @printf(i8* %ptr, i64 %r_i64_1, i64 %r_i64_2)
; CHECK: 12000 9999

       %res_float_start = insertelement <2 x float> <float 53.0, float 54.0>, float 5.0, i32 0    ; yields float
       %res_float = insertelement <2 x float> %res_float_start, float 4.0, i32 1    ; yields float

       %r_float_1 = extractelement <2 x float> %res_float, i32 0
       %r_float_2 = extractelement <2 x float> %res_float, i32 1
       %r_out_1 = fpext float %r_float_1 to double
       %r_out_2 = fpext float %r_float_2 to double
       call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r_out_1, double %r_out_2)
; CHECK: 5.000000 4.000000

       %res_double_start = insertelement <2 x double> <double 17.0, double 8.0>, double 9.0, i32 1    ; yields double
       %res_double = insertelement <2 x double> %res_double_start, double 7.0, i32 0    ; yields double

       %r_double_1 = extractelement <2 x double> %res_double, i32 0
       %r_double_2 = extractelement <2 x double> %res_double, i32 1
       call i32 (i8*, ...)* @printf(i8* %ptr_f, double %r_double_1, double %r_double_2)
; CHECK: 7.000000 9.000000

       ret i32 0
}

define i32 @insertelement_8_i8()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i8 = insertelement <8 x i8> < i8 126, i8 28, i8 34, i8 47, i8 56, i8 67, i8 78, i8 89>, i8 2, i32 0

       %r_i8_1 = extractelement <8 x i8> %res_i8, i32 0
       %r_i8_2 = extractelement <8 x i8> %res_i8, i32 1
       %r_i8_3 = extractelement <8 x i8> %res_i8, i32 2
       %r_i8_4 = extractelement <8 x i8> %res_i8, i32 3
       %r_i8_5 = extractelement <8 x i8> %res_i8, i32 4
       %r_i8_6 = extractelement <8 x i8> %res_i8, i32 5
       %r_i8_7 = extractelement <8 x i8> %res_i8, i32 6
       %r_i8_8 = extractelement <8 x i8> %res_i8, i32 7
       %r_i32_i8_1 = sext i8 %r_i8_1 to i32
       %r_i32_i8_2 = sext i8 %r_i8_2 to i32
       %r_i32_i8_3 = sext i8 %r_i8_3 to i32
       %r_i32_i8_4 = sext i8 %r_i8_4 to i32
       %r_i32_i8_5 = sext i8 %r_i8_5 to i32
       %r_i32_i8_6 = sext i8 %r_i8_6 to i32
       %r_i32_i8_7 = sext i8 %r_i8_7 to i32
       %r_i32_i8_8 = sext i8 %r_i8_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2, i32 %r_i32_i8_3, i32 %r_i32_i8_4, i32 %r_i32_i8_5, i32 %r_i32_i8_6, i32 %r_i32_i8_7, i32 %r_i32_i8_8)
; CHECK: 2 28 34 47 56 67 78 89

       ret i32 0
}

define i32 @insertelement_8_i16()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i16 = insertelement <8 x i16> < i16 1100, i16 -2257, i16 3378, i16 -44, i16 55, i16 -66, i16 9777, i16 8088>, i16 86, i32 1

       %r_i16_1 = extractelement <8 x i16> %res_i16, i32 0
       %r_i16_2 = extractelement <8 x i16> %res_i16, i32 1
       %r_i16_3 = extractelement <8 x i16> %res_i16, i32 2
       %r_i16_4 = extractelement <8 x i16> %res_i16, i32 3
       %r_i16_5 = extractelement <8 x i16> %res_i16, i32 4
       %r_i16_6 = extractelement <8 x i16> %res_i16, i32 5
       %r_i16_7 = extractelement <8 x i16> %res_i16, i32 6
       %r_i16_8 = extractelement <8 x i16> %res_i16, i32 7
       %r_i32_i16_1 = sext i16 %r_i16_1 to i32
       %r_i32_i16_2 = sext i16 %r_i16_2 to i32
       %r_i32_i16_3 = sext i16 %r_i16_3 to i32
       %r_i32_i16_4 = sext i16 %r_i16_4 to i32
       %r_i32_i16_5 = sext i16 %r_i16_5 to i32
       %r_i32_i16_6 = sext i16 %r_i16_6 to i32
       %r_i32_i16_7 = sext i16 %r_i16_7 to i32
       %r_i32_i16_8 = sext i16 %r_i16_8 to i32

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i16_1, i32 %r_i32_i16_2, i32 %r_i32_i16_3, i32 %r_i32_i16_4, i32 %r_i32_i16_5, i32 %r_i32_i16_6, i32 %r_i32_i16_7, i32 %r_i32_i16_8)
; CHECK: 1100 86 3378 -44 55 -66 9777 8088

       ret i32 0
}

define i32 @insertelement_8_i32()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i32 = insertelement <8 x i32> < i32 215478, i32 21267, i32 -325, i32 -422, i32 -53556, i32 6302, i32 688845, i32 -678742>, i32 32, i32 2

       %r_i32_1 = extractelement <8 x i32> %res_i32, i32 0
       %r_i32_2 = extractelement <8 x i32> %res_i32, i32 1
       %r_i32_3 = extractelement <8 x i32> %res_i32, i32 2
       %r_i32_4 = extractelement <8 x i32> %res_i32, i32 3
       %r_i32_5 = extractelement <8 x i32> %res_i32, i32 4
       %r_i32_6 = extractelement <8 x i32> %res_i32, i32 5
       %r_i32_7 = extractelement <8 x i32> %res_i32, i32 6
       %r_i32_8 = extractelement <8 x i32> %res_i32, i32 7

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_1, i32 %r_i32_2, i32 %r_i32_3, i32 %r_i32_4, i32 %r_i32_5, i32 %r_i32_6, i32 %r_i32_7, i32 %r_i32_8)
; CHECK: 215478 21267 32 -422 -53556 6302 688845 -678742

       ret i32 0
}

define i32 @insertelement_8_i64()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i64 = insertelement <8 x i64> < i64 5788215, i64 -67212, i64 -8325, i64 80422, i64 -7, i64 56689632, i64 111645, i64 2742>, i64 55, i32 3

       %r_i64_1 = extractelement <8 x i64> %res_i64, i32 0
       %r_i64_2 = extractelement <8 x i64> %res_i64, i32 1
       %r_i64_3 = extractelement <8 x i64> %res_i64, i32 2
       %r_i64_4 = extractelement <8 x i64> %res_i64, i32 3
       %r_i64_5 = extractelement <8 x i64> %res_i64, i32 4
       %r_i64_6 = extractelement <8 x i64> %res_i64, i32 5
       %r_i64_7 = extractelement <8 x i64> %res_i64, i32 6
       %r_i64_8 = extractelement <8 x i64> %res_i64, i32 7

       call i32 (i8*, ...)* @printf(i8* %ptr, i64 %r_i64_1, i64 %r_i64_2, i64 %r_i64_3, i64 %r_i64_4, i64 %r_i64_5, i64 %r_i64_6, i64 %r_i64_7, i64 %r_i64_8)
; CHECK: 5788215 -67212 -8325 55 -7 56689632 111645 2742

       ret i32 0
}

define i32 @insertelement_8_float()
{
       %ptr = getelementptr [26 x i8]* @r_float_8, i32 0, i32 0 ; to printf

       %res_float = insertelement <8 x float> < float 7.0, float -5788.0, float 832.5, float 367.0, float 7.0, float 3.0, float 4.0, float 5.0>, float 199.0, i32 4

       %r_float_1 = extractelement <8 x float> %res_float, i32 0
       %r_float_2 = extractelement <8 x float> %res_float, i32 1
       %r_float_3 = extractelement <8 x float> %res_float, i32 2
       %r_float_4 = extractelement <8 x float> %res_float, i32 3
       %r_float_5 = extractelement <8 x float> %res_float, i32 4
       %r_float_6 = extractelement <8 x float> %res_float, i32 5
       %r_float_7 = extractelement <8 x float> %res_float, i32 6
       %r_float_8 = extractelement <8 x float> %res_float, i32 7
       %r_double_float_1 = fpext float %r_float_1 to double
       %r_double_float_2 = fpext float %r_float_2 to double
       %r_double_float_3 = fpext float %r_float_3 to double
       %r_double_float_4 = fpext float %r_float_4 to double
       %r_double_float_5 = fpext float %r_float_5 to double
       %r_double_float_6 = fpext float %r_float_6 to double
       %r_double_float_7 = fpext float %r_float_7 to double
       %r_double_float_8 = fpext float %r_float_8 to double

       call i32 (i8*, ...)* @printf(i8* %ptr, double %r_double_float_1, double %r_double_float_2, double %r_double_float_3, double %r_double_float_4, double %r_double_float_5, double %r_double_float_6, double %r_double_float_7, double %r_double_float_8)
; CHECK: 7.000000 -5788.000000 832.500000 367.000000 199.000000 3.000000 4.000000 5.000000

       ret i32 0
}

define i32 @insertelement_8_double()
{
       %ptr = getelementptr [26 x i8]* @r_float_8, i32 0, i32 0 ; to printf

       %res_double = insertelement <8 x double> < double 578821.5, double -6721.2, double -832.5, double 8042.2, double -7.0, double 5668963.2, double 1116.45, double 274.2>, double 4.57, i32 5

       %r_double_1 = extractelement <8 x double> %res_double, i32 0
       %r_double_2 = extractelement <8 x double> %res_double, i32 1
       %r_double_3 = extractelement <8 x double> %res_double, i32 2
       %r_double_4 = extractelement <8 x double> %res_double, i32 3
       %r_double_5 = extractelement <8 x double> %res_double, i32 4
       %r_double_6 = extractelement <8 x double> %res_double, i32 5
       %r_double_7 = extractelement <8 x double> %res_double, i32 6
       %r_double_8 = extractelement <8 x double> %res_double, i32 7

       call i32 (i8*, ...)* @printf(i8* %ptr, double %r_double_1, double %r_double_2, double %r_double_3, double %r_double_4, double %r_double_5, double %r_double_6, double %r_double_7, double %r_double_8)
; CHECK: 578821.500000 -6721.200000 -832.500000 8042.200000 -7.000000 4.570000 1116.450000 274.200000

       ret i32 0
}

define i32 @main()
{
       call i32 @insertelement_2()
       call i32 @insertelement_8_i8()
       call i32 @insertelement_8_i16()
       call i32 @insertelement_8_i32()
       call i32 @insertelement_8_i64()
       call i32 @insertelement_8_float()
       call i32 @insertelement_8_double()

       ret i32 0
}

