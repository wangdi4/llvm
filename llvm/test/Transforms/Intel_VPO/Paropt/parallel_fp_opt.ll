; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s

; Original code:
; void foo() {
;   float x;
;   double y;
;   int i;
;   long long int j;
;   char c;
;   long double ld;
; #pragma omp parallel firstprivate(x, y, i, j, c, ld)
;   {
;     (void)x;(void)y;(void)i;(void)j;(void)c;(void)ld;
;   }
; }

; Check that float, double, i32, i64 and i8 firsrprivates are passed by value:
; CHECK-DAG: %x = alloca float
; CHECK-DAG: %y = alloca double
; CHECK-DAG: %i = alloca i32
; CHECK-DAG: %j = alloca i64
; CHECK-DAG: %c = alloca i8
; CHECK-DAG: %ld = alloca x86_fp80
; CHECK-DAG: [[x_val:%[a-zA-Z._0-9]+]] = load float, float* %x
; CHECK-DAG: [[x_val_bcast:%[a-zA-Z._0-9]+]] = bitcast float [[x_val]] to i32
; CHECK-DAG: [[x_val_bcast_zext:%[a-zA-Z._0-9]+]] = zext i32 [[x_val_bcast]] to i64
; CHECK-DAG: [[y_val:%[a-zA-Z._0-9]+]] = load double, double* %y
; CHECK-DAG: [[y_val_bcast:%[a-zA-Z._0-9]+]] = bitcast double [[y_val]] to i64
; CHECK-DAG: [[i_val:%[a-zA-Z._0-9]+]] = load i32, i32* %i
; CHECK-DAG: [[i_val_zext:%[a-zA-Z._0-9]+]] = zext i32 [[i_val]] to i64
; CHECK-DAG: [[j_val:%[a-zA-Z._0-9]+]] = load i64, i64* %j
; CHECK-DAG: [[c_val:%[a-zA-Z._0-9]+]] = load i8, i8* %c
; CHECK-DAG: [[c_val_zext:%[a-zA-Z._0-9]+]] = zext i8 [[c_val]] to i64
; CHECK: call void{{.*}}@__kmpc_fork_call({{[^,]*}}, i32 6, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, x86_fp80*, i64, i64, i64, i64, i64)* @foo{{[a-zA-Z._0-9]*}} to void (i32*, i32*, ...)*), x86_fp80* %ld, i64 [[c_val_zext]], i64 [[j_val]], i64 [[i_val_zext]], i64 [[y_val_bcast]], i64 [[x_val_bcast_zext]])
; CHECK-DAG: define internal void @foo{{[a-zA-Z._0-9]*}}(i32*{{[^,]*}}, i32*{{[^,]*}}, x86_fp80*{{[^,]*}}, i64{{[^,]*}}, i64{{[^,]*}}, i64{{[^,]*}}, i64{{[^,]*}}, i64{{[^,]*}})


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %x = alloca float, align 4
  %y = alloca double, align 8
  %i = alloca i32, align 4
  %j = alloca i64, align 8
  %c = alloca i8, align 1
  %ld = alloca x86_fp80, align 16
  %0 = bitcast float* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast double* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %1) #2
  %2 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = bitcast i64* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %3) #2
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %c) #2
  %4 = bitcast x86_fp80* %ld to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %4) #2
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"(float* %x), "QUAL.OMP.FIRSTPRIVATE"(double* %y), "QUAL.OMP.FIRSTPRIVATE"(i32* %i), "QUAL.OMP.FIRSTPRIVATE"(i64* %j), "QUAL.OMP.FIRSTPRIVATE"(i8* %c), "QUAL.OMP.FIRSTPRIVATE"(x86_fp80* %ld) ]
  %6 = load float, float* %x, align 4, !tbaa !2
  %7 = load double, double* %y, align 8, !tbaa !6
  %8 = load i32, i32* %i, align 4, !tbaa !8
  %9 = load i64, i64* %j, align 8, !tbaa !10
  %10 = load i8, i8* %c, align 1, !tbaa !12
  %11 = load x86_fp80, x86_fp80* %ld, align 16, !tbaa !13
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL"() ]
  %12 = bitcast x86_fp80* %ld to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %12) #2
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %c) #2
  %13 = bitcast i64* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %13) #2
  %14 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #2
  %15 = bitcast double* %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %15) #2
  %16 = bitcast float* %x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %16) #2
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"double", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !4, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"long long", !4, i64 0}
!12 = !{!4, !4, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"long double", !4, i64 0}
