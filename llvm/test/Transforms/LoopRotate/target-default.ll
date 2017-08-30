; REQUIRES: asserts
; RUN: opt < %s -march=x86 -mcpu=pentium -S -loop-rotate -debug -debug-only=loop-rotate 2>&1 | FileCheck %s -check-prefix=PENTIUM
; RUN: opt < %s -march=x86 -mcpu=lakemont -S -loop-rotate -debug -debug-only=loop-rotate 2>&1 | FileCheck %s -check-prefix=LMT
; RUN: opt < %s -march=x86 -mcpu=pentium -S -loop-rotate -rotation-max-header-size=0 -debug -debug-only=loop-rotate 2>&1 | FileCheck %s -check-prefix=PENTIUM-OPT
; RUN: opt < %s -march=x86 -mcpu=lakemont -S -loop-rotate -rotation-max-header-size=16 -debug -debug-only=loop-rotate 2>&1 | FileCheck %s -check-prefix=LMT-OPT

; Loop should be rotated for Pentium but not for Lakemont.
; PENTIUM: rotating Loop at depth 1
; LMT-NOT: rotating Loop at depth 1

; Specification of -rotation-max-header-size should suppress default
; target threshold.
; PENTIUM-OPT-NOT: rotating Loop at depth 1
; LMT-OPT: rotating Loop at depth 1

target triple = "x86_64-unknown-linux-gnu"

declare void @use(i32*, i32)

define void @test(i32* %x, i32 %y) optsize {
entry:
  br label %for.cond

for.cond:
  %x.addr.0 = phi i32* [ %x, %entry ], [ %incdec.ptr, %for.body ]
  %0 = load i32, i32* %x.addr.0, align 4
  %cmp = icmp sgt i32 %0, 0
  %cmp1 = icmp sgt i32 %y, 0
  %or.cond = and i1 %cmp, %cmp1
  br i1 %or.cond, label %for.body, label %for.end

for.body:
  tail call void @use(i32* %x.addr.0, i32 %y)
  %incdec.ptr = getelementptr inbounds i32, i32* %x.addr.0, i64 1
  br label %for.cond

for.end:
  ret void
}
