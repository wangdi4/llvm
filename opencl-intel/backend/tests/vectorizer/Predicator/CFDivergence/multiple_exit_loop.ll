; RUN: llvm-as %s -o %t.bc
; RUN: opt  -O3 -inline-threshold=4096 -inline -lowerswitch -mergereturn -loop-simplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'multiple_exit_loop.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: void @internalDivBranchMX
; CHECK: @masked_load
; CHECK: header:
; CHECK: @masked_store
; CHECK: footer:
; CHECK: ret

define void @internalDivBranchMX(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %1, 32
  %2 = ashr exact i64 %sext, 32
  br label %3

; <label>:3                                       ; preds = %14, %0
  %indvars.iv = phi i64 [ %indvars.iv.next, %14 ], [ 0, %0 ]
  %4 = trunc i64 %indvars.iv to i32
  %5 = icmp slt i32 %4, %num
  br i1 %5, label %6, label %.loopexit

; <label>:6                                       ; preds = %3
  %7 = add nsw i64 %indvars.iv, %2
  %8 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %7
  %9 = load i32, i32 addrspace(1)* %8, align 4, !tbaa !11
  %10 = trunc i64 %7 to i32
  %11 = icmp eq i32 %10, %9
  br i1 %11, label %12, label %14

; <label>:12                                      ; preds = %6
  %13 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %2
  store i32 7, i32 addrspace(1)* %13, align 4, !tbaa !11
  br label %.loopexit

; <label>:14                                      ; preds = %6
  %indvars.iv.next = add i64 %indvars.iv, 1
  br label %3

.loopexit:                                        ; preds = %3, %12
  ret void
}

declare i64 @_Z13get_global_idj(i32) nounwind readnone

; CHECK: void @externalDivBranchMX
; CHECK: @masked_store
; CHECK-NOT: @masked_load
; CHECK-NOT: @masked_load
; CHECK: footer:
; CHECK: ret

define void @externalDivBranchMX(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = sext i32 %2 to i64
  %4 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %3
  %5 = load i32, i32 addrspace(1)* %4, align 4, !tbaa !11
  %6 = icmp eq i32 %2, %5
  %7 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  br i1 %6, label %8, label %.preheader

; <label>:8                                       ; preds = %0
  store i32 8, i32 addrspace(1)* %7, align 4, !tbaa !11
  br label %.preheader

.preheader:                                       ; preds = %0, %8
  %9 = srem i32 %num, 49
  %10 = icmp eq i32 %9, 0
  br label %11

; <label>:11                                      ; preds = %.preheader, %16
  %i.0 = phi i32 [ %17, %16 ], [ 0, %.preheader ]
  %12 = icmp slt i32 %i.0, %num
  br i1 %12, label %13, label %18

; <label>:13                                      ; preds = %11
  %14 = load i32, i32 addrspace(1)* %7, align 4, !tbaa !11
  %15 = add nsw i32 %14, 7
  store i32 %15, i32 addrspace(1)* %7, align 4, !tbaa !11
  br i1 %10, label %18, label %16

; <label>:16                                      ; preds = %13
  %17 = add nsw i32 %i.0, 1
  br label %11

; <label>:18                                      ; preds = %13, %11
  ret void
}

; CHECK: void @externalDivBranchNestedUnLoopsMX
; CHECK: @masked_store
; CHECK-NOT: @masked_load
; CHECK-NOT: @masked_load
; CHECK: ret

