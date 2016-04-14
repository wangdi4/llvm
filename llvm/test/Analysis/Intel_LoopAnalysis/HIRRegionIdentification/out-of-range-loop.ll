; REQUIRES: asserts
; RUN: opt < %s -analyze -hir-region-identification -debug 2>&1 | FileCheck %s

; Check that we do not form Region for this case. The trip count of this loop is 2 (due to SSA quirk) which is outisde the i1 type range. CG generates an infinite loop for this case.
; CHECK-NOT: Region 1


; Function Attrs: nounwind uwtable
define i32 @main() {
for.body.lr.ph.i:
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.lr.ph.i
  %i.08.i = phi i1 [ true, %for.body.lr.ph.i ], [ false, %for.body.i ]
  br i1 %i.08.i, label %for.body.i, label %cmppt3.exit

cmppt3.exit:                                      ; preds = %for.body.i
  ret i32 0
}
