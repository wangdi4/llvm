; RUN: llvm-as %s.1.rtl -o %t.1.rtl.bc
; RUN: llvm-as %s.2.rtl -o %t.2.rtl.bc
; RUN: opt -runtimelib=%t.1.rtl.bc,%t.2.rtl.bc -builtin-import -verify %s -S | FileCheck %s

;********************************************************************************
; This test checks that the BuiltInFuncImport pass imports globals 
; referenced in the 1st module by declaration, but are defined in another module
;
; Globals are declared in
;   * import_globals_referenced_by_declaration.ll.1.rtl
; Globals are defined in
;   * import_globals_referenced_by_declaration.ll.2.rtl
;
; The expected results:
; the fololowing symbols to be defined in the user module
;    @double_const_expMask
;    @double_const_mantissaBits
;    @double_const_expOffset
;    @_Z12native_ilogbd
;    @__ocl_trunc_v1i64_v1i32
; the fololowing symbol not to be defined in the user module
;    @double_const_not_used
;********************************************************************************


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK-DAG: @double_const_expMask = addrspace(2) constant i64 9218868437227405312, align 8
; CHECK-DAG: @double_const_mantissaBits = addrspace(2) constant i32 52, align 4
; CHECK-DAG: @double_const_expOffset = addrspace(2) constant i32 1023, align 4
; CHECK-NOT: @double_const_not_used

define i32 @kernel_function(double %in) nounwind {
entry:
    %call01 = tail call i32 @_Z12native_ilogbd(double %in)
; CHECK-NOT: %call01 = tail call i32 @_Z12native_ilogbd[0-9]+(double %in)
    ret i32 %call01
}

declare i32 @_Z12native_ilogbd(double)


; CHECK-DAG:  define linkonce_odr i32 @_Z12native_ilogbd(double %x)
; CHECK-DAG:  define linkonce_odr i32 @__ocl_trunc_v1i64_v1i32(i64 %x)

