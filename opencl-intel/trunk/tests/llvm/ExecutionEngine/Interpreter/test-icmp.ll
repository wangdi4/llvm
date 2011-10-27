; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

@r_int_2 = internal constant [8 x i8] c" %d %d\0A\00"
@r_int_8 = internal constant [26 x i8] c" %d %d %d %d %d %d %d %d\0A\00"

declare i32 @printf(i8*, ...)

define i32 @icmp_eq()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

       %res_i8 = icmp eq <2 x i8> < i8 5, i8 2>, < i8 5, i8 3>

       %r_i8_1 = extractelement <2 x i1> %res_i8, i32 0
       %r_i8_2 = extractelement <2 x i1> %res_i8, i32 1
       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2)
; CHECK: 1 0

       %ptr2 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %res_i16 = icmp eq <8 x i16> < i16 15, i16 12, i16 32000, i16 1453, i16 35, i16 -4332, i16 45, i16 -4212>, < i16 1,  i16 3,  i16 11,    i16 1453, i16 21, i16 -4332, i16 -3211, i16 33>

       %r_i16_1 = extractelement <8 x i1> %res_i16, i32 0
       %r_i16_2 = extractelement <8 x i1> %res_i16, i32 1
       %r_i16_3 = extractelement <8 x i1> %res_i16, i32 2
       %r_i16_4 = extractelement <8 x i1> %res_i16, i32 3
       %r_i16_5 = extractelement <8 x i1> %res_i16, i32 4
       %r_i16_6 = extractelement <8 x i1> %res_i16, i32 5
       %r_i16_7 = extractelement <8 x i1> %res_i16, i32 6
       %r_i16_8 = extractelement <8 x i1> %res_i16, i32 7
       %r_i32_i16_1 = zext i1 %r_i16_1 to i32
       %r_i32_i16_2 = zext i1 %r_i16_2 to i32
       %r_i32_i16_3 = zext i1 %r_i16_3 to i32
       %r_i32_i16_4 = zext i1 %r_i16_4 to i32
       %r_i32_i16_5 = zext i1 %r_i16_5 to i32
       %r_i32_i16_6 = zext i1 %r_i16_6 to i32
       %r_i32_i16_7 = zext i1 %r_i16_7 to i32
       %r_i32_i16_8 = zext i1 %r_i16_8 to i32
       
       call i32 (i8*, ...)* @printf(i8* %ptr2, i32 %r_i32_i16_1, i32 %r_i32_i16_2, i32 %r_i32_i16_3, i32 %r_i32_i16_4, i32 %r_i32_i16_5, i32 %r_i32_i16_6, i32 %r_i32_i16_7, i32 %r_i32_i16_8)
; CHECK: 0 0 0 1 0 1 0 0
       ret i32 0
}


define i32 @icmp_ne()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

       %res_i8 = icmp ne <2 x i8> < i8 5, i8 2>, < i8 5, i8 3>

       %r_i8_1 = extractelement <2 x i1> %res_i8, i32 0
       %r_i8_2 = extractelement <2 x i1> %res_i8, i32 1
       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2)
; CHECK: 0 1

       %ptr2 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %res_i16 = icmp ne <8 x i16> < i16 15, i16 12, i16 32000, i16 1453, i16 35, i16 -4332, i16 45, i16 -4212>, < i16 1,  i16 3,  i16 11,    i16 1453, i16 21, i16 -4332, i16 -3211, i16 33>

       %r_i16_1 = extractelement <8 x i1> %res_i16, i32 0
       %r_i16_2 = extractelement <8 x i1> %res_i16, i32 1
       %r_i16_3 = extractelement <8 x i1> %res_i16, i32 2
       %r_i16_4 = extractelement <8 x i1> %res_i16, i32 3
       %r_i16_5 = extractelement <8 x i1> %res_i16, i32 4
       %r_i16_6 = extractelement <8 x i1> %res_i16, i32 5
       %r_i16_7 = extractelement <8 x i1> %res_i16, i32 6
       %r_i16_8 = extractelement <8 x i1> %res_i16, i32 7
       %r_i32_i16_1 = zext i1 %r_i16_1 to i32
       %r_i32_i16_2 = zext i1 %r_i16_2 to i32
       %r_i32_i16_3 = zext i1 %r_i16_3 to i32
       %r_i32_i16_4 = zext i1 %r_i16_4 to i32
       %r_i32_i16_5 = zext i1 %r_i16_5 to i32
       %r_i32_i16_6 = zext i1 %r_i16_6 to i32
       %r_i32_i16_7 = zext i1 %r_i16_7 to i32
       %r_i32_i16_8 = zext i1 %r_i16_8 to i32
       
       call i32 (i8*, ...)* @printf(i8* %ptr2, i32 %r_i32_i16_1, i32 %r_i32_i16_2, i32 %r_i32_i16_3, i32 %r_i32_i16_4, i32 %r_i32_i16_5, i32 %r_i32_i16_6, i32 %r_i32_i16_7, i32 %r_i32_i16_8)
