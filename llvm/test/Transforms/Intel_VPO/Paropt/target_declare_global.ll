; Test that globals marked with "target_declare" are not optimized away by
;   "globalopt" (Global Variable Optimizer) or
;   "ipsccp" (Interprocedural Sparse Conditional Constant Propagation)

; RUN: opt -globalopt -S %s | FileCheck %s
; RUN: opt -ipsccp -S %s | FileCheck %s

; ModuleID = 'target_declare_global.cpp'
; test IR obtained with:   icx -Xclang -disable-llvm-passes -fiopenmp \
;                              -S -emit-llvm  target_declare_global.cpp
; // target_declare_global.cpp
; #pragma omp declare target
; static float var1;         // var1 must not be optimized away
; #pragma omp end declare target
; static float var2 = 2.0;   // var2 is expected to be optimized away
;
; void bar(float);
; void foo() {
;   var2 = 2.0;
;   bar(var2);
;   #pragma omp target
;     bar(3.0);
; }


source_filename = "target_declare_global.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_Z4var1 = internal target_declare global float 0.000000e+00, align 4
; CHECK: @_Z4var1 = {{.*}}target_declare
; verify that the target_declare variable var1 is not optimized away

@_Z4var2 = internal global float 2.000000e+00, align 4
; CHECK-NOT: @_Z4var2 =
; verify that the the regular static variable var2 is optimized away

; Function Attrs: uwtable
define dso_local void @_Z3foov() #0 {
entry:
  store float 2.000000e+00, float* @_Z4var2, align 4, !tbaa !4
  %0 = load float, float* @_Z4var2, align 4, !tbaa !4
  call void @_Z3barf(float %0)
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1) ]
  call void @_Z3barf(float 3.000000e+00)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare dso_local void @_Z3barf(float) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2051, i32 30414024, !"_Z3foov", i32 11, i32 1}
!1 = !{i32 1, !"_Z4var1", i32 0, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{!"clang version 8.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
