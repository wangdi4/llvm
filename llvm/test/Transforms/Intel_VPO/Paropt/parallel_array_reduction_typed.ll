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
; CHECK-DAG: [[DSTSGEP:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], [289 x double]* [[DSTS]], i32 0, i32 0
; CHECK-DAG: [[SRCSGEP:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], [289 x double]* [[SRCS]], i32 0, i32 0
; CHECK-DAG: [[DPTRCAST:%[0-9A-Za-z._]+]] = bitcast double* [[DSTSGEP]] to [289 x double]*
; CHECK-DAG: [[DARRBEGIN:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], [289 x double]* [[DPTRCAST]], i32 0
; CHECK-DAG: [[DARRBEGINCAST:%[0-9A-Za-z._]+]] = bitcast [289 x double]* [[DARRBEGIN]] to double*
; CHECK-DAG: [[SPTRCAST:%[0-9A-Za-z._]+]] = bitcast double* [[SRCSGEP]] to [289 x double]*
; CHECK-DAG: [[SARRBEGIN:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], [289 x double]* [[SPTRCAST]], i32 0
; CHECK-DAG: [[SARRBEGINCAST:%[0-9A-Za-z._]+]] = bitcast [289 x double]* [[SARRBEGIN]] to double*
; CHECK-DAG: [[DARREND:%[0-9A-Za-z._]+]] = getelementptr double, double* [[DARRBEGINCAST]], i64 289
; CHECK-DAG: [[ISEMPTY:%[0-9A-Za-z._]+]] = icmp eq double* [[DARRBEGINCAST]], [[DARREND]]


; OPQPTR: define internal void @foo_tree_reduce{{.*}}(ptr [[DSTP:%[0-9A-Za-z._]+]], ptr [[SRCP:%[0-9A-Za-z._]+]]) {
; OPQPTR-DAG: [[DSTS:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.fast_red_t, ptr [[DSTP]], i32 0, i32 0
; OPQPTR-DAG: [[SRCS:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.fast_red_t, ptr [[SRCP]], i32 0, i32 0
; OPQPTR-DAG: [[DSTSGEP:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], ptr [[DSTS]], i32 0, i32 0
; OPQPTR-DAG: [[SRCSGEP:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], ptr [[SRCS]], i32 0, i32 0
; OPQPTR-DAG: [[DARRBEGIN:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], ptr [[DSTSGEP]], i32 0
; OPQPTR-DAG: [[DARREND:%[0-9A-Za-z._]+]] = getelementptr double, ptr [[DARRBEGIN]], i64 289
; OPQPTR-DAG: [[SARRBEGIN:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], ptr [[SRCSGEP]], i32 0
; OPQPTR-DAG: [[ISEMPTY:%[0-9A-Za-z._]+]] = icmp eq ptr [[DARRBEGIN]], [[DARREND]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %s = alloca [17 x [17 x double]], align 16
  %0 = bitcast [17 x [17 x double]]* %s to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %0, i8 0, i64 2312, i1 false)
  %array.begin = getelementptr inbounds [17 x [17 x double]], [17 x [17 x double]]* %s, i32 0, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.ADD:TYPED"([17 x [17 x double]]* %s, double 0.000000e+00, i64 289) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nounwind willreturn writeonly }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 13.0.0"}
