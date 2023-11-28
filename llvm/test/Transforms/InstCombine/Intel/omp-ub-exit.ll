; RUN: opt -S -passes=instcombine < %s | FileCheck %s
; CHECK-NOT: call{{.*}}llvm.directive

; The call is UB and will cause the directive.region.exit to be deleted.
; The directive.region.entry must be deleted also.
; This needs 2 IC passes.

define void @sub_maxval_() {
alloca_0:
  br label %DIR.OMP.END.ORDERED.113

DIR.OMP.END.ORDERED.113:                          ; preds = %alloca_0
  %omp.ordered = call token @llvm.directive.region.entry() [
    "DIR.OMP.ORDERED"(),
    "QUAL.OMP.ORDERED.SIMD"() ]
  %func_result4 = call i32 null(ptr null, ptr null, ptr null)
  br label %DIR.OMP.END.ORDERED.2

DIR.OMP.END.ORDERED.2:                            ; preds = %DIR.OMP.END.ORDERED.113
  call void @llvm.directive.region.exit(token %omp.ordered) [
  "DIR.OMP.END.ORDERED"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

attributes #0 = { nounwind }
