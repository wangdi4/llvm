; RUN: opt -passes=dpcpp-kernel-barrier %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i64 value.
;;           kernel main also calls same "foo" function with non-uniform returned value value "%r1"
;; The expected result:
;;      1. Kernel "main" contains no more barrier/barrier_dummyinstructions
;;      2. Kernel "main" stores "%y" value to offset 16 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. Kernel "main" loads "%r1" value from offset 24 in the special buffer after calling "foo".
;;      5. Kernel "main" stores "%r1" value to offset 16 in the special buffer before calling "foo".
;;      6. Kernel "main" is still calling function "foo"
;;      7. function "foo" contains no more barrier/barrier_dummyinstructions
;;      8. function "foo" loads "%x" value from offset 16 in the special buffer before xor.
;;      9. function "foo" stores "%y" value to offset 24 in the special buffer before ret.
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
  %r1 = call i64 @foo(i64 %y)
  br label %L2A
L2A:
  call void @barrier_dummy()
  br label %L3
L3:
  call void @_Z18work_group_barrierj(i32 1)
  %r2 = call i64 @foo(i64 %r1)
  br label %L3B
L3B:
  call void @barrier_dummy()
  ret void
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: L2:                                               ; preds = %SyncBB4
; CHECK: %SBIndex2 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset3 = add nuw i64 %SBIndex2, 16
; CHECK: %1 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset3
; CHECK: %pSB_LocalId4 = bitcast i8* %1 to i64*
; CHECK: %SBIndex14 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset15 = add nuw i64 %SBIndex14, 0
; CHECK: %2 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset15
; CHECK: %pSB_LocalId16 = bitcast i8* %2 to i64*
; CHECK: %loadedValue17 = load i64, i64* %pSB_LocalId16
; CHECK: store i64 %loadedValue17, i64* %pSB_LocalId4
; CHECK: br label %CallBB1
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: call i64 @foo
; CHECK: br label %
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: SyncBB3:                                          ; preds = %Dispatch, %L2A
; CHECK: %SBIndex5 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset6 = add nuw i64 %SBIndex5, 24
; CHECK: %11 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset6
; CHECK: %pSB_LocalId7 = bitcast i8* %11 to i64*
; CHECK: %loadedValue = load i64, i64* %pSB_LocalId7
; CHECK: %SBIndex8 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset9 = add nuw i64 %SBIndex8, 8
; CHECK: %12 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset9
; CHECK: %pSB_LocalId10 = bitcast i8* %12 to i64*
; CHECK: store i64 %loadedValue, i64* %pSB_LocalId10
; CHECK: br label %L3
;;;; TODO: add regular expression for the below values.
; CHECK: L3:
; CHECK: %SBIndex = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 16
; CHECK: %13 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %13 to i64*
; CHECK: %SBIndex22 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset23 = add nuw i64 %SBIndex22, 8
; CHECK: %14 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset23
; CHECK: %pSB_LocalId24 = bitcast i8* %14 to i64*
; CHECK: %loadedValue25 = load i64, i64* %pSB_LocalId24
; CHECK: store i64 %loadedValue25, i64* %pSB_LocalId
; CHECK: br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; TODO: add regular expression for the below values.
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call i64 @foo
; CHECK: br label %
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

; CHECK-LABEL: define i64 @foo
define i64 @foo(i64 %x) nounwind {
L1:
  call void @barrier_dummy()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  ret i64 %y
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB1:
; CHECK: %SBIndex = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 16
; CHECK: %0 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %0 to i64*
; CHECK: %loadedValue = load i64, i64* %pSB_LocalId
; CHECK: %y = xor i64 %loadedValue, %loadedValue
; CHECK: %SBIndex1 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset2 = add nuw i64 %SBIndex1, 24
; CHECK: %1 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset2
; CHECK: %pSB_LocalId3 = bitcast i8* %1 to i64*
; CHECK: store i64 %y, i64* %pSB_LocalId3
; CHECK: br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

declare void @_Z18work_group_barrierj(i32)
declare i64 @_Z12get_local_idj(i32)
declare void @barrier_dummy()

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" }
attributes #1 = { "sycl_kernel" }