; CHECK: 1 1 1 0 1 0 1 1
       ret i32 0
}

define i32 @icmp_ugt()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

; uint64 maximum value
       %res_i64 = icmp ugt <2 x i64> < i64 18446744073709551615, i64 3>, < i64 18446744073709551614, i64 3>

       %r_i64_1 = extractelement <2 x i1> %res_i64, i32 0
       %r_i64_2 = extractelement <2 x i1> %res_i64, i32 1
       %r_i32_i64_1 = zext i1 %r_i64_1 to i32
       %r_i32_i64_2 = zext i1 %r_i64_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i64_1, i32 %r_i32_i64_2)
; CHECK: 1 0

       %ptr2 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %res_i16 = icmp ugt <8 x i16> < i16 1, i16 3, i16 32000, i16 1453, i16 35, i16 4332, i16 45, i16 4212>,  < i16 1,  i16 12,  i16 11,    i16 1453, i16 21, i16 4332, i16 3211, i16 33>
       %r_i16_1 = extractelement <8 x i1> %res_i16, i32 0
       %r_i16_2 = extractelement <8 x i1> %res_i16, i32 1
       %r_i16_3 = extractelement <8 x i1> %res_i16, i32 2
       %r_i16_4 = extractelement <8 x i1> %res_i16, i32 3
       %r_i16_5 = extractelement <8 x i1> %res_i16, i32 4
       %r_i16_6 = extractelement <8 x i1> %res_i16, i32 5
       %r_i16_7 = extractelement <8 x i1> %res_i16, i32 6
       %r_i16_8 = extractelement <8 x i1> %res_i16, i32 7
       %r_i32_i16_1 = zext i1 %r_i16_1 to i32
       %r_i32_i16_2 = zext i1 %r_i16_2 to i32
       %r_i32_i16_3 = zext i1 %r_i16_3 to i32
       %r_i32_i16_4 = zext i1 %r_i16_4 to i32
       %r_i32_i16_5 = zext i1 %r_i16_5 to i32
       %r_i32_i16_6 = zext i1 %r_i16_6 to i32
       %r_i32_i16_7 = zext i1 %r_i16_7 to i32
       %r_i32_i16_8 = zext i1 %r_i16_8 to i32
       
       call i32 (i8*, ...)* @printf(i8* %ptr2, i32 %r_i32_i16_1, i32 %r_i32_i16_2, i32 %r_i32_i16_3, i32 %r_i32_i16_4, i32 %r_i32_i16_5, i32 %r_i32_i16_6, i32 %r_i32_i16_7, i32 %r_i32_i16_8)
; CHECK: 0 0 1 0 1 0 0 1
       ret i32 0
}


define i32 @icmp_uge()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

; uint64 maximum value
       %res_i64 = icmp uge <2 x i64> < i64 18446744073709551615, i64 0>, < i64 18446744073709551615, i64 3>

       %r_i64_1 = extractelement <2 x i1> %res_i64, i32 0
       %r_i64_2 = extractelement <2 x i1> %res_i64, i32 1
       %r_i32_i64_1 = zext i1 %r_i64_1 to i32
       %r_i32_i64_2 = zext i1 %r_i64_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i64_1, i32 %r_i32_i64_2)
