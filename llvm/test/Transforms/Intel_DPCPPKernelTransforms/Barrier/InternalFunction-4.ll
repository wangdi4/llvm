; RUN: opt -passes=dpcpp-kernel-barrier %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i32 value type that is crossing barrier!
;; The expected result:
;;      1. Kernel "main" contains no more barrier/barrier_dummy instructions
;;      2. Kernel "main" stores "%y" value to offset 8 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. Kernel "main" loads "%z" value from offset 12 in the special buffer after calling "foo".
;;      5. function "foo" contains no more barrier/barrier_dummy instructions
;;      6. function "foo" stores "0" value to offset 12 in the special buffer before ret.
;;      6.1. This store of value "0" happens in a new created loop over all WIs
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK-LABEL: define void @main
define void @main(i32 %x) #0 {
L1:
  call void @barrier_dummy()
  %lid = call i32 @_Z12get_local_idj(i32 0)
  %y = xor i32 %x, %lid
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  %z = call i32 @foo(i32 %y)
  br label %L3
L3:
  call void @barrier_dummy()
  %w = and i32 %z, %z
  br label %L4
L4:
  call void @_Z18work_group_barrierj(i32 1)
  ret void
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: xor
;;;; TODO: add regular expression for the below values.

; CHECK-LABEL: L2:                                               ; preds = %SyncBB3
; CHECK-NEXT: %SBIndex = load i32, i32* %pCurrSBIndex
; CHECK-NEXT: %SB_LocalId_Offset = add nuw i32 %SBIndex, 8
; CHECK-NEXT: %1 = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset
; CHECK-NEXT: %pSB_LocalId = bitcast i8* %1 to i32*
; CHECK-NEXT: %SBIndex10 = load i32, i32* %pCurrSBIndex
; CHECK-NEXT: %SB_LocalId_Offset11 = add nuw i32 %SBIndex10, 0
; CHECK-NEXT: %2 = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset11
; CHECK-NEXT: %pSB_LocalId12 = bitcast i8* %2 to i32*
; CHECK-NEXT: %loadedValue13 = load i32, i32* %pSB_LocalId12
; CHECK-NEXT: store i32 %loadedValue13, i32* %pSB_LocalId
; CHECK-NEXT: br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call i32 @foo
; CHECK: br label %
;;;; TODO: add regular expression for the below values.
; CHECK-LABEL: SyncBB2:
; CHECK-NEXT: %SBIndex1 = load i32, i32* %pCurrSBIndex
; CHECK-NEXT: %SB_LocalId_Offset2 = add nuw i32 %SBIndex1, 12
; CHECK-NEXT: %11 = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset2
; CHECK-NEXT: %pSB_LocalId3 = bitcast i8* %11 to i32*
; CHECK-NEXT: %loadedValue = load i32, i32* %pSB_LocalId3
; CHECK-NEXT: %SBIndex4 = load i32, i32* %pCurrSBIndex
; CHECK-NEXT: %SB_LocalId_Offset5 = add nuw i32 %SBIndex4, 4
; CHECK-NEXT: %12 = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset5
; CHECK-NEXT: %pSB_LocalId6 = bitcast i8* %12 to i32*
; CHECK-NEXT: store i32 %loadedValue, i32* %pSB_LocalId6
; CHECK-NEXT: %SBIndex18 = load i32, i32* %pCurrSBIndex
; CHECK-NEXT: %SB_LocalId_Offset19 = add nuw i32 %SBIndex18, 4
; CHECK-NEXT: %13 = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset19
; CHECK-NEXT: %pSB_LocalId20 = bitcast i8* %13 to i32*
; CHECK-NEXT: %loadedValue21 = load i32, i32* %pSB_LocalId20
; CHECK-NEXT: %w = and i32 %loadedValue21, %loadedValue21
; CHECK-NEXT: br label %L4
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret void
}

; CHECK-LABEL: define i32 @foo
define i32 @foo(i32 %x) #1 {
L1:
  call void @barrier_dummy()
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  ret i32 0
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: LoopBB:
; CHECK: %SBIndex1 = load i32, i32* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i32 %SBIndex1, 12
; CHECK: %7 = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %7 to i32*
; CHECK: store i32 0, i32* %pSB_LocalId
; CHECK: %LocalId_0 = load i32, i32* %pLocalId_0
; CHECK: %8 = add nuw i32 %LocalId_0, 1
; CHECK: store i32 %8, i32* %pLocalId_0
; CHECK: %9 = icmp ult i32 %8, %LocalSize_0
; CHECK: br i1 %9, label %Dispatch, label %LoopEnd_0
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: ret i32 0
}

declare void @_Z18work_group_barrierj(i32)
declare i32 @_Z12get_local_idj(i32)
declare void @barrier_dummy()

attributes #0 = { "no-barrier-path"="false" "sycl-kernel" }
attributes #1 = { "no-barrier-path"="false" }
