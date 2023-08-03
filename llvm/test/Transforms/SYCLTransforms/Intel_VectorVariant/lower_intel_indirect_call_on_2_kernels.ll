; RUN: opt %s -passes=sycl-kernel-indirect-call-lowering -S | FileCheck %s
; RUN: opt %s -passes=sycl-kernel-indirect-call-lowering -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

@"_Z3addii$SIMDTable" = global [2 x ptr] [ptr @_Z3addii, ptr @_Z3func], align 8
%"class.sycl::_V1::INTEL::function_ref_tuned" = type { ptr }

; Function Attrs: nounwind
define spir_func i32 @_Z3addii(i32 %A, i32 %B) #0 {
entry:
  %add = add nsw i32 %A, %B
  ret i32 %add
}

; Function Attrs: nounwind
define spir_func void @_Z3func() #0 {
entry:
  ret void
}

; __intel_indirect_call w/o vector-variants or w/ unknown ISA vector-variants should not be vectorized.
; Function Attrs: nounwind
define spir_kernel void @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E1K() #1 {
entry:
  %0 = getelementptr inbounds [1 x ptr], ptr @"_Z3addii$SIMDTable", i64 0, i64 0
  %1 = addrspacecast ptr %0 to ptr addrspace(4)
  %2 = call spir_func i32 (ptr addrspace(4), i32, i32, ...) @__intel_indirect_call.1(ptr addrspace(4) %1, i32 2, i32 1) #2
; CHECK: [[GEP:%[0-9]+]] = getelementptr ptr addrspace(4), ptr addrspace(4) %1, i32 0
; CHECK-NEXT: [[LOAD:%[0-9]+]] = load ptr addrspace(4), ptr addrspace(4) [[GEP]], align 8
; CHECK-NEXT: call addrspace(4) i32 [[LOAD]](i32 2, i32 1)
  %3 = getelementptr inbounds [1 x ptr], ptr @"_Z3addii$SIMDTable", i64 0, i64 1
  %4 = addrspacecast ptr %3 to ptr addrspace(4)
  call spir_func void (ptr addrspace(4), ...) @__intel_indirect_call.2(ptr addrspace(4) %4)
; CHECK: [[GEP1:%[0-9]+]] = getelementptr ptr addrspace(4), ptr addrspace(4) {{.*}}, i32 0
; CHECK-NEXT: [[LOAD1:%[0-9]+]] = load ptr addrspace(4), ptr addrspace(4) [[GEP1]], align 8
; CHECK-NEXT: call addrspace(4) void [[LOAD1]]()
  ret void
}

; Only lower vectorized __intel_indirect_call to vector call.
; Function Attrs: nounwind memory(readwrite)
define void @_ZTSZZ6testO2vENKUlRN4sycl3_V17handlerEE_clES2_E2K2() local_unnamed_addr #3 {
entry:
  %func.i = alloca %"class.sycl::_V1::INTEL::function_ref_tuned", align 8
  %ptrs.i.i.i.i = getelementptr inbounds %"class.sycl::_V1::INTEL::function_ref_tuned", ptr %func.i, i64 0, i32 0
  %0 = addrspacecast ptr %func.i to ptr addrspace(4)
  store ptr @_ZGVeM4vv__Z3subii, ptr %ptrs.i.i.i.i, align 8
  call double (ptr addrspace(4), i32, i32, ...) @__intel_indirect_call.3(ptr addrspace(4) %0, i32 1, i32 2) #4
; CHECK: [[GEP1:%[0-9]+]] = getelementptr ptr addrspace(4), ptr addrspace(4) {{.*}}, i32 0
; CHECK-NEXT: [[LOAD1:%[0-9]+]] = load ptr addrspace(4), ptr addrspace(4) [[GEP1]], align 8
; CHECK-NEXT: [[VEC1:%[0-9]+]] = call addrspace(4) <4 x double> [[LOAD1]](<4 x i32> <i32 1, i32 undef, i32 undef, i32 undef>, <4 x i32> <i32 2, i32 undef, i32 undef, i32 undef>, <4 x double> <double 0xFFFFFFFFFFFFFFFF, double 0.000000e+00, double 0.000000e+00, double 0.000000e+00>)
; CHECK-NEXT: extractelement <4 x double> [[VEC1]], i32 0
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite)
define <4 x double> @_ZGVeM4vv__Z3subii(<4 x i32> %A, <4 x i32> %B, <4 x double> %mask) #5 {
entry:
  %vec.retval = alloca <4 x double>, align 32
  %0 = fcmp une <4 x double> %mask, zeroinitializer
  %1 = sub nsw <4 x i32> %A, %B
  %2 = sitofp <4 x i32> %1 to <4 x double>
  call void @llvm.masked.store.v4f64.p0(<4 x double> %2, ptr %vec.retval, i32 32, <4 x i1> %0)
  %vec.ret = load <4 x double>, ptr %vec.retval, align 32
  ret <4 x double> %vec.ret
}

declare i32 @__intel_indirect_call.1(ptr addrspace(4), i32, i32, ...)
declare void @__intel_indirect_call.2(ptr addrspace(4), ...)
declare double @__intel_indirect_call.3(ptr addrspace(4), i32, i32, ...) local_unnamed_addr
; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: write)
declare void @llvm.masked.store.v4f64.p0(<4 x double>, ptr nocapture, i32 immarg, <4 x i1>)

attributes #0 = { nounwind "referenced-indirectly" "vector_function_ptrs"="_Z3addii$SIMDTable()" }
attributes #1 = { nounwind }
attributes #2 = { "vector-variants"="_ZGV_unknown_M4vv__$U0,_ZGV_unknown_N4uu__$U0,_ZGV_unknown_M8vv__$U0,_ZGV_unknown_N8uu__$U0" }
attributes #3 = { nounwind memory(readwrite) "may-have-openmp-directive"="false" "prefer-vector-width"="512" }
attributes #4 = { nounwind "vector-variants"="_ZGVeM4vv__$U0,_ZGVeN4uu__$U0,_ZGVeM8vv__$U0,_ZGVeN8uu__$U0" }
attributes #5 = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite) "may-have-openmp-directive"="false" "prefer-vector-width"="512" "referenced-indirectly" "widened-size"="4" }

; DEBUGIFY-NOT: WARNING
