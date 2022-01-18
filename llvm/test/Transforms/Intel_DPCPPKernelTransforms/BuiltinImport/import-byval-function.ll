; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -dpcpp-kernel-builtin-import %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -dpcpp-kernel-builtin-import %s -S | FileCheck %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -passes=dpcpp-kernel-builtin-import %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -passes=dpcpp-kernel-builtin-import %s -S | FileCheck %s

;********************************************************************************
; When we have a case of pSrcFunction having a byval parameter it means the following:
;
; Frontend generated a definition with a byval parameter in one module, for instance:
;
;     define double @_Z18convert_uchar8_satDv8_j(<8 x i32>* byval nocapture readonly align 32) {...}
;
; and a declaration in another module like this one:
;
;     declare double @_Z18convert_uchar8_satDv8_j(<8 x i32>)
;
; The module with declaration perform calls according to the decl:
;
;     %call = tail call double @_Z18convert_uchar8_satDv8_j(<8 x i32> %val_x)
;
; But if we import the definition we can no longer call the function like we did above.
;
; llvm-link solves this problem by creating a ConstantExpr bitcast for
; function pointer type, for example:
;
;     %call = tail call double bitcast (double (<8 x i32>*)*
;     %@_Z18convert_uchar8_satDv8_j to double (<8 x i32>)*)(<8 x i32> %val_x)
;
; That's what we're going to expect
;*********************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define <8 x i32> @kernel_function(<8 x i32> %in) nounwind {
entry:
  %call = tail call <8 x i32> @_Z18convert_uchar8_satDv8_j(<8 x i32> %in)
  ; CHECK: %call = tail call <8 x i32> bitcast (<8 x i32> (<8 x i32>*)* @_Z18convert_uchar8_satDv8_j to <8 x i32> (<8 x i32>)*)(<8 x i32> %in)
  ret <8 x i32> %call
}

declare <8 x i32> @_Z18convert_uchar8_satDv8_j(<8 x i32>)

; CHECK: define internal <8 x i32> @_Z18convert_uchar8_satDv8_j(<8 x i32>* nocapture readonly byval(<8 x i32>) align 32 %0) {
; CHECK-NOT: declare @_Z18convert_uchar8_satDv8_j

; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function _Z18convert_uchar8_satDv8_j
; DEBUGIFY-NOT: WARNING
