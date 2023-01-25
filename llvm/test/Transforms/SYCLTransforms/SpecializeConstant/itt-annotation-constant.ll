; RUN: opt -passes=sycl-kernel-spec-constant -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-spec-constant -S < %s | FileCheck %s

define weak dso_local spir_func void @__itt_offload_wi_start_wrapper() {
entry:
; CHECK-NOT: __spirv_SpecConstant
; CHECK: %cmp.i.not = icmp eq i8 0, 0
  %call.i = tail call spir_func signext i8 @__spirv_SpecConstant(i32 noundef -9145239, i8 noundef signext 0)
  %cmp.i.not = icmp eq i8 %call.i, 0
  br i1 %cmp.i.not, label %return, label %if.end

if.end:                                           ; preds = %entry
  br label %return

return:                                           ; preds = %if.end, %entry
  ret void
}

declare extern_weak dso_local spir_func signext i8 @__spirv_SpecConstant(i32 noundef, i8 noundef signext)

; DEBUGIFY-NOT: WARNING
