; RUN: llvm-as %s -o %t.bc
; RUN: opt -ocl-syncfunctionattrs -jump-threading -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the the LLVM pass JumpThreading does not add new barrier instructions.
;; The case: kernel with call to internal function with barrier instructions
;;           (just before if()/if()-else basic blocks)
;; Related CQ ticket: CSSD100007107
;; TODO: reduce this test
;; The expected result:
;;      1. same number of call instructions to the internal function
;;*****************************************************************************

; CHECK: @top_scan
; CHECK-NOT: call fastcc i32 @scanLocalMem
; CHECK: call fastcc i32 @scanLocalMem
; CHECK-NOT: call fastcc i32 @scanLocalMem
; CHECK: !opencl.kernels

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

@top_scan.s_seed = internal addrspace(3) global i32 0, align 4

define void @top_scan(i32 addrspace(1)* nocapture %isums, i32 %n, i32 addrspace(3)* nocapture %lmem) nounwind {
entry:
  store i32 0, i32 addrspace(3)* @top_scan.s_seed, align 4
  call void @_Z7barrierj(i64 1) nounwind
  %call = call i64 @_Z12get_local_idj(i32 0) nounwind readnone
  %conv = sext i32 %n to i64
  %cmp = icmp ult i64 %call, %conv
  br i1 %cmp, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %add = add i64 %call, 1
  %cmp4 = icmp eq i64 %add, %conv
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %0 = phi i1 [ false, %entry ], [ %cmp4, %land.rhs ]
  br label %for.cond

for.cond:                                         ; preds = %if.end31, %land.end
  %d.0 = phi i32 [ 0, %land.end ], [ %inc, %if.end31 ]
  %cmp6 = icmp slt i32 %d.0, 16
  br i1 %cmp6, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %mul = mul nsw i32 %d.0, %n
  %conv12 = sext i32 %mul to i64
  %add14 = add i64 %conv12, %call
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %isums, i64 %add14
  %1 = load i32 addrspace(1)* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %val.0 = phi i32 [ %1, %if.then ], [ 0, %for.body ]
  %call15 = call fastcc i32 @scanLocalMem(i32 %val.0, i32 addrspace(3)* %lmem)
  br i1 %cmp, label %if.then20, label %if.end27

if.then20:                                        ; preds = %if.end
  %2 = load i32 addrspace(3)* @top_scan.s_seed, align 4
  %add21 = add i32 %call15, %2
  %mul22 = mul nsw i32 %d.0, %n
  %conv23 = sext i32 %mul22 to i64
  %add25 = add i64 %conv23, %call
  %arrayidx26 = getelementptr inbounds i32 addrspace(1)* %isums, i64 %add25
  store i32 %add21, i32 addrspace(1)* %arrayidx26, align 4
  br label %if.end27

if.end27:                                         ; preds = %if.then20, %if.end
  br i1 %0, label %if.then28, label %if.end31

if.then28:                                        ; preds = %if.end27
  %add29 = add i32 %call15, %val.0
  %3 = load i32 addrspace(3)* @top_scan.s_seed, align 4
  %add30 = add i32 %3, %add29
  store i32 %add30, i32 addrspace(3)* @top_scan.s_seed, align 4
  br label %if.end31

if.end31:                                         ; preds = %if.then28, %if.end27
  call void @_Z7barrierj(i64 1) nounwind
  %inc = add nsw i32 %d.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}
declare void @_Z7barrierj(i64)

declare i64 @_Z12get_local_idj(i32) nounwind readnone

define fastcc i32 @scanLocalMem(i32 %val, i32 addrspace(3)* %lmem) nounwind inlinehint {
entry:
  call void @_Z7barrierj(i64 1)
  ret i32 %val
}

declare i64 @_Z14get_local_sizej(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.cl_kernel_arg_info = !{!1}
!opencl.build.options = !{!7}

!0 = metadata !{void (i32 addrspace(1)*, i32, i32 addrspace(3)*)* @top_scan}
!1 = metadata !{metadata !"cl_kernel_arg_info", void (i32 addrspace(1)*, i32, i32 addrspace(3)*)* @top_scan, metadata !2, metadata !3, metadata !4, metadata !5, metadata !6}
!2 = metadata !{i32 1, i32 0, i32 3}
!3 = metadata !{i32 3, i32 3, i32 3}
!4 = metadata !{metadata !"uint*", metadata !"int", metadata !"uint*"}
!5 = metadata !{i32 0, i32 1, i32 0}
!6 = metadata !{metadata !"isums", metadata !"n", metadata !"lmem"}
!7 = metadata !{metadata !"-cl-kernel-arg-info"}
