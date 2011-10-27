; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32:64"

; simple sturcture to test array of structures
%a1 = type { <2 x i32>, [2 x float], i32 }
; complex structure
%a = type [ 2 x %a1 ]
%b = type [ 3 x [ 4 x i32 ] ]
%c = type [ 3 x < 2 x i32 > ]
@d = global %b zeroinitializer           ; <%struct.B*> [#uses=2]
@n = global %a zeroinitializer           ; <%struct.B*> [#uses=2]
@r = internal constant %b [ [4 x i32] [i32 29, i32 9857, i32 598, i32 587], [4 x i32] [i32 103948, i32 13456, i32 0, i32 1], [4 x i32] [i32 2, i32 87, i32 9, i32 5]]

define i32 @main() {
lbb0:
  ; alloca test
  %g = alloca %a
  %e = alloca %b
  %h = alloca %c
  br label %lbb3
lbb3:
  ; phi
  %j = phi %a [%q, %lbb2], [%o, %lbb3], [zeroinitializer, %lbb0]
  ; load/store
  %o = load %a* @n
  %f = load %b* @d
  call void (%b)* @print_b(%b %f)
  ; CHECK: [ [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0] ]
  store %a [%a1 {<2 x i32> <i32 2938, i32 9238>, [2 x float] [float 1.0, float 2.0], i32 973}, %a1 zeroinitializer], %a* %g
  %p = load %a* %g
  call void (%a)* @print_a(%a %p)
  ; CHECK: [ {<2938, 9238>, [1.000000, 2.000000], 973}, {<0, 0>, [0.000000, 0.000000], 0} ]
  br i1 true, label %lbb2, label %lbb3
lbb2:
  %q = insertvalue %a %o, %a1 {<2 x i32> <i32 2938, i32 9238>, [2 x float] [float 1.0, float 2.0], i32 973}, 1
  call void (%a)* @print_a(%a %q)
  ; CHECK: [ {<0, 0>, [0.000000, 0.000000], 0}, {<2938, 9238>, [1.000000, 2.000000], 973} ]
  store %c [<2 x i32> <i32 235, i32 354>, <2 x i32> <i32 497, i32 54>, <2 x i32> <i32 10347, i32 87> ], %c* %h
  %t = load %c* %h
  %s = load %b* @r
  ; select
  %i = select i1 true, %b %s, %b %f
  call void (%b)* @print_b(%b %i)
  ; CHECK: [ [29, 9857, 598, 587], [103948, 13456, 0, 1], [2, 87, 9, 5] ]
  ; phi
  call void (%a)* @print_a(%a %j)
  ; CHECK: [ {<0, 0>, [0.000000, 0.000000], 0}, {<0, 0>, [0.000000, 0.000000], 0} ]
  ; ret
  %k = call %b (%b)* @ret_func(%b %i)
  call void (%b)* @print_b(%b %k)
  ; CHECK: [ [29, 9857, 598, 587], [103948, 13456, 0, 1], [2, 87, 9, 5] ]
  br i1 false, label %lbb3, label %lbb4
lbb4:
  ret i32 0
}

define %b @ret_func(%b %retb)
{
  ret %b %retb
}

declare void @printf(i8*, ...)

@msg_4i32 = internal constant [17 x i8] c"[%d, %d, %d, %d]\00"
define void @print4i32([4 x i32] %var) {
  %msg0 = getelementptr [17 x i8]* @msg_4i32, i32 0, i32 0 ; <i8*> [#uses=1]
  %var0 = extractvalue [4 x i32] %var, 0
  %var1 = extractvalue [4 x i32] %var, 1
  %var2 = extractvalue [4 x i32] %var, 2
  %var3 = extractvalue [4 x i32] %var, 3
  call void (i8*, ...)* @printf(i8* %msg0, i32 %var0, i32 %var1, i32 %var2, i32 %var3)
  ret void
}

@msg_stb0 = internal constant [3 x i8] c"[ \00"
@msg_stb1 = internal constant [3 x i8] c", \00"
@msg_stb2 = internal constant [4 x i8] c" ]\0A\00"
define void @print_b(%b %bb)
{
  %msg0 = getelementptr [3 x i8]* @msg_stb0, i32 0, i32 0 ; <i8*> [#uses=1]
  %msg1 = getelementptr [3 x i8]* @msg_stb1, i32 0, i32 0 ; <i8*> [#uses=1]
  %msg2 = getelementptr [4 x i8]* @msg_stb2, i32 0, i32 0 ; <i8*> [#uses=1]

  call void (i8*, ...)* @printf(i8* %msg0)
  %bb0 = extractvalue %b %bb, 0
  call void ([4 x i32])* @print4i32([4 x i32] %bb0)
  call void (i8*, ...)* @printf(i8* %msg1)
  %bb1 = extractvalue %b %bb, 1
  call void ([4 x i32])* @print4i32([4 x i32] %bb1)
  call void (i8*, ...)* @printf(i8* %msg1)
  %bb2 = extractvalue %b %bb, 2
  call void ([4 x i32])* @print4i32([4 x i32] %bb2)
  call void (i8*, ...)* @printf(i8* %msg2)
  ret void 
}

define void @print_a(%a %vara) {
  %msg0 = getelementptr [3 x i8]* @msg_stb0, i32 0, i32 0 ; <i8*> [#uses=1]
  %msg1 = getelementptr [3 x i8]* @msg_stb1, i32 0, i32 0 ; <i8*> [#uses=1]
  %msg2 = getelementptr [4 x i8]* @msg_stb2, i32 0, i32 0 ; <i8*> [#uses=1]

  call void (i8*, ...)* @printf(i8* %msg0)
  %vara0 = extractvalue %a %vara, 0
  call void (%a1)* @print_a1(%a1 %vara0)
  call void (i8*, ...)* @printf(i8* %msg1)
  %vara1 = extractvalue %a %vara, 1
  call void (%a1)* @print_a1(%a1 %vara1)
  call void (i8*, ...)* @printf(i8* %msg2)
  ret void 
}

@msg_sta = internal constant [25 x i8] c"{<%d, %d>, [%f, %f], %d}\00"
define void @print_a1(%a1 %aa)
{
  %msg = getelementptr [25 x i8]* @msg_sta, i32 0, i32 0 ; <i8*> [#uses=1]
  %aa.v = extractvalue %a1 %aa, 0
  %aa.v0 = extractelement <2 x i32> %aa.v, i32 0
  %aa.v1 = extractelement <2 x i32> %aa.v, i32 1
  %aa.a0_ = extractvalue %a1 %aa, 1, 0
  %aa.a0 = fpext float %aa.a0_ to double
  %aa.a1_ = extractvalue %a1 %aa, 1, 1
  %aa.a1 = fpext float %aa.a1_ to double
  %aa.i = extractvalue %a1 %aa, 2
  call void (i8*, ...)* @printf(i8* %msg, i32 %aa.v0, i32 %aa.v1, double %aa.a0, double %aa.a1, i32 %aa.i)
  ret void 
}

