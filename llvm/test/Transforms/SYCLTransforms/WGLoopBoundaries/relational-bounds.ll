; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S | FileCheck %s

; The test checks that instructions are placed in order after replacing GID
; calls with the corresponding boundaries.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @testKernel(ptr addrspace(1) noalias %src, ptr addrspace(1) noalias %dst, i64 %offset) local_unnamed_addr !no_barrier_path !1 {
entry:
  %gid0 = tail call i64 @_Z13get_global_idj(i32 0)
  %gid1 = tail call i64 @_Z13get_global_idj(i32 1)
  %gid2 = tail call i64 @_Z13get_global_idj(i32 2)
  %add0 = add i64 %gid0, 1
  %add1 = add i64 %gid1, 2
  %add2 = add i64 %gid2, 3
  %add3 = add i64 %add0, %offset
  %gsize0 = tail call i64 @_Z15get_global_sizej(i32 0)
  %sub0 = sub i64 %gsize0, 1
  %cmp0 = icmp eq i64 %gid0, %sub0
  br i1 %cmp0, label %land.lhs.true, label %cleanup

land.lhs.true:
  %add4 = add i64 %add1, %offset
  %gsize1 = tail call i64 @_Z15get_global_sizej(i32 1)
  %sub1 = sub i64 %gsize1, 1
  %cmp1 = icmp eq i64 %gid1, %sub1
  br i1 %cmp1, label %land.lhs.true7, label %cleanup

land.lhs.true7:
  %add5 = add i64 %add2, %offset
  %gsize2 = tail call i64 @_Z15get_global_sizej(i32 2)
  %sub2 = add i64 %gsize2, -1
  %cmp2 = icmp eq i64 %gid2, %sub2
  br i1 %cmp2, label %for.cond.preheader, label %cleanup

for.cond.preheader:
  ; Uses of sub{0..2} and add{3..5}
  %src.addr.0 = getelementptr i8, ptr addrspace(1) %src, i64 %sub0
  %dst.addr.0 = getelementptr i8, ptr addrspace(1) %dst, i64 %add3
  %load0 = load i8, ptr addrspace(1) %src.addr.0
  store i8 %load0, ptr addrspace(1) %dst.addr.0
  %src.addr.1 = getelementptr i8, ptr addrspace(1) %src, i64 %sub1
  %dst.addr.1 = getelementptr i8, ptr addrspace(1) %dst, i64 %add4
  %load1 = load i8, ptr addrspace(1) %src.addr.1
  store i8 %load1, ptr addrspace(1) %dst.addr.1
  %src.addr.2 = getelementptr i8, ptr addrspace(1) %src, i64 %sub2
  %dst.addr.2 = getelementptr i8, ptr addrspace(1) %dst, i64 %add5
  %load2 = load i8, ptr addrspace(1) %src.addr.2
  store i8 %load2, ptr addrspace(1) %dst.addr.2
  br label %cleanup

cleanup:
  ret void
}

; CHECK-LABEL: define void @testKernel
; CHECK-LABEL:  entry:
; CHECK:  %gsize0 = tail call i64 @_Z15get_global_sizej(i32 0)
; CHECK:  %sub0 = sub i64 %gsize0, 1
; CHECK:  %add0 = add i64 %sub0, 1
; CHECK:  %add3 = add i64 %add0, %offset
; CHECK:  %gsize1 = tail call i64 @_Z15get_global_sizej(i32 1)
; CHECK:  %sub1 = sub i64 %gsize1, 1
; CHECK:  %add1 = add i64 %sub1, 2
; CHECK:  %add4 = add i64 %add1, %offset
; CHECK:  %gsize2 = tail call i64 @_Z15get_global_sizej(i32 2)
; CHECK:  %sub2 = add i64 %gsize2, -1
; CHECK:  %add2 = add i64 %sub2, 3
; CHECK:  %add5 = add i64 %add2, %offset
; CHECK:  %src.addr.0 = getelementptr i8, ptr addrspace(1) %src, i64 %sub0
; CHECK:  %dst.addr.0 = getelementptr i8, ptr addrspace(1) %dst, i64 %add3
; CHECK:  %load0 = load i8, ptr addrspace(1) %src.addr.0
; CHECK:  store i8 %load0, ptr addrspace(1) %dst.addr.0
; CHECK:  %src.addr.1 = getelementptr i8, ptr addrspace(1) %src, i64 %sub1
; CHECK:  %dst.addr.1 = getelementptr i8, ptr addrspace(1) %dst, i64 %add4
; CHECK:  %load1 = load i8, ptr addrspace(1) %src.addr.1
; CHECK:  store i8 %load1, ptr addrspace(1) %dst.addr.1
; CHECK:  %src.addr.2 = getelementptr i8, ptr addrspace(1) %src, i64 %sub2
; CHECK:  %dst.addr.2 = getelementptr i8, ptr addrspace(1) %dst, i64 %add5
; CHECK:  %load2 = load i8, ptr addrspace(1) %src.addr.2
; CHECK:  store i8 %load2, ptr addrspace(1) %dst.addr.2
; CHECK:  br label %cleanup
; CHECK-LABEL: cleanup:
; CHECK:  ret void


declare i64 @_Z13get_global_idj(i32) local_unnamed_addr
declare i64 @_Z15get_global_sizej(i32) local_unnamed_addr

!sycl.kernels = !{!0}

!0 = !{ptr @testKernel}
!1 = !{i1 true}

; DEBUGIFY-COUNT-48: Instruction with empty DebugLoc in function WG.boundaries.
; DEBUGIFY-COUNT-6: Missing line
; DEBUGIFY-NOT: WARNING
