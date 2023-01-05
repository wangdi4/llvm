; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-opt-predicate,print<hir>" -disable-output %s 2>&1 | FileCheck %s

; This test case checks that the Switch instruction inside the default case
; of the outer switch is not transformed since it reached the max threshold.
; It was created from the following test case:

; void foo(int *arr, int n, int size, int size2, int n2) {
;   for(int i = 0; i < size2; i++) {
;     switch (n) {
;       case (50):
;         arr[i] = i + 1;
;         break;
;
;       case(20):
;         arr[i] = i + 2;
;         break;
;
;       case(30):
;         arr[i] = i + 3;
;         break;
;
;       case(40):
;         arr[i] = i + 4;
;         break;
;
;       default:
;         switch (n2) {
;           case(10):
;             arr[i] = 100;
;             break;
;
;           case(20):
;             arr[i] = 0;
;             break;
;
;           default:
;             arr[i] = 1;
;             break;
;         }
;       break;
;     }
;   }
; }

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%size2) + -1, 1
;       |   switch(%n)
;       |   {
;       |   case 50:
;       |      (%arr)[i1] = i1 + 1;
;       |      break;
;       |   case 20:
;       |      (%arr)[i1] = i1 + 2;
;       |      break;
;       |   case 30:
;       |      (%arr)[i1] = i1 + 3;
;       |      break;
;       |   case 40:
;       |      (%arr)[i1] = i1 + 4;
;       |       break;
;       |   default:
;       |     switch(%n2)
;       |     {
;       |      case 10:
;       |         (%arr)[i1] = 100;
;       |         break;
;       |      case 20:
;       |         (%arr)[i1] = 0;
;       |         break;
;       |      default:
;       |         (%arr)[i1] = 1;
;       |         break;
;       |      }
;       |      break;
;       |   }
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       switch(%n)
; CHECK:       {
; CHECK:       case 40:
; CHECK:          + DO i1 = 0, zext.i32.i64(%size2) + -1, 1
; CHECK:          |   (%arr)[i1] = i1 + 4;
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       case 30:
; CHECK:          + DO i1 = 0, zext.i32.i64(%size2) + -1, 1
; CHECK:          |   (%arr)[i1] = i1 + 3;
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       case 20:
; CHECK:          + DO i1 = 0, zext.i32.i64(%size2) + -1, 1
; CHECK:          |   (%arr)[i1] = i1 + 2;
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       case 50:
; CHECK:          + DO i1 = 0, zext.i32.i64(%size2) + -1, 1
; CHECK:          |   (%arr)[i1] = i1 + 1;
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       default:
; CHECK:          + DO i1 = 0, zext.i32.i64(%size2) + -1, 1
; CHECK:          |   switch(%n2)
; CHECK:          |   {
; CHECK:          |   case 10:
; CHECK:          |      (%arr)[i1] = 100;
; CHECK:          |      break;
; CHECK:          |   case 20:
; CHECK:          |      (%arr)[i1] = 0;
; CHECK:          |      break;
; CHECK:          |   default:
; CHECK:          |      (%arr)[i1] = 1;
; CHECK:          |      break;
; CHECK:          |   }
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       }
; CHECK: END REGION

; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-opt-predicate,print<hir>" -hir-opt-predicate-use-reduced-switch-cost=true -disable-output  %s 2>&1 | FileCheck %s --check-prefix CHECKOFF

; This test case checks that the Switch instruction inside the default case
; of the outer switch was transformed since the threshold is disabled.

; CHECKOFF:  switch(%n2)
; CHECKOFF:  {
; CHECKOFF:  case 20:
; CHECKOFF:     + DO i1 = 0, zext.i32.i64(%size2) + -1, 1
; CHECKOFF:     |   (%arr)[i1] = 0;
; CHECKOFF:     + END LOOP
; CHECKOFF:     break;
; CHECKOFF:  case 10:
; CHECKOFF:     + DO i1 = 0, zext.i32.i64(%size2) + -1, 1
; CHECKOFF:     |   (%arr)[i1] = 100;
; CHECKOFF:     + END LOOP
; CHECKOFF:     break;
; CHECKOFF:  default:
; CHECKOFF:     + DO i1 = 0, zext.i32.i64(%size2) + -1, 1
; CHECKOFF:     |   (%arr)[i1] = 1;
; CHECKOFF:     + END LOOP
; CHECKOFF:     break;
; CHECKOFF:  }


;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind writeonly uwtable
define dso_local void @_Z3fooPiiiii(i32* nocapture noundef writeonly %arr, i32 noundef %n, i32 noundef %size, i32 noundef %size2, i32 noundef %n2){
entry:
  %cmp42 = icmp sgt i32 %size2, 0
  br i1 %cmp42, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %size2 to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  switch i32 %n, label %sw.default [
    i32 50, label %sw.bb
    i32 20, label %sw.bb1
    i32 30, label %sw.bb5
    i32 40, label %sw.bb9
  ]

sw.bb:                                            ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  %1 = add i32 %0, 1
  store i32 %1, i32* %arrayidx, align 4
  br label %for.inc

sw.bb1:                                           ; preds = %for.body
  %arrayidx4 = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv to i32
  %3 = add i32 %2, 2
  store i32 %3, i32* %arrayidx4, align 4
  br label %for.inc

sw.bb5:                                           ; preds = %for.body
  %arrayidx8 = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %4 = trunc i64 %indvars.iv to i32
  %5 = add i32 %4, 3
  store i32 %5, i32* %arrayidx8, align 4
  br label %for.inc

sw.bb9:                                           ; preds = %for.body
  %arrayidx12 = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %6 = trunc i64 %indvars.iv to i32
  %7 = add i32 %6, 4
  store i32 %7, i32* %arrayidx12, align 4
  br label %for.inc

sw.default:                                       ; preds = %for.body
  switch i32 %n2, label %sw.default19 [
    i32 10, label %sw.bb13
    i32 20, label %sw.bb16
  ]

sw.bb13:                                          ; preds = %sw.default
  %arrayidx15 = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  store i32 100, i32* %arrayidx15, align 4
  br label %for.inc

sw.bb16:                                          ; preds = %sw.default
  %arrayidx18 = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  store i32 0, i32* %arrayidx18, align 4
  br label %for.inc

sw.default19:                                     ; preds = %sw.default
  %arrayidx21 = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  store i32 1, i32* %arrayidx21, align 4
  br label %for.inc

for.inc:                                          ; preds = %sw.bb, %sw.bb1, %sw.bb5, %sw.bb9, %sw.default19, %sw.bb16, %sw.bb13
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}