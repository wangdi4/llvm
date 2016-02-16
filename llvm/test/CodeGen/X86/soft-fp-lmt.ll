; RUN: llc < %s -march=x86 -mcpu=lakemont | FileCheck %s

; Make sure -mcpu=lakemont implies soft floats.
; CHECK: __addsf3{{.*}}

define float @foo(float %a, float %b) nounwind readnone {
entry:
	%0 = fadd float %a, %b
	ret float %0
}
