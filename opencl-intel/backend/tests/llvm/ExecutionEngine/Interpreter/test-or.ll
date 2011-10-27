; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

@r_int_2 = internal constant [8 x i8] c" %d %d\0A\00"
@r_int_8 = internal constant [26 x i8] c" %d %d %d %d %d %d %d %d\0A\00"

declare i32 @printf(i8*, ...)

; <result> = or <ty> <op1>, <op2>   ; yields {ty}:result

define i32 @or_2()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

       %res_i8 = or <2 x i8> < i8 5, i8 2>, < i8 1, i8 3>

       %r_i8_1 = extractelement <2 x i8> %res_i8, i32 0
       %r_i8_2 = extractelement <2 x i8> %res_i8, i32 1
       %r_i32_i8_1 = sext i8 %r_i8_1 to i32
       %r_i32_i8_2 = sext i8 %r_i8_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2)
; CHECK: 5 3

       %res_i16 = or <2 x i16> < i16 105, i16 207>, < i16 12, i16 39>

       %r_i16_1 = extractelement <2 x i16> %res_i16, i32 0
       %r_i16_2 = extractelement <2 x i16> %res_i16, i32 1
       %r_i32_i16_1 = sext i16 %r_i16_1 to i32
       %r_i32_i16_2 = sext i16 %r_i16_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i16_1, i32 %r_i32_i16_2)
; CHECK: 109 239

       %res_i32 = or <2 x i32> < i32 589, i32 2999>, < i32 781, i32 333>

       %r_i32_1 = extractelement <2 x i32> %res_i32, i32 0
       %r_i32_2 = extractelement <2 x i32> %res_i32, i32 1
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_1, i32 %r_i32_2)
; CHECK: 845 3071

       %res_i64 = or <2 x i64> < i64 145675, i64 4562>, < i64 109999, i64 45603>

       %r_i64_1 = extractelement <2 x i64> %res_i64, i32 0
       %r_i64_2 = extractelement <2 x i64> %res_i64, i32 1
       call i32 (i8*, ...)* @printf(i8* %ptr, i64 %r_i64_1, i64 %r_i64_2)
; CHECK: 245167 46067

       ret i32 0
}

define i32 @or_8_i8()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i8 = or <8 x i8> < i8 15, i8 12, i8 25, i8 22, i8 35, i8 32, i8 45, i8 42>, < i8 1, i8 3, i8 11, i8 13, i8 21, i8 23, i8 31, i8 33>

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
; CHECK: 15 15 27 31 55 55 63 43

       ret i32 0
}

define i32 @or_8_i16()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i16 = or <8 x i16> < i16 15, i16 12, i16 25, i16 22, i16 35, i16 32, i16 45, i16 42>, < i16 1, i16 3, i16 11, i16 13, i16 21, i16 23, i16 31, i16 33>

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
; CHECK: 15 15 27 31 55 55 63 43

       ret i32 0
}


define i32 @or_8_i32()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i32 = or <8 x i32> < i32 215, i32 212, i32 325, i32 422, i32 535, i32 632, i32 645, i32 742>, < i32 451, i32 33, i32 611, i32 513, i32 221, i32 8823, i32 631, i32 9933>

       %r_i32_1 = extractelement <8 x i32> %res_i32, i32 0
       %r_i32_2 = extractelement <8 x i32> %res_i32, i32 1
       %r_i32_3 = extractelement <8 x i32> %res_i32, i32 2
       %r_i32_4 = extractelement <8 x i32> %res_i32, i32 3
       %r_i32_5 = extractelement <8 x i32> %res_i32, i32 4
       %r_i32_6 = extractelement <8 x i32> %res_i32, i32 5
       %r_i32_7 = extractelement <8 x i32> %res_i32, i32 6
       %r_i32_8 = extractelement <8 x i32> %res_i32, i32 7

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_1, i32 %r_i32_2, i32 %r_i32_3, i32 %r_i32_4, i32 %r_i32_5, i32 %r_i32_6, i32 %r_i32_7, i32 %r_i32_8)
; CHECK: 471 245 871 935 735 8831 759 9967

       ret i32 0
}

define i32 @or_8_i64()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i64 = or <8 x i64> < i64 45615, i64 4612, i64 78925, i64 34522, i64 6735, i64 95632, i64 85645, i64 1342>, < i64 2341, i64 23, i64 311, i64 22213, i64 784621, i64 6723, i64 731, i64 99933>

       %r_i64_1 = extractelement <8 x i64> %res_i64, i32 0
       %r_i64_2 = extractelement <8 x i64> %res_i64, i32 1
       %r_i64_3 = extractelement <8 x i64> %res_i64, i32 2
       %r_i64_4 = extractelement <8 x i64> %res_i64, i32 3
       %r_i64_5 = extractelement <8 x i64> %res_i64, i32 4
       %r_i64_6 = extractelement <8 x i64> %res_i64, i32 5
       %r_i64_7 = extractelement <8 x i64> %res_i64, i32 6
       %r_i64_8 = extractelement <8 x i64> %res_i64, i32 7

       call i32 (i8*, ...)* @printf(i8* %ptr, i64 %r_i64_1, i64 %r_i64_2, i64 %r_i64_3, i64 %r_i64_4, i64 %r_i64_5, i64 %r_i64_6, i64 %r_i64_7, i64 %r_i64_8)
; CHECK: 47919 4631 79231 55007 785135 98259 85727 100223

       ret i32 0
}
define i32 @main()
{
       call i32 @or_2()
       call i32 @or_8_i8()
       call i32 @or_8_i16()
       call i32 @or_8_i32()
       call i32 @or_8_i64()
       ret i32 0
}
