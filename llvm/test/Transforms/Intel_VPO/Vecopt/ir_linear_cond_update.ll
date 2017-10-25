; RUN: opt -VPlanDriver -S %s | FileCheck %s

; This test checks for handling of linear values and that we do a unit stride store
; CHECK: vector.ph
; CHECK:  load i32, i32* %i2
; CHECK: vector.body
; CHECK:   store <4 x i32> %vec.linear{{.*}}
; ModuleID = 'll.c'
source_filename = "ll.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %arr, i32 %before) local_unnamed_addr #0 {
entry:
  %i2 = alloca i32, align 4
  %0 = bitcast i32* %i2 to i8*
  call void @llvm.lifetime.start(i64 4, i8* nonnull %0) #3
  store i32 0, i32* %i2, align 4, !tbaa !1
  call void @foo3(i32* nonnull %i2) #3
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LINEAR", i32* nonnull %i2, i32 1)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.SIMD.1
  %tobool = icmp eq i32 %before, 0
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %DIR.QUAL.LIST.END.2
  %.omp.iv.011 = phi i32 [ 0, %DIR.QUAL.LIST.END.2 ], [ %add5, %omp.inner.for.inc ]
  br i1 %tobool, label %if.then2.critedge, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %1 = load i32, i32* %i2, align 4, !tbaa !1
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* %i2, align 4, !tbaa !1
  call void (...) @baz() #3
  %2 = load i32, i32* %i2, align 4, !tbaa !1
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %idxprom
  store i32 %2, i32* %arrayidx, align 4, !tbaa !1
  br label %omp.inner.for.inc

if.then2.critedge:                                ; preds = %omp.inner.for.body
  call void (...) @baz() #3
  %3 = load i32, i32* %i2, align 4, !tbaa !1
  %idxprom.c = sext i32 %3 to i64
  %arrayidx.c = getelementptr inbounds i32, i32* %arr, i64 %idxprom.c
  store i32 %3, i32* %arrayidx.c, align 4, !tbaa !1
  %inc3 = add nsw i32 %3, 1
  store i32 %inc3, i32* %i2, align 4, !tbaa !1
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then, %if.then2.critedge
  %add5 = add nuw nsw i32 %.omp.iv.011, 1
  %exitcond = icmp eq i32 %add5, 299
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.inc
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %omp.loop.exit
  call void @llvm.lifetime.end(i64 4, i8* nonnull %0) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare void @foo3(i32*) local_unnamed_addr #2

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
