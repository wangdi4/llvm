; RUN: llvm-as %p/work-group-builtins-64.ll -o %t.work-group-builtins-64.ll.bc
; RUN: opt -dpcpp-kernel-builtin-lib %t.work-group-builtins-64.ll.bc -passes=dpcpp-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib %t.work-group-builtins-64.ll.bc -dpcpp-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib %t.work-group-builtins-64.ll.bc -passes=dpcpp-kernel-group-builtin -S < %s | FileCheck %s
; RUN: opt -dpcpp-kernel-builtin-lib %t.work-group-builtins-64.ll.bc -dpcpp-kernel-group-builtin -S < %s | FileCheck %s

;;*****************************************************************************
; This test checks the GroupBuiltin pass
;;  - The case is a kernel with a WG function call as first instruction.
;;  - It checks that a call to uniform WG function (like work-group_all)
;;    leads to generation of per-WI function call encompassed by
;;    'barrier' loop which starts after per-WI result accumulator initialization
;;    and ends immediately after the per-WI function call
;;*****************************************************************************
;;__kernel void build_hash_table()
;;{
;; int done = 0;
;; while(!work_group_all(done))
;; {
;;     done = 1;
;;     work_group_barrier(CLK_LOCAL_MEM_FENCE);
;; }
;;}

; ModuleID = 'Program'
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define dso_local void @build_hash_table() {

; CHECK-LABEL: @build_hash_table(
; CHECK:    [[ALLOCAWGRESULT:%.*]] = alloca i32, align 4
; CHECK-NEXT:    store i32 1, i32* [[ALLOCAWGRESULT]], align 4
; CHECK-NEXT:    call void @dummy_barrier.()

; CHECK:    [[CALLWGFORITEM:%.*]] = call i32 @_Z14work_group_alliPi(i32 noundef [[DONE_0:%.*]], i32* [[ALLOCAWGRESULT]]) #[[ATTR4:[0-9]+]]
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    store i32 1, i32* [[ALLOCAWGRESULT]], align 4
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    [[TOBOOL_NOT:%.*]] = icmp eq i32 [[CALLWGFORITEM]], 0

; CHECK:       tail call void @_Z18work_group_barrierj(i32 noundef 1) #[[ATTR5:[0-9]+]]

;
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %done.0 = phi i32 [ 0, %entry ], [ 1, %while.body ]
  %call = tail call i32 @_Z14work_group_alli(i32 noundef %done.0) #5
  %tobool.not = icmp eq i32 %call, 0
  br i1 %tobool.not, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  tail call void @_Z18work_group_barrierj(i32 noundef 1) #6
  br label %while.cond

while.end:                                        ; preds = %while.cond
  ret void
}

; Function Attrs: noduplicate
declare i32 @_Z14work_group_alli(i32) #1

; Function Attrs: noduplicate
declare void @_Z18work_group_barrierj(i32) #1

; Function Attrs: convergent norecurse nounwind
define dso_local void @_ZGVeN4_build_hash_table() {
; CHECK-LABEL: @_ZGVeN4_build_hash_table(
; CHECK:         [[ALLOCAWGRESULT:%.*]] = alloca <4 x i32>, align 16
; CHECK-NEXT:    store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* [[ALLOCAWGRESULT]], align 16
; CHECK-NEXT:    call void @dummy_barrier.()

; CHECK:    [[CALLWGFORITEM:%.*]] = call <4 x i32> @_Z14work_group_allDv4_iPS_(<4 x i32> noundef [[BROADCAST_SPLAT:%.*]], <4 x i32>* [[ALLOCAWGRESULT]]) #[[ATTR4]]
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LOADWGFINALRESULT:%.*]] = load <4 x i32>, <4 x i32>* [[ALLOCAWGRESULT]], align 16
; CHECK-NEXT:    [[CALLFINALIZEWG:%.*]] = call <4 x i32> @_Z25__finalize_work_group_allDv4_i(<4 x i32> [[LOADWGFINALRESULT]])
; CHECK-NEXT:    store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* [[ALLOCAWGRESULT]], align 16
; CHECK-NEXT:    call void @dummy_barrier.()

; CHECK:    tail call void @_Z18work_group_barrierj(i32 noundef 1) #[[ATTR5]]

;
entry:
  br label %VPlannedBB3

VPlannedBB3:                                      ; preds = %VPlannedBB5, %entry
  %uni.phi4 = phi i32 [ 0, %entry ], [ 1, %VPlannedBB5 ]
  %broadcast.splatinsert = insertelement <4 x i32> poison, i32 %uni.phi4, i64 0
  %broadcast.splat = shufflevector <4 x i32> %broadcast.splatinsert, <4 x i32> poison, <4 x i32> zeroinitializer
  %0 = call <4 x i32> @_Z14work_group_allDv4_i(<4 x i32> noundef %broadcast.splat) #5
  %.extract.0. = extractelement <4 x i32> %0, i64 0
  %1 = icmp eq i32 %.extract.0., 0
  br i1 %1, label %VPlannedBB5, label %return

VPlannedBB5:                                      ; preds = %VPlannedBB3
  tail call void @_Z18work_group_barrierj(i32 noundef 1) #6
  br label %VPlannedBB3

return:                                           ; preds = %VPlannedBB3
  ret void
}

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z14work_group_allDv4_i(<4 x i32>) #2

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "vector-variants"="_ZGVeN4_build_hash_table" }
attributes #1 = { convergent "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "no-trapping-math"="true" "opencl-vec-uniform-return" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent norecurse nounwind "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "vector-variants"="_ZGVeN4_build_hash_table" }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #6 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" "kernel-uniform-call" }

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function build_hash_table --  %AllocaWGResult = alloca i32, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function build_hash_table --  store i32 1, i32* %AllocaWGResult, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function build_hash_table --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function build_hash_table --  store i32 1, i32* %AllocaWGResult, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function build_hash_table --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_build_hash_table --  %AllocaWGResult = alloca <4 x i32>, align 16
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_build_hash_table --  store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %AllocaWGResult, align 16
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_build_hash_table --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_build_hash_table --  %LoadWGFinalResult = load <4 x i32>, <4 x i32>* %AllocaWGResult, align 16
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_build_hash_table --  %CallFinalizeWG = call <4 x i32> @_Z25__finalize_work_group_allDv4_i(<4 x i32> %LoadWGFinalResult)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_build_hash_table --  store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %AllocaWGResult, align 16
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4_build_hash_table --  call void @dummy_barrier.()
; DEBUGIFY-NOT: WARNING
