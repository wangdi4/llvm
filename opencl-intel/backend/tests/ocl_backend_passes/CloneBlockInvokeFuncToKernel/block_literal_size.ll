; Check blockliteral size is computed correctly
; RUN: opt -cloneblockinvokefunctokernel <  %s -S | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%struct.__block_descriptor.10 = type { i64, i64 }
%struct.__block_literal_generic.11 = type { i8*, i32, i32, i8*, %struct.__block_descriptor.10* }

define internal void @__block_for_cond_block_invoke(i8* %.block_descriptor) nounwind {
entry:
  %num.addr = alloca i32, align 4
  store i32 37, i32* %num.addr, align 4
  %block = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>*
  %0 = load i32* %num.addr, align 4
  %block.capture.addr = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>* %block, i32 0, i32 5
  %1 = load i32* %block.capture.addr, align 4
  %mul = mul nsw i32 %0, %1
  ret void
}

!opencl.kernels = !{}
; CHECK: !{!"block_literal_size", i32 36}
