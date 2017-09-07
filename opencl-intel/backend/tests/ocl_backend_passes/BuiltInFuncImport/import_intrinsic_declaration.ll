; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -runtimelib=%t.rtl.bc -builtin-import -verify %s -S | FileCheck %s

;********************************************************************************
; This test checks that the BuiltInFuncImport pass handles correctly the
; following case:
; A user module function calls a function from the RTL module that calls
; a compiler intrinsic.
;
; Functions to be imported are defined in the
;   * import_chained_functions_across_modules.ll.rtl
;
; The expected results:
;    @function_from_rtl_1       to be defined in the user module
;    @llvm.lifetime.start       to be declared in the user module
;********************************************************************************


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"


define <4 x double> @kernel_function(<4 x double> %in) nounwind {
entry:
    %call01 = call <4 x double> @function_from_rtl(<4 x double> %in)
    ret <4 x double> %call01
}

declare <4 x double> @function_from_rtl(<4 x double>) nounwind


; CHECK:  define linkonce_odr <4 x double> @function_from_rtl(<4 x double> %in)
; CHECK:  declare void @llvm.lifetime.start
