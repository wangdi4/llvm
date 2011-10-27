; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

@r_i64_1 = internal constant [6 x i8] c" %lld\00"
@r_i32_2 = internal constant [8 x i8] c" %d %d\0A\00"
@r_i64_2 = internal constant [12 x i8] c" %lld %lld\0A\00"
@r_i64_4 = internal constant [22 x i8] c" %lld %lld %lld %lld\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main()
{
    %ptr_1 = getelementptr [6 x i8]* @r_i64_1, i32 0, i32 0 ; to printf
    %ptr_2 = getelementptr [8 x i8]* @r_i32_2, i32 0, i32 0 ; to printf
    %ptr_3 = getelementptr [12 x i8]* @r_i64_2, i32 0, i32 0 ; to printf
    %ptr_4 = getelementptr [22 x i8]* @r_i64_4, i32 0, i32 0 ; to printf

    %tmp1 = sext <2 x i8> <i8 8, i8 -7> to <2 x i32>
    %r1_1 = extractelement <2 x i32> %tmp1, i32 0
    %r1_2 = extractelement <2 x i32> %tmp1, i32 1

    call i32 (i8*, ...)* @printf(i8* %ptr_2, i32 %r1_1, i32 %r1_2)
; CHECK: 8 -7

    %tmp2 = sext <3 x i32> <i32 18, i32 -170, i32 197> to <3 x i64>
    %r2_1 = extractelement <3 x i64> %tmp2, i32 0
    %r2_2 = extractelement <3 x i64> %tmp2, i32 1
    %r2_3 = extractelement <3 x i64> %tmp2, i32 2

    call i32 (i8*, ...)* @printf(i8* %ptr_1, i64 %r2_1)
    call i32 (i8*, ...)* @printf(i8* %ptr_3, i64 %r2_2, i64 %r2_3)
; CHECK: 18 -170 197


    %tmp3 = sext <4 x i16> <i16 185, i16 -2710, i16 3918, i16 -412> to <4 x i64>
    %r3_1 = extractelement <4 x i64> %tmp3, i32 0
    %r3_2 = extractelement <4 x i64> %tmp3, i32 1
    %r3_3 = extractelement <4 x i64> %tmp3, i32 2
    %r3_4 = extractelement <4 x i64> %tmp3, i32 3

    call i32 (i8*, ...)* @printf(i8* %ptr_4, i64 %r3_1, i64 %r3_2, i64 %r3_3, i64 %r3_4)
; CHECK: 185 -2710 3918 -412

    ret i32 0
}

