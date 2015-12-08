; RUN: llc < %s -march=x86 -mattr=-x87 | FileCheck %s
; RUN: llc < %s -march=x86-64 -mattr=-x87 | FileCheck %s

; CHECK-NOT: fadd{{.*}}

define float @foo(float %a, float %b) nounwind readnone {
entry:
	%0 = fadd float %a, %b
	ret float %0
}
