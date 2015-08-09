; RUN: llc -x86-asm-syntax=intel < %s | FileCheck %s
; CHECK: .syntax_intel prefix
define i32 @test(i32 %a) {
entry:
; 	CHECK-LABEL: test:
	%tmp3 = mul i32 %a, 31
	ret i32 %tmp3
}
