; RUN: opt -VPlanDriver -disable-vplan-predicator -disable-vplan-subregions -vplan-force-vf=4 -S -enable-vp-value-codegen=false %s | FileCheck %s

; This test checks for a widened alloca and a wide store to the widened alloca
; CHECK:  %a2.vec = alloca <4 x i32>
; CHECK: vector.ph
; CHECK: vector.body
; CHECK:   store <4 x i32> {{.*}}, <4 x i32>* %a2.vec

; ModuleID = 'p1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture readonly %iarr)  #0 {
entry:
  %a2 = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"(i32* %a2) ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.QUAL.LIST.END.2
  %.omp.iv.05 = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %add1, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %iarr, i64 %.omp.iv.05
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !5
  store i32 %0, i32* %a2, align 4, !tbaa !5
  call void @baz(i32 %0) #3
  %add1 = add nuw nsw i64 %.omp.iv.05, 1
  %exitcond = icmp eq i64 %add1, 100
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %omp.loop.exit
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare void @baz(i32)  #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: argmemonly nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 21166)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
