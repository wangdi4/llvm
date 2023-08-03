; RUN: opt %s -passes=sycl-kernel-indirect-call-lowering -S | FileCheck %s
; RUN: opt %s -passes=sycl-kernel-indirect-call-lowering -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; __intel_create_simd_variant should be resolved on VectorVariantFillIn.
; If __intel_create_simd_variant call still exists because Intel_VectorVariant passes
; didn't run, we need replace __intel_create_simd_variant with its first
; parameter.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.funcPtrArr = type { ptr }

; Function Attrs: nounwind
define double @_Z3addii(i32 %A, i32 %B) #0 {
entry:
  %add = add nsw i32 %A, %B
  %conv = sitofp i32 %add to double
  ret double %conv
}

; Function Attrs: nounwind
define void @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E1K() #1 {
entry:
  %ref.tmp = alloca %struct.funcPtrArr, align 8
  %arrayinit.begin = getelementptr inbounds %struct.funcPtrArr, ptr %ref.tmp, i64 0, i32 0
  %0 = call ptr @__intel_create_simd_variant.4(ptr @_Z3addii) #2
  store ptr %0, ptr %arrayinit.begin, align 8
; CHECK: store ptr @_Z3addii, ptr %arrayinit.begin, align 8
  %1 = addrspacecast ptr %arrayinit.begin to ptr addrspace(4)
  %2 = call double (ptr addrspace(4), i32, i32, ...) @__intel_indirect_call.8(ptr addrspace(4) %1, i32 1, i32 1) #3
; CHECK: [[GEP:%[0-9]+]] = getelementptr ptr addrspace(4), ptr addrspace(4) {{.*}}, i32 0
; CHECK-NEXT: [[LOAD:%[0-9]+]] = load ptr addrspace(4), ptr addrspace(4) [[GEP]], align 8
; CHECK-NEXT: call addrspace(4) double [[LOAD]](i32 1, i32 1)
  ret void
}

declare ptr @__intel_create_simd_variant.4(ptr)

declare double @__intel_indirect_call.8(ptr addrspace(4), i32, i32, ...)

attributes #0 = { nounwind "referenced-indirectly" }
attributes #1 = { nounwind }
attributes #2 = { "vector-variants"="_ZGV_unknown_M4vv__Z3addii" }
attributes #3 = { "vector-variants"="_ZGV_unknown_M4vv__$U0,_ZGV_unknown_N4uu__$U0,_ZGV_unknown_M8vv__$U0,_ZGV_unknown_N8uu__$U0" }

; DEBUGIFY: WARNING: Missing line 6
; DEBUGIFY-NOT: WARNING
