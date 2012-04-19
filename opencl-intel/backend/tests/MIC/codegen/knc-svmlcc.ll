; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;
;
;

; arg1 pass in v0, arg2 passed on the stack
declare x86_svmlcc float @__ocl_svml_b1_fractf1(float, float*)

define void @relaxed_test(float* %arg1, float* %arg2) {
; KNF: vloadd    (%rdi){1to16}, %v0
; KNF: movq      %rsi, (%rsp)
; KNF: addq      $24, %rsp
entry:
  %ld = load float* %arg1
  %call.i.i = call x86_svmlcc float @__ocl_svml_b1_fractf1(float %ld, float* %arg2) nounwind
  ret void
}
