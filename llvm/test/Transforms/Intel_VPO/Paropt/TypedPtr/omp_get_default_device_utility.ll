; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;#include <stdio.h>
;
; int main() {
;   int a[10];
;   int *array_device = &a[0];
; #pragma omp target data use_device_ptr(array_device)
;     {
;      printf("%p\n", &array_device[0]);
;     }
; }
;
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1
@"@tid.addr" = external global i32
@.offload_sizes = private unnamed_addr constant [2 x i64] zeroinitializer
@.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 96, i64 96]
@.source.0.0 = private constant [22 x i8] c";unknown;unknown;0;0;;"
@.source.0.0.1 = private constant [22 x i8] c";unknown;unknown;0;0;;"
@"@bid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %array_device.new = alloca i32*, align 8
  %a = alloca [10 x i32], align 16
  %array_device = alloca i32*, align 8
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %a, i64 0, i64 0
  store i32* %arrayidx, i32** %array_device, align 8
  %0 = load i32*, i32** %array_device, align 8
  br label %DIR.OMP.TARGET.DATA.1

DIR.OMP.TARGET.DATA.1:                            ; preds = %entry
  br label %DIR.OMP.TARGET.DATA.1.split

DIR.OMP.TARGET.DATA.1.split:                      ; preds = %DIR.OMP.TARGET.DATA.1
  %array_device.load = load i32*, i32** %array_device, align 8
  br label %codeRepl

codeRepl:                                         ; preds = %DIR.OMP.TARGET.DATA.1.split
  %.run_host_version = alloca i32, align 4
  %.offload_baseptrs = alloca [2 x i8*], align 8
  %.offload_ptrs = alloca [2 x i8*], align 8
  %1 = bitcast i32* %0 to i8*
  %2 = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
  store i8* %1, i8** %2, align 8
  %3 = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_ptrs, i32 0, i32 0
  %4 = bitcast i32* %0 to i8*
  store i8* %4, i8** %3, align 8
  %5 = bitcast i32* %array_device.load to i8*
  %6 = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 1
  store i8* %5, i8** %6, align 8
  %7 = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_ptrs, i32 0, i32 1
  %8 = bitcast i32* %array_device.load to i8*
  store i8* %8, i8** %7, align 8
  %9 = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
  %10 = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_ptrs, i32 0, i32 0
; check that we get DeviceNum by calling @omp_get_default_device()
; CHECK: [[DeviceNum:%[^ ]+]] = call i32 @omp_get_default_device()
; check that DeviceNum is zero extended
; CHECK-NEXT: [[DeviceNumExt:%[^ ]+]] = zext i32 [[DeviceNum]] to i64
; check that zero extended DeviceNum is the first argument of @__tgt_target_data_begin
; CHECK: call void @__tgt_target_data_begin(i64 [[DeviceNumExt]], i32 2, i8** %{{.*}}, i8** %{{.*}}, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_sizes, i32 0, i32 0), i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes, i32 0, i32 0))
  %11 = call i32 @omp_get_default_device()
  %12 = zext i32 %11 to i64
  call void @__tgt_push_code_location(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0, i32 0, i32 0), i8* bitcast (void (i64, i32, i8**, i8**, i64*, i64*)* @__tgt_target_data_begin to i8*))
  call void @__tgt_target_data_begin(i64 %12, i32 2, i8** %9, i8** %10, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_sizes, i32 0, i32 0), i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes, i32 0, i32 0))
  %.cast = bitcast i8** %6 to i32**
  %array_device.updated.val = load i32*, i32** %.cast, align 8
  store i32* %array_device.updated.val, i32** %array_device.new, align 8
  call void @main.DIR.OMP.TARGET.DATA.1.split.split(i32** %array_device.new, i32* %0)
  %13 = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
  %14 = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_ptrs, i32 0, i32 0
; check that we get DeviceNum by calling @omp_get_default_device()
; CHECK: [[DeviceNum2:%[^ ]+]] = call i32 @omp_get_default_device()
; check that DeviceNum is zero extended
; CHECK-NEXT: [[DeviceNumExt2:%[^ ]+]] = zext i32 [[DeviceNum2]] to i64
; check that zero extended DeviceNum is the first argument of @__tgt_target_data_end
; CHECK: call void @__tgt_target_data_end(i64 [[DeviceNumExt2]], i32 2, i8** %{{.*}}, i8** %{{.*}}, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_sizes, i32 0, i32 0), i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes, i32 0, i32 0))
  %15 = call i32 @omp_get_default_device()
  %16 = zext i32 %15 to i64
  call void @__tgt_push_code_location(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.1, i32 0, i32 0), i8* bitcast (void (i64, i32, i8**, i8**, i64*, i64*)* @__tgt_target_data_end to i8*))
  call void @__tgt_target_data_end(i64 %16, i32 2, i8** %13, i8** %14, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_sizes, i32 0, i32 0), i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes, i32 0, i32 0))
  br label %DIR.OMP.END.TARGET.DATA.4

DIR.OMP.END.TARGET.DATA.4:                        ; preds = %codeRepl
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @main.DIR.OMP.TARGET.DATA.1.split.split(i32** %array_device, i32* %0) #3 {
newFuncRoot:
  br label %DIR.OMP.TARGET.DATA.1.split.split

DIR.OMP.END.TARGET.DATA.4.exitStub:               ; preds = %DIR.OMP.END.TARGET.DATA.3
  ret void

DIR.OMP.TARGET.DATA.1.split.split:                ; preds = %newFuncRoot
  br label %DIR.OMP.TARGET.DATA.22

DIR.OMP.TARGET.DATA.22:                           ; preds = %DIR.OMP.TARGET.DATA.1.split.split
  br label %DIR.OMP.TARGET.DATA.2

DIR.OMP.TARGET.DATA.2:                            ; preds = %DIR.OMP.TARGET.DATA.22
  %1 = load i32*, i32** %array_device, align 8
  %ptridx = getelementptr inbounds i32, i32* %1, i64 0
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* %ptridx) #1
  br label %DIR.OMP.END.TARGET.DATA.3.split

DIR.OMP.END.TARGET.DATA.3.split:                  ; preds = %DIR.OMP.TARGET.DATA.2
  br label %DIR.OMP.END.TARGET.DATA.3

DIR.OMP.END.TARGET.DATA.3:                        ; preds = %DIR.OMP.END.TARGET.DATA.3.split
  br label %DIR.OMP.END.TARGET.DATA.4.exitStub
}

declare i32 @omp_get_default_device()

declare void @__tgt_target_data_begin(i64, i32, i8**, i8**, i64*, i64*)

declare void @__tgt_push_code_location(i8*, i8*)

declare void @__tgt_target_data_end(i64, i32, i8**, i8**, i64*, i64*)

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "target.declare"="true" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}

; __CLANG_OFFLOAD_BUNDLE____END__ host-x86_64-unknown-linux-gnu
