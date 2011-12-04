; RUN: llc %s -march=x86-64 -mcpu=corei7 -o %t.asm
; RUN: FileCheck %s --input-file=%t.asm
; CHECK: BB#0
; CHECK: ptest 	%xmm0, %xmm0
; CHECK: je{{[ \.]*}}LBB0_2

declare i32 @llvm.x86.sse41.ptestz(<4 x float> %p1, <4 x float> %p2) nounwind

define <4 x float> @bypass(<4 x float> %a, <4 x float> %b) nounwind {
entry:
  %res = call i32 @llvm.x86.sse41.ptestz(<4 x float> %a, <4 x float> %a) nounwind 
  %tmp = and i32 %res, 1 
  %one = icmp eq i32 %tmp, 0 
  br i1 %one, label %label1, label %label2
  
label1:
  %c = fadd <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
  br label %end_func
  
label2:
	%d = fdiv <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
	br label %end_func
	
end_func:
  %e = phi <4 x float> [%c, %label1], [%d, %label2]
  ret <4 x float> %e
}
