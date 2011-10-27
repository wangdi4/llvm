; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32:64"

; simple sturcture to test nested structures
%a = type { <2 x i32>, [2 x float], i32 }
; complex structure
%b = type { 
  i32,
  float,
  double,
  i1,
  <2 x float>,
  <3 x double>,
  <4 x i64>,
  %a, 
  [3 x i16]
 }
@c = global %b zeroinitializer           ; <%struct.B*> [#uses=2]

define %b @ret_func(%b %retb)
{
  ret %b %retb
}

declare void @printf(i8*, ...)

@msg_stb0 = internal constant [60 x i8] c"{%d, %f, %f, %d, <%f, %f>, <%f, %f, %f>, <%d, %d, %d, %d>, \00"
@msg_stb1 = internal constant [17 x i8] c", [%d, %d, %d]}\0A\00"
define void @print_b(%b %bb)
{
  %msg0 = getelementptr [60 x i8]* @msg_stb0, i32 0, i32 0 ; <i8*> [#uses=1]
  %msg1 = getelementptr [17 x i8]* @msg_stb1, i32 0, i32 0 ; <i8*> [#uses=1]
  %bb.i = extractvalue %b %bb, 0
  %bb.f_ = extractvalue %b %bb, 1
  %bb.f = fpext float %bb.f_ to double
  %bb.d = extractvalue %b %bb, 2
  %bb.b_ = extractvalue %b %bb, 3
  %bb.b = zext i1 %bb.b_ to i32
  %bb.vf = extractvalue %b %bb, 4
  %bb.vf0_ = extractelement <2 x float> %bb.vf, i32 0
  %bb.vf0 = fpext float %bb.vf0_ to double
  %bb.vf1_ = extractelement <2 x float> %bb.vf, i32 1
  %bb.vf1 = fpext float %bb.vf1_ to double
  %bb.vd = extractvalue %b %bb, 5
  %bb.vd0 = extractelement <3 x double> %bb.vd, i32 0
  %bb.vd1 = extractelement <3 x double> %bb.vd, i32 1
  %bb.vd2 = extractelement <3 x double> %bb.vd, i32 2
  %bb.vi = extractvalue %b %bb, 6
  %bb.vi0 = extractelement <4 x i64> %bb.vi, i32 0
  %bb.vi1 = extractelement <4 x i64> %bb.vi, i32 1
  %bb.vi2 = extractelement <4 x i64> %bb.vi, i32 2
  %bb.vi3 = extractelement <4 x i64> %bb.vi, i32 3
  call void (i8*, ...)* @printf(i8* %msg0, i32 %bb.i, double %bb.f, double %bb.d, i32 %bb.b, double %bb.vf0, double %bb.vf1, double %bb.vd0, double %bb.vd1, double %bb.vd2, i64 %bb.vi0, i64 %bb.vi1, i64 %bb.vi2, i64 %bb.vi3)
  %bb.s = extractvalue %b %bb, 7
  call void (%a)* @print_a(%a %bb.s)
  %bb.a0_ = extractvalue %b %bb, 8, 0
  %bb.a0 = sext i16 %bb.a0_ to i32
  %bb.a1_ = extractvalue %b %bb, 8, 1
  %bb.a1 = sext i16 %bb.a1_ to i32
  %bb.a2_ = extractvalue %b %bb, 8, 2
  %bb.a2 = sext i16 %bb.a2_ to i32
  call void (i8*, ...)* @printf(i8* %msg1, i32 %bb.a0, i32 %bb.a1, i32 %bb.a2)
  ret void 
}

@msg_sta = internal constant [26 x i8] c"{<%d, %d>, [%f, %f], %d}\0A\00"
define void @print_a(%a %aa)
{
  %msg = getelementptr [26 x i8]* @msg_sta, i32 0, i32 0 ; <i8*> [#uses=1]
  %aa.v = extractvalue %a %aa, 0
  %aa.v0 = extractelement <2 x i32> %aa.v, i32 0
  %aa.v1 = extractelement <2 x i32> %aa.v, i32 1
  %aa.a0_ = extractvalue %a %aa, 1, 0
  %aa.a0 = fpext float %aa.a0_ to double
  %aa.a1_ = extractvalue %a %aa, 1, 1
  %aa.a1 = fpext float %aa.a1_ to double
  %aa.i = extractvalue %a %aa, 2
  call void (i8*, ...)* @printf(i8* %msg, i32 %aa.v0, i32 %aa.v1, double %aa.a0, double %aa.a1, i32 %aa.i)
  ret void 
}

define i32 @main() {
bb0:
  ; alloca test
  %d = alloca %a
  %e = alloca %b
  br label %bb3
bb3:
  ; phi
  %j = phi %b [%h, %bb2], [%f, %bb3], [zeroinitializer, %bb0]
  ; load/store
  %f = load %b* @c
  call void (%b)* @print_b(%b %f)
  ; CHECK: {0, 0.000000, 0.000000, 0, <0.000000, 0.000000>, <0.000000, 0.000000, 0.000000>, <0, 0, 0, 0>, {<0, 0>, [0.000000, 0.000000], 0}
  ; CHEKC: , [0, 0, 0]}
  store %a {<2 x i32> <i32 2938, i32 9238>, [2 x float] [float 1.0, float 2.0], i32 973}, %a* %d
  %g = load %a* %d
  call void (%a)* @print_a(%a %g)
  ; CHECK: {<2938, 9238>, [1.000000, 2.000000], 973}
  br i1 true, label %bb2, label %bb3
bb2:
  %h = insertvalue %b %f, %a %g, 7
  call void (%b)* @print_b(%b %h)
  ; CHECK: {0, 0.000000, 0.000000, 0, <0.000000, 0.000000>, <0.000000, 0.000000, 0.000000>, <0, 0, 0, 0>, {<2938, 9238>, [1.000000, 2.000000], 973}
  ; CHECK: , [0, 0, 0]}
  store %b %h, %b* %e
  ; select
  %i = select i1 true, %b %h, %b %f
  call void (%b)* @print_b(%b %i)
  ; CHECK: {0, 0.000000, 0.000000, 0, <0.000000, 0.000000>, <0.000000, 0.000000, 0.000000>, <0, 0, 0, 0>, {<2938, 9238>, [1.000000, 2.000000], 973}
  ; CHECK: , [0, 0, 0]}
  ; phi
  call void (%b)* @print_b(%b %j)
  ; CHECK: {0, 0.000000, 0.000000, 0, <0.000000, 0.000000>, <0.000000, 0.000000, 0.000000>, <0, 0, 0, 0>, {<0, 0>, [0.000000, 0.000000], 0}
  ; CHECK: , [0, 0, 0]}
  ; ret
  %k = call %b (%b)* @ret_func(%b %i)
  ; CHECK: {0, 0.000000, 0.000000, 0, <0.000000, 0.000000>, <0.000000, 0.000000, 0.000000>, <0, 0, 0, 0>, {<2938, 9238>, [1.000000, 2.000000], 973}
  ; CHECK: , [0, 0, 0]}
  call void (%b)* @print_b(%b %k)
  br i1 false, label %bb3, label %bb4
bb4:
  ret i32 0
}
