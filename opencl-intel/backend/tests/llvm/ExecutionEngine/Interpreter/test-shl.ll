; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

@r_int_2 = internal constant [8 x i8] c" %d %d\0A\00"
@r_int_8 = internal constant [26 x i8] c" %d %d %d %d %d %d %d %d\0A\00"

declare i32 @printf(i8*, ...)

; <result> = shl <ty> <op1>, <op2>   ; yields {ty}:result
; If op2 is (statically or dynamically) negative or equal to or larger than the number of bits in op1, the result is undefined

define i32 @shl_2()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

       %res_i8 = shl <2 x i8> < i8 1, i8 2 >, < i8 2, i8 3 >

       %r_i8_1 = extractelement <2 x i8> %res_i8, i32 0
       %r_i8_2 = extractelement <2 x i8> %res_i8, i32 1
       %r_i32_i8_1 = sext i8 %r_i8_1 to i32
       %r_i32_i8_2 = sext i8 %r_i8_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2)
; CHECK:  4 16

       %res_i16 = shl <2 x i16> < i16 22, i16 33 >, < i16 5, i16 8 >

       %r_i16_1 = extractelement <2 x i16> %res_i16, i32 0
       %r_i16_2 = extractelement <2 x i16> %res_i16, i32 1
       %r_i32_i16_1 = sext i16 %r_i16_1 to i32
       %r_i32_i16_2 = sext i16 %r_i16_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i16_1, i32 %r_i32_i16_2)
; CHECK:  704 8448

       %res_i32 = shl <2 x i32> < i32 444, i32 555 >, < i32 12, i32 13 >

       %r_i32_1 = extractelement <2 x i32> %res_i32, i32 0
       %r_i32_2 = extractelement <2 x i32> %res_i32, i32 1
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_1, i32 %r_i32_2)
; CHECK:  1818624 4546560

       %res_i64 = shl <2 x i64> < i64 55, i64 79 >, < i64 31, i64 15 >

       %r_i64_1 = extractelement <2 x i64> %res_i64, i32 0
       %r_i64_2 = extractelement <2 x i64> %res_i64, i32 1
       call i32 (i8*, ...)* @printf(i8* %ptr, i64 %r_i64_1, i64 %r_i64_2)
; CHECK:  -2147483648 2588672

       ret i32 0
}

define i32 @shl_8_i8()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i8 = shl <8 x i8> < i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 8>, < i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 1>

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
; CHECK:  2 8 24 64 -96 -128 -128 16

       ret i32 0
}

define i32 @shl_8_i16()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i16 = shl <8 x i16> < i16 11, i16 22, i16 33, i16 44, i16 55, i16 66, i16 77, i16 88>, < i16 10, i16 2, i16 3, i16 4, i16 5, i16 6, i16 7, i16 15>

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
; CHECK:  11264 88 264 704 1760 4224 9856 0

       ret i32 0
}

define i32 @shl_8_i32()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i32 = and <8 x i32> < i32 215, i32 212, i32 325, i32 422, i32 535, i32 632, i32 645, i32 742>, < i32 2, i32 15, i32 7, i32 3, i32 21, i32 9, i32 31, i32 4>

       %r_i32_1 = extractelement <8 x i32> %res_i32, i32 0
       %r_i32_2 = extractelement <8 x i32> %res_i32, i32 1
       %r_i32_3 = extractelement <8 x i32> %res_i32, i32 2
       %r_i32_4 = extractelement <8 x i32> %res_i32, i32 3
       %r_i32_5 = extractelement <8 x i32> %res_i32, i32 4
       %r_i32_6 = extractelement <8 x i32> %res_i32, i32 5
       %r_i32_7 = extractelement <8 x i32> %res_i32, i32 6
       %r_i32_8 = extractelement <8 x i32> %res_i32, i32 7

       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_1, i32 %r_i32_2, i32 %r_i32_3, i32 %r_i32_4, i32 %r_i32_5, i32 %r_i32_6, i32 %r_i32_7, i32 %r_i32_8)
; CHECK:  2 4 5 2 21 8 5 4

       ret i32 0
}

define i32 @shl_8_i64()
{
       %ptr = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf

       %res_i64 = and <8 x i64> < i64 8215, i64 67212, i64 8325, i64 80422, i64 7, i64 56632, i64 111645, i64 2742>, < i64 2, i64 15, i64 27, i64 53, i64 21, i64 9, i64 31, i64 4>

       %r_i64_1 = extractelement <8 x i64> %res_i64, i32 0
       %r_i64_2 = extractelement <8 x i64> %res_i64, i32 1
       %r_i64_3 = extractelement <8 x i64> %res_i64, i32 2
       %r_i64_4 = extractelement <8 x i64> %res_i64, i32 3
       %r_i64_5 = extractelement <8 x i64> %res_i64, i32 4
       %r_i64_6 = extractelement <8 x i64> %res_i64, i32 5
       %r_i64_7 = extractelement <8 x i64> %res_i64, i32 6
       %r_i64_8 = extractelement <8 x i64> %res_i64, i32 7

       call i32 (i8*, ...)* @printf(i8* %ptr, i64 %r_i64_1, i64 %r_i64_2, i64 %r_i64_3, i64 %r_i64_4, i64 %r_i64_5, i64 %r_i64_6, i64 %r_i64_7, i64 %r_i64_8)
; CHECK:  2 12 1 36 5 8 29 4

       ret i32 0
}

define i32 @main()
{
       call i32 @shl_2()
       call i32 @shl_8_i8()
       call i32 @shl_8_i16()
       call i32 @shl_8_i32()
       call i32 @shl_8_i64()
       ret i32 0
}

