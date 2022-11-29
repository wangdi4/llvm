; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-enable-direct-function-call-vectorization=true -dpcpp-enable-direct-subgroup-function-call-vectorization=true -dpcpp-vector-variant-isa-encoding-override=SSE42 -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-enable-direct-function-call-vectorization=true -dpcpp-enable-direct-subgroup-function-call-vectorization=true -dpcpp-vector-variant-isa-encoding-override=SSE42 -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-enable-direct-function-call-vectorization=true -dpcpp-enable-direct-subgroup-function-call-vectorization=true -dpcpp-vector-variant-isa-encoding-override=SSE42 -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-enable-direct-function-call-vectorization=true -dpcpp-enable-direct-subgroup-function-call-vectorization=true -dpcpp-vector-variant-isa-encoding-override=SSE42 -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

;; Widened inner functions must have "widened-size" attribute so Barrier inserts
;; correct loop increments.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define fastcc i64 @foo() #0 {
entry:
  tail call void @_Z7barrierj(i32 1)
  %call = tail call i64 @_Z12get_local_idj(i32 0)
  ret i64 %call
}

declare void @_Z7barrierj(i32)

declare i64 @_Z12get_local_idj(i32)

declare i32 @_Z22get_sub_group_local_idv()

define void @basic(i64 addrspace(1)* %local_id, i64 addrspace(1)* %sg_local_id) #4 !recommended_vector_length !1 {
entry:
  %call = tail call fastcc i64 @foo()
  %arrayidx = getelementptr inbounds i64, i64 addrspace(1)* %local_id, i64 %call
  store i64 %call, i64 addrspace(1)* %arrayidx, align 8
  %call.i = tail call i32 @_Z22get_sub_group_local_idv()
  %conv.i = zext i32 %call.i to i64
  %arrayidx2 = getelementptr inbounds i64, i64 addrspace(1)* %sg_local_id, i64 %call
  store i64 %conv.i, i64 addrspace(1)* %arrayidx2, align 8
  ret void
}

; CHECK: define fastcc <4 x i64> @_ZGVeM4_foo(<4 x i64> %mask) #2 {
; CHECK: attributes #2 = { memory(readwrite) "may-have-openmp-directive"="true" "vector-variants"="_ZGVeM4_foo,_ZGVeN4_foo" "widened-size"="4" }

attributes #0 = { "vector-variants"="_ZGVeM4_foo,_ZGVeN4_foo" }

!sycl.kernels = !{!0}

!0 = !{void (i64 addrspace(1)*, i64 addrspace(1)*)* @basic}
!1 = !{i32 4}

; DEBUGIFY:      WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} trunc
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} sext
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} getelementptr
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM4_foo {{.*}} ret
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_foo {{.*}} ret
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4uu_basic {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4uu_basic {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4uu_basic {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4uu_basic {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4uu_basic {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4uu_basic {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4uu_basic {{.*}} br
; DEBUGIFY-NEXT: CheckModuleDebugify: PASS