; CHECK: 1 0

       %ptr2 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %res_i32 = icmp uge <8 x i32> < i32 1, i32 3, i32 32000, i32 1453, i32 35, i32 4332, i32 45, i32 4212>,  < i32 1,  i32 12,  i32 11, i32 1453, i32 21, i32 4332, i32 3211, i32 33>
       %r_i32_1 = extractelement <8 x i1> %res_i32, i32 0
       %r_i32_2 = extractelement <8 x i1> %res_i32, i32 1
       %r_i32_3 = extractelement <8 x i1> %res_i32, i32 2
       %r_i32_4 = extractelement <8 x i1> %res_i32, i32 3
       %r_i32_5 = extractelement <8 x i1> %res_i32, i32 4
       %r_i32_6 = extractelement <8 x i1> %res_i32, i32 5
       %r_i32_7 = extractelement <8 x i1> %res_i32, i32 6
       %r_i32_8 = extractelement <8 x i1> %res_i32, i32 7
       %r_i32_i32_1 = zext i1 %r_i32_1 to i32
       %r_i32_i32_2 = zext i1 %r_i32_2 to i32
       %r_i32_i32_3 = zext i1 %r_i32_3 to i32
       %r_i32_i32_4 = zext i1 %r_i32_4 to i32
       %r_i32_i32_5 = zext i1 %r_i32_5 to i32
       %r_i32_i32_6 = zext i1 %r_i32_6 to i32
       %r_i32_i32_7 = zext i1 %r_i32_7 to i32
       %r_i32_i32_8 = zext i1 %r_i32_8 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr2, i32 %r_i32_i32_1, i32 %r_i32_i32_2, i32 %r_i32_i32_3, i32 %r_i32_i32_4, i32 %r_i32_i32_5, i32 %r_i32_i32_6, i32 %r_i32_i32_7, i32 %r_i32_i32_8)
; CHECK: 1 0 1 1 1 1 0 1
       ret i32 0
}

define i32 @icmp_ult()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

; uint64 maximum value
       %res_i64 = icmp ult <2 x i64> < i64 18446744073709551615, i64 2>, < i64 18446744073709551614, i64 3>

       %r_i64_1 = extractelement <2 x i1> %res_i64, i32 0
       %r_i64_2 = extractelement <2 x i1> %res_i64, i32 1
       %r_i32_i64_1 = zext i1 %r_i64_1 to i32
       %r_i32_i64_2 = zext i1 %r_i64_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i64_1, i32 %r_i32_i64_2)
; CHECK: 0 1

       %ptr2 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %res_i32 = icmp ult <8 x i32> < i32 1, i32 3, i32 32000, i32 1453, i32 35, i32 4332, i32 45, i32 4212>,  < i32 1,  i32 12,  i32 11, i32 1453, i32 21, i32 4332, i32 3211, i32 33>
       
       %r_i32_1 = extractelement <8 x i1> %res_i32, i32 0
       %r_i32_2 = extractelement <8 x i1> %res_i32, i32 1
       %r_i32_3 = extractelement <8 x i1> %res_i32, i32 2
       %r_i32_4 = extractelement <8 x i1> %res_i32, i32 3
       %r_i32_5 = extractelement <8 x i1> %res_i32, i32 4
       %r_i32_6 = extractelement <8 x i1> %res_i32, i32 5
       %r_i32_7 = extractelement <8 x i1> %res_i32, i32 6
       %r_i32_8 = extractelement <8 x i1> %res_i32, i32 7
       %r_i32_i32_1 = zext i1 %r_i32_1 to i32
       %r_i32_i32_2 = zext i1 %r_i32_2 to i32
       %r_i32_i32_3 = zext i1 %r_i32_3 to i32
       %r_i32_i32_4 = zext i1 %r_i32_4 to i32
       %r_i32_i32_5 = zext i1 %r_i32_5 to i32
       %r_i32_i32_6 = zext i1 %r_i32_6 to i32
       %r_i32_i32_7 = zext i1 %r_i32_7 to i32
       %r_i32_i32_8 = zext i1 %r_i32_8 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr2, i32 %r_i32_i32_1, i32 %r_i32_i32_2, i32 %r_i32_i32_3, i32 %r_i32_i32_4, i32 %r_i32_i32_5, i32 %r_i32_i32_6, i32 %r_i32_i32_7, i32 %r_i32_i32_8)
; CHECK: 0 1 0 0 0 0 1 0
       ret i32 0
}


define i32 @icmp_ule()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

