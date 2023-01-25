; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import -sycl-kernel-cpu-prefix=z1 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import -sycl-kernel-cpu-prefix=z1 %s -S | FileCheck %s -check-prefix=CHECK-AMX64

; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import -sycl-kernel-cpu-prefix=x1 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import -sycl-kernel-cpu-prefix=x1 %s -S | FileCheck %s -check-prefix=CHECK-AMX32

;*****************************************************************************
; This test checks what the BuiltInFuncImport pass resolves the names of called
; svml functions from shared RTL. The pass called by test knows nothing about
; current cpu architecture, so it replaced "shared" by string which passed through
; "-arch" option.
; In real program "shared" should be replaced by cpu prefix, for example "z1"/"x1"
; for AMX arch.
;
; Imported functions are presented in the "shared_function_name_resolving.ll.rtl"
;
; The expected results:
;    @__ocl_svml_shared_acos4       to be resolved to @__ocl_svml_z1/x1_acos4
;    @__ocl_svml_l9_asin4           to not be changed
;    @blabla__ocl_svml_shared_acos4 to not be changed
;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define <4 x double> @kernel_acos(<4 x double> %in) nounwind {
entry:
  %call04 = call <4 x double> @function_foo4(<4 x double> %in)
  ret <4 x double> %call04
}

declare <4 x double> @function_foo4(<4 x double>) nounwind


; CHECK-AMX64:  %val1 = call intel_ocl_bicc_avx512 <4 x double> @__ocl_svml_z1_acos4(<4 x double> %in)
; CHECK-AMX64:  %val2 = call <4 x double> @__ocl_svml_l9_asin4(<4 x double> %in)
; CHECK-AMX64:  %val3 = call <4 x double> @blabla__ocl_svml_shared_acos4(<4 x double> %in)

; CHECK-AMX64:  declare intel_ocl_bicc_avx512 <4 x double> @__ocl_svml_z1_acos4(<4 x double>)
; CHECK-AMX64:  declare <4 x double> @__ocl_svml_l9_asin4(<4 x double>)
; CHECK-AMX64:  declare <4 x double> @blabla__ocl_svml_shared_acos4(<4 x double>)

; CHECK-AMX32:  %val1 = call intel_ocl_bicc_avx512 <4 x double> @__ocl_svml_x1_acos4(<4 x double> %in)
; CHECK-AMX32:  %val2 = call <4 x double> @__ocl_svml_l9_asin4(<4 x double> %in)
; CHECK-AMX32:  %val3 = call <4 x double> @blabla__ocl_svml_shared_acos4(<4 x double> %in)

; CHECK-AMX32:  declare intel_ocl_bicc_avx512 <4 x double> @__ocl_svml_x1_acos4(<4 x double>)
; CHECK-AMX32:  declare <4 x double> @__ocl_svml_l9_asin4(<4 x double>)
; CHECK-AMX32:  declare <4 x double> @blabla__ocl_svml_shared_acos4(<4 x double>)

; DEBUGIFY-COUNT-4: WARNING: Instruction with empty DebugLoc in function function_foo4
; DEBUGIFY-NOT: WARNING
