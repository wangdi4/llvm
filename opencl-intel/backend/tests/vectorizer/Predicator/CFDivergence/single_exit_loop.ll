; RUN: llvm-as %s -o %t.bc
; RUN: opt -O3 -inline-threshold=4096 -inline -lowerswitch -mergereturn -loop-simplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'single_exit_loop.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @internalDivBranch
; CHECK-NOT: @masked_load
; CHECK: @masked_store
; CHECK: ret

define void @internalDivBranch(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = icmp sgt i32 %num, 0
  br i1 %1, label %.lr.ph, label %._crit_edge

.lr.ph:                                           ; preds = %0
  %2 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %2, 32
  %3 = ashr exact i64 %sext, 32
  %4 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  br label %5

; <label>:5                                       ; preds = %12, %.lr.ph
  %indvars.iv = phi i64 [ 0, %.lr.ph ], [ %indvars.iv.next, %12 ]
  %6 = add nsw i64 %indvars.iv, %3
  %7 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %6
  %8 = load i32, i32 addrspace(1)* %7, align 4, !tbaa !11
  %9 = trunc i64 %6 to i32
  %10 = icmp eq i32 %9, %8
  br i1 %10, label %11, label %12

; <label>:11                                      ; preds = %5
  store i32 7, i32 addrspace(1)* %4, align 4, !tbaa !11
  br label %12

; <label>:12                                      ; preds = %5, %11
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %num
  br i1 %exitcond, label %._crit_edge, label %5

._crit_edge:                                      ; preds = %12, %0
  ret void
}

declare i64 @_Z13get_global_idj(i32) nounwind readnone

; CHECK: @externalDivBranch
; CHECK: @masked_store
; CHECK-NOT: @masked_load
; CHECK-NOT: @masked_store
; CHECK: ret

define void @externalDivBranch(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = sext i32 %2 to i64
  %4 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %3
  %5 = load i32, i32 addrspace(1)* %4, align 4, !tbaa !11
  %6 = icmp eq i32 %2, %5
  br i1 %6, label %7, label %.preheader

; <label>:7                                       ; preds = %0
  %8 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  store i32 8, i32 addrspace(1)* %8, align 4, !tbaa !11
  br label %.preheader

.preheader:                                       ; preds = %7, %0
  %9 = icmp sgt i32 %num, 0
  br i1 %9, label %.lr.ph, label %13

.lr.ph:                                           ; preds = %.preheader
  %10 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  %.promoted = load i32, i32 addrspace(1)* %10, align 4
  %11 = mul i32 %num, 7
  %12 = add i32 %.promoted, %11
  store i32 %12, i32 addrspace(1)* %10, align 4
  br label %13

; <label>:13                                      ; preds = %.lr.ph, %.preheader
  ret void
}

; CHECK: @externalDivBranchNestedUnLoops
; CHECK: @masked_store
; CHECK-NOT: @masked_load
; CHECK-NOT: @masked_store
; CHECK: ret

define void @externalDivBranchNestedUnLoops(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = sext i32 %2 to i64
  %4 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %3
  %5 = load i32, i32 addrspace(1)* %4, align 4, !tbaa !11
  %6 = icmp eq i32 %2, %5
  br i1 %6, label %7, label %.preheader2

; <label>:7                                       ; preds = %0
  %8 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  store i32 8, i32 addrspace(1)* %8, align 4, !tbaa !11
  br label %.preheader2

.preheader2:                                      ; preds = %7, %0
  %9 = icmp sgt i32 %num, 0
  br i1 %9, label %.preheader.lr.ph, label %._crit_edge4

.preheader.lr.ph:                                 ; preds = %.preheader2
  %10 = add nsw i32 %num, 3
  %11 = icmp sgt i32 %10, 0
  %12 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  %13 = mul i32 %num, 7
  %14 = add i32 %13, 21
  br label %.preheader

.preheader:                                       ; preds = %16, %.preheader.lr.ph
  %i.03 = phi i32 [ 0, %.preheader.lr.ph ], [ %17, %16 ]
  br i1 %11, label %.lr.ph, label %16

.lr.ph:                                           ; preds = %.preheader
  %.promoted = load i32, i32 addrspace(1)* %12, align 4
  %15 = add i32 %14, %.promoted
  store i32 %15, i32 addrspace(1)* %12, align 4
  br label %16

; <label>:16                                      ; preds = %.lr.ph, %.preheader
  %17 = add nsw i32 %i.03, 1
  %exitcond = icmp eq i32 %17, %num
  br i1 %exitcond, label %._crit_edge4, label %.preheader

._crit_edge4:                                     ; preds = %16, %.preheader2
  ret void
}

