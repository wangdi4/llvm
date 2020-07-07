; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and function "foo",
;;           which contains barrier itself and returns void,
;;           and receives non-uniform alloca address value "%x" (that also cross barrier).
;; The expected result:
;;      1. Kernel "main" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      2. Kernel "main" stores "%x" address value to offset 0 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. function "foo" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      5. function "foo" loads "%x" address value from offset 4 in the special buffer before loading from it.
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK-LABEL: define void @main
define void @main() #0 {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %x = alloca i32
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  call void @foo(i32* %x)
  br label %L3
L3:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  ret void
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
;;;; TODO: add regular expression for the below values.
; CHECK-LABEL: SyncBB2:
; CHECK:          [[GEP0:%[a-zA-Z0-9]+]] = load i32*, i32** %x.addr
; CHECK-NEXT:   br label %L2
; CHECK-LABEL: L2:
; CHECK-NEXT:   %SBIndex = load i32, i32* %pCurrSBIndex
; CHECK-NEXT:   %SB_LocalId_Offset = add nuw i32 %SBIndex, 4
; CHECK-NEXT:   [[GEP1:%[a-zA-Z0-9]+]] = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset
; CHECK-NEXT:   %pSB_LocalId = bitcast i8* [[GEP1]] to i32**
; CHECK-NEXT:   store i32* [[GEP0]], i32** %pSB_LocalId
; CHECK-NEXT:   br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: ret
}

; CHECK-LABEL: define void @foo
define void @foo(i32* %x) #1 {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  load i32, i32* %x
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 2)
  ret void
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
;;;; TODO: add regular expression for the below values.
; CHECK-LABEL: SyncBB1:
; CHECK-NEXT:   %SBIndex = load i32, i32* %pCurrSBIndex
; CHECK-NEXT:   %SB_LocalId_Offset = add nuw i32 %SBIndex, 4
; CHECK-NEXT:   [[GEP0:%[a-zA-Z0-9]+]] = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset
; CHECK-NEXT:   %pSB_LocalId = bitcast i8* [[GEP0]] to i32**
; CHECK-NEXT:   %loadedValue = load i32*, i32** %pSB_LocalId
; CHECK-NEXT:   load i32, i32* %loadedValue
; CHECK-NEXT:   br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: ret
}

declare void @__builtin_dpcpp_kernel_barrier(i32)
declare void @__builtin_dpcpp_kernel_barrier_dummy()

attributes #0 = { nounwind "dpcpp-no-barrier-path"="false" "sycl_kernel" }
attributes #1 = { nounwind "dpcpp-no-barrier-path"="false" }
