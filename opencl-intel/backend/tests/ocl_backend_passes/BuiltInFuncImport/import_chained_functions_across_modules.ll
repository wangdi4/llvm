; RUN: llvm-as %s.1.rtl -o %t.1.rtl.bc
; RUN: llvm-as %s.2.rtl -o %t.2.rtl.bc
; RUN: opt -runtimelib=%t.1.rtl.bc,%t.2.rtl.bc -builtin-import -verify %s -S | FileCheck %s

;********************************************************************************
; This test checks that the BuiltInFuncImport pass imports functions in 
; the following case: user function calls functions in the 1st bltn module that calls
; a function in the 2nd module that calls another functions in the 1st module
; without recursion.
;
; Functions to be imported are defined in the
;   * import_chained_functions_across_modules_two_levels.ll.1.rtl
;   * import_chained_functions_across_modules_two_levels.ll.2.rtl
;
; The expected results:
;    @_Z21convert_char3_sat_rteDv3_i    to be defined in the user module
;    @_Z17convert_char3_satDv3_i        to be defined in the user module
;    @_Z18convert_short3_satDv3_i       to be defined in the user module
;********************************************************************************


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"


define <3 x i8> @kernel_function(<3 x i32> %in) nounwind {
entry:
    %call01 = call <3 x i8> @_Z21convert_char3_sat_rteDv3_i(<3 x i32> %in)
    ret <3 x i8> %call01
}

declare <3 x i8> @_Z21convert_char3_sat_rteDv3_i(<3 x i32> %x)


; CHECK-DAG:  define linkonce_odr <3 x i8> @_Z21convert_char3_sat_rteDv3_i(<3 x i32> %x)
; CHECK-DAG:  define linkonce_odr <3 x i8> @_Z17convert_char3_satDv3_i(<3 x i32> %x)
; CHECK-DAG:  define linkonce_odr <3 x i8> @_Z18convert_short3_satDv3_i(<3 x i32> %x)