; CHECK: @externalDivBranchNestedLoops
; CHECK: header{{[0-9]*}}:
; CHECK: @masked_store
; CHECK: footer{{[0-9]*}}:  
; CHECK: header{{[0-9]*}}:
; CHECK: @masked_load
; CHECK: @masked_store
; CHECK: footer{{[0-9]*}}:  
; CHECK: ret

define void @externalDivBranchNestedLoops(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = sext i32 %2 to i64
  %4 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %3
  %5 = load i32, i32 addrspace(1)* %4, align 4, !tbaa !11
  %6 = icmp eq i32 %2, %5
  br i1 %6, label %7, label %.preheader2

; <label>:7                                       ; preds = %0
  %8 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  store i32 8, i32 addrspace(1)* %8, align 4, !tbaa !11
  br label %.preheader2

.preheader2:                                      ; preds = %7, %0
  %9 = icmp sgt i32 %num, 0
  br i1 %9, label %.preheader.lr.ph, label %._crit_edge4

.preheader.lr.ph:                                 ; preds = %.preheader2
  %10 = add nsw i32 %2, 3
  %11 = icmp sgt i32 %10, 0
  %12 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %3
  %13 = mul i32 %2, 7
  %14 = add i32 %13, 21
  br label %.preheader

.preheader:                                       ; preds = %16, %.preheader.lr.ph
  %i.03 = phi i32 [ 0, %.preheader.lr.ph ], [ %17, %16 ]
  br i1 %11, label %.lr.ph, label %16

.lr.ph:                                           ; preds = %.preheader
  %.promoted = load i32, i32 addrspace(1)* %12, align 4
  %15 = add i32 %14, %.promoted
  store i32 %15, i32 addrspace(1)* %12, align 4
  br label %16

; <label>:16                                      ; preds = %.lr.ph, %.preheader
  %17 = add nsw i32 %i.03, 1
  %exitcond = icmp eq i32 %17, %num
  br i1 %exitcond, label %._crit_edge4, label %.preheader

._crit_edge4:                                     ; preds = %16, %.preheader2
  ret void
}

; CHECK: @internalDivBranchNestedUnLoops
; CHECK-NOT: @masked_load
; CHECK: @masked_load
; CHECK: @masked_store
; CHECK: ret

define void @internalDivBranchNestedUnLoops(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = icmp sgt i32 %num, 0
  br i1 %3, label %.preheader.lr.ph, label %._crit_edge3

.preheader.lr.ph:                                 ; preds = %0
  %4 = add nsw i32 %num, 3
  %5 = icmp sgt i32 %4, 0
  %6 = sext i32 %2 to i64
  %7 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %6
  %8 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %6
  br label %.preheader

.preheader:                                       ; preds = %._crit_edge, %.preheader.lr.ph
  %i.02 = phi i32 [ 0, %.preheader.lr.ph ], [ %15, %._crit_edge ]
  br i1 %5, label %.lr.ph, label %._crit_edge

.lr.ph:                                           ; preds = %.preheader, %._crit_edge5
  %j.01 = phi i32 [ %phitmp, %._crit_edge5 ], [ 1, %.preheader ]
  %9 = load i32, i32 addrspace(1)* %7, align 4, !tbaa !11
  %10 = icmp eq i32 %2, %9
  br i1 %10, label %11, label %14

; <label>:11                                      ; preds = %.lr.ph
  %12 = load i32, i32 addrspace(1)* %8, align 4, !tbaa !11
  %13 = add nsw i32 %12, 8
  store i32 %13, i32 addrspace(1)* %8, align 4, !tbaa !11
  br label %14

; <label>:14                                      ; preds = %.lr.ph, %11
  %exitcond = icmp eq i32 %j.01, %4
  br i1 %exitcond, label %._crit_edge, label %._crit_edge5

._crit_edge5:                                     ; preds = %14
  %phitmp = add i32 %j.01, 1
  br label %.lr.ph

._crit_edge:                                      ; preds = %14, %.preheader
  %15 = add nsw i32 %i.02, 1
  %exitcond4 = icmp eq i32 %15, %num
  br i1 %exitcond4, label %._crit_edge3, label %.preheader

._crit_edge3:                                     ; preds = %._crit_edge, %0
  ret void
}