define void @externalDivBranchNestedUnLoopsMX(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = sext i32 %2 to i64
  %4 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %3
  %5 = load i32, i32 addrspace(1)* %4, align 4, !tbaa !11
  %6 = icmp eq i32 %2, %5
  br i1 %6, label %7, label %.preheader1

; <label>:7                                       ; preds = %0
  %8 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  store i32 8, i32 addrspace(1)* %8, align 4, !tbaa !11
  br label %.preheader1

.preheader1:                                      ; preds = %7, %0
  %9 = icmp sgt i32 %num, 0
  br i1 %9, label %.preheader.lr.ph, label %._crit_edge

.preheader.lr.ph:                                 ; preds = %.preheader1
  %10 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  %11 = add nsw i32 %num, 3
  %12 = srem i32 %num, 49
  %13 = icmp eq i32 %12, 0
  br label %.preheader

.preheader:                                       ; preds = %21, %.preheader.lr.ph
  %i.02 = phi i32 [ 0, %.preheader.lr.ph ], [ %22, %21 ]
  br label %14

; <label>:14                                      ; preds = %.preheader, %19
  %j.0 = phi i32 [ %20, %19 ], [ 0, %.preheader ]
  %15 = icmp slt i32 %j.0, %11
  br i1 %15, label %16, label %21

; <label>:16                                      ; preds = %14
  %17 = load i32, i32 addrspace(1)* %10, align 4, !tbaa !11
  %18 = add nsw i32 %17, 7
  store i32 %18, i32 addrspace(1)* %10, align 4, !tbaa !11
  br i1 %13, label %21, label %19

; <label>:19                                      ; preds = %16
  %20 = add nsw i32 %j.0, 1
  br label %14

; <label>:21                                      ; preds = %14, %16
  %22 = add nsw i32 %i.02, 1
  %exitcond = icmp eq i32 %22, %num
  br i1 %exitcond, label %._crit_edge, label %.preheader

._crit_edge:                                      ; preds = %21, %.preheader1
  ret void
}

; CHECK: void @externalDivBranchNestedLoopsMX
; CHECK: @masked_store
; CHECK: @masked_load
; CHECK: @masked_store
; CHECK: @masked_store
; CHECK: ret

define void @externalDivBranchNestedLoopsMX(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = sext i32 %2 to i64
  %4 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %3
  %5 = load i32, i32 addrspace(1)* %4, align 4, !tbaa !11
  %6 = icmp eq i32 %2, %5
  br i1 %6, label %7, label %.preheader1

; <label>:7                                       ; preds = %0
  %8 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  store i32 8, i32 addrspace(1)* %8, align 4, !tbaa !11
  br label %.preheader1

.preheader1:                                      ; preds = %7, %0
  %9 = icmp sgt i32 %num, 0
  br i1 %9, label %.preheader.lr.ph, label %._crit_edge

.preheader.lr.ph:                                 ; preds = %.preheader1
  %10 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  %11 = add nsw i32 %2, 3
  %12 = srem i32 %num, 49
  %13 = icmp eq i32 %12, 0
  br label %.preheader

.preheader:                                       ; preds = %21, %.preheader.lr.ph
  %i.02 = phi i32 [ 0, %.preheader.lr.ph ], [ %22, %21 ]
  br label %14

; <label>:14                                      ; preds = %.preheader, %19
  %j.0 = phi i32 [ %20, %19 ], [ 0, %.preheader ]
  %15 = icmp slt i32 %j.0, %11
  br i1 %15, label %16, label %21

; <label>:16                                      ; preds = %14
  %17 = load i32, i32 addrspace(1)* %10, align 4, !tbaa !11
  %18 = add nsw i32 %17, 7
  store i32 %18, i32 addrspace(1)* %10, align 4, !tbaa !11
  br i1 %13, label %21, label %19

; <label>:19                                      ; preds = %16
  %20 = add nsw i32 %j.0, 1
  br label %14

; <label>:21                                      ; preds = %14, %16
  %22 = add nsw i32 %i.02, 1
  %exitcond = icmp eq i32 %22, %num
  br i1 %exitcond, label %._crit_edge, label %.preheader

._crit_edge:                                      ; preds = %21, %.preheader1
  ret void
}

; CHECK: void @internalDivBranchNestedUnLoopsMX
; CHECK: load i32
; CHECK: @masked_load
; CHECK: @masked_store
; CHECK: ret

define void @internalDivBranchNestedUnLoopsMX(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = icmp sgt i32 %num, 0
  br i1 %3, label %.preheader.lr.ph, label %._crit_edge

.preheader.lr.ph:                                 ; preds = %0
  %4 = add nsw i32 %num, 3
  %5 = sext i32 %2 to i64
  %6 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %5
  br label %.preheader

.preheader:                                       ; preds = %.loopexit, %.preheader.lr.ph
  %i.01 = phi i32 [ 0, %.preheader.lr.ph ], [ %18, %.loopexit ]
  br label %7

; <label>:7                                       ; preds = %.preheader, %16
  %j.0 = phi i32 [ %17, %16 ], [ 0, %.preheader ]
  %8 = icmp slt i32 %j.0, %4
  br i1 %8, label %9, label %.loopexit

; <label>:9                                       ; preds = %7
  %10 = load i32, i32 addrspace(1)* %6, align 4, !tbaa !11
  %11 = icmp eq i32 %2, %10
  br i1 %11, label %12, label %16

; <label>:12                                      ; preds = %9
  %13 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %5
  %14 = load i32, i32 addrspace(1)* %13, align 4, !tbaa !11
  %15 = add nsw i32 %14, 8
  store i32 %15, i32 addrspace(1)* %13, align 4, !tbaa !11
  br label %.loopexit

; <label>:16                                      ; preds = %9
  %17 = add nsw i32 %j.0, 1
  br label %7

.loopexit:                                        ; preds = %7, %12
  %18 = add nsw i32 %i.01, 1
  %exitcond = icmp eq i32 %18, %num
  br i1 %exitcond, label %._crit_edge, label %.preheader

._crit_edge:                                      ; preds = %.loopexit, %0
  ret void
}

