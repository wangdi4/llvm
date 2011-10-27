; RUN: llvm-as %s -o %t.bc
; RUN: opt -predicate %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare i32 @get_global_id(i32)

declare void @testin1(i32 %x) nounwind 

declare void @testin2(i32 %x) nounwind 

declare void @testout1(i32 %x) nounwind 

declare void @testout2(i32 %x) nounwind 

; CHECK: @test1
; CHECK: @maskedf_{{[0-9]*}}_testin1
; CHECK-NOT: @maskedf_{{[0-9]*}}_testout1
; CHECK: ret void
define void @test1(i32 addrspace(1)* nocapture %pOutputs) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds i32 addrspace(1)* %pOutputs, i32 %1
  %3 = load i32 addrspace(1)* %2, align 4
  %4 = icmp ugt i32 %1, 10
  br i1 %4, label %5, label %6

; <label>:5                                       ; preds = %0
  tail call void @testin1(i32 %3)
  br label %6

; <label>:6                                       ; preds = %5, %0
  tail call void @testout1(i32 %3)
  ret void
}

; CHECK: @test2
; CHECK: @maskedf_{{[0-9]*}}_testin1
; CHECK-NOT: @maskedf_{{[0-9]*}}_testout1
; CHECK: @maskedf_{{[0-9]*}}_testin2
; CHECK-NOT: @maskedf_{{[0-9]*}}_testout2
; CHECK: ret void
define void @test2(i32 addrspace(1)* nocapture %pOutputs) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds i32 addrspace(1)* %pOutputs, i32 %1
  %3 = load i32 addrspace(1)* %2, align 4
  %4 = icmp ugt i32 %1, 10
  br i1 %4, label %5, label %.thread


; <label>:5                                       ; preds = %0
  tail call void @testin1(i32 %3)
  br label %.thread
  
.thread:                                          ; preds = %0
  tail call void @testout1(i32 %3)
  %6 = icmp ult i32 %1, 20
  br i1 %6, label %7, label %8

; <label>:7                                       ; preds = %.thread, %5
  tail call void @testin2(i32 %3)
  br label %8

; <label>:8                                       ; preds = %7, %5
  tail call void @testout2(i32 %3)
  ret void
}

; CHECK: @testloop
; CHECK: @maskedf_{{[0-9]*}}_testin1
; CHECK-NOT: @maskedf_{{[0-9]*}}_testout1
; CHECK: ret void
define void @testloop(i32 addrspace(1)* nocapture %pOutputs) nounwind {
; <label>:0
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds i32 addrspace(1)* %pOutputs, i32 %1
  %3 = load i32 addrspace(1)* %2, align 4
  %tmp = icmp ugt i32 %1, 1
  %umax = select i1 %tmp, i32 %1, i32 1
  br label %4

; <label>:4                                       ; preds = %4, %0
  %i.0 = phi i32 [ 0, %0 ], [ %5, %4 ]
  tail call void @testin1(i32 %3)
  %5 = add i32 %i.0, 1
  %exitcond = icmp eq i32 %5, %umax
  br i1 %exitcond, label %6, label %4

; <label>:6                                       ; preds = %4
  tail call void @testout1(i32 %3)
  ret void
}

