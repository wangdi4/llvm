; RUN: opt -passes=dpcpp-kernel-barrier %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns void.
;;           kernel main also calls same "foo" function with uniform value "%x"
;; The expected result:
;;      1. Kernel "main" contains no more barrier/barrier_dummy instructions
;;      2. Kernel "main" stores "%y" value to offset 8 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. Kernel "main" stores "%x" value to offset 8 in the special buffer before calling "foo".
;;      5. Kernel "main" is still calling function "foo"
;;      6. function "foo" contains no more barrier/barrier_dummy instructions
;;      7. function "foo" loads "%x" value from offset 8 in the special buffer before xor.
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
  br label %L2A
L2A:
  call void @barrier_dummy()
  br label %L3
L3:
  call void @_Z18work_group_barrierj(i32 1)
  call void @foo(i64 %x)
  br label %L3A
L3A:
  call void @barrier_dummy()
  ret void
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: xor
; CHECK: br label %
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: L2:                                               ; preds = %SyncBB{{[0-9]*}}
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: [[SBINDEX1:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET1:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX1]], 8
; CHECK: [[GEP1:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[SB_LOCALID_OFFSET1]]
; CHECK: [[PSB_LOCALID1:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP1]] to i64*
; CHECK: [[SBINDEX2:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET2:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX2]], 0
; CHECK: [[GEP2:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[SB_LOCALID_OFFSET2]]
; CHECK: [[PSB_LOCALID2:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP2]] to i64*
; CHECK: [[LOADED_VALUE:%loadedValue[0-9]*]] = load i64, i64* [[PSB_LOCALID2]]
; CHECK: store i64 [[LOADED_VALUE]], i64* [[PSB_LOCALID1]]
; CHECK: br label %CallBB{{[0-9]*}}
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: L3:
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: [[SBINDEX3:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET3:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX3]], 8
; CHECK: [[GEP3:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[SB_LOCALID_OFFSET3]]
; CHECK: [[PSB_LOCALID3:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP3]] to i64*
; CHECK: store i64 %x, i64* [[PSB_LOCALID3]]
; CHECK: br label %CallBB{{[0-9]*}}
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @barrier_dummy
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

; CHECK-LABEL: define void @foo
define void @foo(i64 %x) #1 {
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
; CHECK: SyncBB1:
; CHECK: %SBIndex = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 8
; CHECK: %0 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %0 to i64*
; CHECK: %loadedValue = load i64, i64* %pSB_LocalId
; CHECK: %y = xor i64 %loadedValue, %loadedValue
; CHECK: br label %L2
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

!sycl.kernels = !{!0}
!0 = !{void (i64)* @main}
