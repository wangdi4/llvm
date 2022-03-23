; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; This test is similar to target_is_device_ptr_i32Ptr.ll, with the difference that the
; is_device_ptr operand is an i32** instead of an i32*, and the clause has a
; PTR_TO_PTR modifier.

; Test src:

; #include <stdio.h>
;
; int main() {
;   int a = 10;
;   int *ap = &a;
;
; //#pragma omp target data map(a) use_device_ptr(ap)
; #pragma omp target is_device_ptr(ap)
;   ap[0] = 100;
;
; //  printf("%d\n", a);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Check that the map created for is_device_ptr has the correct map-type (288) and size(8).
; CHECK: @.offload_sizes = private unnamed_addr constant [1 x i64] [i64 8]
; CHECK: @.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 288]

; Check that 'ap' is loaded, then mapped and passed through the outlined function's parameter list.
; CHECK: %ap.load = load i32*, i32** %ap, align 8
; CHECK: [[AP_LOAD_CAST:%[^ ]+]] = bitcast i32* %ap.load to i8*
; CHECK: [[GEP:%[^ ]+]] = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
; CHECK: store i8* [[AP_LOAD_CAST]], i8** [[GEP]], align 8
; CHECK: call i32 @__tgt_target({{.+}})
; CHECK: call void @__omp_offloading{{[^ ]*}}main{{[^ ]*}}(i32* %ap.load)

; Check that a private copy for %ap is created and used in the outlined function.
; CHECK-LABEL: define internal void @__omp_offloading{{[^ ]*}}main{{[^ ]*}}(i32* %ap.load)
; CHECK: %ap.priv = alloca i32*, align 8
; CHECK: store i32* %ap.load, i32** %ap.priv, align 8
; CHECK: [[AP_PRIV_LOAD:%[^ ]+]] = load i32*, i32** %ap.priv, align 8
; CHECK: [[AP_PRIV_LOAD_GEP:%[^ ]+]] = getelementptr inbounds i32, i32* [[AP_PRIV_LOAD]], i64 0
; CHECK: store i32 100, i32* [[AP_PRIV_LOAD_GEP]], align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %a = alloca i32, align 4
  %ap = alloca i32*, align 8
  store i32 10, i32* %a, align 4
  store i32* %a, i32** %ap, align 8

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(i32** %ap) ]

  %1 = load i32*, i32** %ap, align 8
  %ptridx = getelementptr inbounds i32, i32* %1, i64 0
  store i32 100, i32* %ptridx, align 4

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}

!0 = !{i32 0, i32 2055, i32 153697839, !"_Z4main", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
