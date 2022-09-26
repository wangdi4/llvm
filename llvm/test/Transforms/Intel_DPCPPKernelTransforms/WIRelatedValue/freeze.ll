; RUN: opt -dpcpp-kernel-barrier-wi-analysis -analyze -enable-new-pm=0 %s | FileCheck %s
; RUN: opt '-passes=print<dpcpp-kernel-barrier-wi-analysis>' -disable-output %s 2>&1 | FileCheck %s

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
