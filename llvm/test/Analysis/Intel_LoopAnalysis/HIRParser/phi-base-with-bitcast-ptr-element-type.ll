; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes="hir-ssa-deconstruction,print<hir-framework>" 2>&1 | FileCheck %s --check-prefix=CHECK-OPAQUE

; A test case where the parsing will be different with opaque ptrs.
; The store ref (%phi.ptr) will be parsed as something like: (ptr)(%ptr.init)[24 * i1 + 24]
; with the base ptr type of i8 extracted from %gep but then the 'bitcast dest type'
; will be applied using the store instruction storing a ptr (currently i8*).

; CHECK: + DO i1 = 0, 168, 1   <DO_LOOP>
; CHECK: |   (%bc1)[3 * i1] = &((%ptr.init)[24 * i1 + 24]);
; CHECK: + END LOOP

; Element type found for the store using GEP (%gep) is i8 but we are storing a
; pointer to the location hence it gets a bitcasted type.
; CHECK-OPAQUE: + DO i1 = 0, 168, 1   <DO_LOOP>
; CHECK-OPAQUE: |   (ptr)(%bc1)[24 * i1] = &((%ptr.init)[24 * i1 + 24]);
; CHECK-OPAQUE: + END LOOP


define void @foo() {
entry:
  %ptr.init = tail call noalias align 16 dereferenceable_or_null(4080) i8* @malloc(i64 4080)
  %bc1 = bitcast i8* %ptr.init to i8**
  br label %loop

loop:                                              ; preds = %loop, %entry
  %phi.ptr = phi i8** [ %bc2, %loop ], [ %bc1, %entry ]
  %iv = phi i64 [ %iv.inc, %loop ], [ 24, %entry ]
  %gep = getelementptr inbounds i8, i8* %ptr.init, i64 %iv
  store i8* %gep, i8** %phi.ptr, align 8
  %iv.inc = add nuw nsw i64 %iv, 24
  %cmp = icmp ult i64 %iv, 4056
  %bc2 = bitcast i8* %gep to i8**
  br i1 %cmp, label %loop, label %exit

exit:
 ret void
}

declare dso_local noalias noundef align 16 i8* @malloc(i64 noundef)
