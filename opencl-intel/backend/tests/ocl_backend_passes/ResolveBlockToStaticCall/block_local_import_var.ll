; call local block  and import variables from calling kernel
; RUN: opt -resolve-block-call -S < %s | FileCheck %s


;kernel void kernel_scope(__global int* res)
;{
;  int multiplier = 3;
;  int (^kernelBlock)(int) = ^(int num)
;  {
;    return num * multiplier;
;  };
;  int tid = get_global_id(0);
;  res[tid] = -1;
;  multiplier = 8;
;  res[tid] = kernelBlock(7) - 21;
;}

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

%struct.__block_descriptor = type { i64, i64 }
%struct.__block_literal_generic = type { i8*, i32, i32, i8*, %struct.__block_descriptor* }

@_NSConcreteStackBlock = external global i8*
@.str = private unnamed_addr constant [9 x i8] c"i12@?0i8\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, i8*, i8* } { i64 0, i64 36, i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i8* null }

define void @kernel_scope(i32 addrspace(1)* %res) nounwind {
entry:
  %res.addr = alloca i32 addrspace(1)*, align 8
  %multiplier = alloca i32, align 4
  %kernelBlock = alloca i32 (i32)*, align 8
  %block = alloca <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>, align 8
  %tid = alloca i32, align 4
  store i32 addrspace(1)* %res, i32 addrspace(1)** %res.addr, align 8
  store i32 3, i32* %multiplier, align 4
  %block.isa = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>* %block, i32 0, i32 0
  store i8* bitcast (i8** @_NSConcreteStackBlock to i8*), i8** %block.isa
  %block.flags = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>* %block, i32 0, i32 1
  store i32 1073741824, i32* %block.flags
  %block.reserved = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>* %block, i32 0, i32 2
  store i32 0, i32* %block.reserved
  %block.invoke = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>* %block, i32 0, i32 3
  store i8* bitcast (i32 (i8*, i32)* @__kernel_scope_block_invoke to i8*), i8** %block.invoke
  %block.descriptor = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>* %block, i32 0, i32 4
  store %struct.__block_descriptor* bitcast ({ i64, i64, i8*, i8* }* @__block_descriptor_tmp to %struct.__block_descriptor*), %struct.__block_descriptor** %block.descriptor
  %block.captured = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>* %block, i32 0, i32 5
  %0 = load i32* %multiplier, align 4
  store i32 %0, i32* %block.captured, align 4
  %1 = bitcast <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>* %block to i32 (i32)*
  store i32 (i32)* %1, i32 (i32)** %kernelBlock, align 8
  %call = call i64 @get_global_id(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* %tid, align 4
  %2 = load i32* %tid, align 4
  %idxprom = sext i32 %2 to i64
  %3 = load i32 addrspace(1)** %res.addr, align 8
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %3, i64 %idxprom
  store i32 -1, i32 addrspace(1)* %arrayidx, align 4
  store i32 8, i32* %multiplier, align 4
  %4 = load i32 (i32)** %kernelBlock, align 8
  %block.literal = bitcast i32 (i32)* %4 to %struct.__block_literal_generic*
  %5 = getelementptr inbounds %struct.__block_literal_generic* %block.literal, i32 0, i32 3
  %6 = bitcast %struct.__block_literal_generic* %block.literal to i8*
  %7 = load i8** %5
  %8 = bitcast i8* %7 to i32 (i8*, i32)*
; CHECK: call i32 @__kernel_scope_block_invoke
; CHECK-NOT: call i32 %
  %call1 = call i32 %8(i8* %6, i32 7)
  %sub = sub nsw i32 %call1, 21
  %9 = load i32* %tid, align 4
  %idxprom2 = sext i32 %9 to i64
  %10 = load i32 addrspace(1)** %res.addr, align 8
  %arrayidx3 = getelementptr inbounds i32 addrspace(1)* %10, i64 %idxprom2
  store i32 %sub, i32 addrspace(1)* %arrayidx3, align 4
  ret void
}

define internal i32 @__kernel_scope_block_invoke(i8* %.block_descriptor, i32 %num) nounwind {
entry:
  %num.addr = alloca i32, align 4
  store i32 %num, i32* %num.addr, align 4
  %block = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>*
  %0 = load i32* %num.addr, align 4
  %block.capture.addr = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>* %block, i32 0, i32 5
  %1 = load i32* %block.capture.addr, align 4
  %mul = mul nsw i32 %0, %1
  ret i32 %mul
}

declare i64 @get_global_id(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = !{void (i32 addrspace(1)*)* @kernel_scope, !1}
!1 = !{!"argument_attribute", i32 0}
!2 = !{!"-cl-std=CL1.2"}

