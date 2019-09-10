; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-opt-predicate -print-before=hir-opt-predicate -print-after=hir-opt-predicate %s 2>&1 | FileCheck %s
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,print<hir>,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" %s 2>&1 | FileCheck %s

; Check unswitching of the sibling switch and an if statements.
; The test also checks that "if (%m != 0)" is added as a new candidate after cloning for case 1.

;        BEGIN REGION { }
;             + DO i1 = 0, 99, 1   <DO_LOOP>
;             |   switch(%n)
;             |   {
;             |   case 1:
;             |      @bar1();
;             |      break;
;             |   default:
;             |      break;
;             |   }
;             |   if (%m != 0)
;             |   {
;             |      @bar2();
;             |   }
;             + END LOOP
;        END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:      switch(%n)
; CHECK:      {
; CHECK:      case 1:
; CHECK:         if (%m != 0)
; CHECK:         {
; CHECK:            + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:            |   @bar1();
; CHECK:            |   @bar2();
; CHECK:            + END LOOP
; CHECK:         }
; CHECK:         else
; CHECK:         {
; CHECK:            + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:            |   @bar1();
; CHECK:            + END LOOP
; CHECK:         }
; CHECK:         break;
; CHECK:      default:
; CHECK:         if (%m != 0)
; CHECK:         {
; CHECK:            + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:            |   @bar2();
; CHECK:            + END LOOP
; CHECK:         }
; CHECK:         break;
; CHECK:      }
; CHECK: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* %a, i32 %n, i32 %m) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  switch i32 %n, label %sw.epilog [
    i32 1, label %sw.bb
  ]

sw.bb:                                            ; preds = %for.body
  call void (...) @bar1()
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.bb, %for.body
  %tobool = icmp ne i32 %m, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %sw.epilog
  call void (...) @bar2()
  br label %if.end

if.end:                                           ; preds = %if.then, %sw.epilog
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

declare dso_local void @bar1(...)
declare dso_local void @bar2(...)

