; RUN: llvm-as %s.1.rtl -o %t.1.rtl.bc
; RUN: llvm-as %s.2.rtl -o %t.2.rtl.bc
; RUN: opt -runtimelib=%t.1.rtl.bc,%t.2.rtl.bc -builtin-import -verify %s -S | FileCheck %s

;*****************************************************************************
; This test checks that the BuiltInFuncImport pass imports a global variable
; referenced twice by 2 builtin functions
;
;*****************************************************************************


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare <4 x double> @module_1_function(<4 x double>) nounwind
declare <4 x double> @module_2_function(<4 x double>) nounwind

; CHECK: @float_const_expMask = addrspace(2) constant i32 2139095040, align 4
; CHECK-NOT: @float_const_expMask1

define <4 x double> @kernel_1(<4 x double> %in) nounwind {
entry:
	%call01 = call <4 x double> @module_1_function(<4 x double> %in)
	%call02 = call <4 x double> @module_2_function(<4 x double> %call01)
    ret <4 x double> %call02
}

; CHECK-DAG: define linkonce_odr <4 x double> @module_1_function(<4 x double> %in)
; CHECK-DAG: define linkonce_odr <4 x double> @module_2_function(<4 x double> %in)
