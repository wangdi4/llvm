; RUN: opt -passes="sycl-kernel-analysis,sycl-kernel-wg-loop-bound" %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes="sycl-kernel-analysis,sycl-kernel-wg-loop-bound" %s -S -debug -disable-output 2>&1 | FileCheck %s

; CHECK: WGLoopBoundaries
; CHECK: found 2 early exit boundaries
;; boundary output pattern: dim, contains_val(T/F), is_global_id(T/F), IsSigned(T/F), IsUpperBound(T,F)
; CHECK: Dim=0, Contains=T, IsGID=T, IsSigned=T, IsUpperBound=F
; CHECK-SAME: i32 1
; CHECK: Dim=1, Contains=T, IsGID=T, IsSigned=T, IsUpperBound=F
; CHECK-SAME: i32 1
; CHECK: found 0 uniform early exit conditions

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

declare i32 @_Z13get_global_idj(i32) nounwind readnone


define void @cmp_select_max_kernel(ptr addrspace(1) nocapture %dst, i32 %width, i32 %height) nounwind {
  %id0 = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %id1 = tail call i32 @_Z13get_global_idj(i32 1) nounwind readnone
  %cmp0 = icmp slt i32 %id0, 1
  %select0 = select i1 %cmp0, i32 1, i32 %id0
  %cmp1 = icmp sgt i32 %id1, 1
  %select1 = select i1 %cmp1, i32 %id1, i32 1
  %mul = mul nsw i32 %select1, %width
  %ind = add nsw i32 %mul, %select0
  %ptr = getelementptr inbounds <4 x i8>, ptr addrspace(1) %dst, i32 %ind
  store <4 x i8> <i8 100, i8 100, i8 100, i8 100>, ptr addrspace(1) %ptr, align 4
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @cmp_select_max_kernel}

; DEBUGIFY-COUNT-27: Instruction with empty DebugLoc in function WG.boundaries.
; DEBUGIFY-COUNT-4: Missing line
; DEBUGIFY-NOT: WARNING
