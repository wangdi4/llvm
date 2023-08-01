; Test to verify that LoopOpt framework does not identify instructions
; that redefine temps to be FP inductions.

; HIR before vectorizer
;  BEGIN REGION { modified }
;        %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; 
;        + DO i1 = 0, 1023, 1   <DO_LOOP>
;        |   %0 = (%A)[i1];
;        |   %t1.115 = %0  +  1.000000e+00;
;        |   %t1.115 = %t1.115  +  1.000000e+00;
;        + END LOOP
; 
;        @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;  END REGION

; RUN: opt -passes='hir-ssa-deconstruction,hir-pre-vec-complete-unroll,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -disable-output < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local float @foo(ptr nocapture noundef readonly %A) {
; CHECK-LABEL: Function: foo
; CHECK-EMPTY: 
; CHECK-NEXT:  BEGIN REGION { modified }
; CHECK-NEXT:        + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   %.vec = (<4 x float>*)(%A)[i1];
; CHECK-NEXT:        |   %.vec2 = %.vec  +  1.000000e+00;
; CHECK-NEXT:        |   %.vec3 = %.vec2  +  1.000000e+00;
; CHECK-NEXT:        + END LOOP

; CHECK:             %t1.115 = extractelement %.vec3,  3;
; CHECK-NEXT:  END REGION
;
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc4
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc4 ]
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.body3
  %t1.115 = phi float [ %0, %for.body ], [ %add, %for.body3 ]
  %cmp2 = phi i1 [ true, %for.body ], [ false, %for.body3 ]
  %add = fadd fast float %t1.115, 1.000000e+00
  br i1 %cmp2, label %for.body3, label %for.inc4, !llvm.loop !0

for.inc4:                                         ; preds = %for.body3
  %add.lcssa = phi float [ %add, %for.body3 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.end6, label %for.body

for.end6:                                         ; preds = %for.inc4
  %add.lcssa.lcssa = phi float [ %add.lcssa, %for.inc4 ]
  ret float %add.lcssa.lcssa
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 2}
