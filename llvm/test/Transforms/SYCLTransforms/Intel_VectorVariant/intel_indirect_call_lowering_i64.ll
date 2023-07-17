; RUN: opt %s -passes=sycl-kernel-indirect-call-lowering -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=sycl-kernel-indirect-call-lowering -S | FileCheck %s

@"_Z2f5PPi$SIMDTable" = global [2 x ptr] [ptr @_ZGVeM16v__Z2f5PPi, ptr @_ZGVeN16v__Z2f5PPi]

declare <16 x ptr addrspace(4)> @_ZGVeM16v__Z2f5PPi(<16 x ptr addrspace(4)> readonly, <16 x i64>)

declare <16 x ptr addrspace(4)> @_ZGVeN16v__Z2f5PPi(<16 x ptr addrspace(4)> readonly)

define void @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_7nd_itemILi1EEEE_() {
  %1 = extractvalue [7 x i64] undef, 0
  %2 = trunc i64 %1 to i1
  ret void

vect_if:                                          ; No predecessors!
  %3 = icmp ne i64 undef, 0
  br label %scalar_if

entryvector_func:                                 ; preds = %entryvector_func
  %4 = extractelement <16 x ptr> undef, i32 0
  %5 = addrspacecast <16 x ptr> undef to <16 x ptr addrspace(4)>
  %6 = extractelement <16 x ptr> undef, i32 0
  %7 = addrspacecast <16 x ptr> undef to <16 x ptr addrspace(4)>
  %8 = load ptr addrspace(4), ptr addrspace(4) getelementptr ([2 x ptr], ptr addrspace(4) addrspacecast (ptr @"_Z2f5PPi$SIMDTable" to ptr addrspace(4)), i64 0, i64 1), align 8
  %9 = call addrspace(4) <16 x ptr addrspace(4)> %8(<16 x ptr addrspace(4)> %7)
  br label %entryvector_func

scalar_if:                                        ; preds = %vect_if
  %10 = icmp ne i64 undef, 0
  ret void

scalar_kernel_entry:                              ; preds = %scalar_kernel_entry
  %11 = addrspacecast ptr undef to ptr addrspace(4)
  %12 = addrspacecast ptr undef to ptr addrspace(4)
; CHECK: %13 = insertelement <16 x ptr addrspace(4)> undef, ptr addrspace(4) %12, i32 0
; CHECK-NEXT: %14 = load ptr addrspace(4), ptr addrspace(4) addrspacecast (ptr @"_Z2f5PPi$SIMDTable" to ptr addrspace(4)), align 8
; CHECK-NEXT: %15 = call addrspace(4) <16 x ptr addrspace(4)> %14(<16 x ptr addrspace(4)> %13, <16 x i64> <i64 -1, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0>)
; CHECK-NEXT: %16 = extractelement <16 x ptr addrspace(4)> %15, i32 0
; CHECK-NOT:  call {{.*}} @__intel_indirect_call
  %13 = call ptr addrspace(4) (ptr addrspace(4), ptr addrspace(4), ...) @__intel_indirect_call(ptr addrspace(4) addrspacecast (ptr @"_Z2f5PPi$SIMDTable" to ptr addrspace(4)), ptr addrspace(4) %12) #0
  br label %scalar_kernel_entry
}

declare ptr addrspace(4) @__intel_indirect_call(ptr addrspace(4), ptr addrspace(4), ...) local_unnamed_addr

attributes #0 = { "vector-variants"="_ZGVeM16v___intel_indirect_call_XXX,_ZGVeN16v___intel_indirect_call_XXX" }

; DEBUGIFY-NOT: WARNING
