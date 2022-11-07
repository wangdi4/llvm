; RUN: opt -dpcpp-kernel-barrier-wi-analysis -analyze -enable-new-pm=0 %s | FileCheck %s
; RUN: opt '-passes=print<dpcpp-kernel-barrier-wi-analysis>' -disable-output %s 2>&1 | FileCheck %s

declare i64 @bar(i64)
declare i64 @foo(i64)

declare void @_Z7barrierj(i32)
declare i64 @_Z13get_global_idj(i32)

define void @kernel(i64 %x) {
; CHECK: %c0 is not WI related
; CHECK: %f0 is not WI related
; CHECK: %r0 is WI related
; CHECK: %r1 is not WI related
; CHECK: %gid is WI related
; CHECK: %r2 is WI related
; CHECK: %c1 is WI related
; CHECK: %f1 is WI related
; CHECK: %r3 is WI related
  %c0 = icmp eq i64 %x, 0
  %f0 = select i1 %c0, i64 (i64)* @bar, i64 (i64)* @foo
  %r0 = call i64 (i64) %f0(i64 %x)
  %r1 = call i64 (i64) %f0(i64 %x) #0
  %gid = call i64 @_Z13get_global_idj(i32 0)
  %r2 = call i64 (i64) %f0(i64 %gid) #0
  %c1 = icmp ne i64 %gid, 0
  %f1 = select i1 %c1, i64 (i64)* @bar, i64 (i64)* @foo
  %r3 = call i64 (i64) %f1(i64 %x) #0
  call void @_Z7barrierj(i32 2)
  ret void
}

attributes #0 = { memory(none) }