; uint64 maximum value
       %res_i64 = icmp ule <2 x i64> < i64 18446744073709551615, i64 2>, < i64 18446744073709551615, i64 1>

       %r_i64_1 = extractelement <2 x i1> %res_i64, i32 0
       %r_i64_2 = extractelement <2 x i1> %res_i64, i32 1
       %r_i32_i64_1 = zext i1 %r_i64_1 to i32
       %r_i32_i64_2 = zext i1 %r_i64_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i64_1, i32 %r_i32_i64_2)
; CHECK: 1 0

       %ptr2 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %res_i32 = icmp ule <8 x i32> < i32 1, i32 3, i32 32000, i32 1453, i32 35, i32 4332, i32 45, i32 4212>,  < i32 1,  i32 12,  i32 11, i32 1453, i32 21, i32 4332, i32 3211, i32 33>           
       
       %r_i32_1 = extractelement <8 x i1> %res_i32, i32 0
       %r_i32_2 = extractelement <8 x i1> %res_i32, i32 1
       %r_i32_3 = extractelement <8 x i1> %res_i32, i32 2
       %r_i32_4 = extractelement <8 x i1> %res_i32, i32 3
       %r_i32_5 = extractelement <8 x i1> %res_i32, i32 4
       %r_i32_6 = extractelement <8 x i1> %res_i32, i32 5
       %r_i32_7 = extractelement <8 x i1> %res_i32, i32 6
       %r_i32_8 = extractelement <8 x i1> %res_i32, i32 7
       %r_i32_i32_1 = zext i1 %r_i32_1 to i32
       %r_i32_i32_2 = zext i1 %r_i32_2 to i32
       %r_i32_i32_3 = zext i1 %r_i32_3 to i32
       %r_i32_i32_4 = zext i1 %r_i32_4 to i32
       %r_i32_i32_5 = zext i1 %r_i32_5 to i32
       %r_i32_i32_6 = zext i1 %r_i32_6 to i32
       %r_i32_i32_7 = zext i1 %r_i32_7 to i32
       %r_i32_i32_8 = zext i1 %r_i32_8 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr2, i32 %r_i32_i32_1, i32 %r_i32_i32_2, i32 %r_i32_i32_3, i32 %r_i32_i32_4, i32 %r_i32_i32_5, i32 %r_i32_i32_6, i32 %r_i32_i32_7, i32 %r_i32_i32_8)
; CHECK: 1 1 0 1 0 1 1 0
       ret i32 0
}

define i32 @icmp_sgt()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

       %res_i8 = icmp sgt <2 x i8> < i8 5, i8 -128>, < i8 -5, i8 -5>

       %r_i8_1 = extractelement <2 x i1> %res_i8, i32 0
       %r_i8_2 = extractelement <2 x i1> %res_i8, i32 1
       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2)
; CHECK: 1 0

       %ptr2 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %res_i16 = icmp sgt <8 x i16> < i16 15, i16 12, i16 32000, i16 -1453, i16 35, i16 -4332, i16 45, i16 -4212>, < i16 1,  i16 3,  i16 11,    i16 1453, i16 -21, i16 -4332, i16 -3211, i16 33>

       %r_i16_1 = extractelement <8 x i1> %res_i16, i32 0
       %r_i16_2 = extractelement <8 x i1> %res_i16, i32 1
       %r_i16_3 = extractelement <8 x i1> %res_i16, i32 2
       %r_i16_4 = extractelement <8 x i1> %res_i16, i32 3
       %r_i16_5 = extractelement <8 x i1> %res_i16, i32 4
       %r_i16_6 = extractelement <8 x i1> %res_i16, i32 5
       %r_i16_7 = extractelement <8 x i1> %res_i16, i32 6
       %r_i16_8 = extractelement <8 x i1> %res_i16, i32 7
       %r_i32_i16_1 = zext i1 %r_i16_1 to i32
       %r_i32_i16_2 = zext i1 %r_i16_2 to i32
       %r_i32_i16_3 = zext i1 %r_i16_3 to i32
       %r_i32_i16_4 = zext i1 %r_i16_4 to i32
       %r_i32_i16_5 = zext i1 %r_i16_5 to i32
       %r_i32_i16_6 = zext i1 %r_i16_6 to i32
       %r_i32_i16_7 = zext i1 %r_i16_7 to i32
       %r_i32_i16_8 = zext i1 %r_i16_8 to i32
       
       call i32 (i8*, ...)* @printf(i8* %ptr2, i32 %r_i32_i16_1, i32 %r_i32_i16_2, i32 %r_i32_i16_3, i32 %r_i32_i16_4, i32 %r_i32_i16_5, i32 %r_i32_i16_6, i32 %r_i32_i16_7, i32 %r_i32_i16_8)
