; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -runtimelib=%t.rtl.bc -builtin-import -verify %s -S | FileCheck %s

;;*****************************************************************************
;; This test checks what the BuiltInFuncImport pass preserves opaque type names
;; at importing any built-in function to user module.
;; The expected results:
;;    @callee built-in is imported.
;;    The module verifier is passed.
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl.pipe_t = type opaque
%opencl.reserve_id_t = type opaque

;CHECK: define {{.*}} @callee
declare %opencl.reserve_id_t* @callee(%opencl.pipe_t addrspace(1)* nocapture, %opencl.reserve_id_t**, i32) nounwind

define %opencl.reserve_id_t* @caller(%opencl.pipe_t addrspace(1)* nocapture %p, %opencl.reserve_id_t** %prid, i32 %packet) nounwind {
  %ret = call %opencl.reserve_id_t* @callee(%opencl.pipe_t addrspace(1)* nocapture %p, %opencl.reserve_id_t** %prid, i32 %packet)
  ret %opencl.reserve_id_t* %ret
}
