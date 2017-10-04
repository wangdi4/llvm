; RUN: llvm-as %s.1.rtl -o %t.1.rtl.bc
; RUN: llvm-as %s.2.rtl -o %t.2.rtl.bc
; RUN: %oclopt -runtimelib=%t.1.rtl.bc,%t.2.rtl.bc -builtin-import -verify %s -S | FileCheck %s

;*****************************************************************************
; This test checks that the BuiltInFuncImport pass imports a function
; that is called directly in the first kernel and indirectly in the second.
;
; Imported functions are presented in the "shared_function_name_resolving.ll.rtl"
;
; The expected result:
;    @function_module_1_foo_1       to be defined in user module
;    @function_module_2_foo         to be defined in user module
; collision like declaring @function_module_2_foo1 is not happening
;*****************************************************************************


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare <4 x double> @function_module_1_foo_1(<4 x double>) nounwind
declare <4 x double> @function_module_2_foo(<4 x double>) nounwind

define <4 x double> @kernel_1(<4 x double> %in) nounwind {
entry:
    %call01 = call <4 x double> @function_module_1_foo_1(<4 x double> %in)
    ret <4 x double> %call01
}

define <4 x double> @kernel_2(<4 x double> %in) nounwind {
entry:
; CHECK-NOT: %call02 = call <4 x double> @function_module_2_foo[0-9]+(<4 x double> %in)
    %call02 = call <4 x double> @function_module_2_foo(<4 x double> %in)
    ret <4 x double> %call02
}

; CHECK: define linkonce_odr <4 x double> @function_module_1_foo_1(<4 x double> %in)
; CHECK: define linkonce_odr <4 x double> @function_module_2_foo(<4 x double> %in)
; CHECK-NOT: declare <4 x double> @function_module_2_foo[0-9]+(<4 x double>)
