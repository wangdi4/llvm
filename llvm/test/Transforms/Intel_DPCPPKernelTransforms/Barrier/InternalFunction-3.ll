; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and function "foo",
;;           which contains barrier itself and returns i32 value type,
;;           and receives uniform value "%x" that does not cross barrier.
;; The expected result:
;;      1. Kernel "main" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      2. Kernel "main" is still calling function "foo"
;;      3. Kernel "main" loads "%z" value from offset 4 in the special buffer after calling "foo".
;;      4. function "foo" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      5. function "foo" stores "%x" value to offset 4 in the special buffer before ret.
;;      5.1. This store of value "%x" happens in a new created loop over all WIs
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK-LABEL: define void @main
define void @main(i32 %x) #0 {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %lid = call i32 @__builtin_get_local_id(i32 0)
  %y = xor i32 %x, %lid
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  %z = call i32 @foo(i32 %x)
  br label %L3
L3:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %w = and i32 %z, %z
  br label %L4
L4:
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  ret void
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: xor
; CHECK-NOT: store i32 %x
; CHECK: call i32 @foo
; CHECK: br label %
;;;; TODO: add regular expression for the below values.

; CHECK: SyncBB2:                                          ; preds = %Dispatch, %L3
; CHECK: %SBIndex = load i32, i32* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i32 %SBIndex, 4
; CHECK: %7 = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %7 to i32*
; CHECK: %loadedValue = load i32, i32* %pSB_LocalId
; CHECK: %SBIndex1 = load i32, i32* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset2 = add nuw i32 %SBIndex1, 0
; CHECK: %8 = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset2
; CHECK: %pSB_LocalId3 = bitcast i8* %8 to i32*
; CHECK: store i32 %loadedValue, i32* %pSB_LocalId3
; CHECK: %SBIndex4 = load i32, i32* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset5 = add nuw i32 %SBIndex4, 0
; CHECK: %9 = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset5
; CHECK: %pSB_LocalId6 = bitcast i8* %9 to i32*
; CHECK: %loadedValue7 = load i32, i32* %pSB_LocalId6
; CHECK: %w = and i32 %loadedValue7, %loadedValue7
; CHECK: br label %L4
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: ret
}

; CHECK-LABEL: define i32 @foo
define i32 @foo(i32 %x) #1 {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 2)
  ret i32 %x
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; The return BB
;;;; TODO: add regular expression for the below values.

; CHECK: LoopBB:                                           ; preds = %Dispatch, %SyncBB0
; CHECK: %SBIndex1 = load i32, i32* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i32 %SBIndex1, 4
; CHECK: %7 = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %7 to i32*
; CHECK: store i32 %x, i32* %pSB_LocalId
; CHECK: %LocalId_0 = load i32, i32* %pLocalId_0
; CHECK: %8 = add nuw i32 %LocalId_0, 1
; CHECK: store i32 %8, i32* %pLocalId_0
; CHECK: %9 = icmp ult i32 %8, %LocalSize_0
; CHECK: br i1 %9, label %Dispatch, label %LoopEnd_0
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: ret i32 %x
}

declare void @__builtin_dpcpp_kernel_barrier(i32)
declare i32 @__builtin_get_local_id(i32)
declare void @__builtin_dpcpp_kernel_barrier_dummy()

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" }
attributes #1 = { "dpcpp-no-barrier-path"="false" }
