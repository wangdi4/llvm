; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output 2>&1 | FileCheck %s

; A test case where the parsing will be different with opaque ptrs.
; The store ref (%phi.ptr) will be parsed as something like: (ptr)(%ptr.init)[24 * i1 + 24]
; with the base ptr type of i8 extracted from %gep but then the 'bitcast dest type'
; will be applied using the store instruction storing a ptr (currently ptr).


; Element type found for the store using GEP (%gep) is i8 but we are storing a
; pointer to the location hence it gets a bitcasted type.
; CHECK: + DO i1 = 0, 168, 1   <DO_LOOP>
; CHECK: |   (ptr)(%ptr.init)[24 * i1] = &((%ptr.init)[24 * i1 + 24]);
; CHECK: + END LOOP


define void @foo() {
entry:
  %ptr.init = tail call noalias align 16 dereferenceable_or_null(4080) ptr @malloc(i64 4080)
  br label %loop

loop:                                              ; preds = %loop, %entry
  %phi.ptr = phi ptr [ %gep, %loop ], [ %ptr.init, %entry ]
  %iv = phi i64 [ %iv.inc, %loop ], [ 24, %entry ]
  %gep = getelementptr inbounds i8, ptr %ptr.init, i64 %iv
  store ptr %gep, ptr %phi.ptr, align 8
  %iv.inc = add nuw nsw i64 %iv, 24
  %cmp = icmp ult i64 %iv, 4056
  br i1 %cmp, label %loop, label %exit

exit:
 ret void
}

declare dso_local noalias noundef align 16 ptr @malloc(i64 noundef)
