; RUN: opt %s -passes=dpcpp-kernel-indirect-call-lowering -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=dpcpp-kernel-indirect-call-lowering -S | FileCheck %s

@"_Z2f5PPi$SIMDTable" = global [2 x i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)*] [i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)* bitcast (<16 x i32 addrspace(4)* addrspace(4)*> (<16 x i32 addrspace(4)* addrspace(4)*>, <16 x i64>)* @_ZGVeM16v__Z2f5PPi to i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)*), i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)* bitcast (<16 x i32 addrspace(4)* addrspace(4)*> (<16 x i32 addrspace(4)* addrspace(4)*>)* @_ZGVeN16v__Z2f5PPi to i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)*)]

declare <16 x i32 addrspace(4)* addrspace(4)*> @_ZGVeM16v__Z2f5PPi(<16 x i32 addrspace(4)* addrspace(4)*> readonly %res, <16 x i64> %mask)
declare <16 x i32 addrspace(4)* addrspace(4)*> @_ZGVeN16v__Z2f5PPi(<16 x i32 addrspace(4)* addrspace(4)*> readonly %res)

define void @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_7nd_itemILi1EEEE_() {
  %1 = extractvalue [7 x i64] undef, 0
  %2 = trunc i64 %1 to i1
  ret void

vect_if:
  %3 = icmp ne i64 undef, 0
  br label %scalar_if

entryvector_func:                                 ; preds = %entryvector_func
  %4 = extractelement <16 x i8*> undef, i32 0
  %5 = addrspacecast <16 x i32*> undef to <16 x i32 addrspace(4)*>
  %6 = extractelement <16 x i8*> undef, i32 0
  %7 = addrspacecast <16 x i32 addrspace(4)**> undef to <16 x i32 addrspace(4)* addrspace(4)*>
  %8 = load <16 x i32 addrspace(4)* addrspace(4)*> (<16 x i32 addrspace(4)* addrspace(4)*>) addrspace(4)*, <16 x i32 addrspace(4)* addrspace(4)*> (<16 x i32 addrspace(4)* addrspace(4)*>) addrspace(4)* addrspace(4)* bitcast (i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)* addrspace(4)* getelementptr ([2 x i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)*], [2 x i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)*] addrspace(4)* addrspacecast ([2 x i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)*]* @"_Z2f5PPi$SIMDTable" to [2 x i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)*] addrspace(4)*), i64 0, i64 1) to <16 x i32 addrspace(4)* addrspace(4)*> (<16 x i32 addrspace(4)* addrspace(4)*>) addrspace(4)* addrspace(4)*), align 8
  %9 = call addrspace(4) <16 x i32 addrspace(4)* addrspace(4)*> %8(<16 x i32 addrspace(4)* addrspace(4)*> %7)
  br label %entryvector_func

scalar_if:                                        ; preds = %vect_if
  %10 = icmp ne i64 undef, 0
  ret void

scalar_kernel_entry:                              ; preds = %scalar_kernel_entry
  %11 = addrspacecast i32* undef to i32 addrspace(4)*
  %12 = addrspacecast i32 addrspace(4)** undef to i32 addrspace(4)* addrspace(4)*
  %13 = bitcast i32* undef to i8*
  %14 = bitcast i32 addrspace(4)** undef to i8*
; CHECK:      %15 = insertelement <16 x i32 addrspace(4)* addrspace(4)*> undef, i32 addrspace(4)* addrspace(4)* %12, i32 0
; CHECK-NEXT: %16 = load <16 x i32 addrspace(4)* addrspace(4)*> (<16 x i32 addrspace(4)* addrspace(4)*>, <16 x i64>) addrspace(4)*, <16 x i32 addrspace(4)* addrspace(4)*> (<16 x i32 addrspace(4)* addrspace(4)*>, <16 x i64>) addrspace(4)* addrspace(4)* addrspacecast (<16 x i32 addrspace(4)* addrspace(4)*> (<16 x i32 addrspace(4)* addrspace(4)*>, <16 x i64>) addrspace(4)** bitcast ([2 x i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)*]* @"_Z2f5PPi$SIMDTable" to <16 x i32 addrspace(4)* addrspace(4)*> (<16 x i32 addrspace(4)* addrspace(4)*>, <16 x i64>) addrspace(4)**) to <16 x i32 addrspace(4)* addrspace(4)*> (<16 x i32 addrspace(4)* addrspace(4)*>, <16 x i64>) addrspace(4)* addrspace(4)*), align 8
; CHECK-NEXT: %17 = call addrspace(4) <16 x i32 addrspace(4)* addrspace(4)*> %16(<16 x i32 addrspace(4)* addrspace(4)*> %15, <16 x i64> <i64 -1, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0>)
; CHECK-NEXT: %18 = extractelement <16 x i32 addrspace(4)* addrspace(4)*> %17, i32 0
; CHECK-NOT:  call {{.*}} @__intel_indirect_call
  %15 = call i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)* addrspace(4)*, i32 addrspace(4)* addrspace(4)*, ...) @__intel_indirect_call(i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)* addrspace(4)* addrspacecast (i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)** getelementptr inbounds ([2 x i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)*], [2 x i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)*]* @"_Z2f5PPi$SIMDTable", i64 0, i64 0) to i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)* addrspace(4)*), i32 addrspace(4)* addrspace(4)* %12) #1
  br label %scalar_kernel_entry
}

declare i32 addrspace(4)* addrspace(4)* @__intel_indirect_call(i32 addrspace(4)* addrspace(4)* (i32 addrspace(4)* addrspace(4)*)* addrspace(4)*, i32 addrspace(4)* addrspace(4)*, ...) local_unnamed_addr

attributes #1 = { "vector-variants"="_ZGVeM16v___intel_indirect_call_XXX,_ZGVeN16v___intel_indirect_call_XXX" }

; DEBUGIFY-NOT: WARNING
