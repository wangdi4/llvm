target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

; RUN: opt -S -instcombine < %s

; This code runs into an endless loop.
define void @test() nounwind {
entry:
  %load_call = call <4 x float> @foocall() nounwind
  %0 = shufflevector <4 x float> %load_call, <4 x float> undef, <4 x i32> <i32 3, i32 3, i32 3, i32 3>
  %bitcast19 = bitcast <4 x float> %0 to <4 x i32>
  %1 = icmp ne <4 x i32> %bitcast19, zeroinitializer
  %movcsext20 = sext <4 x i1> %1 to <4 x i32>
  %movcnot24 = xor <4 x i32> <i32 undef, i32 undef, i32 undef, i32 -1>, %movcsext20
  %movcand25 = and <4 x i32> <i32 undef, i32 undef, i32 undef, i32 -1>, %movcnot24
  %movcor26 = or <4 x i32> %movcand25, undef
  %2 = bitcast <4 x i32> %movcor26 to <4 x float>
  ret void
}
declare <4 x float> @foocall()