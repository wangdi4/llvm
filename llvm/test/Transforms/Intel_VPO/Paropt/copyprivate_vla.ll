; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
;
; #include <stdlib.h>
;
; void foo(int64_t n) {
;   int a[n], b[20];
; // #pragma omp single copyprivate(b)
; #pragma omp single copyprivate(a)
;   ;
; }

; Since the frontend does not allow VLA in the copyprivate clause, the IR is modified to include VLA manually.

; CHECK: %[[CP_STRUCT:copyprivate.agg.*]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[CP_ARRAY_INFO:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[CP_STRUCT]], i32 0, i32 0
; CHECK: %[[CP_A:.*]] = getelementptr inbounds %[[ARRAY_INFO:__struct.kmp_copy_privates_t.array.*]], ptr %[[CP_ARRAY_INFO]], i32 0, i32 0
; CHECK: store ptr %vla, ptr %[[CP_A]], align 8
; CHECK: %[[CP_A_SIZE:.*]] = getelementptr inbounds %[[ARRAY_INFO]], ptr %[[CP_ARRAY_INFO]], i32 0, i32 1
; CHECK: store i64 %n, ptr %[[CP_A_SIZE]], align 8
; CHECK: call void @__kmpc_copyprivate(ptr @{{.*}}, i32 %{{.*}}, i32 16, ptr %[[CP_STRUCT]], ptr @[[CPRIV_CALLBACK:_Z3fool_copy_priv.*]], i32 %{{.*}})
; CHECK: define internal void @[[CPRIV_CALLBACK]](ptr %[[STRUCT_DST:.*]], ptr %[[STRUCT_SRC:.*]])
; CHECK: %[[SRC_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_SRC]], i32 0, i32 0
; CHECK: %[[DST_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_DST]], i32 0, i32 0
; CHECK: %[[SRC_GEP:.*]] = getelementptr inbounds %[[ARRAY_INFO]], ptr %[[SRC_INFO_GEP]], i32 0, i32 0
; CHECK: %[[SRC:.*]] = load ptr, ptr %[[SRC_GEP]], align 8
; CHECK: %[[DST_GEP:.*]] = getelementptr inbounds %[[ARRAY_INFO]], ptr %[[DST_INFO_GEP]], i32 0, i32 0
; CHECK: %[[DST:.*]] = load ptr, ptr %[[DST_GEP]], align 8
; CHECK: %[[SRC_SIZE_GEP:.*]] = getelementptr inbounds %[[ARRAY_INFO]], ptr %[[SRC_INFO_GEP]], i32 0, i32 1
; CHECK: %[[SRC_SIZE:.*]] = load i64, ptr %[[SRC_SIZE_GEP]], align 8
; CHECK: %[[DATA_SIZE:.*]] = mul i64 4, %[[SRC_SIZE]]
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %[[DST]], ptr align 4 %[[SRC]], i64 %[[DATA_SIZE]], i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fool(i64 noundef %n) #0 {
entry:
  %n.addr = alloca i64, align 8
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %b = alloca [20 x i32], align 16
  store i64 %n, ptr %n.addr, align 8
  %0 = load i64, ptr %n.addr, align 8
  %1 = call ptr @llvm.stacksave()
  store ptr %1, ptr %saved_stack, align 8
  %vla = alloca i32, i64 %0, align 16
  store i64 %0, ptr %__vla_expr0, align 8
  %array.begin = getelementptr inbounds [20 x i32], ptr %b, i32 0, i32 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.COPYPRIVATE:TYPED"(ptr %vla, i32 0, i64 %n) ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SINGLE"() ]
  %3 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %3)
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
