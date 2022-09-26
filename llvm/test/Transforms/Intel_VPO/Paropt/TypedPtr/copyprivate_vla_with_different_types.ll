; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
;
; #include <stdlib.h>
;
; void foo(int64_t n) {
;   short a[n], b[20];
;   double c[n], d[10];
; // #pragma omp single copyprivate(b, d)
; #pragma omp single copyprivate(a, c)
;   ;
; }

; Since the frontend does not allow VLA in the copyprivate clause, the IR is modified to include VLA manually.

; Check the different copyprivate structs for array A and C.
; CHECK: %__struct.kmp_copy_privates_t = type { %[[A_ARRAY_INFO:__struct.kmp_copy_privates_t.array.*]], %[[C_ARRAY_INFO:__struct.kmp_copy_privates_t.array.*]] }
; CHECK: %[[A_ARRAY_INFO]] = type { i16*, i64 }
; CHECK: %[[C_ARRAY_INFO]] = type { double*, i64 }

; Check storing the array A's starting addr and size
; CHECK: %[[CP_STRUCT:copyprivate.agg.*]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[CP_A_ARRAY_INFO:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[CP_STRUCT]], i32 0, i32 0
; CHECK: %[[CP_A:.*]] = getelementptr inbounds %[[A_ARRAY_INFO]], %[[A_ARRAY_INFO]]* %[[CP_A_ARRAY_INFO]], i32 0, i32 0
; CHECK: store i16* %vla, i16** %[[CP_A]], align 8
; CHECK: %[[CP_A_SIZE:.*]] = getelementptr inbounds %[[A_ARRAY_INFO]], %[[A_ARRAY_INFO]]* %[[CP_A_ARRAY_INFO]], i32 0, i32 1
; CHECK: store i64 %0, i64* %[[CP_A_SIZE]], align 8

; Check storing the array C's starting addr and size
; CHECK: %[[CP_C_ARRAY_INFO:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[CP_STRUCT]], i32 0, i32 1
; CHECK: %[[CP_C:.*]] = getelementptr inbounds %[[C_ARRAY_INFO]], %[[C_ARRAY_INFO]]* %[[CP_C_ARRAY_INFO]], i32 0, i32 0
; CHECK: store double* %vla1, double** %[[CP_C]], align 8
; CHECK: %[[CP_C_SIZE:.*]] = getelementptr inbounds %[[C_ARRAY_INFO]], %[[C_ARRAY_INFO]]* %[[CP_C_ARRAY_INFO]], i32 0, i32 1
; CHECK: store i64 %2, i64* %[[CP_C_SIZE]], align 8
; CHECK: %[[CP_STRUCT_BITCAST:.*]] = bitcast %__struct.kmp_copy_privates_t* %[[CP_STRUCT]] to i8*
; CHECK: call void @__kmpc_copyprivate(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 32, i8* %[[CP_STRUCT_BITCAST]], i8* bitcast (void (%__struct.kmp_copy_privates_t*, %__struct.kmp_copy_privates_t*)* @[[CPRIV_CALLBACK:_Z3fool_copy_priv.*]] to i8*), i32 %{{.*}})

; Check loading the array A's starting addr and size
; CHECK: define internal void @[[CPRIV_CALLBACK]](%__struct.kmp_copy_privates_t* %[[STRUCT_DST:.*]], %__struct.kmp_copy_privates_t* %[[STRUCT_SRC:.*]])
; CHECK: %[[A_SRC_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[STRUCT_SRC]], i32 0, i32 0
; CHECK: %[[A_DST_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[STRUCT_DST]], i32 0, i32 0
; CHECK: %[[A_SRC_GEP:.*]] = getelementptr inbounds %[[A_ARRAY_INFO]], %[[A_ARRAY_INFO]]* %[[A_SRC_INFO_GEP]], i32 0, i32 0
; CHECK: %[[A_SRC:.*]] = load i16*, i16** %[[A_SRC_GEP]], align 8
; CHECK: %[[A_DST_GEP:.*]] = getelementptr inbounds %[[A_ARRAY_INFO]], %[[A_ARRAY_INFO]]* %[[A_DST_INFO_GEP]], i32 0, i32 0
; CHECK: %[[A_DST:.*]] = load i16*, i16** %[[A_DST_GEP]], align 8
; CHECK: %[[A_SRC_SIZE_GEP:.*]] = getelementptr inbounds %[[A_ARRAY_INFO]], %[[A_ARRAY_INFO]]* %[[A_SRC_INFO_GEP]], i32 0, i32 1
; CHECK: %[[A_SRC_SIZE:.*]] = load i64, i64* %[[A_SRC_SIZE_GEP]], align 8
; CHECK: %[[A_DATA_SIZE:.*]] = mul i64 2, %[[A_SRC_SIZE]]

; Check loading the array C's starting addr and size
; CHECK: %[[C_SRC_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[STRUCT_SRC]], i32 0, i32 1
; CHECK: %[[C_DST_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[STRUCT_DST]], i32 0, i32 1
; CHECK: %[[C_SRC_GEP:.*]] = getelementptr inbounds %[[C_ARRAY_INFO]], %[[C_ARRAY_INFO]]* %[[C_SRC_INFO_GEP]], i32 0, i32 0
; CHECK: %[[C_SRC:.*]] = load double*, double** %[[C_SRC_GEP]], align 8
; CHECK: %[[C_DST_GEP:.*]] = getelementptr inbounds %[[C_ARRAY_INFO]], %[[C_ARRAY_INFO]]* %[[C_DST_INFO_GEP]], i32 0, i32 0
; CHECK: %[[C_DST:.*]] = load double*, double** %[[C_DST_GEP]], align 8
; CHECK: %[[C_SRC_SIZE_GEP:.*]] = getelementptr inbounds %[[C_ARRAY_INFO]], %[[C_ARRAY_INFO]]* %[[C_SRC_INFO_GEP]], i32 0, i32 1
; CHECK: %[[C_SRC_SIZE:.*]] = load i64, i64* %[[C_SRC_SIZE_GEP]], align 8
; CHECK: %[[C_DATA_SIZE:.*]] = mul i64 8, %[[C_SRC_SIZE]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fool(i64 noundef %n) #0 {
entry:
  %n.addr = alloca i64, align 8
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %b = alloca [20 x i16], align 16
  %__vla_expr1 = alloca i64, align 8
  %d = alloca [10 x double], align 16
  store i64 %n, i64* %n.addr, align 8
  %0 = load i64, i64* %n.addr, align 8
  %1 = call i8* @llvm.stacksave()
  store i8* %1, i8** %saved_stack, align 8
  %vla = alloca i16, i64 %0, align 16
  store i64 %0, i64* %__vla_expr0, align 8
  %2 = load i64, i64* %n.addr, align 8
  %vla1 = alloca double, i64 %2, align 16
  store i64 %2, i64* %__vla_expr1, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.COPYPRIVATE"(i16* %vla),
    "QUAL.OMP.COPYPRIVATE"(double* %vla1) ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SINGLE"() ]
  %4 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %4)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8*) #1

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
