; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; typedef short TYPE;
;
; static TYPE x[10];
; static TYPE y;
;
; typedef struct my_struct {
;   TYPE &yref;
;   void work();
;   my_struct(TYPE &y) : yref(y){};
; } MY_STRUCT_TYPE;
;
; void MY_STRUCT_TYPE::work() {
;
; #pragma omp parallel private(yref)
;   { yref = x[1]; }
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { ptr }

@_ZL1x = internal global [10 x i16] zeroinitializer, align 16

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_ZN9my_struct4workEv(ptr noundef nonnull align 8 dereferenceable(8) %this) #0 align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %yref = getelementptr inbounds %struct.my_struct, ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %yref, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %0, i16 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @_ZL1x, i16 0, i64 10) ]

; Check that a local copy is created for %0, and the store of the gep on
; @_ZL1x happens to that local copy.
; CHECK: [[PRIV_COPY:%[^ ]+]] = alloca i16
; CHECK: [[X_GEP_LOAD:%[^ ]+]] = load i16, ptr getelementptr inbounds ([10 x i16], ptr @_ZL1x, i64 0, i64 1)
; CHECK: store i16 [[X_GEP_LOAD]], ptr [[PRIV_COPY]]
  %2 = load i16, ptr getelementptr inbounds ([10 x i16], ptr @_ZL1x, i64 0, i64 1), align 2
  store i16 %2, ptr %0, align 2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
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