; CHECK: @internalDivBranchNestedLoops
; CHECK: header{{[0-9]*}}:
; CHECK: @masked_load
; CHECK: header{{[0-9]*}}:
; CHECK: @masked_load
; CHECK: @masked_store
; CHECK: footer{{[0-9]*}}: 
; CHECK: footer{{[0-9]*}}: 
; CHECK: ret

define void @internalDivBranchNestedLoops(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = icmp sgt i32 %num, 0
  br i1 %3, label %.preheader.lr.ph, label %._crit_edge3

.preheader.lr.ph:                                 ; preds = %0
  %4 = add nsw i32 %2, 3
  %5 = icmp sgt i32 %4, 0
  %6 = sext i32 %2 to i64
  %7 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %6
  %8 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %6
  br label %.preheader

.preheader:                                       ; preds = %._crit_edge, %.preheader.lr.ph
  %i.02 = phi i32 [ 0, %.preheader.lr.ph ], [ %15, %._crit_edge ]
  br i1 %5, label %.lr.ph, label %._crit_edge

.lr.ph:                                           ; preds = %.preheader, %._crit_edge5
  %j.01 = phi i32 [ %phitmp, %._crit_edge5 ], [ 1, %.preheader ]
  %9 = load i32, i32 addrspace(1)* %7, align 4, !tbaa !11
  %10 = icmp eq i32 %2, %9
  br i1 %10, label %11, label %14

; <label>:11                                      ; preds = %.lr.ph
  %12 = load i32, i32 addrspace(1)* %8, align 4, !tbaa !11
  %13 = add nsw i32 %12, 8
  store i32 %13, i32 addrspace(1)* %8, align 4, !tbaa !11
  br label %14

; <label>:14                                      ; preds = %.lr.ph, %11
  %exitcond = icmp eq i32 %j.01, %4
  br i1 %exitcond, label %._crit_edge, label %._crit_edge5

._crit_edge5:                                     ; preds = %14
  %phitmp = add i32 %j.01, 1
  br label %.lr.ph

._crit_edge:                                      ; preds = %14, %.preheader
  %15 = add nsw i32 %i.02, 1
  %exitcond4 = icmp eq i32 %15, %num
  br i1 %exitcond4, label %._crit_edge3, label %.preheader

._crit_edge3:                                     ; preds = %._crit_edge, %0
  ret void
}

; CHECK: @internalDivBranchThreeNestedUnLoops
; CHECK-NOT: @masked_load
; CHECK: @masked_load
; CHECK: @masked_store
; CHECK: ret

define void @internalDivBranchThreeNestedUnLoops(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = icmp sgt i32 %num, 0
  br i1 %3, label %.preheader2.lr.ph, label %._crit_edge6

.preheader2.lr.ph:                                ; preds = %0
  %4 = add nsw i32 %num, 3
  %5 = icmp sgt i32 %4, 0
  %6 = add nsw i32 %num, 7
  %7 = icmp sgt i32 %6, 0
  %8 = sext i32 %2 to i64
  %9 = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %8
  %10 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %8
  br label %.preheader2

.preheader2:                                      ; preds = %._crit_edge4, %.preheader2.lr.ph
  %i.05 = phi i32 [ 0, %.preheader2.lr.ph ], [ %18, %._crit_edge4 ]
  br i1 %5, label %.preheader, label %._crit_edge4

.preheader:                                       ; preds = %.preheader2, %._crit_edge
  %j.03 = phi i32 [ %17, %._crit_edge ], [ 0, %.preheader2 ]
  br i1 %7, label %.lr.ph, label %._crit_edge

.lr.ph:                                           ; preds = %.preheader, %._crit_edge9
  %k.01 = phi i32 [ %phitmp, %._crit_edge9 ], [ 1, %.preheader ]
  %11 = load i32, i32 addrspace(1)* %9, align 4, !tbaa !11
  %12 = icmp eq i32 %2, %11
  br i1 %12, label %13, label %16

; <label>:13                                      ; preds = %.lr.ph
  %14 = load i32, i32 addrspace(1)* %10, align 4, !tbaa !11
  %15 = add nsw i32 %14, 8
  store i32 %15, i32 addrspace(1)* %10, align 4, !tbaa !11
  br label %16

; <label>:16                                      ; preds = %.lr.ph, %13
  %exitcond = icmp eq i32 %k.01, %6
  br i1 %exitcond, label %._crit_edge, label %._crit_edge9

._crit_edge9:                                     ; preds = %16
  %phitmp = add i32 %k.01, 1
  br label %.lr.ph

._crit_edge:                                      ; preds = %16, %.preheader
  %17 = add nsw i32 %j.03, 1
  %exitcond7 = icmp eq i32 %17, %4
  br i1 %exitcond7, label %._crit_edge4, label %.preheader

._crit_edge4:                                     ; preds = %._crit_edge, %.preheader2
  %18 = add nsw i32 %i.05, 1
  %exitcond8 = icmp eq i32 %18, %num
  br i1 %exitcond8, label %._crit_edge6, label %.preheader2

._crit_edge6:                                     ; preds = %._crit_edge4, %0
  ret void
}

