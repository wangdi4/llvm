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

; CHECK-LABEL: Function: foo
; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:           %tgu = 101  /u  4;
; CHECK-NEXT:           %vec.tc = %tgu  *  4;
; CHECK-NEXT:           %.vec = 0 == %vec.tc;
; CHECK-NEXT:           %phi.temp = 0;
; CHECK-NEXT:           %unifcond = extractelement %.vec,  0;
; CHECK-NEXT:           if (%unifcond == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto merge.blk10.28;
; CHECK-NEXT:           }
; FIXME: This makes the TC of main loop non-constant prohibiting further
; transforms like vector loop unrolling. Inline these computations instead.
; CHECK-NEXT:           %tgu2 = 101  /u  4;
; CHECK-NEXT:           %vec.tc3 = %tgu2  *  4;

; CHECK:                + DO i1 = 0, %vec.tc3 + -1, 4   <DO_LOOP> <simd-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:           |   (<4 x i64>*)(%A)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:           + END LOOP

; CHECK:                %.vec4 = 101 == %vec.tc3;
; CHECK-NEXT:           %phi.temp = %vec.tc3;
; CHECK-NEXT:           %phi.temp6 = %vec.tc3;
; CHECK-NEXT:           %unifcond8 = extractelement %.vec4,  0;
; CHECK-NEXT:           if (%unifcond8 == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto final.merge.48;
; CHECK-NEXT:           }
; CHECK-NEXT:           merge.blk10.28:
; CHECK-NEXT:           %lb.tmp = %phi.temp;

; CHECK:                + DO i1 = %lb.tmp, 100, 1   <DO_LOOP> <vectorize>
; CHECK-NEXT:           |   (%A)[i1] = i1;
; CHECK-NEXT:           + END LOOP

; CHECK:                %phi.temp6 = 100;
; CHECK-NEXT:           final.merge.48:
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
