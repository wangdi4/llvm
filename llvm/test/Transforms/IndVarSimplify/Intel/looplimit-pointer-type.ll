; RUN: opt %s -passes=indvars -S | FileCheck %s

; Check the case SCEV base pointer type isn't the same as indvar pointer type.

; CHECK-NOT: %wstr.addr = phi

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@.str.1 = internal addrspace(1) constant [93 x i16] [i16 68, i16 58, i16 92, i16 105, i16 117, i16 115, i16 101, i16 114, i16 115, i16 92, i16 119, i16 101, i16 110, i16 106, i16 117, i16 104, i16 101, i16 92, i16 119, i16 115, i16 92, i16 120, i16 45, i16 119, i16 101, i16 98, i16 92, i16 100, i16 101, i16 112, i16 108, i16 111, i16 121, i16 92, i16 119, i16 105, i16 110, i16 95, i16 112, i16 114, i16 111, i16 100, i16 92, i16 98, i16 105, i16 110, i16 45, i16 108, i16 108, i16 118, i16 109, i16 92, i16 46, i16 46, i16 92, i16 105, i16 110, i16 99, i16 108, i16 117, i16 100, i16 101, i16 92, i16 115, i16 121, i16 99, i16 108, i16 47, i16 101, i16 120, i16 116, i16 47, i16 111, i16 110, i16 101, i16 97, i16 112, i16 105, i16 47, i16 115, i16 117, i16 98, i16 95, i16 103, i16 114, i16 111, i16 117, i16 112, i16 46, i16 104, i16 112, i16 112, i16 0]

define void @test() {
entry:
  %0 = getelementptr [256 x i8], [256 x i8]* null, i64 0, i64 0
  br label %while.body

while.body:                                   ; preds = %while.body, %entry
  %str.addr = phi i8* [ %0, %entry ], [ %incdec.ptr7, %while.body ]
  %wstr.addr = phi i16 addrspace(4)* [ addrspacecast (i16 addrspace(1)* getelementptr inbounds ([93 x i16], [93 x i16] addrspace(1)* @.str.1, i64 0, i64 0) to i16 addrspace(4)*), %entry ], [ %incdec.ptr, %while.body ]
  %incdec.ptr = getelementptr i16, i16 addrspace(4)* %wstr.addr, i64 1
  %incdec.ptr7 = getelementptr i8, i8* %str.addr, i64 1
  store i8 0, i8* %str.addr, align 1
  %1 = load i16, i16 addrspace(4)* %wstr.addr, align 2
  %cmp1.not.i.i = icmp eq i16 %1, 0
  br i1 %cmp1.not.i.i, label %exit, label %while.body

exit:      ; preds = %while.body
  ret void
}
