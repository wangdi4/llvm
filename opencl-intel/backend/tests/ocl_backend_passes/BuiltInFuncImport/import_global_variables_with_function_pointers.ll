; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -runtimelib=%t.rtl.bc -builtin-import -verify %s -S | FileCheck %s

;********************************************************************************
; This test checks that the BuiltInFuncImport pass correctly handles the following
; case when a user module calls a function that references global variable
; containing function pointer.
;
;********************************************************************************


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: @coord_translate_i_callback = addrspace(2) constant [1 x <4 x i32> (i8*, <4 x i32>)*] [<4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i], align 16

define i8 addrspace(2)* @kernel_function(i32 %index) nounwind {
entry:
    %call01 = call i8 addrspace(2)* @call_coord_translate_i_callback(i32 %index)
    ret i8 addrspace(2)* %call01
}

declare i8 addrspace(2)* @call_coord_translate_i_callback(i32 %samplerIndex)

; CHECK-DAG:  define linkonce_odr i8 addrspace(2)* @call_coord_translate_i_callback(i32 %samplerIndex)
; CHECK-DAG:  define linkonce_odr <4 x i32> @_Z25trans_coord_int_UNDEFINEDPvDv4_i(i8* nocapture %image, <4 x i32> %coord)
