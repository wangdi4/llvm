; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck --check-prefix=OPQPTR %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck --check-prefix=OPQPTR %s

; Original code:
;int fp[10];
;void foo() {
;#pragma omp parallel firstprivate(fp)
;  ;
;}

; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 {{[^,]+}},
; CHECK-SAME: i8* align 4 {{[^,]+}}, i64 40, i1 false)

; OPQPTR: call void @llvm.memcpy.p0.p0.i64(ptr align 4 {{[^,]+}},
; OPQPTR-SAME: ptr align 4 {{[^,]+}}, i64 40, i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@fp = dso_local global [10 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE:TYPED"([10 x i32]* @fp, i32 0, i64 10) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 13.0.0"}
