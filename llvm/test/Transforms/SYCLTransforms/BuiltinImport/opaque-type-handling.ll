; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import %s -S | FileCheck %s

;*****************************************************************************
; This test checks what the BuiltInFuncImport pass preserves opaque type names
; at importing any built-in function to user module.
; The expected results:
;    @callee built-in is imported.
;    The module verifier is passed.
;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

;CHECK: define {{.*}} @callee
declare ptr @callee(ptr addrspace(1) nocapture, ptr, i32) nounwind

define ptr @caller(ptr addrspace(1) nocapture %p, ptr %prid, i32 %packet) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %ret = call ptr @callee(ptr addrspace(1) nocapture %p, ptr %prid, i32 %packet)
  ret ptr %ret
}

; DEBUGIFY-COUNT-10: WARNING: Instruction with empty DebugLoc in function callee
; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function load
; DEBUGIFY-NOT: WARNING

!0 = !{!"int", !"reserve_id_t*", !"int"}
!1 = !{target("spirv.Pipe", 1) zeroinitializer, ptr null, i32 0}
