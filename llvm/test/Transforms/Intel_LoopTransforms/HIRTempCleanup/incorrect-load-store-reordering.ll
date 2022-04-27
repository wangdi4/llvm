; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup" -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s

; Verify that load is not moved past the store.

; Before this change, after we reordered the add and store instruction to
; eliminate liveout temp copy, we also incorrectly forward substituted the load
; so the loop body looked like this instead-

; + DO i1 = 0, %t239 + -2, 1   <DO_LOOP>
; |   (%t11)[i1] = %t242;
; |   %t242 = (%t11)[i1]  +  %t242;
; + END LOOP


; CHECK: Before

; CHECK: + DO i1 = 0, %t239 + -2, 1   <DO_LOOP>
; CHECK: |   %t242.out = %t242;
; CHECK: |   %t245 = (%t11)[i1];
; CHECK: |   %t242 = %t245  +  %t242;
; CHECK: |   (%t11)[i1] = %t242.out;
; CHECK: + END LOOP

; CHECK: After

; CHECK: + DO i1 = 0, %t239 + -2, 1   <DO_LOOP>
; CHECK: |   %t245 = (%t11)[i1];
; CHECK: |   (%t11)[i1] = %t242;
; CHECK: |   %t242 = %t245  +  %t242;
; CHECK: + END LOOP
 

define void @foo(i32* %t11, i64 %t239) {
entry:
  br label %t240

t240:                                              ; preds = %t240, %entry
  %t241 = phi i64 [ 1, %entry ], [ %t247, %t240 ]
  %t242 = phi i32 [ 1, %entry ], [ %t246, %t240 ]
  %t244 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %t11, i64 %t241) #27
  %t245 = load i32, i32* %t244, align 1
  %t246 = add nsw i32 %t245, %t242
  store i32 %t242, i32* %t244, align 1
  %t247 = add nuw nsw i64 %t241, 1
  %t248 = icmp eq i64 %t247, %t239
  br i1 %t248, label %t250, label %t240

t250:                                              ; preds = %t240
  %t251 = phi i32 [ %t246, %t240 ]
  ret void
}

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #10

attributes #10 = { nofree nosync nounwind readnone speculatable }

