; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
;
; void foo() {
;   int a = 10;
;   int &b = a;
; #pragma omp single copyprivate(b)
;   ;
; }

; CHECK: store ptr %a, ptr %b, align 8
; CHECK-NEXT: %[[BREF:.*]] = load ptr, ptr %b, align 8
; CHECK: %[[CP_STRUCT:copyprivate.agg.*]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[CP_BREF:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[CP_STRUCT]], i32 0, i32 0
; CHECK: store ptr %[[BREF]], ptr %[[CP_BREF]], align 8
; CHECK: call void @__kmpc_copyprivate(ptr @{{.*}}, i32 %{{.*}}, i32 8, ptr %[[CP_STRUCT]], ptr @[[CPRIV_CALLBACK:_Z3foov_copy_priv.*]], i32 %{{.*}})
; CHECK: define internal void @[[CPRIV_CALLBACK]](ptr %[[STRUCT_DST:.*]], ptr %[[STRUCT_SRC:.*]])
; CHECK: %[[SRC_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_SRC]], i32 0, i32 0
; CHECK: %[[DST_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_DST]], i32 0, i32 0
; CHECK: %[[SRC:.*]] = load ptr, ptr %[[SRC_GEP]], align 8
; CHECK: %[[DST:.*]] = load ptr, ptr %[[DST_GEP]], align 8
; CHECK: %[[SRC_VAL:.*]] = load i32, ptr %[[SRC]], align 4
; CHECK: store i32 %[[SRC_VAL]], ptr %[[DST]], align 4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %a = alloca i32, align 4
  %b = alloca ptr, align 8
  store i32 10, ptr %a, align 4
  store ptr %a, ptr %b, align 8
  %0 = load ptr, ptr %b, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.COPYPRIVATE:TYPED"(ptr %0, i32 0, i32 1) ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SINGLE"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