; CHECK: void @internalDivBranchNestedLoopsMX
; CHECK: @masked_load
; CHECK: @masked_load
; CHECK: @masked_store
; CHECK: ret

define void @internalDivBranchNestedLoopsMX(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = icmp sgt i32 %num, 0
  br i1 %3, label %.preheader.lr.ph, label %._crit_edge

.preheader.lr.ph:                                 ; preds = %0
  %4 = add nsw i32 %2, 3
  %5 = sext i32 %2 to i64
  %6 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %5
  br label %.preheader

.preheader:                                       ; preds = %.loopexit, %.preheader.lr.ph
  %i.01 = phi i32 [ 0, %.preheader.lr.ph ], [ %18, %.loopexit ]
  br label %7

; <label>:7                                       ; preds = %.preheader, %16
  %j.0 = phi i32 [ %17, %16 ], [ 0, %.preheader ]
  %8 = icmp slt i32 %j.0, %4
  br i1 %8, label %9, label %.loopexit

; <label>:9                                       ; preds = %7
  %10 = load i32, i32 addrspace(1)* %6, align 4, !tbaa !11
  %11 = icmp eq i32 %2, %10
  br i1 %11, label %12, label %16

; <label>:12                                      ; preds = %9
  %13 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %5
  %14 = load i32, i32 addrspace(1)* %13, align 4, !tbaa !11
  %15 = add nsw i32 %14, 8
  store i32 %15, i32 addrspace(1)* %13, align 4, !tbaa !11
  br label %.loopexit

; <label>:16                                      ; preds = %9
  %17 = add nsw i32 %j.0, 1
  br label %7

.loopexit:                                        ; preds = %7, %12
  %18 = add nsw i32 %i.01, 1
  %exitcond = icmp eq i32 %18, %num
  br i1 %exitcond, label %._crit_edge, label %.preheader

._crit_edge:                                      ; preds = %.loopexit, %0
  ret void
}

; CHECK: void @internalDivBranchThreeNestedUnLoopsMX
; CHECK: load i32
; CHECK: @masked_load
; CHECK: @masked_store
; CHECK: ret

define void @internalDivBranchThreeNestedUnLoopsMX(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = icmp sgt i32 %num, 0
  br i1 %3, label %.preheader1.lr.ph, label %._crit_edge4

.preheader1.lr.ph:                                ; preds = %0
  %4 = add nsw i32 %num, 3
  %5 = icmp sgt i32 %4, 0
  %6 = add nsw i32 %num, 7
  %7 = sext i32 %2 to i64
  %8 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %7
  br label %.preheader1

.preheader1:                                      ; preds = %._crit_edge, %.preheader1.lr.ph
  %i.03 = phi i32 [ 0, %.preheader1.lr.ph ], [ %21, %._crit_edge ]
  br i1 %5, label %.preheader, label %._crit_edge

.preheader:                                       ; preds = %.preheader1, %.loopexit
  %j.02 = phi i32 [ %20, %.loopexit ], [ 0, %.preheader1 ]
  br label %9

; <label>:9                                       ; preds = %.preheader, %18
  %k.0 = phi i32 [ %19, %18 ], [ 0, %.preheader ]
  %10 = icmp slt i32 %k.0, %6
  br i1 %10, label %11, label %.loopexit

; <label>:11                                      ; preds = %9
  %12 = load i32, i32 addrspace(1)* %8, align 4, !tbaa !11
  %13 = icmp eq i32 %2, %12
  br i1 %13, label %14, label %18

; <label>:14                                      ; preds = %11
  %15 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %7
  %16 = load i32, i32 addrspace(1)* %15, align 4, !tbaa !11
  %17 = add nsw i32 %16, 8
  store i32 %17, i32 addrspace(1)* %15, align 4, !tbaa !11
  br label %.loopexit

; <label>:18                                      ; preds = %11
  %19 = add nsw i32 %k.0, 1
  br label %9

.loopexit:                                        ; preds = %9, %14
  %20 = add nsw i32 %j.02, 1
  %exitcond = icmp eq i32 %20, %4
  br i1 %exitcond, label %._crit_edge, label %.preheader

._crit_edge:                                      ; preds = %.loopexit, %.preheader1
  %21 = add nsw i32 %i.03, 1
  %exitcond5 = icmp eq i32 %21, %num
  br i1 %exitcond5, label %._crit_edge4, label %.preheader1

._crit_edge4:                                     ; preds = %._crit_edge, %0
  ret void
}

