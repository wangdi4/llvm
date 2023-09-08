; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' < %s 2>&1 | FileCheck %s

; LIT test to verify that we correctly handle a conditional assignment to a private
; inside an inner loop where the loops may not be in LCSSA form. Such loops can result
; from HIR transforms which do not guarantee that LCSSA form will be preserved. Currently
; we do not apply LCSSA transform to inner loops and the entities lowering code incorrectly
; tries to change the conditional last private to NonLast private which causes problems
; later. The fix avoids changing the private kind. For the longer term we may want to consider
; running LCSSA transform for inner loops as well.
;
; Incoming HIR:
;
;      + DO i1 = 0, 15, 1   <DO_LOOP> <simd>
;      |   %0 = (%arr1)[i1];
;      |
;      |   + DO i2 = 0, 7, 1   <DO_LOOP>
;      |   |   %cp.017 = (i1 + i2 > %0) ? %0 : %cp.017;
;      |   + END LOOP
;     + END LOOP
;
; Relevant VPlan dump after plain CFG is shown below. The conditional private
; exit instruction %vp61004 ends up with 3 uses vs the expected 2.
;
;   BB3: # preds: BB2, BB6
;    i64 %vp61986 = phi  [ i64 %cp.017, BB2 ],  [ i64 %vp61004, BB6 ]
;    i64 %vp52314 = phi  [ i64 0, BB2 ],  [ i64 %vp61546, BB6 ]
;    ptr %vp52224 = subscript inbounds ptr %arr1 i64 %vp52314
;    i64 %vp60038 = load ptr %vp52224
;    br BB4
;
;   BB4: # preds: BB3
;    br BB5
;
;   BB5: # preds: BB4, BB5
;    i64 %vp60442 = phi  [ i64 %vp61986, BB4 ],  [ i64 %vp61004, BB5 ]
;    i64 %vp23730 = phi  [ i64 0, BB4 ],  [ i64 %vp61070, BB5 ]
;    i64 %vp60382 = add i64 %vp52314 i64 %vp23730
;    i1 %vp60976 = icmp sgt i64 %vp60382 i64 %vp60038
;    i64 %vp61004 = select i1 %vp60976 i64 %vp60038 i64 %vp60442
;    i64 %vp61070 = add i64 %vp23730 i64 1
;    i1 %vp61260 = icmp slt i64 %vp61070 i64 8
;    br i1 %vp61260, BB5, BB6
;
;   BB6: # preds: BB5
;    i64 %vp61546 = add i64 %vp52314 i64 1
;    i1 %vp61620 = icmp slt i64 %vp61546 i64 16
;    br i1 %vp61620, BB3, BB7
;
;   BB7: # preds: BB6
;    br BB8
;
;   BB8: # preds: BB7
;    br <External Block>
;
; External Uses:
; Id: 0   i64 %vp61004 -> %vp29648 = {%cp.017}
;
; Check that we do not crash during vectorization
; CHECK:  DO i1 = 0, 15, 8   <DO_LOOP> <simd-vectorized>
;
define i64 @foo(ptr noalias nocapture noundef readonly %arr1) {
entry:
  %cp.lpriv = alloca i64, align 8
  store i64 0, ptr %cp.lpriv, align 8
  br label %entry.split

entry.split:
  %tok= call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LASTPRIVATE:CONDITIONAL.TYPED"(ptr %cp.lpriv, i64 0, i32 1) ]
  %cp.lpriv.val = load i64, ptr %cp.lpriv, align 8
  br label %for.body

for.body:
  %l1.018 = phi i64 [ 0, %entry.split ], [ %inc6, %for.end ]
  %cp.017 = phi i64 [ %cp.lpriv.val, %entry.split ], [ %1, %for.end ]
  %arrayidx = getelementptr inbounds i64, ptr %arr1, i64 %l1.018
  %0 = load i64, ptr %arrayidx, align 8
  br label %for.body3

for.body3:
  %l2.016 = phi i64 [ 0, %for.body ], [ %inc, %for.body3 ]
  %cp.115 = phi i64 [ %cp.017, %for.body ], [ %1, %for.body3 ]
  %add = add nuw nsw i64 %l2.016, %l1.018
  %cmp4 = icmp sgt i64 %add, %0
  %1 = select i1 %cmp4, i64 %0, i64 %cp.115
  %inc = add nuw nsw i64 %l2.016, 1
  %exitcond.not = icmp eq i64 %inc, 8
  br i1 %exitcond.not, label %for.end, label %for.body3

for.end:
  %inc6 = add nuw nsw i64 %l1.018, 1
  %exitcond19.not = icmp eq i64 %inc6, 16
  br i1 %exitcond19.not, label %for.end7, label %for.body

for.end7:
  store i64 %1, ptr %cp.lpriv, align 8
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret i64 %1
}


declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