; CHECK: @internalUnBranchDivLoop
; CHECK: header{{[0-9]*}}:
; CHECK: @masked_load
; CHECK: @masked_store
; CHECK: footer{{[0-9]*}}: 
; CHECK: ret

define void @internalUnBranchDivLoop(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %.lr.ph, label %._crit_edge

.lr.ph:                                           ; preds = %0
  %4 = icmp eq i32 %num, 7
  %5 = sext i32 %2 to i64
  %6 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %5
  br label %7

; <label>:7                                       ; preds = %11, %.lr.ph
  %i.01 = phi i32 [ 0, %.lr.ph ], [ %12, %11 ]
  br i1 %4, label %8, label %11

; <label>:8                                       ; preds = %7
  %9 = load i32, i32 addrspace(1)* %6, align 4, !tbaa !11
  %10 = add nsw i32 %9, 8
  store i32 %10, i32 addrspace(1)* %6, align 4, !tbaa !11
  br label %11

; <label>:11                                      ; preds = %7, %8
  %12 = add nsw i32 %i.01, 1
  %exitcond = icmp eq i32 %12, %2
  br i1 %exitcond, label %._crit_edge, label %7

._crit_edge:                                      ; preds = %11, %0
  ret void
}

; CHECK: @externalUnBranchDivLoop
; CHECK-NOT: @masked_load
; CHECK-NOT: @masked_store
; CHECK: @masked_load
; CHECK: @masked_store
; CHECK: ret

define void @externalUnBranchDivLoop(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = icmp eq i32 %num, 7
  br i1 %3, label %4, label %.preheader

; <label>:4                                       ; preds = %0
  %5 = sext i32 %2 to i64
  %6 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %5
  %7 = load i32, i32 addrspace(1)* %6, align 4, !tbaa !11
  %8 = add nsw i32 %7, 8
  store i32 %8, i32 addrspace(1)* %6, align 4, !tbaa !11
  br label %.preheader

.preheader:                                       ; preds = %4, %0
  %9 = icmp sgt i32 %2, 0
  br i1 %9, label %.lr.ph, label %14

.lr.ph:                                           ; preds = %.preheader
  %10 = sext i32 %2 to i64
  %11 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %10
  %.promoted = load i32, i32 addrspace(1)* %11, align 4
  %12 = mul i32 %2, 6
  %13 = add i32 %.promoted, %12
  store i32 %13, i32 addrspace(1)* %11, align 4
  br label %14

; <label>:14                                      ; preds = %.lr.ph, %.preheader
  ret void
}

!opencl.kernels = !{!0, !2, !3, !4, !5, !6, !7, !8, !9}
!opencl.compiler.options = !{!10}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @internalDivBranch, !1}
!1 = !{!"image_access_qualifier", i32 3, i32 3, i32 3}
!2 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @externalDivBranch, !1}
!3 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @externalDivBranchNestedUnLoops, !1}
!4 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @externalDivBranchNestedLoops, !1}
!5 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @internalDivBranchNestedUnLoops, !1}
!6 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @internalDivBranchNestedLoops, !1}
!7 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @internalDivBranchThreeNestedUnLoops, !1}
!8 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @internalUnBranchDivLoop, !1}
!9 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @externalUnBranchDivLoop, !1}
!10 = !{!"-cl-std=CL1.2"}
!11 = !{!"int", !12}
!12 = !{!"omnipotent char", !13}
!13 = !{!"Simple C/C++ TBAA"}

