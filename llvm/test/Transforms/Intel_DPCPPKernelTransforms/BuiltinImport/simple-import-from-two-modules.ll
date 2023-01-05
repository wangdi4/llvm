; RUN: llvm-as %s.1.rtl -o %t.1.rtl.bc
; RUN: llvm-as %s.2.rtl -o %t.2.rtl.bc
; RUN: opt -dpcpp-kernel-builtin-lib=%t.1.rtl.bc,%t.2.rtl.bc -passes=dpcpp-kernel-builtin-import %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.1.rtl.bc,%t.2.rtl.bc -passes=dpcpp-kernel-builtin-import %s -S | FileCheck %s

;********************************************************************************
; This test checks that the BuiltInFuncImport pass imports function definitions from
; two RTL modules to the user module.
;
; Functions to be imported are defined in the
;   * import_from_two_modules.ll.1.rtl
;   * import_from_two_modules.ll.2.rtl
;
; The expected results:
;    @function_from_rtl_1       to be defined in the user module
;    @function_from_rtl_2       to be defined in the user module
;********************************************************************************

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


; CHECK:  define internal <4 x double> @function_from_rtl_1(<4 x double> %in)
; CHECK:  define internal <4 x double> @function_from_rtl_2(<4 x double> %in)

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function function_from_rtl_1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function function_from_rtl_2
; DEBUGIFY-NOT: WARNING
