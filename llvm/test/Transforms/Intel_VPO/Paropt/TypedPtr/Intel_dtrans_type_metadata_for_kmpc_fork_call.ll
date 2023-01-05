; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; NOTE: intel.dtrans.types metadata was added manually below.
;void foo() {
;#pragma omp parallel
;  ;
;}

; CHECK: %struct.ident_t = type { i32, i32, i32, i32, i8* }
; CHECK: declare{{.*}}!intel.dtrans.func.type ![[FMD:[0-9]+]] void @__kmpc_fork_call(%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...)
; CHECK: !intel.dtrans.types = !{![[TYMD:[0-9]+]]}
; CHECK: ![[TYMD]] = !{!"S", %struct.ident_t zeroinitializer, i32 5, ![[I32MD:[0-9]+]],
; CHECK-SAME: ![[I32MD]], ![[I32MD]], ![[I32MD]], ![[I8PTRMD:[0-9]+]]}
; CHECK-DAG: ![[I32MD]] = !{i32 0, i32 0}
; CHECK-DAG: ![[I8PTRMD]] = !{i8 0, i32 1}
; CHECK-DAG: ![[FMD]] = distinct !{![[IDENTPTRMD:[0-9]+]], ![[CCHECKBACKPTRMD:[0-9]+]]}
; CHECK-DAG: ![[IDENTPTRMD]] = !{%struct.ident_t zeroinitializer, i32 1}
; CHECK-DAG: ![[CCHECKBACKPTRMD]] = !{![[CCHECKBACKMD:[0-9]+]], i32 1}
; CHECK-DAG: ![[CCHECKBACKMD]] = !{!"F", i1 true, i32 2, ![[VOIDMD:[0-9]+]], ![[I32PTRMD:[0-9]+]], ![[I32PTRMD]]}
; CHECK-DAG: ![[VOIDMD]] = !{!"void", i32 0}
; CHECK-DAG: ![[I32PTRMD]] = !{i32 0, i32 1}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!intel.dtrans.types = !{}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 13.0.0"}
; end INTEL_FEATURE_SW_DTRANS
