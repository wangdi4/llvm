; RUN: opt '-passes=print<sycl-kernel-barrier-wi-analysis>' -disable-output %s 2>&1 | FileCheck %s

declare void @_Z7barrierj(i32)
declare i64 @_Z13get_global_idj(i32)

define void @kernel(i64 %x) {
; CHECK: %gid is WI related
; CHECK: %gid.fr is WI related
  %gid = call i64 @_Z13get_global_idj(i32 0)
  %gid.fr = freeze i64 %gid
  call void @_Z7barrierj(i32 2)
  ret void
}
