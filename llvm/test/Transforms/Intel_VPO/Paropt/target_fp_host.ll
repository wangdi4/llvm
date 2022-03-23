; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=rtl -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=RTL
; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=rtl -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=RTL

; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=module -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=NOTRTL
; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=module -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=NOTRTL

; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=wilocal -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=NOTRTL
; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=wilocal -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=NOTRTL

; Test src:
; #include <stdio.h>
;
; int x = 0;
; void foo() {
; #pragma omp target firstprivate(x)
;   printf("%p\n", &x);
; }

; For "rtl" allocation mode, host maps "x" with TO|PRIVATE to let
; libomptarget handle the allocateion/initialization of the private copy.

; RTL:     @.offload_sizes = private unnamed_addr constant [1 x i64] [i64 4]
; RTL:     @.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 161]
; RTL:       %x = bitcast i8* bitcast (i32* @x to i8*) to i32*
; RTL:       [[X_CAST:%[^ ]+]] = bitcast i32* %x to i8*
; RTL:       [[BASEPTRS_GEP:%[^ ]+]] = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
; RTL:       store i8* [[X_CAST]], i8** [[BASEPTRS_GEP]], align 8
; RTL:       call void [[KERNEL:@__omp_offloading.*foo.*]](i32* %x)
;
; RTL:     define internal void [[KERNEL]](i32* %x)
; RTL:       %x.fpriv = alloca i32, align 1
; RTL:       [[X_VAL:[^ ]+]] = load i32, i32* %x, align 4
; RTL:       store i32 [[X_VAL]], i32* %x.fpriv, align 4
; RTL:       call {{.*}} @printf({{.*}}, i32* noundef %x.fpriv)


; For "module"/"wilocal" modes, the host passes the value of "x" as a literal
; to the kernel.

; NOTRTL:  @.offload_sizes = private unnamed_addr constant [1 x i64] zeroinitializer
; NOTRTL:  @.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 288]
; NOTRTL:    %x = bitcast i8* bitcast (i32* @x to i8*) to i32*
; NOTRTL:    %x.val = load i32, i32* %x, align 4
; NOTRTL:    [[X_VAL_CAST:%x.val.*]] = zext i32 %x.val to i64
; NOTRTL:    [[X_VAL_TO_PTR:%[^ ]+]] = inttoptr i64 %x.val.zext to i8*
; NOTRTL:    [[BASEPTRS_GEP:%[^ ]+]] = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
; NOTRTL:    store i8* [[X_VAL_TO_PTR]], i8** [[BASEPTRS_GEP]]
; NOTRTL:    call void [[KERNEL:@__omp_offloading.*foo.*]](i64 [[X_VAL_CAST]])
;
; NOTRTL:  define internal void [[KERNEL]](i64 [[X_VAL_PARM:[^ ]+]])
; NOTRTL:    %x.fpriv = alloca i32, align 1
; NOTRTL:    [[X_VAL_CAST:%[^ ]+]] = trunc i64 [[X_VAL_PARM]] to i32
; NOTRTL:    store i32 [[X_VAL_CAST]], i32* %x.fpriv, align 4
; NOTRTL:    call {{.*}} @printf({{.*}}, i32* noundef %x.fpriv)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@x = dso_local global i32 0, align 4
@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32* @x) ]

  %call = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* noundef @x) #1

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8* noundef, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { noinline nounwind uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 66313, i32 117992173, !"_Z3foo", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
