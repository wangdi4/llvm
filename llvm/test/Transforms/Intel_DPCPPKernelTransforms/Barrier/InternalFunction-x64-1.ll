; RUN: opt -passes=dpcpp-kernel-barrier %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns void.
;; The expected result:
;;      1. Kernel "main" contains no more barrier/barrier_dummy instructions
;;      2. Kernel "main" stores "%y" value to offset 8 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. function "foo" contains no more barrier/barrier_dummy instructions
;;      5. Kernel "main" loads "%x" value from offset 8 in the special buffer before xor.
;;*****************************************************************************

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"
; CHECK-LABEL: define void @main
define void @main(i64 %x) #0 {
L1:
  call void @barrier_dummy()
  %lid = call i64 @_Z12get_local_idj(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  call void @foo(i64 %y)
  br label %L3
L3:
  call void @barrier_dummy()
  ret void
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: xor
;;;; TODO: add regular expression for the below values.

; CHECK-LABEL: L2:
; CHECK-NEXT: %SBIndex = load i64, i64* %pCurrSBIndex
; CHECK-NEXT: %SB_LocalId_Offset = add nuw i64 %SBIndex, 8
; CHECK-NEXT: %1 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK-NEXT: %pSB_LocalId = bitcast i8* %1 to i64*
; CHECK-NEXT: %SBIndex4 = load i64, i64* %pCurrSBIndex
; CHECK-NEXT: %SB_LocalId_Offset5 = add nuw i64 %SBIndex4, 0
; CHECK-NEXT: %2 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset5
; CHECK-NEXT: %pSB_LocalId6 = bitcast i8* %2 to i64*
; CHECK-NEXT: %loadedValue = load i64, i64* %pSB_LocalId6
; CHECK-NEXT: store i64 %loadedValue, i64* %pSB_LocalId
; CHECK-NEXT: br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call void @foo
; CHECK-NEXT: br label %L3
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

; CHECK-LABEL: define void @foo
define void @foo(i64 %x) {
L1:
  call void @barrier_dummy()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  ret void
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.

; CHECK-LABEL: SyncBB1:                                          ; preds = %Dispatch, %FirstBB
; CHECK-NEXT: %SBIndex = load i64, i64* %pCurrSBIndex
; CHECK-NEXT: %SB_LocalId_Offset = add nuw i64 %SBIndex, 8
; CHECK-NEXT: %0 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK-NEXT: %pSB_LocalId = bitcast i8* %0 to i64*
; CHECK-NEXT: %loadedValue = load i64, i64* %pSB_LocalId
; CHECK-NEXT: %y = xor i64 %loadedValue, %loadedValue
; CHECK-NEXT: br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

declare void @_Z18work_group_barrierj(i32)
declare i64 @_Z12get_local_idj(i32)
declare void @barrier_dummy()

attributes #0 = { "no-barrier-path"="false" "sycl-kernel" }
attributes #1 = { "no-barrier-path"="false" }
