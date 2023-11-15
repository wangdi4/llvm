; Test that we don't crash on calls to @llvm.intel.directive.elementsize
; when the parameter is obscured by an addrspacecast.  Extracted from
; test omp-org_gpuF/declare_target.5.

; RUN: opt -passes=vec-clone -S < %s 2>&1 | FileCheck %s

; CHECK: define spir_func float @my_global_array_mp_p_
; CHECK: define spir_func <8 x float> @_ZGVdN8Lu_my_global_array_mp_p_
; CHECK: define spir_func <4 x float> @_ZGVbN4Lu_my_global_array_mp_p_
; CHECK: define spir_func <8 x float> @_ZGVcN8Lu_my_global_array_mp_p_
; CHECK: define spir_func <16 x float> @_ZGVeN16Lu_my_global_array_mp_p_

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@q = common target_declare addrspace(1) global [10000 x [10000 x float]] zeroinitializer, align 8

define spir_func float @my_global_array_mp_p_(ptr addrspace(4) dereferenceable(4) %K, ptr addrspace(4) dereferenceable(4) %I) #1 {
alloca_1:
  %"ascast$K" = addrspacecast ptr addrspace(4) %K to ptr
  call spir_func void @llvm.intel.directive.elementsize(ptr %"ascast$K", i64 4)
  %"ascast$I" = addrspacecast ptr addrspace(4) %I to ptr
  call spir_func void @llvm.intel.directive.elementsize(ptr %"ascast$I", i64 4)
  %K_fetch.1 = load i32, ptr addrspace(4) %K, align 1
  %int_sext = sext i32 %K_fetch.1 to i64
  %I_fetch.2 = load i32, ptr addrspace(4) %I, align 1
  %int_sext1 = sext i32 %I_fetch.2 to i64
  %0 = sub nsw i64 %int_sext1, 1
  %1 = mul nsw i64 10000, %0
  %2 = getelementptr inbounds float, ptr addrspace(1) @q, i64 %1
  %3 = sub nsw i64 %int_sext, 1
  %4 = getelementptr inbounds float, ptr addrspace(1) %2, i64 %3
  %5 = load float, ptr addrspace(1) %4, align 4
  ret float %5
}

declare void @llvm.intel.directive.elementsize(ptr, i64 immarg)

attributes #1 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "openmp-target-declare"="true" "vector-variants"="_ZGVdN8Lu_my_global_array_mp_p_,_ZGVbN4Lu_my_global_array_mp_p_,_ZGVcN8Lu_my_global_array_mp_p_,_ZGVeN16Lu_my_global_array_mp_p_" }
