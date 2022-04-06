; Test for basic functionality of HIR vectorizer CG for merged CFG with constant TC loop.

; Input HIR
; BEGIN REGION { }
;       %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
;
;       + DO i1 = 0, 100, 1   <DO_LOOP> <simd>
;       |   (%A)[i1] = i1;
;       + END LOOP
;
;       @llvm.directive.region.exit(%tok); [ DIR.OMP.END.SIMD() ]
; END REGION

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-enable-new-cfg-merge-hir -vplan-vec-scenario="n0;v4;s1" -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-enable-new-cfg-merge-hir -vplan-vec-scenario="n0;v4;s1" -disable-output < %s 2>&1 | FileCheck %s

; CHECK-LABEL: Function: foo
; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:           %.vec = 0 == 100;
; CHECK-NEXT:           %phi.temp = 0;
; CHECK-NEXT:           %extract.0. = extractelement %.vec,  0;
; CHECK-NEXT:           if (%extract.0. == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto merge.blk10.26;
; CHECK-NEXT:           }

; CHECK:                + DO i1 = 0, 99, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:           |   (<4 x i64>*)(%A)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:           + END LOOP

; CHECK:                %.vec2 = 101 == 100;
; CHECK-NEXT:           %phi.temp = 100;
; CHECK-NEXT:           %phi.temp4 = 100;
; CHECK-NEXT:           %extract.0.6 = extractelement %.vec2,  0;
; CHECK-NEXT:           if (%extract.0.6 == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto final.merge.44;
; CHECK-NEXT:           }
; CHECK-NEXT:           merge.blk10.26:
; CHECK-NEXT:           %lb.tmp = %phi.temp;

; CHECK:                + DO i1 = %lb.tmp, 100, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3> <nounroll> <novectorize> <max_trip_count = 3>
; CHECK-NEXT:           |   (%A)[i1] = i1;
; CHECK-NEXT:           + END LOOP

; CHECK:                %phi.temp4 = 100;
; CHECK-NEXT:           final.merge.44:
; CHECK-NEXT:     END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i64* nocapture readonly %A) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                           ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i64, i64* %A, i64 %indvars.iv
  store i64 %indvars.iv, i64* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                             ; preds = %for.body
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret void

}
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
