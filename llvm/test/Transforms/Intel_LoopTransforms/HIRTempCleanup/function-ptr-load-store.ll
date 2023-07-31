; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>,hir-temp-cleanup,print<hir>" 2>&1 | FileCheck %s


; Verify that temp cleanup is able to forward substitute function pointer loads
; into eligible uses like stores.

; Print Before
; CHECK: Function:

; CHECK: + DO i1 = 0, 80, 1   <DO_LOOP>
; CHECK: |   %ld = (%ld.func.ptr.arr)[0][i1];
; CHECK: |   (%st.func.ptr.arr)[0][i1] = &((%ld)[0]);
; CHECK: + END LOOP


; Print After
; CHECK: Function:

; CHECK: + DO i1 = 0, 80, 1   <DO_LOOP>
; CHECK: |   (%st.func.ptr.arr)[0][i1] = (%ld.func.ptr.arr)[0][i1];
; CHECK: + END LOOP


define void @foo(ptr %ld.func.ptr.arr, ptr %st.func.ptr.arr) {
  br label %loop

loop:                                      ; preds = %loop, %0
  %iv = phi i64 [ 0, %0 ], [ %inc, %loop ]
  %ld.gep = getelementptr [81 x ptr], ptr %ld.func.ptr.arr, i64 0, i64 %iv
  %ld = load ptr, ptr %ld.gep
  %st.gep = getelementptr [81 x ptr], ptr %st.func.ptr.arr, i64 0, i64 %iv
  store ptr %ld, ptr %st.gep
  %inc = add nuw nsw i64 %iv, 1
  %cmp = icmp eq i64 %inc, 81
  br i1 %cmp, label %exit, label %loop

exit:                                      ; preds = %loop
  ret void
}

