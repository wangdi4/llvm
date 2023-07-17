; RUN: opt --passes=sycl-kernel-sg-remap-wi-call -sycl-sg-construction-mode=0 -S %s | FileCheck %s
; RUN: opt --passes=sycl-kernel-sg-remap-wi-call -sycl-sg-construction-mode=0 -enable-debugify -disable-output -S %s 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Under default sg construction mode, the pass doesn't do anything.

define void @gid() {
; CHECK-LABEL: @gid(
; CHECK-NOT: !sg_construction_mode
entry:
  %gid0 = call i64 @_Z13get_global_idj(i32 0)
  %gid1 = call i64 @_Z13get_global_idj(i32 1)
  %gid2 = call i64 @_Z13get_global_idj(i32 2)
  ret void
}

define void @lid() {
; CHECK-LABEL: @lid(
; CHECK-NOT: !sg_construction_mode
entry:
  %lid0 = call i64 @_Z12get_local_idj(i32 0)
  %lid1 = call i64 @_Z12get_local_idj(i32 1)
  %lid2 = call i64 @_Z12get_local_idj(i32 2)
  ret void
}

define void @groupid() {
; CHECK-LABEL: @groupid(
; CHECK-NOT: !sg_construction_mode
entry:
  %groupid0 = call i64 @_Z12get_group_idj(i32 0)
  %groupid1 = call i64 @_Z12get_group_idj(i32 1)
  %groupid2 = call i64 @_Z12get_group_idj(i32 2)
  ret void
}

declare i64 @_Z13get_global_idj(i32 %x)
declare i64 @_Z12get_local_idj(i32 %x)
declare i64 @_Z12get_group_idj(i32 %x)

!sycl.kernels = !{!0}

!0 = !{ptr @gid, ptr @lid, ptr @groupid}

; DEBUGIFY-NOT: WARNING
