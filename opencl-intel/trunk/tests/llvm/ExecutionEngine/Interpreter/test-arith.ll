; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32:64"

@msg_int2 = internal constant [7 x i8] c"%d %d\0A\00"
@msg_int3 = internal constant [10 x i8] c"%d %d %d\0A\00"
@msg_int4 = internal constant [13 x i8] c"%d %d %d %d\0A\00"

declare void @printf(i8*, ...)

define i32 @main() {
  %msg_int2 = getelementptr [7 x i8]* @msg_int2, i32 0, i32 0

  ; ============= TEST FOR <2 x i8> =======================
  %A = add <2 x i8> <i8 0, i8 2>, <i8 12, i8 14>		; <<2 x i8>> [#uses=1]
  call void (<2 x i8>)* @print_2i8(<2 x i8> %A)
  ; CHECK: 12 16
  %B = sub <2 x i8> %A, <i8 1, i8 2>		; <<2 x i8>> [#uses=2]; CHECK to i32 
  call void (<2 x i8>)* @print_2i8(<2 x i8> %B)
  ; CHECK: 11 14
  %C = mul <2 x i8> %B, %B		; <<2 x i8>> [#uses=2]
  call void (<2 x i8>)* @print_2i8(<2 x i8> %C)
  ; CHECK: 121 -60
  %D = sdiv <2 x i8> %C, %B		; <<2 x i8>> [#uses=2]
  call void (<2 x i8>)* @print_2i8(<2 x i8> %D)
  ; CHECK: 11 -4
  %E = srem <2 x i8> %C, %A		; <<2 x i8>> [#uses=0]
  call void (<2 x i8>)* @print_2i8(<2 x i8> %E)
  ; CHECK: 1 -12
  %F = udiv <2 x i8> %C, %B		; <<2 x i8>> [#uses=0]
  call void (<2 x i8>)* @print_2u8(<2 x i8> %F)
  ; CHECK: 11 14
  %G = urem <2 x i8> <i8 6, i8 8>, <i8 5, i8 7>		; <<2 x i8>> [#uses=0]
  call void (<2 x i8>)* @print_2u8(<2 x i8> %G)
  ; CHECK: 1 1

  ; ============= TEST FOR <3 x i32> =======================
  %A.upgrd.8 = add <3 x i32> <i32 0, i32 1, i32 2>,  <i32 12, i32 13, i32 14>		; <<3 x i32>> [#uses=1]
  call void (<3 x i32>)* @print_3i32(<3 x i32> %A.upgrd.8)
  ; CHECK: 12 14 16
  %B.upgrd.9 = sub <3 x i32> %A.upgrd.8, <i32 1, i32 2, i32 3>		; <<3 x i32>> [#uses=2]
  call void (<3 x i32>)* @print_3i32(<3 x i32> %B.upgrd.9)
  ; CHECK: 11 12 13
  %C.upgrd.10 = mul <3 x i32> %B.upgrd.9, %B.upgrd.9		; <<3 x i32>> [#uses=2]
  call void (<3 x i32>)* @print_3i32(<3 x i32> %C.upgrd.10)
  ; CHECK: 121 144 169
  %D.upgrd.11 = sdiv <3 x i32> %C.upgrd.10, %B.upgrd.9
  call void (<3 x i32>)* @print_3i32(<3 x i32> %D.upgrd.11)
  ; CHECK: 11 12 13
  %E.upgrd.12 = srem <3 x i32> %C.upgrd.10, %A.upgrd.8
  call void (<3 x i32>)* @print_3i32(<3 x i32> %E.upgrd.12)
  ; CHECK: 1 4 9
  %F.upgrd.13 = udiv <3 x i32> %C.upgrd.10, %B.upgrd.9
  call void (<3 x i32>)* @print_3i32(<3 x i32> %F.upgrd.13)
  ; CHECK: 11 12 13
  %G21 = urem <3 x i32> <i32 6, i32 7, i32 8>, <i32 5, i32 6, i32 7>		; <<3 x i32>> [#uses=0]
  call void (<3 x i32>)* @print_3i32(<3 x i32> %G21)
  ; CHECK: 1 1 1

  ; ============= TEST FOR <4 x i64> =======================
  %A.upgrd.14 = add <4 x i64> <i64 0, i64 1, i64 2, i64 3>, <i64 12, i64 13, i64 14, i64 15>		; <<4 x i64>> [#uses=1]
  call void (<4 x i64>)* @print_4i64(<4 x i64> %A.upgrd.14)
  ; CHECK: 12 14 16 18
  %B.upgrd.15 = sub <4 x i64> %A.upgrd.14, <i64 1, i64 2, i64 3, i64 4>		; <<4 x i64>> [#uses=2]
  call void (<4 x i64>)* @print_4i64(<4 x i64> %B.upgrd.15)
  ; CHECK: 11 12 13 14
  %C.upgrd.16 = mul <4 x i64> %B.upgrd.15, %B.upgrd.15
  call void (<4 x i64>)* @print_4i64(<4 x i64> %C.upgrd.16)
  ; CHECK: 121 144 169 196
  %D.upgrd.17 = sdiv <4 x i64> %C.upgrd.16, %B.upgrd.15		; <<4 x i64>> [#uses=2]
  call void (<4 x i64>)* @print_4i64(<4 x i64> %D.upgrd.17)
  ; CHECK: 11 12 13 14
  %E.upgrd.18 = srem <4 x i64> %C.upgrd.16, %A.upgrd.14		; <<4 x i64>> [#uses=0]
  call void (<4 x i64>)* @print_4i64(<4 x i64> %E.upgrd.18)
  ; CHECK: 1 4 9 16
  %F.upgrd.19 = udiv <4 x i64> %C.upgrd.16, %B.upgrd.15
  call void (<4 x i64>)* @print_4i64(<4 x i64> %F.upgrd.19)
  ; CHECK: 11 12 13 14
  %G.upgrd.20 = urem <4 x i64> <i64 6, i64 7, i64 8, i64 9>, <i64 5, i64 6, i64 7, i64 8>		; <<4 x i64>> [#uses=0]
  call void (<4 x i64>)* @print_4i64(<4 x i64> %G.upgrd.20)
  ; CHECK: 1 1 1 1
  ret i32 0
}

define void @print_2u8(<2 x i8> %P)
{
  %msg_int2 = getelementptr [7 x i8]* @msg_int2, i32 0, i32 0
  %P0_ = extractelement <2 x i8> %P, i32 0
  %P0 = zext i8 %P0_ to i32 
  %P1_ = extractelement <2 x i8> %P, i32 1
  %P1 = zext i8 %P1_ to i32 
  call void (i8*, ...)* @printf(i8* %msg_int2, i32 %P0, i32 %P1)
  ret void
}

define void @print_2i8(<2 x i8> %P)
{
  %msg_int2 = getelementptr [7 x i8]* @msg_int2, i32 0, i32 0
  %P0_ = extractelement <2 x i8> %P, i32 0
  %P0 = sext i8 %P0_ to i32 
  %P1_ = extractelement <2 x i8> %P, i32 1
  %P1 = sext i8 %P1_ to i32 
  call void (i8*, ...)* @printf(i8* %msg_int2, i32 %P0, i32 %P1)
  ret void
}

define void @print_3i32(<3 x i32> %P)
{
  %msg_int3 = getelementptr [10 x i8]* @msg_int3, i32 0, i32 0
  %P0 = extractelement <3 x i32> %P, i32 0
  %P1 = extractelement <3 x i32> %P, i32 1
  %P2 = extractelement <3 x i32> %P, i32 2
  call void (i8*, ...)* @printf(i8* %msg_int3, i32 %P0, i32 %P1, i32 %P2)
  ret void
}

define void @print_4i64(<4 x i64> %P)
{
  %msg_int4 = getelementptr [13 x i8]* @msg_int4, i32 0, i32 0
  %P0 = extractelement <4 x i64> %P, i32 0
  %P1 = extractelement <4 x i64> %P, i32 1
  %P2 = extractelement <4 x i64> %P, i32 2
  %P3 = extractelement <4 x i64> %P, i32 3
  call void (i8*, ...)* @printf(i8* %msg_int4, i64 %P0, i64 %P1, i64 %P2, i64 %P3)
  ret void
}

