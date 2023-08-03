; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

; CHECK: Function: _Z14sdiv_neg_denomPll
; CHECK:        + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:   |   (<4 x i64>*)(%arr)[i1] = (-1 * i1 + -1 * %n + -1 * <i64 0, i64 1, i64 2, i64 3>)/100;
; CHECK-NEXT:   + END LOOP

; CHECK: Function: _Z16udiv_large_denomPll
; CHECK:        + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:   |   %.vec = i1 + %n + <i64 0, i64 1, i64 2, i64 3> /u -9223372036854775807;
; CHECK-NEXT:   |   (<4 x i64>*)(%arr)[i1] = %.vec;
; CHECK-NEXT:   + END LOOP

; CHECK: Function: _Z20sdiv_small_neg_denomPll
; CHECK:        + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:   |   %.vec = i1 + %n + <i64 0, i64 1, i64 2, i64 3>  /u  -9223372036854775808;
; CHECK-NEXT:   |   (<4 x i64>*)(%arr)[i1] = %.vec;
; CHECK-NEXT:   + END LOOP

; CHECK: Function: _Z19sdiv_maxvalue_denomPll
; CHECK:        + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:   |   (<4 x i64>*)(%arr)[i1] = (i1 + %n + <i64 0, i64 1, i64 2, i64 3>)/9223372036854775807;
; CHECK-NEXT:   + END LOOP

; CHECK: Function: _Z17sdiv_32bmin_denomPii
; CHECK:        + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:   |   (<4 x i32>*)(%arr)[i1] = (-1 * i1 + -1 * %n + -1 * <i64 0, i64 1, i64 2, i64 3>)/2147483648;
; CHECK-NEXT:   + END LOOP

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: write) uwtable
define dso_local void @_Z14sdiv_neg_denomPll(ptr nocapture noundef writeonly %arr, i64 noundef %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %i.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nsw i64 %i.05, %n
  %div = sdiv i64 %add, -100
  %arrayidx = getelementptr inbounds i64, ptr %arr, i64 %i.05
  store i64 %div, ptr %arrayidx, align 8 
  %inc = add nuw nsw i64 %i.05, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}
; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: write) uwtable
define dso_local void @_Z16udiv_large_denomPll(ptr nocapture noundef writeonly %arr, i64 noundef %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %i.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nsw i64 %i.05, %n
  %div = udiv i64 %add, 9223372036854775809
  %arrayidx = getelementptr inbounds i64, ptr %arr, i64 %i.05
  store i64 %div, ptr %arrayidx, align 8
  %inc = add nuw nsw i64 %i.05, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: write) uwtable
define dso_local void @_Z20sdiv_small_neg_denomPll(ptr nocapture noundef writeonly %arr, i64 noundef %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %i.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nsw i64 %i.05, %n
  %add.lobit = lshr i64 %add, 63
  %arrayidx = getelementptr inbounds i64, ptr %arr, i64 %i.05
  store i64 %add.lobit, ptr %arrayidx, align 8 
  %inc = add nuw nsw i64 %i.05, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: write) uwtable
define dso_local void @_Z19sdiv_maxvalue_denomPll(ptr nocapture noundef writeonly %arr, i64 noundef %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %i.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nsw i64 %i.05, %n
  %div = sdiv i64 %add, 9223372036854775807
  %arrayidx = getelementptr inbounds i64, ptr %arr, i64 %i.05
  store i64 %div, ptr %arrayidx, align 8 
  %inc = add nuw nsw i64 %i.05, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: write) uwtable
define dso_local void @_Z17sdiv_32bmin_denomPii(ptr nocapture noundef writeonly %arr, i64 noundef %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %add = add nsw i64 %indvars.iv, %n
  %div = sdiv i64 %add, -2147483648
  %conv1 = trunc i64 %div to i32
  %arrayidx = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv
  store i32 %conv1, ptr %arrayidx, align 4 
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}
