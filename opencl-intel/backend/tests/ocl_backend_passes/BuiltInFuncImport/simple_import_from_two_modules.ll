; RUN: llvm-as %s.1.rtl -o %t.1.rtl.bc
; RUN: llvm-as %s.2.rtl -o %t.2.rtl.bc
; RUN: opt -runtimelib=%t.1.rtl.bc,%t.2.rtl.bc -builtin-import -verify %s -S | FileCheck %s

;********************************************************************************
; This test checks that the BuiltInFuncImport pass imports function definitions from
; two RTL modules to the user module
;
; Functions to be imported are defined in the
;   * import_from_two_modules.ll.1.rtl
;   * import_from_two_modules.ll.2.rtl
;
; The expected results:
;    @function_from_rtl_1       to be defined in the user module
;    @function_from_rtl_2       to be defined in the user module
;********************************************************************************


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"


define <4 x double> @kernel_function(<4 x double> %in) nounwind {
entry:
    %call01 = call <4 x double> @function_from_rtl_1(<4 x double> %in)
    %call02 = call <4 x double> @function_from_rtl_2(<4 x double> %in)
    ret <4 x double> %call01
}

declare <4 x double> @function_from_rtl_1(<4 x double>) nounwind
declare <4 x double> @function_from_rtl_2(<4 x double>) nounwind


; CHECK:  define linkonce_odr <4 x double> @function_from_rtl_1(<4 x double> %in)
; CHECK:  define linkonce_odr <4 x double> @function_from_rtl_2(<4 x double> %in)
