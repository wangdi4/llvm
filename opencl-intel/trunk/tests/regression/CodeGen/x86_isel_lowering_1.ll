; RUN: llc < %s -march=x86-64

;;LLVMIR start
; ModuleID = 'dx2llvm'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

define fastcc void @aos_shader() nounwind {
entry:
  %0 = load <4 x float>* undef
  %bitcast = bitcast <4 x float> %0 to <4 x i32>
  %1 = uitofp <4 x i32> %bitcast to <4 x float>
  %2 = shufflevector <4 x float> undef, <4 x float> %1, <4 x i32> <i32 0, i32 1, i32 6, i32 3>
  store <4 x float> %2, <4 x float> addrspace(1)* undef
  ret void
}
;;LLVMIR end
