; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32:64"

; This tests to make sure that we can evaluate weird constant expressions
@msg_int = internal constant [4 x i8] c"%d\0A\00"
@msg_int2 = internal constant [7 x i8] c"%d %d\0A\00"
@msg_int3 = internal constant [10 x i8] c"%d %d %d\0A\00"
@msg_int4 = internal constant [13 x i8] c"%d %d %d %d\0A\00"
@msg_int8 = internal constant [25 x i8] c"%d %d %d %d %d %d %d %d\0A\00"
@msg_int16 = internal constant [49 x i8] c"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\0A\00"
@msg_fp2 = internal constant [7 x i8] c"%f %f\0A\00"
@msg_fp3 = internal constant [10 x i8] c"%f %f %f\0A\00"
@msg_fp4 = internal constant [13 x i8] c"%f %f %f %f\0A\00"

declare void @printf(i8*, ...)

%struct.A = type { <2 x i32> }
%struct.B = type { %struct.A }
@aa = global %struct.B zeroinitializer           ; <%struct.B*> [#uses=2]

define i32 @main() {
  %msg_int = getelementptr [4 x i8]* @msg_int, i32 0, i32 0
  %msg_int2 = getelementptr [7 x i8]* @msg_int2, i32 0, i32 0
  %msg_int3 = getelementptr [10 x i8]* @msg_int3, i32 0, i32 0
  %msg_int4 = getelementptr [13 x i8]* @msg_int4, i32 0, i32 0
  %msg_int8 = getelementptr [25 x i8]* @msg_int8, i32 0, i32 0
  %msg_int16 = getelementptr [49 x i8]* @msg_int16, i32 0, i32 0
  %msg_fp2 = getelementptr [7 x i8]* @msg_fp2, i32 0, i32 0
  %msg_fp3 = getelementptr [10 x i8]* @msg_fp3, i32 0, i32 0
  %msg_fp4 = getelementptr [13 x i8]* @msg_fp4, i32 0, i32 0
  ; fptoui 
  %a = add <3 x i32> <i32 0, i32 3, i32 4>, fptoui (<3 x float> <float 2.0e+0, float 3.e+4, float 5.e+2> to <3 x i32>)
  %a0 = extractelement <3 x i32> %a, i32 0
  %a1 = extractelement <3 x i32> %a, i32 1
  %a2 = extractelement <3 x i32> %a, i32 2
  call void (i8*, ...)* @printf(i8* %msg_int3, i32 %a0, i32 %a1, i32 %a2)
  ; CHECK: 2 30003 504

  ; fptosi 
  %b = add <3 x i32> <i32 0, i32 3, i32 4>, fptosi (<3 x float> <float -2.0e+0, float -3.e+4, float 5.e+2> to <3 x i32>)
  %b0 = extractelement <3 x i32> %b, i32 0
  %b1 = extractelement <3 x i32> %b, i32 1
  %b2 = extractelement <3 x i32> %b, i32 2
  call void (i8*, ...)* @printf(i8* %msg_int3, i32 %b0, i32 %b1, i32 %b2)
  ; CHECK: -2 -29997 504

  ; uitofp
  %c = fadd <4 x float> <float 2.e+3, float 1.0e+0, float 4.e+2, float -9.e+0>, uitofp (<4 x i32> <i32 200, i32 0, i32 -1, i32 12> to <4 x float>)
  %c0_ = extractelement <4 x float> %c, i32 0
  %c0 = fpext float %c0_ to double
  %c1_ = extractelement <4 x float> %c, i32 1
  %c1 = fpext float %c1_ to double
  %c2_ = extractelement <4 x float> %c, i32 2
  %c2 = fpext float %c2_ to double
  %c3_ = extractelement <4 x float> %c, i32 3
  %c3 = fpext float %c3_ to double
  call void (i8*, ...)* @printf(i8* %msg_fp4, double %c0, double %c1, double %c2, double %c3)
  ; CHECK: 2200.000000 1.000000 4294967808.000000 3.000000

  ; sitofp
  %d = fadd <2 x double> <double 2.e-3, double 1.0e+0>, uitofp (<2 x i64> <i64 200, i64 0> to <2 x double>)
  %d0 = extractelement <2 x double> %d, i32 0
  %d1 = extractelement <2 x double> %d, i32 1
  call void (i8*, ...)* @printf(i8* %msg_fp2, double %d0, double %d1)
  ; CHECK: 200.002000 1.000000

  ; add
  %e = add <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, add (<8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, <8 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>)
  %e0 = extractelement <8 x i32> %e, i32 0
  %e1 = extractelement <8 x i32> %e, i32 1
  %e2 = extractelement <8 x i32> %e, i32 2
  %e3 = extractelement <8 x i32> %e, i32 3
  %e4 = extractelement <8 x i32> %e, i32 4
  %e5 = extractelement <8 x i32> %e, i32 5
  %e6 = extractelement <8 x i32> %e, i32 6
  %e7 = extractelement <8 x i32> %e, i32 7
  call void (i8*, ...)* @printf(i8* %msg_int8, i32 %e0, i32 %e1, i32 %e2, i32 %e3, i32 %e4, i32 %e5, i32 %e6, i32 %e7)
  ; CHECK: 24 27 30 33 36 39 42 45

  ; fadd
  %f = fadd <2 x float> <float 2.e+3, float 1.0e+0>, fadd (<2 x float> <float 1.2e+3, float 4.5e+6>, <2 x float> <float 7.8e+1, float 9.0e+3>)
  %f0_ = extractelement <2 x float> %f, i32 0
  %f0 = fpext float %f0_ to double
  %f1_ = extractelement <2 x float> %f, i32 1
  %f1 = fpext float %f1_ to double
  call void (i8*, ...)* @printf(i8* %msg_fp2, double %f0, double %f1)
  ; CHECK: 3278.000000 4509001.000000

  ; sub
  %g = add <16 x i16> <i16 0, i16 1, i16 2, i16 3, i16 4, i16 5, i16 6, i16 7, i16 0, i16 1, i16 2, i16 3, i16 4, i16 5, i16 6, i16 7>, sub (<16 x i16> <i16 8, i16 9, i16 10, i16 11, i16 12, i16 13, i16 14, i16 15, i16 0, i16 1, i16 2, i16 3, i16 4, i16 5, i16 6, i16 7>, <16 x i16> <i16 16, i16 17, i16 18, i16 19, i16 20, i16 21, i16 22, i16 23, i16 8, i16 9, i16 10, i16 11, i16 12, i16 13, i16 14, i16 15>)
  %_g0  = extractelement <16 x i16> %g, i32 0
  %g0  = sext i16 %_g0 to i32
  %_g1  = extractelement <16 x i16> %g, i32 1
  %g1  = sext i16 %_g1 to i32
  %_g2  = extractelement <16 x i16> %g, i32 2
  %g2  = sext i16 %_g2 to i32
  %_g3  = extractelement <16 x i16> %g, i32 3
  %g3  = sext i16 %_g3 to i32
  %_g4  = extractelement <16 x i16> %g, i32 4
  %g4  = sext i16 %_g4 to i32
  %_g5  = extractelement <16 x i16> %g, i32 5
  %g5  = sext i16 %_g5 to i32
  %_g6  = extractelement <16 x i16> %g, i32 6
  %g6  = sext i16 %_g6 to i32
  %_g7  = extractelement <16 x i16> %g, i32 7
  %g7  = sext i16 %_g7 to i32
  %_g8  = extractelement <16 x i16> %g, i32 8 
  %g8  = sext i16 %_g8 to i32
  %_g9  = extractelement <16 x i16> %g, i32 9 
  %g9  = sext i16 %_g9 to i32
  %_g10 = extractelement <16 x i16> %g, i32 10
  %g10 = sext i16 %_g10 to i32
  %_g11 = extractelement <16 x i16> %g, i32 11
  %g11 = sext i16 %_g11 to i32
  %_g12 = extractelement <16 x i16> %g, i32 12
  %g12 = sext i16 %_g12 to i32
  %_g13 = extractelement <16 x i16> %g, i32 13
  %g13 = sext i16 %_g13 to i32
  %_g14 = extractelement <16 x i16> %g, i32 14
  %g14 = sext i16 %_g14 to i32
  %_g15 = extractelement <16 x i16> %g, i32 15
  %g15 = sext i16 %_g15 to i32
  call void (i8*, ...)* @printf(i8* %msg_int16, i32 %g0, i32 %g1, i32 %g2, i32 %g3, i32 %g4, i32 %g5, i32 %g6, i32 %g7, i32 %g8, i32 %g9, i32 %g10, i32 %g11, i32 %g12, i32 %g13, i32 %g14, i32 %g15)
  ; CHECK: -8 -7 -6 -5 -4 -3 -2 -1 -8 -7 -6 -5 -4 -3 -2 -1

  ; fsub
  %h = fadd <2 x double> <double 2.e+3, double 1.0e+0>, fsub (<2 x double> <double 1.2e+3, double 4.5e+6>, <2 x double> <double 7.8e+1, double 9.0e+3>)
  %h0 = extractelement <2 x double> %h, i32 0
  %h1 = extractelement <2 x double> %h, i32 1
  call void (i8*, ...)* @printf(i8* %msg_fp2, double %h0, double %h1)
  ; CHECK: 3122.000000 4491001.000000

  ; mul
  ; <3 x i32>
  %i = add <3 x i32> <i32 0, i32 3, i32 4>, mul (<3 x i32> <i32 1, i32 2, i32 5>, <3 x i32><i32 6, i32 7, i32 8>)
  %i0 = extractelement <3 x i32> %i, i32 0
  %i1 = extractelement <3 x i32> %i, i32 1
  %i2 = extractelement <3 x i32> %i, i32 2
  call void (i8*, ...)* @printf(i8* %msg_int3, i32 %i0, i32 %i1, i32 %i2)
  ; CHECK: 6 17 44

  ; fmul
  ; <4 x float>
  %j = fadd <4 x float> <float 2.e+3, float 1.0e+0, float 4.e+2, float -9.e+0>, fmul (<4 x float> <float 5.e+1, float 6.e+2, float 7.e+3, float 8.e+4>, <4 x float> <float 1.e+0, float 0.e+0, float 2.e+2, float 3.e+3>)
  %j0_ = extractelement <4 x float> %j, i32 0
  %j0 = fpext float %j0_ to double
  %j1_ = extractelement <4 x float> %j, i32 1
  %j1 = fpext float %j1_ to double
  %j2_ = extractelement <4 x float> %j, i32 2
  %j2 = fpext float %j2_ to double
  %j3_ = extractelement <4 x float> %j, i32 3
  %j3 = fpext float %j3_ to double
  call void (i8*, ...)* @printf(i8* %msg_fp4, double %j0, double %j1, double %j2, double %j3)
  ; CHECK: 2050.000000 1.000000 1400400.000000 239999984.000000

  ; udiv
  ; <2 x i64>
  %k = add <2 x i64> <i64 20, i64 100>, udiv (<2 x i64> <i64 99, i64 30>, <2 x i64> <i64 7, i64 8>)
  %k0 = extractelement <2 x i64> %k, i32 0
  %k1 = extractelement <2 x i64> %k, i32 1
  call void (i8*, ...)* @printf(i8* %msg_int2, i64 %k0, i64 %k1)
  ; CHECK: 34 103

  ; sdiv
  ; <4 x i8>
  %l = add <4 x i8> <i8 0, i8 1, i8 2, i8 3>, sdiv (<4 x i8> <i8 -80, i8 89, i8 -180, i8 110>, <4 x i8> <i8 16, i8 -17, i8 18, i8 19>)
  %l0_  = extractelement <4 x i8> %l, i32 0
  %l0  = sext i8 %l0_ to i32
  %l1_  = extractelement <4 x i8> %l, i32 1
  %l1  = sext i8 %l1_ to i32
  %l2_  = extractelement <4 x i8> %l, i32 2
  %l2  = sext i8 %l2_ to i32
  %l3_  = extractelement <4 x i8> %l, i32 3
  %l3  = sext i8 %l3_ to i32
  call void (i8*, ...)* @printf(i8* %msg_int4, i32 %l0, i32 %l1, i32 %l2, i32 %l3)
  ; CHECK: -5 -4 6 8

  ; fdiv
  ; <4 x double>
  %m = fadd <4 x double> <double 2.e-3, double 1.0e+0, double 3.e-1, double 8.23e+1>, fdiv (<4 x double> <double -7.56e-2, double 8.23e+4, double 3.e+0, double 6.19e-1>, <4 x double> <double -1.e+0, double 4.e-2, double 2.e+4, double -7.e-4>)
  %m0 = extractelement <4 x double> %m, i32 0
  %m1 = extractelement <4 x double> %m, i32 1
  %m2 = extractelement <4 x double> %m, i32 2
  %m3 = extractelement <4 x double> %m, i32 3
  call void (i8*, ...)* @printf(i8* %msg_fp4, double %m0, double %m1, double %m2, double %m3)
  ; CHECK: 0.077600 2057501.000000 0.300150 -801.985714

  ; urem
  ; <3 x i64>
  %n = add <3 x i64> <i64 20, i64 100, i64 5>, urem (<3 x i64> <i64 99, i64 30, i64 64>, <3 x i64> <i64 7, i64 8, i64 81>)
  %n0 = extractelement <3 x i64> %n, i32 0
  %n1 = extractelement <3 x i64> %n, i32 1
  %n2 = extractelement <3 x i64> %n, i32 2
  call void (i8*, ...)* @printf(i8* %msg_int3, i64 %n0, i64 %n1, i64 %n2)
  ; CHECK: 21 106 69

  ; srem
  ; <2 x i32>
  %o = add <2 x i32> <i32 20, i32 100>, srem (<2 x i32> <i32 -99, i32 30>, <2 x i32> <i32 7, i32 -8>)
  %o0 = extractelement <2 x i32> %o, i32 0
  %o1 = extractelement <2 x i32> %o, i32 1
  call void (i8*, ...)* @printf(i8* %msg_int2, i32 %o0, i32 %o1)
  ; CHECK: 19 106

  ; frem
  ; <3 x float>
  %p = fadd <3 x float> <float 2.e+1, float 1.0e+0, float 4.e+2>, frem (<3 x float> <float 1.2e+3, float 4.5e+6, float 2.e+0>, <3 x float> <float 7.8e+1, float 9.0e+3, float 8.e+2>)
  %p0_ = extractelement <3 x float> %p, i32 0
  %p0 = fpext float %p0_ to double
  %p1_ = extractelement <3 x float> %p, i32 1
  %p1 = fpext float %p1_ to double
  %p2_ = extractelement <3 x float> %p, i32 2
  %p2 = fpext float %p2_ to double
  call void (i8*, ...)* @printf(i8* %msg_fp3, double %p0, double %p1, double %p2)
  ; CHECK: 50.000000 1.000000 402.000000

  ; shl
  ; <4 x i8>
  %q = add <4 x i8> <i8 0, i8 1, i8 2, i8 3>, shl (<4 x i8> <i8 -80, i8 89, i8 -180, i8 110>, <4 x i8> <i8 2, i8 3, i8 1, i8 0>)
  %q0_  = extractelement <4 x i8> %q, i32 0
  %q0  = sext i8 %q0_ to i32
  %q1_  = extractelement <4 x i8> %q, i32 1
  %q1  = sext i8 %q1_ to i32
  %q2_  = extractelement <4 x i8> %q, i32 2
  %q2  = sext i8 %q2_ to i32
  %q3_  = extractelement <4 x i8> %q, i32 3
  %q3  = sext i8 %q3_ to i32
  call void (i8*, ...)* @printf(i8* %msg_int4, i32 %q0, i32 %q1, i32 %q2, i32 %q3)
  ; CHECK: -64 -55 -102 113

  ; lshr
  ; <3 x i16>
  %r = add <3 x i16> <i16 0, i16 1, i16 2>, lshr (<3 x i16> <i16 -80, i16 89, i16 -180>, <3 x i16> <i16 2, i16 3, i16 1>)
  %r0_  = extractelement <3 x i16> %r, i32 0
  %r0  = sext i16 %r0_ to i32
  %r1_  = extractelement <3 x i16> %r, i32 1
  %r1  = sext i16 %r1_ to i32
  %r2_  = extractelement <3 x i16> %r, i32 2
  %r2  = sext i16 %r2_ to i32
  call void (i8*, ...)* @printf(i8* %msg_int3, i32 %r0, i32 %r1, i32 %r2)
  ; CHECK: 16364 12 32680

  ; ashr
  ; <2 x i32>
  %s = add <2 x i32> <i32 20, i32 100>, ashr (<2 x i32> <i32 -99, i32 30>, <2 x i32> <i32 7, i32 8>)
  %s0 = extractelement <2 x i32> %s, i32 0
  %s1 = extractelement <2 x i32> %s, i32 1
  call void (i8*, ...)* @printf(i8* %msg_int2, i32 %s0, i32 %s1)
  ; CHECK: 19 100

  ; and
  ; <8 x i32>
  %t = add <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, and (<8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, <8 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>)
  %t0 = extractelement <8 x i32> %t, i32 0
  %t1 = extractelement <8 x i32> %t, i32 1
  %t2 = extractelement <8 x i32> %t, i32 2
  %t3 = extractelement <8 x i32> %t, i32 3
  %t4 = extractelement <8 x i32> %t, i32 4
  %t5 = extractelement <8 x i32> %t, i32 5
  %t6 = extractelement <8 x i32> %t, i32 6
  %t7 = extractelement <8 x i32> %t, i32 7
  call void (i8*, ...)* @printf(i8* %msg_int8, i32 %t0, i32 %t1, i32 %t2, i32 %t3, i32 %t4, i32 %t5, i32 %t6, i32 %t7)
  ; CHECK: 0 2 4 6 8 10 12 14

  ; or
  ; <4 x i64>
  %u = add <4 x i64> <i64 20, i64 100, i64 5, i64 8>, or (<4 x i64> <i64 99, i64 30, i64 64, i64 230498>, <4 x i64> <i64 7, i64 8, i64 81, i64 98375>)
  %u0 = extractelement <4 x i64> %u, i32 0
  %u1 = extractelement <4 x i64> %u, i32 1
  %u2 = extractelement <4 x i64> %u, i32 2
  %u3 = extractelement <4 x i64> %u, i32 3
  call void (i8*, ...)* @printf(i8* %msg_int4, i64 %u0, i64 %u1, i64 %u2, i64 %u3)
  ; CHECK: 123 130 86 230511

  ; xor
  ; <4 x i8>
  %v = add <4 x i8> <i8 0, i8 1, i8 2, i8 3>, xor (<4 x i8> <i8 -80, i8 89, i8 -180, i8 110>, <4 x i8> <i8 2, i8 3, i8 1, i8 0>)
  %v0_  = extractelement <4 x i8> %v, i32 0
  %v0  = sext i8 %v0_ to i32
  %v1_  = extractelement <4 x i8> %v, i32 1
  %v1  = sext i8 %v1_ to i32
  %v2_  = extractelement <4 x i8> %v, i32 2
  %v2  = sext i8 %v2_ to i32
  %v3_  = extractelement <4 x i8> %v, i32 3
  %v3  = sext i8 %v3_ to i32
  call void (i8*, ...)* @printf(i8* %msg_int4, i32 %v0, i32 %v1, i32 %v2, i32 %v3)
  ; CHECK: -78 91 79 113

  ; getelementptr
  %w = load <2 x i32>* getelementptr (%struct.B* @aa, i32 0, i32 0, i32 0)
  %w0 = extractelement <2 x i32> %w, i32 0
  %w1 = extractelement <2 x i32> %w, i32 1
  call void (i8*, ...)* @printf(i8* %msg_int2, i32 %w0, i32 %w1)
  ; CHECK: 0 0

  ; select: vector condition case doesn't work! Code genrator doesn't yet support conditions with vector type as well as bitecode reader.
  ;%x = add <4 x i64> <i64 20, i64 100, i64 5, i64 8>, select (<4 x i1> <i1 1, i1 0, i1 1, i1 0>, <4 x i64> <i64 99, i64 30, i64 64, i64 230498>, <4 x i64> <i64 7, i64 8, i64 81, i64 98375>)
  %x = add <4 x i64> <i64 20, i64 100, i64 5, i64 8>, select (i1 0, <4 x i64> <i64 99, i64 30, i64 64, i64 230498>, <4 x i64> <i64 7, i64 8, i64 81, i64 98375>)
  %x0 = extractelement <4 x i64> %x, i32 0
  %x1 = extractelement <4 x i64> %x, i32 1
  %x2 = extractelement <4 x i64> %x, i32 2
  %x3 = extractelement <4 x i64> %x, i32 3
  call void (i8*, ...)* @printf(i8* %msg_int4, i64 %x0, i64 %x1, i64 %x2, i64 %x3)
  ; CHECK: 27 108 86 98383

  ; icmp
  %y = and <3 x i1> <i1 1, i1 0, i1 1>, icmp eq (<3 x i64> <i64 99, i64 30, i64 64>, <3 x i64> <i64 99, i64 8, i64 98375>)
  %y0_ = extractelement <3 x i1> %y, i32 0
  %y0 = zext i1 %y0_ to i32
  %y1_ = extractelement <3 x i1> %y, i32 1
  %y1 = zext i1 %y1_ to i32
  %y2_ = extractelement <3 x i1> %y, i32 2
  %y2 = zext i1 %y2_ to i32
  call void (i8*, ...)* @printf(i8* %msg_int3, i32 %y0, i32 %y1, i32 %y2)
  ; CHECK: 1 0 0

  ; fcmp
  %z = or <3 x i1> <i1 1, i1 0, i1 1>, fcmp olt (<3 x double> <double 4.25, double 9.8e-4, double 1.0>, <3 x double> <double -7.9, double 0.0, double 1.98>)
  %z0_ = extractelement <3 x i1> %z, i32 0
  %z0 = zext i1 %z0_ to i32
  %z1_ = extractelement <3 x i1> %z, i32 1
  %z1 = zext i1 %z1_ to i32
  %z2_ = extractelement <3 x i1> %z, i32 2
  %z2 = zext i1 %z2_ to i32
  call void (i8*, ...)* @printf(i8* %msg_int3, i32 %z0, i32 %z1, i32 %z2)
  ; CHECK: 1 0 1

  ; extractelement
  call void (i8*, ...)* @printf(i8* %msg_int, i32 extractelement (<2 x i32> <i32 8, i32 9>, i32 1))
  ; CHECK: 9

  ; insertelement
  %bb = or <3 x i32> <i32 9, i32 9234, i32 5398>, insertelement (<3 x i32> <i32 2938, i32 49384, i32 4>, i32 234, i32 1)
  %bb0 = extractelement <3 x i32> %bb, i32 0
  %bb1 = extractelement <3 x i32> %bb, i32 1
  %bb2 = extractelement <3 x i32> %bb, i32 2
  call void (i8*, ...)* @printf(i8* %msg_int3, i32 %bb0, i32 %bb1, i32 %bb2)
  ; CHECK: 2939 9466 5398

  ; shufflevector
  %cc = fadd <4 x double> <double 1.0, double 2.0, double 3.0, double -4.0>, shufflevector (<2 x double> <double 3.0, double 4.0>, <2 x double> <double 5.0, double 6.0>, <4 x i32> < i32 3, i32 1, i32 2, i32 0 >)
  %cc0 = extractelement <4 x double> %cc, i32 0
  %cc1 = extractelement <4 x double> %cc, i32 1
  %cc2 = extractelement <4 x double> %cc, i32 2
  %cc3 = extractelement <4 x double> %cc, i32 3
  call void (i8*, ...)* @printf(i8* %msg_fp4, double %cc0, double %cc1, double %cc2, double %cc3)
  ; CHECK: 7.000000 6.000000 8.000000 -1.000000

  ; extractvalue
  %dd = fadd <2 x double> <double 1.0, double 2.0>, extractvalue ({<2 x double>, i32} {<2 x double> <double 3.0, double 4.0>, i32 8293}, 0)
  %dd0 = extractelement <2 x double> %dd, i32 0
  %dd1 = extractelement <2 x double> %dd, i32 1
  call void (i8*, ...)* @printf(i8* %msg_fp2, double %dd0, double %dd1)
  ; CHECK: 4.000000 6.000000

  ; insertvalue
  %ee = fadd <2 x double> <double 1.0, double 2.0>, extractvalue ({<2 x double>, i32} insertvalue ({<2 x double>, i32} {<2 x double> <double 3.0, double 4.0>, i32 8293}, <2 x double> <double -10.0, double -20.0>, 0), 0)
  %ee0 = extractelement <2 x double> %ee, i32 0
  %ee1 = extractelement <2 x double> %ee, i32 1
  call void (i8*, ...)* @printf(i8* %msg_fp2, double %ee0, double %ee1)
  ; CHECK: -9.000000 -18.000000

  ; bitcast
  %ff = add <2 x i64> <i64 1034259876, i64 4584395469>, bitcast (<2 x double> <double 3.0, double 4.0> to <2 x i64>)
  %ff0 = extractelement <2 x i64> %ff, i32 0
  %ff1 = extractelement <2 x i64> %ff, i32 1
  call void (i8*, ...)* @printf(i8* %msg_int2, i64 %ff0, i64 %ff1)
  ; CHECK: 1034259876 289428173
  
  ret i32 0
}

