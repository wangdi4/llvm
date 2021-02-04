; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s

; Check parsing output for the loop verifying that structure reference is parsed correctly.

; CHECK: + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   |   (@obj2)[0][i1].1[i2].2 = i1;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -force-hir-cg -S -instcombine | FileCheck -check-prefix=CHECK-CG %s

; Verify that CG generates the correct GEP for the reference.

; CHECK-CG: [[IV1:%[0-9].*]] = load i64, i64* %i1.i64
; CHECK-CG: [[IV2:%[0-9].*]] = load i64, i64* %i2.i64
; CHECK-CG: getelementptr inbounds [50 x %struct.S2], [50 x %struct.S2]* @obj2, i64 0, i64 [[IV1]], i32 1, i64 [[IV2]], i32 2


;Module Before HIR; ModuleID = 'struct_array_struct_array_offset.c'
source_filename = "struct_array_struct_array_offset.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S2 = type { float, [100 x %struct.S1] }
%struct.S1 = type { i32, i32, i32 }

@obj2 = common local_unnamed_addr global [50 x %struct.S2] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture readnone %A, i32 %n) local_unnamed_addr {
entry:
  %cmp18 = icmp sgt i32 %n, 0
  br i1 %cmp18, label %for.body3.lr.ph.preheader, label %for.end8

for.body3.lr.ph.preheader:                        ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body3.lr.ph.preheader, %for.inc6
  %indvars.iv20 = phi i64 [ %indvars.iv.next21, %for.inc6 ], [ 0, %for.body3.lr.ph.preheader ]
  %0 = trunc i64 %indvars.iv20 to i32
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.body3 ]
  %c = getelementptr inbounds [50 x %struct.S2], [50 x %struct.S2]* @obj2, i64 0, i64 %indvars.iv20, i32 1, i64 %indvars.iv, i32 2
  store i32 %0, i32* %c, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %exitcond23 = icmp eq i64 %indvars.iv.next21, %wide.trip.count
  br i1 %exitcond23, label %for.end8.loopexit, label %for.body3.lr.ph

for.end8.loopexit:                                ; preds = %for.inc6
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit, %entry
  ret void
}

