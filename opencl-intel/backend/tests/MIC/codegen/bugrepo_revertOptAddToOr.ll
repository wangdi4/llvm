; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc

target datalayout =
"e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test(i32 %x, <16 x i32>* %y, i32* %z) nounwind {
%a = and i32 %x, 15
%b = load <16 x i32>* %y, align 4
%c = extractelement <16 x i32> %b, i32 %a
store i32 %c, i32* %z, align 4
ret void
}


