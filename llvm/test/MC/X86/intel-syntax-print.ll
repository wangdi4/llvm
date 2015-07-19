; RUN: llc -x86-asm-syntax=intel < %s | FileCheck %s
; CHECK: .intel_syntax noprefix
define i32 @test(i32 %a) {
entry:
; 	CHECK-LABEL: test:
	%tmp3 = mul i32 %a, 31
	ret i32 %tmp3
}
