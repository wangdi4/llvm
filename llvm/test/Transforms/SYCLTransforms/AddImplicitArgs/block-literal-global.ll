; This test checks definition of __block_literal_global variable contains
; bitcast with correct block function name (kernelBlock_block_invoke).
; Issue tested: previously kernelBlock_block_invoke was renamed to
;               kernelBlock_block_invoke_before.AddImplicitArgs in bitcast.

; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%struct.__block_descriptor = type { i64, i64 }

@_NSConcreteGlobalBlock = external global ptr
@.str = private unnamed_addr constant [9 x i8] c"i12@?0i8\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, ptr, ptr } { i64 0, i64 32, ptr @.str, ptr null }

; CHECK:    ptr @kernelBlock_block_invoke
@__block_literal_global = internal constant { ptr, i32, i32, ptr, ptr } { ptr @_NSConcreteGlobalBlock, i32 1342177280, i32 0, ptr @kernelBlock_block_invoke, ptr @__block_descriptor_tmp }, align 8

define internal i32 @kernelBlock_block_invoke(ptr nocapture %.block_descriptor, i32 %num) nounwind readnone {
entry:
  %mul = mul nsw i32 %num, 5
  ret i32 %mul
}

; DEBUGIFY-NOT: WARNING
