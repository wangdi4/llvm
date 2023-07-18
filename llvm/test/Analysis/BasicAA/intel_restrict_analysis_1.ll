; It checks that basic-aa helps to detect %data, which is marked
; as noalias (restrict) argument and %twp.addr.029, which
; is a copy of another argument %twp, never overlap.
;
; RUN: opt -aa-pipeline="basic-aa" -passes="aa-eval" < %s -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=convert-to-subscript -S | opt -aa-pipeline="basic-aa" -passes="aa-eval" -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; CHECK-DAG:  NoAlias:      double* %data, double* %twp.addr.029
; CHECK-DAG:  NoAlias:      double* %data, double* %twp

define void @FFT_transform_internal(i32 %N, ptr noalias %data, i32 %direction, ptr %twp) {
entry:
  %cmp27 = icmp sgt i32 %N, 1
  br i1 %cmp27, label %for.body.lr.ph, label %for.end10

for.body.lr.ph:                                   ; preds = %entry
  %div30 = lshr i32 %N, 1
  %0 = zext i32 %div30 to i64
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.body.lr.ph, %for.end
  %twp.addr.029 = phi ptr [ %twp, %for.body.lr.ph ], [ %incdec.ptr, %for.end ]
  %a.028 = phi i32 [ 1, %for.body.lr.ph ], [ %inc9, %for.end ]
  %incdec.ptr = getelementptr inbounds double, ptr %twp.addr.029, i64 1
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds double, ptr %data, i64 %indvars.iv
  %1 = load double, ptr %arrayidx, align 8
  %arrayidx5 = getelementptr inbounds double, ptr %incdec.ptr, i64 %indvars.iv
  %2 = load double, ptr %arrayidx5, align 8
  %add = fadd double %1, %2
  store double %add, ptr %arrayidx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1

; dead loads, needed to get aa-eval to trigger
  %ld.data = load double, ptr %data, align 8
  %ld.twp.addr.029 = load double, ptr %twp.addr.029, align 8
  %ld.twp = load double, ptr %twp, align 8

  %cmp2 = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp2, label %for.body3, label %for.end

for.end:                                          ; preds = %for.body3
  %inc9 = add nuw nsw i32 %a.028, 1
  %exitcond = icmp eq i32 %inc9, %N
  br i1 %exitcond, label %for.end10, label %for.body3.preheader

for.end10:                                        ; preds = %for.end, %entry
  ret void
}
