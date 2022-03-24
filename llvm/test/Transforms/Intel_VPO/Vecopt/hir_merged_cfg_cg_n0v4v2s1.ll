; Test for basic functionality of HIR vectorizer CG for merged CFG.
; Vec Scenario:
;    - No peel
;    - Main vector loop with VF=4
;    - Vectorized remainder loop with VF=2
;    - Scalar remainder

; Input HIR
; BEGIN REGION { }
;       %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
;       %sum.07 = %init;
;
;       + DO i1 = 0, %N + -1, 1   <DO_LOOP> <simd>
;       |   %A.i = (%A)[i1];
;       |   %sum.07 = %A.i  +  %sum.07;
;       + END LOOP
;
;       @llvm.directive.region.exit(%tok); [ DIR.OMP.END.SIMD() ]
; END REGION

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-enable-new-cfg-merge-hir -vplan-vec-scenario="n0;v4;v2s1" -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s

; CHECK-LABEL: Function: foo
; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:           %sum.07 = %init;
; CHECK-NEXT:           %tgu = %N  /u  2;
; CHECK-NEXT:           %vec.tc = %tgu  *  2;
; CHECK-NEXT:           %.vec = 0 == %vec.tc;
; CHECK-NEXT:           %phi.temp = %sum.07;
; CHECK-NEXT:           %phi.temp2 = 0;
; CHECK-NEXT:           %extract.0. = extractelement %.vec,  0;
; CHECK-NEXT:           if (%extract.0. == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto merge.blk15.31;
; CHECK-NEXT:           }
; CHECK-NEXT:           %tgu4 = %N  /u  4;
; CHECK-NEXT:           %vec.tc5 = %tgu4  *  4;
; CHECK-NEXT:           %.vec6 = 0 == %vec.tc5;
; CHECK-NEXT:           %phi.temp7 = %sum.07;
; CHECK-NEXT:           %phi.temp9 = 0;
; CHECK-NEXT:           %extract.0.11 = extractelement %.vec6,  0;
; CHECK-NEXT:           if (%extract.0.11 == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto merge.blk19.42;
; CHECK-NEXT:           }
; CHECK-NEXT:           %tgu12 = %N  /u  4;
; CHECK-NEXT:           %vec.tc13 = %tgu12  *  4;
; CHECK-NEXT:           %red.init = 0;
; CHECK-NEXT:           %red.init.insert = insertelement %red.init,  %sum.07,  0;
; CHECK-NEXT:           %phi.temp14 = %red.init.insert;

; CHECK:                + DO i1 = 0, %vec.tc13 + -1, 4   <DO_LOOP> <simd-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:           |   %.vec16 = (<4 x i32>*)(%A)[i1];
; CHECK-NEXT:           |   %.vec17 = %.vec16  +  %phi.temp14;
; CHECK-NEXT:           |   %phi.temp14 = %.vec17;
; CHECK-NEXT:           + END LOOP

; CHECK:                %sum.07 = @llvm.vector.reduce.add.v4i32(%.vec17);
; CHECK-NEXT:           %tgu19 = %N  /u  2;
; CHECK-NEXT:           %vec.tc20 = %tgu19  *  2;
; CHECK-NEXT:           %.vec21 = %vec.tc20 == %vec.tc13;
; CHECK-NEXT:           %phi.temp7 = %sum.07;
; CHECK-NEXT:           %phi.temp9 = %vec.tc13;
; CHECK-NEXT:           %phi.temp24 = %sum.07;
; CHECK-NEXT:           %phi.temp26 = %vec.tc13;
; CHECK-NEXT:           %extract.0.28 = extractelement %.vec21,  0;
; CHECK-NEXT:           if (%extract.0.28 == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto merge.blk18.72;
; CHECK-NEXT:           }
; CHECK-NEXT:           merge.blk19.42:
; CHECK-NEXT:           %tgu29 = %N  /u  2;
; CHECK-NEXT:           %vec.tc30 = %tgu29  *  2;
; CHECK-NEXT:           %red.init31 = 0;
; CHECK-NEXT:           %red.init.insert32 = insertelement %red.init31,  %phi.temp7,  0;
; CHECK-NEXT:           %phi.temp33 = %red.init.insert32;

; CHECK:                + DO i1 = %phi.temp9, %vec.tc30 + -1, 2   <DO_LOOP> <nounroll> <novectorize>
; CHECK-NEXT:           |   %.vec35 = (<2 x i32>*)(%A)[i1];
; CHECK-NEXT:           |   %.vec36 = %.vec35  +  %phi.temp33;
; CHECK-NEXT:           |   %phi.temp33 = %.vec36;
; CHECK-NEXT:           + END LOOP

; CHECK:                %sum.07 = @llvm.vector.reduce.add.v2i32(%.vec36);
; CHECK-NEXT:           %phi.temp24 = %sum.07;
; CHECK-NEXT:           %phi.temp26 = %vec.tc30;
; CHECK-NEXT:           merge.blk18.72:
; CHECK-NEXT:           %tgu41 = %N  /u  2;
; CHECK-NEXT:           %vec.tc42 = %tgu41  *  2;
; CHECK-NEXT:           %.vec43 = %N == %vec.tc42;
; CHECK-NEXT:           %phi.temp = %phi.temp24;
; CHECK-NEXT:           %phi.temp2 = %phi.temp26;
; CHECK-NEXT:           %phi.temp46 = %phi.temp24;
; CHECK-NEXT:           %phi.temp48 = %phi.temp26;
; CHECK-NEXT:           %extract.0.50 = extractelement %.vec43,  0;
; CHECK-NEXT:           if (%extract.0.50 == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto final.merge.107;
; CHECK-NEXT:           }
; CHECK-NEXT:           merge.blk15.31:
; CHECK-NEXT:           %lb.tmp = %phi.temp2;
; CHECK-NEXT:           %sum.07 = %phi.temp;

; CHECK:                + DO i1 = %lb.tmp, %N + -1, 1   <DO_LOOP> <vectorize>
; CHECK-NEXT:           |   %A.i = (%A)[i1];
; CHECK-NEXT:           |   %sum.07 = %A.i  +  %sum.07;
; CHECK-NEXT:           + END LOOP

; CHECK:                %phi.temp46 = %sum.07;
; CHECK-NEXT:           %phi.temp48 = %N + -1;
; CHECK-NEXT:           final.merge.107:
; CHECK-NEXT: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32* nocapture readonly %A, i64 %N, i32 %init) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                           ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %sum.07 = phi i32 [ %add, %for.body ], [ %init, %entry ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %A.i = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %A.i, %sum.07
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %N
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                             ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret i32 %add.lcssa

}
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