; CHECK: 1 1 1 0 1 0 1 0
       ret i32 0
}


define i32 @icmp_sge()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

       %res_i8 = icmp sge <2 x i8> < i8 -5, i8 -128>, < i8 -5, i8 -5>

       %r_i8_1 = extractelement <2 x i1> %res_i8, i32 0
       %r_i8_2 = extractelement <2 x i1> %res_i8, i32 1
       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2)
; CHECK: 1 0

       %ptr2 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %res_i16 = icmp sge <8 x i16> < i16 15, i16 12, i16 32000, i16 -1453, i16 35, i16 -4332, i16 45, i16 -4212>, < i16 1,  i16 3,  i16 11,    i16 1453, i16 -21, i16 -4332, i16 -3211, i16 33>

       %r_i16_1 = extractelement <8 x i1> %res_i16, i32 0
       %r_i16_2 = extractelement <8 x i1> %res_i16, i32 1
       %r_i16_3 = extractelement <8 x i1> %res_i16, i32 2
       %r_i16_4 = extractelement <8 x i1> %res_i16, i32 3
       %r_i16_5 = extractelement <8 x i1> %res_i16, i32 4
       %r_i16_6 = extractelement <8 x i1> %res_i16, i32 5
       %r_i16_7 = extractelement <8 x i1> %res_i16, i32 6
       %r_i16_8 = extractelement <8 x i1> %res_i16, i32 7
       %r_i32_i16_1 = zext i1 %r_i16_1 to i32
       %r_i32_i16_2 = zext i1 %r_i16_2 to i32
       %r_i32_i16_3 = zext i1 %r_i16_3 to i32
       %r_i32_i16_4 = zext i1 %r_i16_4 to i32
       %r_i32_i16_5 = zext i1 %r_i16_5 to i32
       %r_i32_i16_6 = zext i1 %r_i16_6 to i32
       %r_i32_i16_7 = zext i1 %r_i16_7 to i32
       %r_i32_i16_8 = zext i1 %r_i16_8 to i32
       
       call i32 (i8*, ...)* @printf(i8* %ptr2, i32 %r_i32_i16_1, i32 %r_i32_i16_2, i32 %r_i32_i16_3, i32 %r_i32_i16_4, i32 %r_i32_i16_5, i32 %r_i32_i16_6, i32 %r_i32_i16_7, i32 %r_i32_i16_8)
; CHECK: 1 1 1 0 1 1 1 0

       ret i32 0
}


define i32 @icmp_slt()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

       %res_i8 = icmp slt <2 x i8> < i8 5, i8 -128>, < i8 -5, i8 -5>

       %r_i8_1 = extractelement <2 x i1> %res_i8, i32 0
       %r_i8_2 = extractelement <2 x i1> %res_i8, i32 1
       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2)
; CHECK: 0 1

       %ptr2 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %res_i16 = icmp slt <8 x i16> < i16 15, i16 12, i16 32000, i16 -1453, i16 35, i16 -4332, i16 45, i16 -4212>, < i16 1,  i16 3,  i16 11,    i16 1453, i16 -21, i16 -4332, i16 -3211, i16 33>
       %r_i16_1 = extractelement <8 x i1> %res_i16, i32 0
       %r_i16_2 = extractelement <8 x i1> %res_i16, i32 1
       %r_i16_3 = extractelement <8 x i1> %res_i16, i32 2
       %r_i16_4 = extractelement <8 x i1> %res_i16, i32 3
       %r_i16_5 = extractelement <8 x i1> %res_i16, i32 4
       %r_i16_6 = extractelement <8 x i1> %res_i16, i32 5
       %r_i16_7 = extractelement <8 x i1> %res_i16, i32 6
       %r_i16_8 = extractelement <8 x i1> %res_i16, i32 7
       %r_i32_i16_1 = zext i1 %r_i16_1 to i32
       %r_i32_i16_2 = zext i1 %r_i16_2 to i32
       %r_i32_i16_3 = zext i1 %r_i16_3 to i32
       %r_i32_i16_4 = zext i1 %r_i16_4 to i32
       %r_i32_i16_5 = zext i1 %r_i16_5 to i32
       %r_i32_i16_6 = zext i1 %r_i16_6 to i32
       %r_i32_i16_7 = zext i1 %r_i16_7 to i32
       %r_i32_i16_8 = zext i1 %r_i16_8 to i32
       
       call i32 (i8*, ...)* @printf(i8* %ptr2, i32 %r_i32_i16_1, i32 %r_i32_i16_2, i32 %r_i32_i16_3, i32 %r_i32_i16_4, i32 %r_i32_i16_5, i32 %r_i32_i16_6, i32 %r_i32_i16_7, i32 %r_i32_i16_8)