; CHECK: void @internalUnBranchDivLoopMX
; CHECK: @masked_load
; CHECK: ret

define void @internalUnBranchDivLoopMX(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = icmp eq i32 %num, 7
  br label %4

; <label>:4                                       ; preds = %12, %0
  %i.0 = phi i32 [ 0, %0 ], [ %13, %12 ]
  %5 = icmp slt i32 %i.0, %2
  br i1 %5, label %6, label %.loopexit

; <label>:6                                       ; preds = %4
  br i1 %3, label %7, label %12

; <label>:7                                       ; preds = %6
  %8 = sext i32 %2 to i64
  %9 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %8
  %10 = load i32, i32 addrspace(1)* %9, align 4, !tbaa !11
  %11 = add nsw i32 %10, 8
  store i32 %11, i32 addrspace(1)* %9, align 4, !tbaa !11
  br label %.loopexit

; <label>:12                                      ; preds = %6
  %13 = add nsw i32 %i.0, 1
  br label %4

.loopexit:                                        ; preds = %4, %7
  ret void
}

; CHECK: void @externalUnBranchDivLoopMX
; CHECK: @masked_load
; CHECK: ret

define void @externalUnBranchDivLoopMX(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = icmp eq i32 %num, 7
  %4 = sext i32 %2 to i64
  %5 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %4
  br i1 %3, label %6, label %.preheader

; <label>:6                                       ; preds = %0
  %7 = load i32, i32 addrspace(1)* %5, align 4, !tbaa !11
  %8 = add nsw i32 %7, 8
  store i32 %8, i32 addrspace(1)* %5, align 4, !tbaa !11
  br label %.preheader

.preheader:                                       ; preds = %0, %6
  %9 = srem i32 %2, 5
  %10 = icmp eq i32 %9, 0
  br label %11

; <label>:11                                      ; preds = %.preheader, %16
  %i.0 = phi i32 [ %17, %16 ], [ 0, %.preheader ]
  %12 = icmp slt i32 %i.0, %2
  br i1 %12, label %13, label %18

; <label>:13                                      ; preds = %11
  %14 = load i32, i32 addrspace(1)* %5, align 4, !tbaa !11
  %15 = add nsw i32 %14, 6
  store i32 %15, i32 addrspace(1)* %5, align 4, !tbaa !11
  br i1 %10, label %18, label %16

; <label>:16                                      ; preds = %13
  %17 = add nsw i32 %i.0, 1
  br label %11

; <label>:18                                      ; preds = %13, %11
  ret void
}

!opencl.kernels = !{!0, !2, !3, !4, !5, !6, !7, !8, !9}
!opencl.compiler.options = !{!10}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @internalDivBranchMX, !1}
!1 = !{!"image_access_qualifier", i32 3, i32 3, i32 3}
!2 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @externalDivBranchMX, !1}
!3 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @externalDivBranchNestedUnLoopsMX, !1}
!4 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @externalDivBranchNestedLoopsMX, !1}
!5 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @internalDivBranchNestedUnLoopsMX, !1}
!6 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @internalDivBranchNestedLoopsMX, !1}
!7 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @internalDivBranchThreeNestedUnLoopsMX, !1}
!8 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @internalUnBranchDivLoopMX, !1}
!9 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @externalUnBranchDivLoopMX, !1}
!10 = !{!"-cl-std=CL1.2"}
!11 = !{!"int", !12}
!12 = !{!"omnipotent char", !13}
!13 = !{!"Simple C/C++ TBAA"}
