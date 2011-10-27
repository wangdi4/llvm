; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

@r_f = internal constant [5 x i8] c" %f\0A\00"
@r_i = internal constant [5 x i8] c" %d\0A\00"

declare i32 @printf(i8*, ...)

; <result> = extractelement <n x <ty>> <val>, i32 <idx>    ; yields <ty>

define i32 @main()
{
       %ptr = getelementptr [5 x i8]* @r_i, i32 0, i32 0 ; to printf
       %ptr_f = getelementptr [5 x i8]* @r_f, i32 0, i32 0 ; to printf

       %res_3_i8 = extractelement <3 x i8> <i8 102, i8 19, i8 27>, i32 0    ; yields i8
       %r_i32_i8_3 = sext i8 %res_3_i8 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_3)
; CHECK: 102

       %res_3_i16 = extractelement <3 x i16> <i16 12, i16 14, i16 16>, i32 1    ; yields i16
       %r_i32_i16_3 = sext i16 %res_3_i16 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i16_3)
; CHECK: 14

       %res_3_i32 = extractelement <3 x i32> <i32 1023, i32 9, i32 457>, i32 2    ; yields i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %res_3_i32)
; CHECK: 457

       %res_3_i64 = extractelement <3 x i64> <i64 12000, i64 100451, i64 95>, i32 0    ; yields i64
       call i32 (i8*, ...)* @printf(i8* %ptr, i64 %res_3_i64)
; CHECK: 12000

       %res_3_float = extractelement <3 x float> <float 5.0, float 4.0, float 3.0>, i32 1    ; yields float
       %res_float_double_3 = fpext float %res_3_float to double
       call i32 (i8*, ...)* @printf( i8* %ptr_f, double %res_float_double_3 )
; CHECK: 4.000000

       %res_3_double = extractelement <3 x double> <double 7.0, double 12.0, double 456.0>, i32 2    ; yields double
       call i32 (i8*, ...)* @printf (i8* %ptr_f, double %res_3_double )
; CHECK: 456.000000

       %res_8_i8 = extractelement <8 x i8> <i8 102, i8 19, i8 -27, i8 12, i8 109, i8 72, i8 91, i8 -55>, i32 7    ; yields i8
       %r_i32_i8_8 = sext i8 %res_8_i8 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_8)
; CHECK: -55

       %res_8_i16 = extractelement <8 x i16> <i16 1082, i16 1669, i16 2897, i16 -12, i16 1089, i16 -792, i16 9001, i16 5895>, i32 5    ; yields i16
       %r_i32_i16_8 = sext i16 %res_8_i16 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i16_8)
; CHECK: -792

       %res_8_i32 = extractelement <8 x i32> <i32 89102, i32 88819, i32 -24867, i32 -45812, i32 16909, i32 79278, i32 96891, i32 558777>, i32 3    ; yields i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %res_8_i32)
; CHECK: -45812

       %res_8_i64 = extractelement <8 x i64> <i64 -13476202, i64 16796989, i64 -22567, i64 3625612, i64 15867509, i64 7239772, i64 9451151, i64 5151525>, i32 1    ; yields i64
       call i32 (i8*, ...)* @printf(i8* %ptr, i64 %res_8_i64)
; CHECK: 16796989

       %res_8_float = extractelement <8 x float> <float 105.0, float -104.0, float 103.0, float 102.0, float 101.0, float -100.0, float 99.0, float 98.0>, i32 6    ; yields float
       %res_8_float_double = fpext float %res_8_float to double
       call i32 (i8*, ...)* @printf( i8* %ptr_f, double %res_8_float_double )
; CHECK: 99.000000

       %res_8_double = extractelement <8 x double> <double 37.0, double 1244.0, double 456.0, double 7.0, double -15462.0, double 456.0, double 2347.0, double 10562.0>, i32 4    ; yields double
       call i32 (i8*, ...)* @printf (i8* %ptr_f, double %res_8_double )
; CHECK: -15462.000000

       ret i32 0
}

