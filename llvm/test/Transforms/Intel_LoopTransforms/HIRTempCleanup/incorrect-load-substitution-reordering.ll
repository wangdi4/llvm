; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup" -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s

; Verify that load %ld = (%t11)[%xor.phi.out] is not substituted into single use
; in store because %xor.phi is redefined after we move its definition past the load.

; Before this change, after we reordered the xor instruction past the two loads
; to eliminate liveout temp copy, we also incorrectly forward substituted the load
; so the loop body looked like this instead-

; + DO i1 = 0, %n + -2, 1   <DO_LOOP>
; |   %ld.1 = (%t11)[sext.i32.i64(%xor.phi) + 1];
; |   %xor.phi = 10  ^  %xor.phi;
; |   (%t11)[i1 + 1] = (%t11)[%xor.phi];
; + END LOOP


; CHECK: Before

; CHECK: + DO i1 = 0, %n + -2, 1   <DO_LOOP>
; CHECK: |   %xor.phi.out = %xor.phi;
; CHECK: |   %xor.phi = 10  ^  %xor.phi;
; CHECK: |   %ld = (%t11)[%xor.phi.out];
; CHECK: |   %ld.1 = (%t11)[sext.i32.i64(%xor.phi.out) + 1];
; CHECK: |   (%t11)[i1 + 1] = %ld;
; CHECK: + END LOOP

; CHECK: After

; CHECK: + DO i1 = 0, %n + -2, 1   <DO_LOOP>
; CHECK: |   %ld = (%t11)[%xor.phi];
; CHECK: |   %ld.1 = (%t11)[sext.i32.i64(%xor.phi) + 1];
; CHECK: |   %xor.phi = 10  ^  %xor.phi;
; CHECK: |   (%t11)[i1 + 1] = %ld;
; CHECK: + END LOOP
 

define void @foo(ptr %t11, i64 %n) {
entry:
  br label %t240

t240:                                              ; preds = %t240, %entry
  %iv = phi i64 [ 1, %entry ], [ %iv.inc, %t240 ]
  %xor.phi = phi i32 [ 1, %entry ], [ %xor.i, %t240 ]
  %sext = sext i32 %xor.phi to i64
  %subs = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %t11, i64 %sext) #27
  %xor.i = xor i32 10, %xor.phi
  %ld = load i32, ptr %subs, align 1
  %sext.1 = add i64 %sext, 1 
  %subs.1 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %t11, i64 %sext.1) #27
  %ld.1 = load i32, ptr %subs.1, align 1
  %subs2 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %t11, i64 %iv) #27
  store i32 %ld, ptr %subs2, align 1
  %iv.inc = add nuw nsw i64 %iv, 1
  %t248 = icmp eq i64 %iv.inc, %n
  br i1 %t248, label %t250, label %t240

t250:                                              ; preds = %t240
  %t251 = phi i32 [ %xor.i, %t240 ]
  %ld.out = phi i32 [ %ld.1, %t240 ]
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #10

attributes #10 = { nofree nosync nounwind readnone speculatable }

