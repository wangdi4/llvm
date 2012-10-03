; RUN: llc < %s -mcpu=knc -march=y86-64 | FileCheck %s

define i32 @foo(i32 %x, i32 %y, i32 %z) nounwind readnone {
entry:
; CHECK: foo:
; CHECK: roll %cl
	%0 = shl i32 %x, %z
	%1 = sub i32 32, %z
	%2 = lshr i32 %x, %1
	%3 = or i32 %2, %0
	ret i32 %3
}

define i32 @xfoo(i32 %x, i32 %y, i32 %z) nounwind readnone {
entry:
; CHECK: xfoo:
; CHECK: roll $7
	%0 = lshr i32 %x, 25
	%1 = shl i32 %x, 7
	%2 = or i32 %0, %1
	ret i32 %2
}

define i32 @xun(i32 %x, i32 %y, i32 %z) nounwind readnone {
entry:
; CHECK: xun:
; CHECK: roll $25
	%0 = lshr i32 %x, 7
	%1 = shl i32 %x, 25
	%2 = or i32 %0, %1
	ret i32 %2
}
