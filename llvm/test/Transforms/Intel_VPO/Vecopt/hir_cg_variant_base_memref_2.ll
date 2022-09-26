; Test to check correctness of HIR vector CG for loops containing memrefs with loop variant
; base CEs that are of the form (%base)[0] i.e. an access without GEPs.

; HIR incoming to vectorizer
; <0>     BEGIN REGION { }
; <13>          %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; <12>
; <12>          + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>           |   %0 = (%arr)[i1];
; <5>           |   %sum.011 = (%0)[0]  +  %sum.011; <Safe Reduction>
; <12>          + END LOOP
; <12>
; <14>          @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; <0>     END REGION


; Check CG when VPLoopEntities representation is used for reduction.
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s --check-prefix=VPRED
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 < %s 2>&1 | FileCheck %s --check-prefix=VPRED


; VPRED:             + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; VPRED-NEXT:        |   %.vec = (<4 x i32*>*)(%arr)[i1];
; VPRED-NEXT:        |   %.vec1 = (<4 x i32>*)(%.vec)[0];
; VPRED-NEXT:        |   %.vec2 = %.vec1  +  %phi.temp;
; VPRED-NEXT:        |   %phi.temp = %.vec2;
; VPRED-NEXT:        + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo2(i32** nocapture readonly %arr) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.011 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds i32*, i32** %arr, i64 %indvars.iv
  %0 = load i32*, i32** %arrayidx, align 8
  %1 = load i32, i32* %0, align 4
  %add = add nsw i32 %1, %sum.011
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %add.lcssa
}
