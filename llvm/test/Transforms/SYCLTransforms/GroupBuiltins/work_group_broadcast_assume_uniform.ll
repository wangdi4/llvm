; RUN: opt -S -sycl-kernel-builtin-lib=%p/work-group-builtins-64.rtl -passes=sycl-kernel-group-builtin %s | FileCheck %s
; RUN: opt -S -sycl-kernel-builtin-lib=%p/work-group-builtins-64.rtl -passes=sycl-kernel-group-builtin %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

declare <16 x i32> @_Z20work_group_broadcastDv16_jDv16_mS0_S0_(<16 x i32> %src, <16 x i64> %local_id_x, <16 x i64> %local_id_y, <16 x i64> %local_id_z)
declare <16 x i32> @_Z22__work_group_broadcastDv16_jDv16_mS0_S0_S_(<16 x i32> %src, <16 x i64> %local_id_x, <16 x i64> %local_id_y, <16 x i64> %local_id_z, <16 x i32> %mask)

define void @test(<16 x i64> %x, <16 x i64> %y, <16 x i64> %z) {
; CHECK: %x.assume.uniform = extractelement <16 x i64> %x, i32 0
; CHECK: %y.assume.uniform = extractelement <16 x i64> %y, i32 0
; CHECK: %z.assume.uniform = extractelement <16 x i64> %z, i32 0
; CHECK: call <16 x i32> @_Z20work_group_broadcastDv16_jmmPS_(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, ptr
  %call = call <16 x i32> @_Z20work_group_broadcastDv16_jDv16_mS0_S0_(<16 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>, <16 x i64> %x, <16 x i64> %y, <16 x i64> %z)
; CHECK: %x.assume.uniform{{.*}} = extractelement <16 x i64> %x, i32 0
; CHECK: %y.assume.uniform{{.*}} = extractelement <16 x i64> %y, i32 0
; CHECK: %z.assume.uniform{{.*}} = extractelement <16 x i64> %z, i32 0
; CHECK: call <16 x i32> @_Z22__work_group_broadcastDv16_jmmS_PS_(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}}, ptr
  %call.with.mask = call <16 x i32> @_Z22__work_group_broadcastDv16_jDv16_mS0_S0_S_(<16 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>, <16 x i64> %x, <16 x i64> %y, <16 x i64> %z, <16 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>)
  ret void
}

;; Instructions inserted by GroupBuiltin should not have debug info
; DEBUGIFY-COUNT-47: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
