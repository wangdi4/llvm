; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32:64"

; To print 32-bit integer
@msg_i32 = internal global [4 x i8] c"%d\0A\00"    ; <[4 x i8]*> [#uses=1]
@msg_fp = internal global [4 x i8] c"%f\0A\00"    ; <[4 x i8]*> [#uses=1]
@msg_ptr = internal global [4 x i8] c"%p\0A\00"    ; <[4 x i8]*> [#uses=1]
declare void @printf(i8*, ...)

; To print 4-element vector of i64
;declare void @printf([34 x i8]*, ...)
@msg_4i64 = internal global [38 x i8] c"<i64 %lu, i64 %lu, i64 %lu, i64 %lu>\0A\00"
define void @print_4i64(<4 x i64> %arg) {
  %a = extractelement <4 x i64> %arg, i32 0
  %b = extractelement <4 x i64> %arg, i32 1
  %c = extractelement <4 x i64> %arg, i32 2
  %d = extractelement <4 x i64> %arg, i32 3
  %msg = getelementptr [38 x i8]* @msg_4i64, i32 0, i32 0
  call void (i8*, ...)* @printf(i8* %msg, i64 %a, i64 %b, i64 %c, i64 %d)
  ret void
}

; To print 3-element vector of i16
;declare void @printf([26 x i8]*, ...)
@msg_3i16 = internal global [26 x i8] c"<i16 %d, i16 %d, i16 %d>\0A\00"
define void @print_3i16(<3 x i16> %arg) {
  %a_ = extractelement <3 x i16> %arg, i32 0
  %a = sext i16 %a_ to i32
  %b_ = extractelement <3 x i16> %arg, i32 1
  %b = sext i16 %b_ to i32
  %c_ = extractelement <3 x i16> %arg, i32 2
  %c = sext i16 %c_ to i32
  %msg = getelementptr [26 x i8]* @msg_3i16, i32 0, i32 0
  call void (i8*, ...)* @printf(i8* %msg, i32 %a, i32 %b, i32 %c)
  ret void
}

; To print 2-element vector of i8
;declare void @printf([16 x i8]*, ...)
@msg_2i8 = internal global [16 x i8] c"<i8 %d, i8 %d>\0A\00"
define void @print_2i8(<2 x i8> %arg) {
  %a_ = extractelement <2 x i8> %arg, i32 0
  %a = sext i8 %a_ to i32
  %b_ = extractelement <2 x i8> %arg, i32 1
  %b = sext i8 %b_ to i32
  %msg = getelementptr [16 x i8]* @msg_2i8, i32 0, i32 0
  call void (i8*, ...)* @printf(i8* %msg, i32 %a, i32 %b)
  ret void
}

define void @test(<2 x i8>* %P, < 3 x i16 >* %P.upgrd.1, i32* %P.upgrd.2, < 4 x i64 >* %P.upgrd.3) {
  %V = load <2 x i8>* %P    ; < 2 x i8 > [#uses=1]
  %msg1 = getelementptr [4 x i8]* @msg_ptr, i32 0, i32 0
  call void (<2 x i8>)* @print_2i8(<2 x i8> %V)
; CHECK: <i8 8, i8 9>
  store < 2 x i8 > %V, < 2 x i8 >* %P
  %V.upgrd.4 = load < 3 x i16 >* %P.upgrd.1    ; <i16> [#uses=1]
  call void (<3 x i16>)* @print_3i16(<3 x i16> %V.upgrd.4)
; CHECK: <i16 10, i16 11, i16 12>
  store < 3 x i16 > %V.upgrd.4, < 3 x i16 >* %P.upgrd.1
  %V.upgrd.5 = load i32* %P.upgrd.2    ; <i32> [#uses=1]
  %msg = getelementptr [4 x i8]* @msg_i32, i32 0, i32 0
  call void (i8*, ...)* @printf( i8* %msg, i32 %V.upgrd.5 )
; CHECK: 13
  store i32 %V.upgrd.5, i32* %P.upgrd.2
  %V.upgrd.6 = load < 4 x i64 >* %P.upgrd.3    ; <i64> [#uses=1]
  call void (<4 x i64>)* @print_4i64(<4 x i64> %V.upgrd.6)
; CHECK: <i64 14, i64 15, i64 16, i64 17>
  store < 4 x i64 > %V.upgrd.6, < 4 x i64 >* %P.upgrd.3

  ret void
}

define i32 @main() {
  ; Integer type tests
  ; i8
  %A = alloca < 2 x i8 >, i32 2    ; << 2 x i8 >*> [#uses=1]
  store <2 x i8> <i8 8, i8 9>, <2 x i8>* %A
  %a2 = load <2 x i8>* %A
  %msg1 = getelementptr [4 x i8]* @msg_ptr, i32 0, i32 0

  ; i16
  %B = alloca < 3 x i16 >, i32 4    ; << 3 x i16 >*> [#uses=1]
  store <3 x i16> <i16 10, i16 11, i16 12>, <3 x i16>* %B

  ; i32
  %C = alloca i32    ; <i32*> [#uses=1]
  store i32 13, i32* %C
  
  ; i64
  %D = alloca < 4 x i64 >, i32 3  ; << 4 x i64 >*> [#uses=1]
  store <4 x i64> <i64 14, i64 15, i64 16, i64 17>, <4 x i64>* %D

  call void @test( < 2 x i8 >* %A, < 3 x i16 >* %B, i32* %C, < 4 x i64 >* %D )

  ; float
  %E = alloca < 3 x float >, i32 4    ; << 3 x i16 >*> [#uses=1]
  store <3 x float> <float 1.0e+0, float 2.0e+1, float 3.0e+2>, <3 x float>* %E
  %E_ = load <3 x float>* %E
  %fl_ = extractelement <3 x float> %E_, i32 0
  %fl0 = fpext float %fl_ to double
  %msgFP = getelementptr [4 x i8]* @msg_fp, i32 0, i32 0
  call void (i8*, ...)* @printf (i8* %msgFP, double %fl0)
; CHECK: 1.000000
  %fl1_ = extractelement <3 x float> %E_, i32 1
  %fl1 = fpext float %fl1_ to double
  call void (i8*, ...)* @printf (i8* %msgFP, double %fl1)
; CHECK: 20.000000
  %fl2_ = extractelement <3 x float> %E_, i32 2
  %fl2 = fpext float %fl2_ to double
  call void (i8*, ...)* @printf (i8* %msgFP, double %fl2)
; CHECK: 300.000000

  ; double
  %F = alloca < 2 x double >
  store <2 x double> <double 2.0e-1, double 3.0e-2>, <2 x double>* %F
  %F_ = load <2 x double>* %F
  %rl_ = extractelement <2 x double> %F_, i32 0
  call void (i8*, ...)* @printf (i8* %msgFP, double %rl_)
; CHECK: 0.200000
  %rl1_ = extractelement <2 x double> %F_, i32 1
  call void (i8*, ...)* @printf (i8* %msgFP, double %rl1_)
; CHECK: 0.030000

  ret i32 0
}
