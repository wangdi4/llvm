; RUN: opt -VPlanDriver -S %s | FileCheck %s

; This test checks for handling of linear values and that we do a unit stride store
; CHECK: vector.ph
; CHECK:  load i32, i32* %i2
; CHECK: vector.body
; CHECK:  call void @llvm.masked.scatter.v4i32(<4 x i32> %vec.linear, {{.*}})
; CHECK:  %lin.cast{{.*}} = trunc i64 {{.*}} to i32
; CHECK-NEXT:  mul i32 %lin.cast{{.*}}, 3
; CHECK-NEXT:  add i32 {{.*}}
; CHECK-NEXT: store i32 {{.*}}, i32* %i2

; ModuleID = 'll2.c'
source_filename = "ll2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %arr) local_unnamed_addr #0 {
entry:
  %i2 = alloca i32, align 4
  %0 = bitcast i32* %i2 to i8*
  call void @llvm.lifetime.start(i64 4, i8* nonnull %0) #3
  store i32 0, i32* %i2, align 4, !tbaa !1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LINEAR", i32* nonnull %i2, i32 3)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %.omp.iv.06 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add1, %omp.inner.for.body ]
  call void (...) @baz() #3
  %1 = load i32, i32* %i2, align 4, !tbaa !1
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %idxprom
  store i32 %1, i32* %arrayidx, align 4, !tbaa !1
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* %i2, align 4, !tbaa !1
  %add1 = add nuw nsw i64 %.omp.iv.06, 1
  %exitcond = icmp eq i64 %add1, 300
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %omp.loop.exit
  call void @llvm.lifetime.end(i64 4, i8* nonnull %0) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare void @baz(...) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 21412)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
