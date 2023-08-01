; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -hir-details-dims -disable-output < %s 2>&1 | FileCheck %s
;
;
; *** Source Code ***
;int foo(unsigned P, unsigned Q) {
;  int A[P][100][Q]; // Declare a variable length array
;  unsigned i, j, k;
;
;  for (i = 0; i < P; ++i) {
;    for (j = 0; j < 100; ++j) {
;      for (k = 0; k < Q; ++k) {
;        A[i][j][k] = A[i][j][k] + 1; // can collapse, notice the perfect loopnest has P-100-Q trip count
;      }
;    }
;  }
;
;  return A[0][1][2] + 1;
;}
;
; CHECK: Function
;
; CHECK:     + DO i1 = 0, zext.i32.i64(%P) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:     |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   |   + DO i3 = 0, zext.i32.i64(%Q) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:     |   |   |   %7 = (%vla)[0:100 * zext.i32.i64(%Q) * i1 + zext.i32.i64(%Q) * i2 + i3:4(i32:0)];
; CHECK:     |   |   |   (%vla)[0:100 * zext.i32.i64(%Q) * i1 + zext.i32.i64(%Q) * i2 + i3:4(i32:0)] = %7 + 1;
; CHECK:     |   |   + END LOOP
; CHECK:     |   + END LOOP
; CHECK:     + END LOOP
;
;
; CHECK: Function
;
; CHECK:     + DO i1 = 0, 100 * (zext.i32.i64(%P) * zext.i32.i64(%Q)) + -1, 1   <DO_LOOP>
; CHECK:     |   %7 = (%vla)[0:i1:4(i32:0)];
; CHECK:     |   (%vla)[0:i1:4(i32:0)] = %7 + 1;
; CHECK:     + END LOOP
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readnone uwtable
define i32 @foo(i32 %P, i32 %Q) local_unnamed_addr #0 {
entry:
  %0 = zext i32 %P to i64
  %1 = zext i32 %Q to i64
  %2 = mul nuw nsw i64 %0, 100
  %3 = mul nuw i64 %2, %1
  %vla = alloca i32, i64 %3, align 16
  %cmp44 = icmp eq i32 %P, 0
  br i1 %cmp44, label %for.end22, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp541 = icmp eq i32 %Q, 0
  %4 = mul nuw nsw i64 %1, 100
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc20, %for.cond1.preheader.lr.ph
  %indvars.iv49 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next50, %for.inc20 ]
  %5 = mul nsw i64 %4, %indvars.iv49
  %arrayidx = getelementptr inbounds i32, ptr %vla, i64 %5
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc17, %for.cond1.preheader
  %indvars.iv46 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next47, %for.inc17 ]
  br i1 %cmp541, label %for.inc17, label %for.body6.lr.ph

for.body6.lr.ph:                                  ; preds = %for.cond4.preheader
  %6 = mul nuw nsw i64 %indvars.iv46, %1
  %arrayidx8 = getelementptr inbounds i32, ptr %arrayidx, i64 %6
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body6.lr.ph ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds i32, ptr %arrayidx8, i64 %indvars.iv
  %7 = load i32, ptr %arrayidx10, align 4, !tbaa !1
  %add = add nsw i32 %7, 1
  store i32 %add, ptr %arrayidx10, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %1
  br i1 %exitcond, label %for.inc17.loopexit, label %for.body6

for.inc17.loopexit:                               ; preds = %for.body6
  br label %for.inc17

for.inc17:                                        ; preds = %for.inc17.loopexit, %for.cond4.preheader
  %indvars.iv.next47 = add nuw nsw i64 %indvars.iv46, 1
  %exitcond48 = icmp eq i64 %indvars.iv.next47, 100
  br i1 %exitcond48, label %for.inc20, label %for.cond4.preheader

for.inc20:                                        ; preds = %for.inc17
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1
  %exitcond52 = icmp eq i64 %indvars.iv.next50, %0
  br i1 %exitcond52, label %for.end22.loopexit, label %for.cond1.preheader

for.end22.loopexit:                               ; preds = %for.inc20
  %arrayidx24.phi.trans.insert = getelementptr inbounds i32, ptr %vla, i64 %1
  %arrayidx25.phi.trans.insert = getelementptr inbounds i32, ptr %arrayidx24.phi.trans.insert, i64 2
  %.pre = load i32, ptr %arrayidx25.phi.trans.insert, align 4, !tbaa !1
  %phitmp = add i32 %.pre, 1
  br label %for.end22

for.end22:                                        ; preds = %entry, %for.end22.loopexit
  %8 = phi i32 [ %phitmp, %for.end22.loopexit ], [ undef, %entry ]
  ret i32 %8
}

; Function Attrs: nounwind
declare ptr @llvm.stacksave() #1

; Function Attrs: nounwind
declare void @llvm.stackrestore(ptr) #1

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21400) (llvm/branches/loopopt 21445)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
