; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; int main() {
;   int a[10];
;   int *array_device = &a[0];
; #pragma omp target data use_device_ptr(array_device)
;     {
;      printf("%p\n", &array_device[0]);
;     }
; }

; check that we get DeviceNum by calling @omp_get_default_device()
; CHECK: [[DeviceNum:%[^ ]+]] = call i32 @omp_get_default_device()
; check that DeviceNum is zero extended
; CHECK-NEXT: [[DeviceNumExt:%[^ ]+]] = zext i32 [[DeviceNum]] to i64
; check that zero extended DeviceNum is the second argument of @__tgt_target_data_begin
; CHECK: call void @__tgt_target_data_begin_mapper(ptr @{{.*}}, i64 [[DeviceNumExt]], i32 2, ptr %{{.*}}, ptr %{{.*}}, ptr @.offload_sizes, ptr @.offload_maptypes, ptr null, ptr null)

; check that we get DeviceNum by calling @omp_get_default_device()
; CHECK: [[DeviceNum2:%[^ ]+]] = call i32 @omp_get_default_device()
; check that DeviceNum is zero extended
; CHECK-NEXT: [[DeviceNumExt2:%[^ ]+]] = zext i32 [[DeviceNum2]] to i64
; check that zero extended DeviceNum is the second argument of @__tgt_target_data_end
; CHECK: call void @__tgt_target_data_end_mapper(ptr @{{.*}}, i64 [[DeviceNumExt2]], i32 2, ptr %{{.*}}, ptr %{{.*}}, ptr @.offload_sizes, ptr @.offload_maptypes, ptr null, ptr null)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %a = alloca [10 x i32], align 16
  %array_device = alloca ptr, align 8
  %arrayidx = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 0
  store ptr %arrayidx, ptr %array_device, align 8
  %0 = load ptr, ptr %array_device, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(ptr %array_device),
    "QUAL.OMP.MAP.TOFROM"(ptr %0, ptr %0, i64 0, i64 64, ptr null, ptr null) ]
  %2 = load ptr, ptr %array_device, align 8
  %arrayidx1 = getelementptr inbounds i32, ptr %2, i64 0
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, ptr noundef %arrayidx1) #1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(ptr noundef, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
