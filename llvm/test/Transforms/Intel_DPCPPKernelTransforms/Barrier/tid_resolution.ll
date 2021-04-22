; Checks barrier pass resolves get_local_id and get_global_id correctly.
; RUN: opt -passes=dpcpp-kernel-barrier %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK-LABEL: define void @main
define void @main(i32 %x) #0 {
entry:
  call void @barrier_dummy()
;CHECK: %base_gid = call i32 @get_base_global_id.(i32 0)
;CHECK-NEXT: [[LID:%LocalId_[0-9]*]] = load i32, i32* %pLocalId_0, align 4
;CHECK-NEXT: {{%gid[0-9]*}} = add i32 [[LID]], %base_gid
;CHECK-NEXT: {{%LocalId_[0-9]*}} = load i32, i32* %pLocalId_0, align 4
;CHECK-NEXT: call void @foo([3 x i32]* %pLocalIds)
  %gid = call i32 @_Z13get_global_idj(i32 0)
  %lid = call i32 @_Z12get_local_idj(i32 0)
  call void @foo()
  br label %L
L:
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

; CHECK-LABEL: define void @foo
define void @foo() {
entry:
;CHECK: %pLocalId_0 = getelementptr inbounds [3 x i32], [3 x i32]* %pLocalIdValues, i32 0, i32 0
;CHECK-NEXT: %base_gid = call i32 @get_base_global_id.(i32 0)
;CHECK-NEXT: [[LID:%LocalId_[0-9]*]] = load i32, i32* %pLocalId_0, align 4
;CHECK-NEXT: {{%gid[0-9]*}} = add i32 [[LID]], %base_gid
;CHECK-NEXT: {{%LocalId_[0-9]*}} = load i32, i32* %pLocalId_0, align 4
  %gid = call i32 @_Z13get_global_idj(i32 0)
  %lid = call i32 @_Z12get_local_idj(i32 0)
  ret void
}

declare void @_Z18work_group_barrierj(i32)
declare i32 @_Z12get_local_idj(i32)
declare i32 @_Z13get_global_idj(i32)
declare void @barrier_dummy()

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" }
