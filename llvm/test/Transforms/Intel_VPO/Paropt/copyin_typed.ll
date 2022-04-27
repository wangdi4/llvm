; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s

; #include <omp.h>
; int nder;
; #pragma omp threadprivate(nder)
; void foo() {
; #pragma omp parallel copyin(nder)
;       nder = 12;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; ALL: Enter VPOParoptTransform::genMultiThreadedCode
; CHECK: genTpvCopyIn: Copyin for 'i32* @nder' (Typed):: Type: i32
; OPQPTR: genTpvCopyIn: Copyin for 'ptr @nder' (Typed):: Type: i32
; ALL: Exit VPOParoptTransform::genMultiThreadedCode

@nder = dso_local thread_private global i32 0, align 4

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.COPYIN:TYPED"(i32* @nder, i32 0, i32 1) ]
  store i32 12, i32* @nder, align 4, !tbaa !4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
