; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; This code tests TYPED clause
; The test is passed if PRIVATE:TYPED and FIRSTPRIVATE:TYPED clauses are parsed correctly

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: PRIVATE clause (size=1):{{.*}} , TYPED (TYPE: float, NUM_ELEMENTS: i32 1)
; CHECK: FIRSTPRIVATE clause (size=1): {{.*}} , TYPED (TYPE: float, NUM_ELEMENTS: i32 10)

; #include <omp.h>
; int main() {
;     float x = 5.0;
;     float y[10];
;     #pragma omp parallel private (x) firstprivate(y)
;     { y[5] = x; }
; }

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %x = alloca float, align 4
  %y = alloca [10 x float], align 16
  %0 = bitcast float* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  store float 5.000000e+00, float* %x, align 4, !tbaa !4
  %1 = bitcast [10 x float]* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 40, i8* %1) #2
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE:TYPED"(float* %x, float 0.000000e+00, i32 1), "QUAL.OMP.FIRSTPRIVATE:TYPED"([10 x float]* %y, float 0.000000e+00, i32 10) ]
  %3 = load float, float* %x, align 4, !tbaa !4
  %arrayidx = getelementptr inbounds [10 x float], [10 x float]* %y, i64 0, i64 5, !intel-tbaa !8
  store float %3, float* %arrayidx, align 4, !tbaa !8
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  %4 = bitcast [10 x float]* %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 40, i8* %4) #2
  %5 = bitcast float* %x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %5) #2
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 10.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !5, i64 0}
!9 = !{!"array@_ZTSA10_f", !5, i64 0}
