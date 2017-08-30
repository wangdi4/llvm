;    for (j=0; j <  n; j++) {
;        B[2*j] +=  1;
;    }
;    for (j=0; j <  2*n; j++) {
;       B[2*j+1] +=  1;
;    }
;
; RUN:  opt < %s -hir-ssa-deconstruction -hir-create-function-level-region | opt -hir-create-function-level-region -hir-dd-test-assume-loop-fusion -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze  | FileCheck %s 
; No dep for even/odd when invoked from Loop Fusion demand driven DD 
; CHECK-NOT:  (%B)[0][2 * i1] --> (%B)[0][2 * i1 + 1] 
;
; ModuleID = 'fuse3.c'
source_filename = "fuse3.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone uwtable
define void @sub1(float* nocapture %A, i32 %n) local_unnamed_addr #0 {
entry:
  %B = alloca [1000 x float], align 16
  %0 = bitcast [1000 x float]* %B to i8*
  call void @llvm.lifetime.start.p0i8(i64 4000, i8* nonnull %0) #2
  %cmp23 = icmp sgt i32 %n, 0
  br i1 %cmp23, label %for.body.preheader, label %for.end12

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond1.preheader:                              ; preds = %for.body
  %cmp321 = icmp sgt i32 %n, 0
  br i1 %cmp321, label %for.body4.preheader, label %for.end12

for.body4.preheader:                              ; preds = %for.cond1.preheader
  %mul2 = shl nsw i32 %n, 1
  %1 = sext i32 %mul2 to i64
  br label %for.body4

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv27 = phi i64 [ %indvars.iv.next28, %for.body ], [ 0, %for.body.preheader ]
  %2 = shl nsw i64 %indvars.iv27, 1
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* %B, i64 0, i64 %2
  %3 = load float, float* %arrayidx, align 8, !tbaa !1
  %add = fadd float %3, 1.000000e+00
  store float %add, float* %arrayidx, align 8, !tbaa !1
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond = icmp eq i64 %indvars.iv.next28, %wide.trip.count
  br i1 %exitcond, label %for.cond1.preheader, label %for.body

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %indvars.iv = phi i64 [ 0, %for.body4.preheader ], [ %indvars.iv.next, %for.body4 ]
  %indvars.iv.tr = trunc i64 %indvars.iv to i32
  %4 = shl i32 %indvars.iv.tr, 1
  %5 = or i32 %4, 1
  %idxprom7 = sext i32 %5 to i64
  %arrayidx8 = getelementptr inbounds [1000 x float], [1000 x float]* %B, i64 0, i64 %idxprom7
  %6 = load float, float* %arrayidx8, align 4, !tbaa !1
  %add9 = fadd float %6, 1.000000e+00
  store float %add9, float* %arrayidx8, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp3 = icmp slt i64 %indvars.iv.next, %1
  br i1 %cmp3, label %for.body4, label %for.end12.loopexit

for.end12.loopexit:                               ; preds = %for.body4
  br label %for.end12

for.end12:                                        ; preds = %for.end12.loopexit, %entry, %for.cond1.preheader
  call void @llvm.lifetime.end.p0i8(i64 4000, i8* nonnull %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
