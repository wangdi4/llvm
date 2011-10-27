; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

@r_int_1 = internal constant [4 x i8] c" %u\00"
@r_int_2 = internal constant [8 x i8] c" %u %u\0A\00"
@r_int_4 = internal constant [22 x i8] c" %llu %llu %llu %llu\0A\00"
@r_int_8 = internal constant [26 x i8] c" %u %u %u %u %u %u %u %u\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main()
{
    %ptr_1 = getelementptr [4 x i8]* @r_int_1, i32 0, i32 0 ; to printf
    %ptr_2 = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf
    %ptr_4 = getelementptr [22 x i8]* @r_int_4, i32 0, i32 0 ; to printf
    %ptr_8 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

    %tmp1 = fptoui <2 x float> <float 8.0, float -7.0> to <2 x i32>
    %r1_1 = extractelement <2 x i32> %tmp1, i32 0
    %r1_2 = extractelement <2 x i32> %tmp1, i32 1

    call i32 (i8*, ...)* @printf(i8* %ptr_2, i32 %r1_1, i32 %r1_2)
; CHECK: 8 4294967289

    %tmp2 = fptoui <3 x double> <double 18.0, double -17.0, double 0.197> to <3 x i16>
    %r2_1 = extractelement <3 x i16> %tmp2, i32 0
    %r2_2 = extractelement <3 x i16> %tmp2, i32 1
    %r2_3 = extractelement <3 x i16> %tmp2, i32 2
    %r3_1 = zext i16 %r2_1 to i32
    %r3_2 = zext i16 %r2_2 to i32
    %r3_3 = zext i16 %r2_3 to i32

    call i32 (i8*, ...)* @printf(i8* %ptr_1, i32 %r3_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_2, i32 %r3_2, i32 %r3_3)
; CHECK: 18 65519 0

    %tmp3 = fptoui <8 x float> <float -0.75, float 202.0, float 33.5, float 1.55e+07, float -55.0, float 68.75, float -33.5, float 81.25> to <8 x i8>
    %r4_1 = extractelement <8 x i8> %tmp3, i32 0
    %r4_2 = extractelement <8 x i8> %tmp3, i32 1
    %r4_3 = extractelement <8 x i8> %tmp3, i32 2
    %r4_4 = extractelement <8 x i8> %tmp3, i32 3
    %r4_5 = extractelement <8 x i8> %tmp3, i32 4
    %r4_6 = extractelement <8 x i8> %tmp3, i32 5
    %r4_7 = extractelement <8 x i8> %tmp3, i32 6
    %r4_8 = extractelement <8 x i8> %tmp3, i32 7
    %r5_1 = zext i8 %r4_1 to i32
    %r5_2 = zext i8 %r4_2 to i32
    %r5_3 = zext i8 %r4_3 to i32
    %r5_4 = zext i8 %r4_4 to i32
    %r5_5 = zext i8 %r4_5 to i32
    %r5_6 = zext i8 %r4_6 to i32
    %r5_7 = zext i8 %r4_7 to i32
    %r5_8 = zext i8 %r4_8 to i32

    call i32 (i8*, ...)* @printf(i8* %ptr_8, i32 %r5_1, i32 %r5_2, i32 %r5_3, i32 %r5_4, i32 %r5_5, i32 %r5_6, i32 %r5_7, i32 %r5_8)
; CHECK: 0 202 33 224 201 68 223 81

    %tmp4 = fptoui <4 x double> <double 1.8e+5, double -2.7e+10, double 3.97e+18, double -4.1e+2> to <4 x i64>
    %r6_1 = extractelement <4 x i64> %tmp4, i32 0
    %r6_2 = extractelement <4 x i64> %tmp4, i32 1
    %r6_3 = extractelement <4 x i64> %tmp4, i32 2
    %r6_4 = extractelement <4 x i64> %tmp4, i32 3

    call i32 (i8*, ...)* @printf(i8* %ptr_4, i64 %r6_1, i64 %r6_2, i64 %r6_3, i64 %r6_4)
; CHECK: 180000 18446744046709551616 3970000000000000000 18446744073709551206

    ret i32 0
}

