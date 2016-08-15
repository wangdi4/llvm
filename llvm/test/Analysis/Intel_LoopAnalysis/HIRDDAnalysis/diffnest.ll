;	for (jx = 27; jx > 1; --jx) {
;		mt  += xk[jx+1];
;		for (j = jx; j < 32; ++j) {
;			xk[j+1] = -xk[j-1];           S1
;		}
;		xk[jx-1] = (xk[jx]) + 1;        S2
;}
;  Between xk[jx -1 ] and xk[j-1]  
;  Both forward & backward edges are expected
;  S1 -> S2 anti (=) 
;  S2 -> S1 flow (<)  
; RUN:  opt < %s    -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK: 'HIR Data Dependence Analysis'
; CHECK-DAG:  (@xk)[0][-1 * i1 + i2 + 26] --> (@xk)[0][-1 * i1 + 26] ANTI (=)
; CHECK-DAG:  (@xk)[0][-1 * i1 + 26] --> (@xk)[0][-1 * i1 + i2 + 26] FLOW (<)
;
;
; ModuleID = 'diffloopnests.c'
source_filename = "diffloopnests.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@yy = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@xk = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @sub() local_unnamed_addr #0 {
entry:
  br label %for.body4.preheader

for.body4.preheader:                              ; preds = %for.end, %entry
  %indvars.iv39 = phi i64 [ 27, %entry ], [ %indvars.iv.next40, %for.end ]
  %mt.034 = phi i32 [ 0, %entry ], [ %add1, %for.end ]
  %0 = add nuw nsw i64 %indvars.iv39, 1
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @yy, i64 0, i64 %0
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add1 = add nsw i32 %1, %mt.034
  br label %for.body4

for.body4:                                        ; preds = %for.body4, %for.body4.preheader
  %indvars.iv36 = phi i64 [ %indvars.iv39, %for.body4.preheader ], [ %indvars.iv.next37, %for.body4 ]
  %2 = add nsw i64 %indvars.iv36, -1
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* @xk, i64 0, i64 %2
  %3 = load i32, i32* %arrayidx6, align 4, !tbaa !1
  %sub7 = sub nsw i32 0, %3
  %indvars.iv.next37 = add nuw nsw i64 %indvars.iv36, 1
  %arrayidx10 = getelementptr inbounds [100 x i32], [100 x i32]* @yy, i64 0, i64 %indvars.iv.next37
  store i32 %sub7, i32* %arrayidx10, align 4, !tbaa !1
  %lftr.wideiv = trunc i64 %indvars.iv.next37 to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 32
  br i1 %exitcond, label %for.end, label %for.body4

for.end:                                          ; preds = %for.body4
  %arrayidx12 = getelementptr inbounds [100 x i32], [100 x i32]* @yy, i64 0, i64 %indvars.iv39
  %4 = load i32, i32* %arrayidx12, align 4, !tbaa !1
  %add13 = add nsw i32 %4, 1
  %indvars.iv.next40 = add nsw i64 %indvars.iv39, -1
  %arrayidx16 = getelementptr inbounds [100 x i32], [100 x i32]* @xk, i64 0, i64 %indvars.iv.next40
  store i32 %add13, i32* %arrayidx16, align 4, !tbaa !1
  %cmp = icmp sgt i64 %indvars.iv.next40, 1
  br i1 %cmp, label %for.body4.preheader, label %for.end18

for.end18:                                        ; preds = %for.end
  %5 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @xk, i64 0, i64 10), align 8, !tbaa !1
  %add19 = add nsw i32 %5, %add1
  ret i32 %add19
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15884) (llvm/branches/loopopt 17800)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}



