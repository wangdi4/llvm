; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-opt-predicate -print-before=hir-opt-predicate -print-after=hir-opt-predicate %s 2>&1 | FileCheck %s
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,print<hir>,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" %s 2>&1 | FileCheck %s

; Check the unswitching of two sibling switch statements with equivalent conditions but different set of cases.

;        BEGIN REGION { }
;             + DO i1 = 0, 99, 1   <DO_LOOP>
;             |   switch(%n)
;             |   {
;             |   case 1:
;             |      @bar1();
;             |      break;
;             |   default:
;             |      @bar2();
;             |      break;
;             |   }
;             |   switch(%n)
;             |   {
;             |   case 2:
;             |      @bar3();
;             |      break;
;             |   default:
;             |      @bar4();
;             |      break;
;             |   }
;             + END LOOP
;        END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:      switch(%n)
; CHECK:      {
; CHECK:      case 1:
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   @bar1();
; CHECK:         |   @bar4();
; CHECK:         + END LOOP
; CHECK:         break;
; CHECK:      case 2:
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   @bar2();
; CHECK:         |   @bar3();
; CHECK:         + END LOOP
; CHECK:         break;
; CHECK:      default:
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   @bar2();
; CHECK:         |   @bar4();
; CHECK:         + END LOOP
; CHECK:         break;
; CHECK:      }
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* %a, i32 %n) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  switch i32 %n, label %sw.default [
    i32 1, label %sw.bb
  ]

sw.bb:                                            ; preds = %for.body
  call void (...) @bar1()
  br label %sw.epilog

sw.default:                                       ; preds = %for.body
  call void (...) @bar2()
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb
  switch i32 %n, label %sw.default2 [
    i32 2, label %sw.bb1
  ]

sw.bb1:                                           ; preds = %sw.epilog
  call void (...) @bar3()
  br label %sw.epilog3

sw.default2:                                      ; preds = %sw.epilog
  call void (...) @bar4()
  br label %sw.epilog3

sw.epilog3:                                       ; preds = %sw.default2, %sw.bb1
  br label %for.inc

for.inc:                                          ; preds = %sw.epilog3
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

declare dso_local void @bar1(...)
declare dso_local void @bar2(...)
declare dso_local void @bar3(...)
declare dso_local void @bar4(...)

