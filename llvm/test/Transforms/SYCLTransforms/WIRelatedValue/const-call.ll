; RUN: opt '-passes=print<sycl-kernel-barrier-wi-analysis>' -disable-output %s 2>&1 | FileCheck %s

declare i64 @foo(i64) #0
declare i64 @bar(i64)

declare void @_Z7barrierj(i32)
declare i64 @_Z13get_global_idj(i32)

define void @kernel(i64 %x) {
; CHECK: %gid is WI related
; CHECK: %r0 is not WI related
; CHECK: %r1 is WI related
; CHECK: %r2 is not WI related
  %gid = call i64 @_Z13get_global_idj(i32 0)
  %r0 = call i64 @foo(i64 %x) #0
  %r1 = call i64 @foo(i64 %gid) #0
  %r2 = call i64 @bar(i64 %x)
  call void @_Z7barrierj(i32 2)
  ret void
}

attributes #0 = { readnone }
