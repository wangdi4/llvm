; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" %s 2>&1 | FileCheck %s

; This test case checks that opt-predicate wasn't applied since the loop
; is SIMD, and the directives are are at the region level. This case handles
; when the candidate is a Select instruction.

; HIR before transformation

; BEGIN REGION { }
;       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
;
;       + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
;       |   %tmp1 = (%n > 0) ? 0 : 1;
;       |   (%p)[i1] = i1 + %tmp1;
;       + END LOOP
;
;       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;       ret ;
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
; CHECK:       |   %tmp1 = (%n > 0) ? 0 : 1;
; CHECK:       |   (%p)[i1] = i1 + %tmp1;
; CHECK:       + END LOOP
; CHECK:       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK:       ret ;
; CHECK: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture %p, ptr nocapture %q, i32 %x, i32 %n) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  %comp = icmp sgt i32 %n, 0
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  br label %new.for.preheader

sw.exit:
  br label %for.inc

for.inc:                                          ; preds = %sw.bb, %sw.bb1, %sw.default
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

new.for.preheader:
  br label %new.for.body

new.for.body:
  %tmp1 = select i1 %comp, i32 0, i32 1
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  %tr = trunc i64 %indvars.iv to i32
  %a = add i32 %tmp1, %tr
  store i32 %a, ptr %arrayidx, align 4
  br label %new.exit

new.exit:
  br label %for.inc
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