; CHECK: 0 0 0 1 0 0 0 1
       ret i32 0
}


define i32 @icmp_sle()
{
       %ptr = getelementptr [8 x i8]* @r_int_2, i32 0, i32 0 ; to printf

       %res_i8 = icmp sle <2 x i8> < i8 -5, i8 -128>, < i8 -5, i8 -5>

       %r_i8_1 = extractelement <2 x i1> %res_i8, i32 0
       %r_i8_2 = extractelement <2 x i1> %res_i8, i32 1
       %r_i32_i8_1 = zext i1 %r_i8_1 to i32
       %r_i32_i8_2 = zext i1 %r_i8_2 to i32
       call i32 (i8*, ...)* @printf(i8* %ptr, i32 %r_i32_i8_1, i32 %r_i32_i8_2)
; CHECK: 1 1

       %ptr2 = getelementptr [26 x i8]* @r_int_8, i32 0, i32 0 ; to printf
       %res_i16 = icmp sle <8 x i16> < i16 -1, i16 12, i16 32000, i16 -1453, i16 35, i16 -4332, i16 45, i16 -4212>, < i16 -1,  i16 3,  i16 11,    i16 1453, i16 -21, i16 -4332, i16 -3211, i16 33>

       %r_i16_1 = extractelement <8 x i1> %res_i16, i32 0
       %r_i16_2 = extractelement <8 x i1> %res_i16, i32 1
       %r_i16_3 = extractelement <8 x i1> %res_i16, i32 2
       %r_i16_4 = extractelement <8 x i1> %res_i16, i32 3
       %r_i16_5 = extractelement <8 x i1> %res_i16, i32 4
       %r_i16_6 = extractelement <8 x i1> %res_i16, i32 5
       %r_i16_7 = extractelement <8 x i1> %res_i16, i32 6
       %r_i16_8 = extractelement <8 x i1> %res_i16, i32 7
       %r_i32_i16_1 = zext i1 %r_i16_1 to i32
       %r_i32_i16_2 = zext i1 %r_i16_2 to i32
       %r_i32_i16_3 = zext i1 %r_i16_3 to i32
       %r_i32_i16_4 = zext i1 %r_i16_4 to i32
       %r_i32_i16_5 = zext i1 %r_i16_5 to i32
       %r_i32_i16_6 = zext i1 %r_i16_6 to i32
       %r_i32_i16_7 = zext i1 %r_i16_7 to i32
       %r_i32_i16_8 = zext i1 %r_i16_8 to i32
       
       call i32 (i8*, ...)* @printf(i8* %ptr2, i32 %r_i32_i16_1, i32 %r_i32_i16_2, i32 %r_i32_i16_3, i32 %r_i32_i16_4, i32 %r_i32_i16_5, i32 %r_i32_i16_6, i32 %r_i32_i16_7, i32 %r_i32_i16_8)
; CHECK: 1 0 0 1 0 1 0 1

       ret i32 0
}



define i32 @main()
{
       call i32 @icmp_eq()
       call i32 @icmp_ne()
       call i32 @icmp_ugt()
       call i32 @icmp_uge()
       call i32 @icmp_ult()
       call i32 @icmp_ule()
       call i32 @icmp_sgt()
	   call i32 @icmp_sge()
       call i32 @icmp_slt()
	   call i32 @icmp_sle()
       
       ret i32 0
}

