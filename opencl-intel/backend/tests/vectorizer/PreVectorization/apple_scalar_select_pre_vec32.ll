

; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -runtime=apple -CLBltnPreVec %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"


; CHECK: @test___select_1i32_sext
; CHECK: sext
; CHECK: __select_1i32
; CHECK: ret void
define void @test___select_1i32_sext(i32 addrspace(1)* nocapture %src1, i32 addrspace(1)* nocapture %src2, i32 addrspace(1)* nocapture %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds i32 addrspace(1)* %src1, i32 %1
  %3 = load i32 addrspace(1)* %2, align 4
  %4 = getelementptr inbounds i32 addrspace(1)* %src2, i32 %1
  %5 = load i32 addrspace(1)* %4, align 4
  %6 = icmp sgt i32 %3, %5
  %7 = sext i1 %6 to i32
  %8 = tail call i32 @__select_1i32(i32 %3, i32 %5, i32 %7) nounwind
  %9 = getelementptr inbounds i32 addrspace(1)* %dst, i32 %1
  store i32 %8, i32 addrspace(1)* %9, align 4
  ret void
}


; CHECK: @test___select_1i32_zext
; CHECK-NOT: zext
; CHECK: sext
; CHECK: __select_1i32
; CHECK: ret void
define void @test___select_1i32_zext(i32 addrspace(1)* nocapture %src1, i32 addrspace(1)* nocapture %src2, i32 addrspace(1)* nocapture %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds i32 addrspace(1)* %src1, i32 %1
  %3 = load i32 addrspace(1)* %2, align 4
  %4 = getelementptr inbounds i32 addrspace(1)* %src2, i32 %1
  %5 = load i32 addrspace(1)* %4, align 4
  %6 = icmp sgt i32 %3, %5
  %7 = zext i1 %6 to i32
  %8 = tail call i32 @__select_1i32(i32 %3, i32 %5, i32 %7) nounwind
  %9 = getelementptr inbounds i32 addrspace(1)* %dst, i32 %1
  store i32 %8, i32 addrspace(1)* %9, align 4
  ret void
}

declare i32 @__select_1i32(i32 %x, i32 %y, i32 %m) nounwind readnone 

declare i32 @get_global_id(i32)


; CHECK: @test___select_1i32_fake
; CHECK-NOT: __select_1i32
; CHECK: _f_v.__select_1i32
; CHECK-NOT: __select_1i32
; CHECK: ret void
define void @test___select_1i32_fake(i32 addrspace(1)* nocapture %src1, i32 addrspace(1)* nocapture %src2, i32 addrspace(1)* nocapture %src3, i32 addrspace(1)* nocapture %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds i32 addrspace(1)* %src1, i32 %1
  %3 = load i32 addrspace(1)* %2, align 4
  %4 = getelementptr inbounds i32 addrspace(1)* %src2, i32 %1
  %5 = load i32 addrspace(1)* %4, align 4
  %6 = getelementptr inbounds i32 addrspace(1)* %src3, i32 %1
  %7 = load i32 addrspace(1)* %4, align 4
  %8 = tail call i32 @__select_1i32(i32 %3, i32 %5, i32 %7) nounwind
  %9 = getelementptr inbounds i32 addrspace(1)* %dst, i32 %1
  store i32 %8, i32 addrspace(1)* %9, align 4
  ret void
}

declare i32 @__select_1i32(i32 %x, i32 %y, i32 %m) nounwind readnone 

declare i32 @get_global_id(i32)