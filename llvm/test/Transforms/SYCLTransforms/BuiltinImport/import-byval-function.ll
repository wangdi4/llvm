; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import %s -S | FileCheck %s

;********************************************************************************
; When we have a case of pSrcFunction having a byval parameter it means the following:
;
; Frontend generated a definition with a byval parameter in one module, for instance:
;
;     define <8 x i32> @_Z18convert_uchar8_satDv8_j(ptr nocapture readonly byval(<8 x i32>) align 32 %0)
;
; and a declaration in another module like this one:
;
;     declare <8 x i32> @_Z18convert_uchar8_satDv8_j(<8 x i32>)
;
; The module with declaration perform calls according to the decl:
;
;     %call = tail call <8 x i32> @_Z18convert_uchar8_satDv8_j(<8 x i32> %in)
;
; If we import the definition we still get the call like we did above.
;
; That's what we're going to expect
;*********************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define <8 x i32> @kernel_function(<8 x i32> %in) nounwind {
entry:
  %call = tail call <8 x i32> @_Z18convert_uchar8_satDv8_j(<8 x i32> %in)
  ; CHECK: %call = tail call <8 x i32> @_Z18convert_uchar8_satDv8_j(<8 x i32> %in)
  ret <8 x i32> %call
}

declare <8 x i32> @_Z18convert_uchar8_satDv8_j(<8 x i32>)

; CHECK: define internal <8 x i32> @_Z18convert_uchar8_satDv8_j(ptr nocapture readonly byval(<8 x i32>) align 32 %0)
; CHECK-NOT: declare @_Z18convert_uchar8_satDv8_j

; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function _Z18convert_uchar8_satDv8_j
; DEBUGIFY-NOT: WARNING
