; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s


;
; Check folding of sext/zext/trunc into a canon expression. A load of an
; unsigned int is stored into an unsigned long. A load of a signed int is
; stored into a signed long and finally an unsigned int is stored to an
; unsigned short. The test expects zext/sext/trunc to be folded into the canon
; expression without seeing explicit instructions for the same.
;
; CHECK:       DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:    %.vec = (<4 x i32>*)(@uintarr)[0][i1];
; CHECK-NEXT:    (<4 x i64>*)(@ulongarr)[0][i1] = %.vec;
; CHECK-NEXT:    %.vec1 = (<4 x i32>*)(@sintarr)[0][i1];
; CHECK-NEXT:    (<4 x i64>*)(@slongarr)[0][i1] = %.vec1;
; CHECK-NEXT:    (<4 x i16>*)(@ushortarr)[0][i1] = %.vec;
; CHECK-NEXT:  END LOOP
;
@uintarr = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@ulongarr = dso_local local_unnamed_addr global [100 x i64] zeroinitializer, align 16
@sintarr = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@slongarr = dso_local local_unnamed_addr global [100 x i64] zeroinitializer, align 16
@ushortarr = dso_local local_unnamed_addr global [100 x i16] zeroinitializer, align 16

define void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.016 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @uintarr, i64 0, i64 %l1.016
  %0 = load i32, i32* %arrayidx, align 4
  %zextval = zext i32 %0 to i64
  %arrayidx1 = getelementptr inbounds [100 x i64], [100 x i64]* @ulongarr, i64 0, i64 %l1.016
  store i64 %zextval, i64* %arrayidx1, align 8
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @sintarr, i64 0, i64 %l1.016
  %1 = load i32, i32* %arrayidx2, align 4
  %sextval = sext i32 %1 to i64
  %arrayidx4 = getelementptr inbounds [100 x i64], [100 x i64]* @slongarr, i64 0, i64 %l1.016
  store i64 %sextval, i64* %arrayidx4, align 8
  %truncval = trunc i32 %0 to i16
  %arrayidx7 = getelementptr inbounds [100 x i16], [100 x i16]* @ushortarr, i64 0, i64 %l1.016
  store i16 %truncval, i16* %arrayidx7, align 2
  %inc = add nuw nsw i64 %l1.016, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
