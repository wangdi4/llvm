; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" %s 2>&1 | FileCheck %s

; This test case checks that opt-predicate wasn't applied since the loop
; is SIMD, and the directives are are at the region level. This case handles
; when the candidate is a Switch condition.

; HIR before transformation

; BEGIN REGION { }
;       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
;
;       + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
;       |   switch(%x)
;       |   {
;       |   case 0:
;       |      (%p)[i1] = i1;
;       |      break;
;       |   case 2:
;       |      (%q)[i1] = i1;
;       |      break;
;       |   default:
;       |      (%q)[i1 + 1] = i1;
;       |      break;
;       |   }
;       + END LOOP
;
;       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;       ret ;
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
; CHECK:       |   switch(%x)
; CHECK:       |   {
; CHECK:       |   case 0:
; CHECK:       |      (%p)[i1] = i1;
; CHECK:       |      break;
; CHECK:       |   case 2:
; CHECK:       |      (%q)[i1] = i1;
; CHECK:       |      break;
; CHECK:       |   default:
; CHECK:       |      (%q)[i1 + 1] = i1;
; CHECK:       |      break;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK:       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK:       ret ;
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture %p, ptr nocapture %q, i32 %x) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  switch i32 %x, label %sw.default [
    i32 0, label %sw.bb
    i32 2, label %sw.bb1
  ]

sw.bb:                                            ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  %tmp = trunc i64 %indvars.iv to i32
  store i32 %tmp, ptr %arrayidx, align 4
  br label %for.inc

sw.bb1:                                           ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv
  %tmp1 = trunc i64 %indvars.iv to i32
  store i32 %tmp1, ptr %arrayidx3, align 4
  br label %for.inc

sw.default:                                       ; preds = %for.body
  %tmp2 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds i32, ptr %q, i64 %tmp2
  %tmp3 = trunc i64 %indvars.iv to i32
  store i32 %tmp3, ptr %arrayidx5, align 4
  br label %for.inc

for.inc:                                          ; preds = %sw.bb, %sw.bb1, %sw.default
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
