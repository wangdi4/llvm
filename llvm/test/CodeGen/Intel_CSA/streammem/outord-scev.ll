; RUN: llc < %s | FileCheck %s --check-prefix=CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* nocapture readnone, i32 %num_samples, float** nocapture readonly %distance_array) local_unnamed_addr {
entry:
  %cmp19 = icmp sgt i32 %num_samples, 0
  br i1 %cmp19, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %num_samples to i64
  br label %for.body

for.body:                                         ; preds = %omp.loop.exit.i, %for.body.preheader
  %indvars.iv28 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next29, %omp.loop.exit.i ]
  %arrayidx5 = getelementptr inbounds float*, float** %distance_array, i64 %indvars.iv28
; CHECK: sld64
  %1 = load float*, float** %arrayidx5, align 8, !tbaa !2
  %2 = tail call i32 @llvm.csa.lic.init(i8 4, i64 0, i64 0) #0
  tail call void @llvm.csa.lic.write.f32(i32 %2, float undef) #0
  br label %omp.inner.for.body.i

omp.inner.for.body.i:                             ; preds = %omp.inner.for.body.i, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %omp.inner.for.body.i ]
  %3 = tail call fast float @llvm.csa.lic.read.f32(i32 %2) #0
  %arrayidx.i = getelementptr inbounds float, float* %1, i64 %indvars.iv
; CHECK: sst32
  store float %3, float* %arrayidx.i, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond, label %omp.loop.exit.i, label %omp.inner.for.body.i

omp.loop.exit.i:                                  ; preds = %omp.inner.for.body.i
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %exitcond30 = icmp eq i64 %indvars.iv.next29, %wide.trip.count
  br i1 %exitcond30, label %for.end, label %for.body

for.end:                                          ; preds = %omp.loop.exit.i, %entry
  ret void
}

; Function Attrs: nounwind
declare i32 @llvm.csa.lic.init(i8, i64, i64) #0

; Function Attrs: nounwind
declare void @llvm.csa.lic.write.f32(i32, float) #0

; Function Attrs: nounwind
declare float @llvm.csa.lic.read.f32(i32) #0

attributes #0 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPf", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}
