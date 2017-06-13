; RUN: llvm-as %s -o %t.bc
; RUN: opt -presucf=false -O3 -inline-threshold=4096 -inline -lowerswitch -mergereturn -loop-simplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'branches.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @divBranchNestedUnLoop
; CHECK: header:
; CHECK: header
; CHECK: @masked_store
; CHECK: footer:
; CHECK: header
; CHECK: @masked_store
; CHECK: footer
; CHECK: ret

define void @divBranchNestedUnLoop(i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = and i32 %2, 1
  %4 = icmp eq i32 %3, 0
  br i1 %4, label %.preheader, label %8

.preheader:                                       ; preds = %0
  %5 = icmp sgt i32 %num, 0
  br i1 %5, label %.lr.ph, label %.loopexit

.lr.ph:                                           ; preds = %.preheader
  %6 = sext i32 %2 to i64
  %7 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %6
  store i32 7, i32 addrspace(1)* %7, align 4
  br label %.loopexit

; <label>:8                                       ; preds = %0
  %9 = sext i32 %2 to i64
  %10 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %9
  store i32 8, i32 addrspace(1)* %10, align 4, !tbaa !7
  br label %.loopexit

.loopexit:                                        ; preds = %.preheader, %.lr.ph, %8
  ret void
}

declare i64 @_Z13get_global_idj(i32) nounwind readnone

; CHECK: define void @divBranchNestedUnBranch
; CHECK: header:
; CHECK: header
; CHECK-NOT: @masked_load
; CHECK: @masked_store
; CHECK: footer:
; CHECK: header
; CHECK: @masked_store
; CHECK: footer
; CHECK: footer
; CHECK: ret

define void @divBranchNestedUnBranch(i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = and i32 %2, 1
  %4 = icmp eq i32 %3, 0
  br i1 %4, label %5, label %10

; <label>:5                                       ; preds = %0
  %6 = icmp sgt i32 %num, 10
  br i1 %6, label %7, label %13

; <label>:7                                       ; preds = %5
  %8 = sext i32 %2 to i64
  %9 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %8
  store i32 7, i32 addrspace(1)* %9, align 4, !tbaa !7
  br label %13

; <label>:10                                      ; preds = %0
  %11 = sext i32 %2 to i64
  %12 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %11
  store i32 8, i32 addrspace(1)* %12, align 4, !tbaa !7
  br label %13

; <label>:13                                      ; preds = %5, %7, %10
  ret void
}

; CHECK: define void @divBranchNestedUnBranchI
; CHECK: header:
; CHECK-NOT: @masked_load
; CHECK: header
; CHECK: @masked_store
; CHECK: footer
; CHECK: header
; CHECK: @masked_store
; CHECK: footer:
; CHECK: header
; CHECK: @masked_store
; CHECK: footer
; CHECK: footer
; CHECK: ret

define void @divBranchNestedUnBranchI(i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = and i32 %2, 1
  %4 = icmp eq i32 %3, 0
  br i1 %4, label %5, label %11

; <label>:5                                       ; preds = %0
  %6 = icmp sgt i32 %num, 10
  %7 = sext i32 %2 to i64
  %8 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %7
  br i1 %6, label %9, label %10

; <label>:9                                       ; preds = %5
  store i32 7, i32 addrspace(1)* %8, align 4, !tbaa !7
  br label %14

; <label>:10                                      ; preds = %5
  store i32 9, i32 addrspace(1)* %8, align 4, !tbaa !7
  br label %14

; <label>:11                                      ; preds = %0
  %12 = sext i32 %2 to i64
  %13 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %12
  store i32 8, i32 addrspace(1)* %13, align 4, !tbaa !7
  br label %14

; <label>:14                                      ; preds = %9, %10, %11
  ret void
}

; CHECK: define void @divBranchedUnBranch
; CHECK-NOT: @masked_load
; CHECK: header:
; CHECK: @masked_store
; CHECK: footer:
; CHECK-NOT: @masked_store
; CHECK: ret

define void @divBranchedUnBranch(i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = and i32 %2, 1
  %4 = icmp eq i32 %3, 0
  br i1 %4, label %5, label %8

; <label>:5                                       ; preds = %0
  %6 = sext i32 %2 to i64
  %7 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %6
  store i32 8, i32 addrspace(1)* %7, align 4, !tbaa !7
  br label %8

; <label>:8                                       ; preds = %5, %0
  %9 = and i32 %num, 1
  %10 = icmp eq i32 %9, 0
  br i1 %10, label %11, label %14

; <label>:11                                      ; preds = %8
  %12 = sext i32 %2 to i64
  %13 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %12
  store i32 7, i32 addrspace(1)* %13, align 4, !tbaa !7
  br label %14

; <label>:14                                      ; preds = %11, %8
  ret void
}

; CHECK: define void @nestedUnBranchedUnBranchDivBranch
; CHECK-NOT: @masked_load
; CHECK: header:
; CHECK: @masked_store
; CHECK: footer:
; CHECK: ret

define void @nestedUnBranchedUnBranchDivBranch(i32 addrspace(1)* nocapture %res, i32 %num) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %2 = trunc i64 %1 to i32
  %3 = and i32 %num, 1
  %4 = icmp eq i32 %3, 0
  %5 = icmp sgt i32 %num, 9
  %or.cond = and i1 %4, %5
  br i1 %or.cond, label %6, label %12

; <label>:6                                       ; preds = %0
  %7 = and i32 %2, 1
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %9, label %12

; <label>:9                                       ; preds = %6
  %10 = sext i32 %2 to i64
  %11 = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %10
  store i32 8, i32 addrspace(1)* %11, align 4, !tbaa !7
  br label %12

; <label>:12                                      ; preds = %9, %6, %0
  ret void
}

!opencl.kernels = !{!0, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{void (i32 addrspace(1)*, i32)* @divBranchNestedUnLoop, !1}
!1 = !{!"image_access_qualifier", i32 3, i32 3}
!2 = !{void (i32 addrspace(1)*, i32)* @divBranchNestedUnBranch, !1}
!3 = !{void (i32 addrspace(1)*, i32)* @divBranchNestedUnBranchI, !1}
!4 = !{void (i32 addrspace(1)*, i32)* @divBranchedUnBranch, !1}
!5 = !{void (i32 addrspace(1)*, i32)* @nestedUnBranchedUnBranchDivBranch, !1}
!6 = !{!"-cl-std=CL1.2"}
!7 = !{!"int", !8}
!8 = !{!"omnipotent char", !9}
!9 = !{!"Simple C/C++ TBAA"}
