; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

@r_f_1 = internal constant [4 x i8] c" %f\00"
@r_f_2 = internal constant [8 x i8] c" %f %f\0A\00"
@r_f_4 = internal constant [14 x i8] c" %f %f %f %f\0A\00"
@r_f_8 = internal constant [26 x i8] c" %f %f %f %f %f %f %f %f\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main()
{
    %ptr_1 = getelementptr [4 x i8]* @r_f_1, i32 0, i32 0 ; to printf
    %ptr_2 = getelementptr [8 x i8]* @r_f_2, i32 0, i32 0 ; to printf
    %ptr_4 = getelementptr [14 x i8]* @r_f_4, i32 0, i32 0 ; to printf
    %ptr_8 = getelementptr [26 x i8]* @r_f_8, i32 0, i32 0 ; to printf

    %tmp1 = uitofp <2 x i32> <i32 8860, i32 -7001> to <2 x float>
    %r1_1 = extractelement <2 x float> %tmp1, i32 0
    %r1_2 = extractelement <2 x float> %tmp1, i32 1
    %r2_1 = fpext float %r1_1 to double
    %r2_2 = fpext float %r1_2 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_2, double %r2_1, double %r2_2)
; CHECK: 8860.000000 4294960384.000000

    %tmp2 = uitofp <8 x i8> <i8 1, i8 -1, i8 120, i8 -120, i8 4, i8 -4, i8 19, i8 -19 > to <8 x double>
    %r3_1 = extractelement <8 x double> %tmp2, i32 0
    %r3_2 = extractelement <8 x double> %tmp2, i32 1
    %r3_3 = extractelement <8 x double> %tmp2, i32 2
    %r3_4 = extractelement <8 x double> %tmp2, i32 3
    %r3_5 = extractelement <8 x double> %tmp2, i32 4
    %r3_6 = extractelement <8 x double> %tmp2, i32 5
    %r3_7 = extractelement <8 x double> %tmp2, i32 6
    %r3_8 = extractelement <8 x double> %tmp2, i32 7

    call i32 (i8*, ...)* @printf(i8* %ptr_8, double %r3_1, double %r3_2, double %r3_3, double %r3_4, double %r3_5, double %r3_6, double %r3_7, double %r3_8)
; CHECK: 1.000000 255.000000 120.000000 136.000000 4.000000 252.000000 19.000000 237.000000

    %tmp3 = uitofp <4 x i16> <i16 1000, i16 -1000, i16 127, i16 -127> to <4 x float>
    %r4_1 = extractelement <4 x float> %tmp3, i32 0
    %r4_2 = extractelement <4 x float> %tmp3, i32 1
    %r4_3 = extractelement <4 x float> %tmp3, i32 2
    %r4_4 = extractelement <4 x float> %tmp3, i32 3
    %r5_1 = fpext float %r4_1 to double
    %r5_2 = fpext float %r4_2 to double
    %r5_3 = fpext float %r4_3 to double
    %r5_4 = fpext float %r4_4 to double

    call i32 (i8*, ...)* @printf(i8* %ptr_4, double %r5_1, double %r5_2, double %r5_3, double %r5_4)
; CHECK: 1000.000000 64536.000000 127.000000 65409.000000

    %tmp4 = uitofp <3 x i64> <i64 1000001, i64 1, i64 9999999999999999> to <3 x double>
    %r6_1 = extractelement <3 x double> %tmp4, i32 0
    %r6_2 = extractelement <3 x double> %tmp4, i32 1
    %r6_3 = extractelement <3 x double> %tmp4, i32 2

    call i32 (i8*, ...)* @printf(i8* %ptr_1, double %r6_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_2, double %r6_2, double %r6_3)
; CHECK: 1000001.000000 1.000000 10000000000000000.000000

    ret i32 0
}

