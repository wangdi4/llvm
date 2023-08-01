; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check that we are able to parse the loopnest successfully. It was asserting during ztt recognition.

; HIR-
; + DO i1 = 0, 63, 1   <DO_LOOP>
; |      %v_i.016 = i1;
; |   + DO i2 = 0, i1 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 63>
; |   |   if (i1 + -1 * i2 <u i1)
; |   |   {
; |   |      + DO i3 = 0, trunc.i64.i32((1 + zext.i32.i64((-1 + trunc.i64.i32(%indvars.iv))) + umax((-1 + (-1 * zext.i32.i64((-1 + trunc.i64.i32(%indvars.iv))))), (-1 + (-1 * %v_i.016))))), 1   <DO_LOOP>
; |   |      |   %1 = i1 + -1 * i3 <u 193;
; |   |      |   @llvm.assume(%1);
; |   |      |   (@a1_n)[0][i1 + -1 * i3] = 29;
; |   |      + END LOOP
; |   |   }
; |   |   %v_i.016 = i1 + -1 * i2 + -1;
; |   + END LOOP
; |
; |   %indvars.iv = i1 + 1;
; + END LOOP

; FIXME: The i3 loop is parsed as unknown after the pulldown.
; ScalarEvolution is apparently returning different results for 2 queries to get backedge count for the same loop.
; From the initial investigation, it is not clear to me whether this is a ScalarEvolution or HIR issue.
; More investigation is required.

; XFAIL: *

; CHECK: DO i3


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a1_n = local_unnamed_addr global [192 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @main() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.end10, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.end10 ]
  %cmp215 = icmp eq i64 %indvars.iv, 0
  br i1 %cmp215, label %for.end10, label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body
  %0 = trunc i64 %indvars.iv to i32
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc8
  %v_i.016 = phi i64 [ %indvars.iv, %for.body3.lr.ph ], [ %dec9, %for.inc8 ]
  %cmp612 = icmp ult i64 %v_i.016, %indvars.iv
  br i1 %cmp612, label %for.body7.lr.ph, label %for.inc8

for.body7.lr.ph:                                  ; preds = %for.body3
  br label %for.body7

for.body7:                                        ; preds = %for.body7.lr.ph, %for.body7
  %conv514 = phi i64 [ %indvars.iv, %for.body7.lr.ph ], [ %conv5, %for.body7 ]
  %i.013 = phi i32 [ %0, %for.body7.lr.ph ], [ %dec, %for.body7 ]
  %1 = icmp ult i32 %i.013, 193
  tail call void @llvm.assume(i1 %1)
  %arrayidx = getelementptr inbounds [192 x i32], ptr @a1_n, i64 0, i64 %conv514
  store i32 29, ptr %arrayidx, align 4
  %dec = add nsw i32 %i.013, -1
  %conv5 = zext i32 %dec to i64
  %cmp6 = icmp ult i64 %v_i.016, %conv5
  br i1 %cmp6, label %for.body7, label %for.inc8.loopexit

for.inc8.loopexit:                                ; preds = %for.body7
  br label %for.inc8

for.inc8:                                         ; preds = %for.inc8.loopexit, %for.body3
  %dec9 = add nsw i64 %v_i.016, -1
  %cmp2 = icmp eq i64 %dec9, 0
  br i1 %cmp2, label %for.end10.loopexit, label %for.body3

for.end10.loopexit:                               ; preds = %for.inc8
  br label %for.end10

for.end10:                                        ; preds = %for.end10.loopexit, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond, label %for.end12, label %for.body

for.end12:                                        ; preds = %for.end10
  ret i32 0
}

; Function Attrs: nounwind
declare void @llvm.assume(i1)

