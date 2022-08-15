; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
;
; void foo(int n) {
;   short a[n], b[20];
;   double c[n], d[10];
; // #pragma omp single copyprivate(b, d)
; #pragma omp single copyprivate(a, c)
;   ;
; }

; Since the frontend does not allow VLA in the copyprivate clause, the IR is modified to include VLA manually.

; Check the different copyprivate structs for array A and C.
; CHECK: %__struct.kmp_copy_privates_t = type { %[[A_ARRAY_INFO:__struct.kmp_copy_privates_t.array.*]], %[[C_ARRAY_INFO:__struct.kmp_copy_privates_t.array.*]] }
; CHECK: %[[A_ARRAY_INFO]] = type { ptr, i64 }
; CHECK: %[[C_ARRAY_INFO]] = type { ptr, i32 }

; Check storing the array A's starting addr and size
; CHECK: %[[CP_STRUCT:copyprivate.agg.*]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[CP_A_ARRAY_INFO:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[CP_STRUCT]], i32 0, i32 0
; CHECK: %[[CP_A:.*]] = getelementptr inbounds %[[A_ARRAY_INFO]], ptr %[[CP_A_ARRAY_INFO]], i32 0, i32 0
; CHECK: store ptr %vla, ptr %[[CP_A]], align 8
; CHECK: %[[CP_A_SIZE:.*]] = getelementptr inbounds %[[A_ARRAY_INFO]], ptr %[[CP_A_ARRAY_INFO]], i32 0, i32 1
; CHECK: store i64 %1, ptr %[[CP_A_SIZE]], align 8

; Check storing the array C's starting addr and size
; CHECK: %[[CP_C_ARRAY_INFO:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[CP_STRUCT]], i32 0, i32 1
; CHECK: %[[CP_C:.*]] = getelementptr inbounds %[[C_ARRAY_INFO]], ptr %[[CP_C_ARRAY_INFO]], i32 0, i32 0
; CHECK: store ptr %vla1, ptr %[[CP_C]], align 8
; CHECK: %[[CP_C_SIZE:.*]] = getelementptr inbounds %[[C_ARRAY_INFO]], ptr %[[CP_C_ARRAY_INFO]], i32 0, i32 1
; CHECK: store i32 %n, ptr %[[CP_C_SIZE]], align 4
; CHECK: call void @__kmpc_copyprivate(ptr @{{.*}}, i32 %{{.*}}, i32 32, ptr %[[CP_STRUCT]], ptr @[[CPRIV_CALLBACK:_Z3fooi_copy_priv.*]], i32 %{{.*}})

; Check loading the array A's starting addr and size
; CHECK: define internal void @[[CPRIV_CALLBACK]](ptr %[[STRUCT_DST:.*]], ptr %[[STRUCT_SRC:.*]])
; CHECK: %[[A_SRC_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_SRC]], i32 0, i32 0
; CHECK: %[[A_DST_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_DST]], i32 0, i32 0
; CHECK: %[[A_SRC_GEP:.*]] = getelementptr inbounds %[[A_ARRAY_INFO]], ptr %[[A_SRC_INFO_GEP]], i32 0, i32 0
; CHECK: %[[A_SRC:.*]] = load ptr, ptr %[[A_SRC_GEP]], align 8
; CHECK: %[[A_DST_GEP:.*]] = getelementptr inbounds %[[A_ARRAY_INFO]], ptr %[[A_DST_INFO_GEP]], i32 0, i32 0
; CHECK: %[[A_DST:.*]] = load ptr, ptr %[[A_DST_GEP]], align 8
; CHECK: %[[A_SRC_SIZE_GEP:.*]] = getelementptr inbounds %[[A_ARRAY_INFO]], ptr %[[A_SRC_INFO_GEP]], i32 0, i32 1
; CHECK: %[[A_SRC_SIZE:.*]] = load i64, ptr %[[A_SRC_SIZE_GEP]], align 8
; CHECK: %[[A_DATA_SIZE:.*]] = mul i64 2, %[[A_SRC_SIZE]]
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 2 %[[A_DST]], ptr align 2 %[[A_SRC]], i64 %[[A_DATA_SIZE]], i1 false)

; Check loading the array C's starting addr and size
; CHECK: %[[C_SRC_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_SRC]], i32 0, i32 1
; CHECK: %[[C_DST_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_DST]], i32 0, i32 1
; CHECK: %[[C_SRC_GEP:.*]] = getelementptr inbounds %[[C_ARRAY_INFO]], ptr %[[C_SRC_INFO_GEP]], i32 0, i32 0
; CHECK: %[[C_SRC:.*]] = load ptr, ptr %[[C_SRC_GEP]], align 8
; CHECK: %[[C_DST_GEP:.*]] = getelementptr inbounds %[[C_ARRAY_INFO]], ptr %[[C_DST_INFO_GEP]], i32 0, i32 0
; CHECK: %[[C_DST:.*]] = load ptr, ptr %[[C_DST_GEP]], align 8
; CHECK: %[[C_SRC_SIZE_GEP:.*]] = getelementptr inbounds %[[C_ARRAY_INFO]], ptr %[[C_SRC_INFO_GEP]], i32 0, i32 1
; CHECK: %[[C_SRC_SIZE:.*]] = load i32, ptr %[[C_SRC_SIZE_GEP]], align 4
; CHECK: %[[C_SRC_SIZE_I64:.*]] = zext i32 %[[C_SRC_SIZE]] to i64
; CHECK: %[[C_DATA_SIZE:.*]] = mul i64 8, %[[C_SRC_SIZE_I64]]
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 8 %[[C_DST]], ptr align 8 %[[C_SRC]], i64 %[[C_DATA_SIZE]], i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fooi(i32 noundef %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %b = alloca [20 x i16], align 16
  %__vla_expr1 = alloca i64, align 8
  %d = alloca [10 x double], align 16
  store i32 %n, ptr %n.addr, align 4
  %0 = load i32, ptr %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr %saved_stack, align 8
  %vla = alloca i16, i64 %1, align 16
  store i64 %1, ptr %__vla_expr0, align 8
  %3 = load i32, ptr %n.addr, align 4
  %4 = zext i32 %3 to i64
  %vla1 = alloca double, i64 %4, align 16
  store i64 %4, ptr %__vla_expr1, align 8
  %array.begin = getelementptr inbounds [20 x i16], ptr %b, i32 0, i32 0
  %array.begin2 = getelementptr inbounds [10 x double], ptr %d, i32 0, i32 0
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.COPYPRIVATE:TYPED"(ptr %vla, i16 0, i64 %1),
    "QUAL.OMP.COPYPRIVATE:TYPED"(ptr %vla1, double 0.000000e+00, i32 %n) ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SINGLE"() ]
  %6 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %6)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #1

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
