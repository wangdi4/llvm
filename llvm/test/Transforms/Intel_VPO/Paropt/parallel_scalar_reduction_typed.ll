; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck --check-prefix=OPQPTR %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck --check-prefix=OPQPTR %s

; Original code:
;void foo(void) {
;  double s[17][17] = {0.0};
;#pragma omp parallel reduction(+:s)
;  ;
;}

; CHECK: define internal void @foo_tree_reduce{{.*}}(i8* [[DSTP:%[0-9A-Za-z._]+]], i8* [[SRCP:%[0-9A-Za-z._]+]]) {
; CHECK-DAG: [[DSTCAST:%[0-9A-Za-z._]+]] = bitcast i8* [[DSTP]] to %struct.fast_red_t*
; CHECK-DAG: [[SRCCAST:%[0-9A-Za-z._]+]] = bitcast i8* [[SRCP]] to %struct.fast_red_t*
; CHECK-DAG: [[DSTS:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* [[DSTCAST]], i32 0, i32 0
; CHECK-DAG: [[SRCS:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* [[SRCCAST]], i32 0, i32 0
; CHECK-DAG: [[DSTV:%[0-9A-Za-z._]+]] = load double, double* [[DSTS]]
; CHECK-DAG: [[SRCV:%[0-9A-Za-z._]+]] = load double, double* [[SRCS]]
; CHECK: [[ADD:%[0-9A-Za-z._]+]] = fadd double [[DSTV]], [[SRCV]]
; CHECK: store double [[ADD]], double* [[DSTS]]


; OPQPTR: define internal void @foo_tree_reduce{{.*}}(ptr [[DSTP:%[0-9A-Za-z._]+]], ptr [[SRCP:%[0-9A-Za-z._]+]]) {
; OPQPTR-DAG: [[DSTS:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.fast_red_t, ptr [[DSTP]], i32 0, i32 0
; OPQPTR-DAG: [[SRCS:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.fast_red_t, ptr [[SRCP]], i32 0, i32 0
; OPQPTR-DAG: [[DSTV:%[0-9A-Za-z._]+]] = load double, ptr [[DSTS]]
; OPQPTR-DAG: [[SRCV:%[0-9A-Za-z._]+]] = load double, ptr [[SRCS]]
; OPQPTR: [[ADD:%[0-9A-Za-z._]+]] = fadd double [[DSTV]], [[SRCV]]
; OPQPTR: store double [[ADD]], ptr [[DSTS]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %s = alloca double, align 8
  store double 0.000000e+00, double* %s, align 8, !tbaa !4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(double* %s, double 0.000000e+00, i32 1) ]
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
!4 = !{!5, !5, i64 0}
!5 = !{!"double", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
