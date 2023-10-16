; RUN: opt -S -sycl-kernel-builtin-lib=%p/work-group-builtins-64.rtl -passes=sycl-kernel-group-builtin %s | FileCheck %s
; RUN: opt -S -sycl-kernel-builtin-lib=%p/work-group-builtins-64.rtl -passes=sycl-kernel-group-builtin %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

declare <4 x i32> @_Z22__work_group_broadcastDv4_imS_(<4 x i32> %src, i64 %local_id, <4 x i32> %mask)

define void @test() {
; CHECK: call <4 x i32> @_Z22__work_group_broadcastDv4_immS_PS_(<4 x i32> zeroinitializer, i64 0, i64 %WIcall, <4 x i32> zeroinitializer, ptr %AllocaWGResult)
  %1 = call <4 x i32> @_Z22__work_group_broadcastDv4_imS_(<4 x i32> zeroinitializer, i64 0, <4 x i32> zeroinitializer)
  ret void
}

;; Instructions inserted by GroupBuiltin should not have debug info
; DEBUGIFY-COUNT-6: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
