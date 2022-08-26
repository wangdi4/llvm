; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec  -vplan-force-vf=4 -vplan-enable-peeling -hir-details < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 -vplan-enable-peeling -hir-details < %s 2>&1 | FileCheck %s
;
; LIT test to show issue with ref consistency when the peeled memref has a nested
; blob that is defined in the parent loop of the loop being vectorized. When creating
; the widened ref, we were using the original loop's nesting level causing HIR
; verification errors.
;
; CHECK:                    + DO i64 i1 = 0, 1023, 1   <DO_LOOP>
; CHECK-NEXT:               |   [[TMP0:%.*]] = (%off)[i1];

; CHECK:                    |   [[TMP1:%.*]] = (%lpp)[i1];

; CHECK:                    |   %.vec = ptrtoint.<4 x i64*>.<4 x i64>(&((<4 x i64*>)([[TMP1]])[sext.i32.i64([[TMP0]])]));
; CHECK-NEXT:               |   <LVAL-REG> NON-LINEAR <4 x i64> %.vec {sb:22}
; CHECK-NEXT:               |   <RVAL-REG> &((<4 x i64*>)(NON-LINEAR <4 x i64*> [[TMP1]])[NON-LINEAR <4 x i64> sext.i32.i64([[TMP0]])]) inbounds
; CHECK-NEXT:               |      <BLOB> NON-LINEAR i32 [[TMP0]]
; CHECK-NEXT:               |      <BLOB> NON-LINEAR i64* [[TMP1]]
;
define  void @baz(i64**  %lpp, i32* %off) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.end
  %l1.021 = phi i64 [ 0, %entry ], [ %inc9, %for.end ]
  %arrayidx = getelementptr inbounds i32, i32* %off, i64 %l1.021
  %0 = load i32, i32* %arrayidx, align 4
  %conv = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds i64*, i64** %lpp, i64 %l1.021
  %1 = load i64*, i64** %arrayidx2, align 8
  br label %for.body6

for.body6:                                        ; preds = %for.body, %for.body6
  %l2.020 = phi i64 [ 0, %for.body ], [ %inc, %for.body6 ]
  %add = add nsw i64 %l2.020, %conv
  %arrayidx7 = getelementptr inbounds i64, i64* %1, i64 %add
  store i64 %l2.020, i64* %arrayidx7, align 8
  %inc = add nuw nsw i64 %l2.020, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body6

for.end:                                          ; preds = %for.body6
  %inc9 = add nuw nsw i64 %l1.021, 1
  %exitcond22.not = icmp eq i64 %inc9, 1024
  br i1 %exitcond22.not, label %for.end10, label %for.body

for.end10:                                        ; preds = %for.end
  ret void
}
