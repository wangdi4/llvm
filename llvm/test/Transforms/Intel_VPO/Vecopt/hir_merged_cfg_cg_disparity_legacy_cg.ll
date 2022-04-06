; Test for basic functionality of HIR vectorizer CG for parity between merged CFG
; based CG and legacy CG approaches.

; Input HIR
; BEGIN REGION { }
;       %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
;
;       + DO i1 = 0, 9, 1   <DO_LOOP> <simd>
;       |   (%A)[i1] = i1;
;       + END LOOP
;
;       @llvm.directive.region.exit(%tok); [ DIR.OMP.END.SIMD() ]
; END REGION

; RUN: opt -enable-new-pm=false -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-enable-new-cfg-merge-hir=false -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=LEGACY-CG
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-enable-new-cfg-merge-hir=false -disable-output < %s 2>&1 | FileCheck %s --check-prefix=LEGACY-CG
; RUN: opt -enable-new-pm=false -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-enable-new-cfg-merge-hir -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=MERGED-CFG-CG
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-enable-new-cfg-merge-hir -disable-output < %s 2>&1 | FileCheck %s --check-prefix=MERGED-CFG-CG

; LEGACY-CG:                BEGIN REGION { modified }
; LEGACY-CG-NEXT:                 + DO i1 = 0, 7, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; LEGACY-CG-NEXT:                 |   (<4 x i64>*)(%A)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; LEGACY-CG-NEXT:                 + END LOOP

; LEGACY-CG:                      + DO i1 = 8, 9, 1   <DO_LOOP> <novectorize>
; LEGACY-CG-NEXT:                 |   (%A)[i1] = i1;
; LEGACY-CG-NEXT:                 + END LOOP
; LEGACY-CG-NEXT:           END REGION

; MERGED-CFG-CG:            BEGIN REGION { modified }
; MERGED-CFG-CG-NEXT:             %.vec = 0 == 8;
; MERGED-CFG-CG-NEXT:             %phi.temp = 0;
; MERGED-CFG-CG-NEXT:             %extract.0. = extractelement %.vec,  0;
; MERGED-CFG-CG-NEXT:             if (%extract.0. == 1)
; MERGED-CFG-CG-NEXT:             {
; MERGED-CFG-CG-NEXT:                goto merge.blk10.26;
; MERGED-CFG-CG-NEXT:             }

; MERGED-CFG-CG:                  + DO i1 = 0, 7, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; MERGED-CFG-CG-NEXT:             |   (<4 x i64>*)(%A)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; MERGED-CFG-CG-NEXT:             + END LOOP

; MERGED-CFG-CG:                  %.vec2 = 10 == 8;
; MERGED-CFG-CG-NEXT:             %phi.temp = 8;
; MERGED-CFG-CG-NEXT:             %phi.temp4 = 8;
; MERGED-CFG-CG-NEXT:             %extract.0.6 = extractelement %.vec2,  0;
; MERGED-CFG-CG-NEXT:             if (%extract.0.6 == 1)
; MERGED-CFG-CG-NEXT:             {
; MERGED-CFG-CG-NEXT:                goto final.merge.44;
; MERGED-CFG-CG-NEXT:             }
; MERGED-CFG-CG-NEXT:             merge.blk10.26:
; MERGED-CFG-CG-NEXT:             %lb.tmp = %phi.temp;

; MERGED-CFG-CG:                  + DO i1 = %lb.tmp, 9, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3> <nounroll> <novectorize> <max_trip_count = 3>
; MERGED-CFG-CG-NEXT:             |   (%A)[i1] = i1;
; MERGED-CFG-CG-NEXT:             + END LOOP

; MERGED-CFG-CG:                  %phi.temp4 = 9;
; MERGED-CFG-CG-NEXT:             final.merge.44:
; MERGED-CFG-CG-NEXT:       END REGION


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
  %exitcond = icmp eq i64 %indvars.iv.next, 10
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
