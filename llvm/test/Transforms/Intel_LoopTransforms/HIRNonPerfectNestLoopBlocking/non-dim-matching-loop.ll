; RUN: opt -intel-libirc-allowed -hir-non-perfect-nest-loop-blocking-stripmine-size=2 --passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-non-perfect-nest-loop-blocking,print<hir>" -disable-hir-non-perfect-nest-loop-blocking=false -disable-output 2>&1 < %s | FileCheck %s

; Verify that i2-loops are stripmined by 2 and their byStripLoops are placed
; outside of i1-loop.
;
; This is a type of inter loop blocking with the existence of
; i2-level sibling loop. Notice that i2-loop matches with the dimension [i2] of
; memrefs (%1)[i2].1 whereas i2-loop's indunction variable doesn't appear in any
; dimension. That makes i2-loop a non-dimension-matcing loop.
;
; For the loads/stores of memrefs in (%1)[i2].1, loop blocking of i2-loop doesn't violate
; dependence. For the ease of understanding, ignore temps for now.
; For a given i2 value, (%1)[i2].1 reads or writes are not dependent on other
; iteration value of i2 because the index of array is i2's val.
; Intra-iteration dependence among reads/writes on (%1)[i2].1 within i2-loop is
; preserved by loop blocking of i2-loop.
; Inter-iteration dependence over i1-loop's iterations among reads/writes on (%1)[i2].1
; is preserved. For a given i2-value, i1 iteration values order doesn't change.
; Example:
; Direction of flow dependence of (%1)[i2].1 before loop blocking of i2.
;    i1 iters  i2 iters
;    1         1 --> to (2, 1) // (i1, i2)
;              2 --> to (2, 2)
;              3 --> to (2, 3)
;              4 --> to (2, 4)
;
;    2         1
;              2
;              3
;              4
;
; Direction of flow dependence of (%1)[i2].1 before loop blocking of i2 by 2.
;    i1 iters  i2 iters
;    1         1 --> to (2, 1) // (i1, i2)
;              2 --> to (2, 2)
;    2         1
;              2
;    1         3 --> to (2, 3)
;              4 --> to (2, 4)
;    2         3
;              4

; Before transformation

; CHECK: Function: test_sum.for.body85
;           BEGIN REGION { }
; CHECK:          + DO i1 = 0, %0 + -1 * smin(1, %0), 1   <DO_LOOP>
;                 |   %shl86 = 1  <<  -1 * i1 + %0;
;                 |   %and87 = %shl86  &  %compare;
;                 |   if (%and87 == 0)
;                 |   {
;                 |      if (%cmp93785 != 0)
;                 |      {
;                 |         %shl173 = 1  <<  -1 * i1 + trunc.i64.i32(%0) + %width;
;                 |         %shl185 = 2  <<  -1 * i1 + %0;
;                 |         %5 = freeze(%shl173);
;                 |         %6 = %5  |  %shl185;
;                 |         %shl200 = 1  <<  -1 * i1 + %0;
;                 |
; CHECK:          |         + DO i2 = 0, %wide.trip.count821 + -1, 1   <DO_LOOP>
;                 |         |   %xor178 = (%1)[i2].1  ^  %5;
;                 |         |   %8 = %xor178  &  %6;
;                 |         |   %xor205 = (%8 == %6) ? %shl200 : 0;
;                 |         |   %9 = %xor178  ^  %xor205;
;                 |         |   (%1)[i2].1 = %9;
;                 |         + END LOOP
;                 |      }
;                 |   }
;                 |   else
;                 |   {
;                 |      if (%cmp93785 != 0)
;                 |      {
;                 |         %shl102 = 2  <<  -1 * i1 + %0;
;                 |         %shl127 = 1  <<  -1 * i1 + trunc.i64.i32(%0) + %width;
;                 |         %3 = freeze(%shl127);
;                 |
;  CHECK:         |         + DO i2 = 0, %wide.trip.count821 + -1, 1   <DO_LOOP>
;                 |         |   %xor132 = (%1)[i2].1  ^  %3;
;                 |         |   %and140 = %xor132  &  %shl102;
;                 |         |   %tobool141.not = %and140 != 0;
;                 |         |   %and150 = %xor132  &  %3;
;                 |         |   %tobool151.not = %and150 != 0;
;                 |         |   %or.cond.not = %tobool141.not  &  %tobool151.not;
;                 |         |   %spec.select774 = %xor132  ^  %or.cond.not;
;                 |         |   (%1)[i2].1 = %spec.select774;
;                 |         + END LOOP
;                 |      }
;                 |   }
;                 + END LOOP
;           END REGION

; After transformation

; CHECK: Function: test_sum.for.body85

;        BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, (-1 + %wide.trip.count821), 2   <DO_LOOP>
; CHECK:       |   %tile_e_min = (i1 + 1 <= (-1 + %wide.trip.count821)) ? i1 + 1 : (-1 + %wide.trip.count821);
;              |
; CHECK:       |   + DO i2 = 0, %0 + -1 * smin(1, %0), 1   <DO_LOOP>
;              |   |   %shl86 = 1  <<  -1 * i2 + %0;
;              |   |   %and87 = %shl86  &  %compare;
;              |   |   if (%and87 == 0)
;              |   |   {
;              |   |      if (%cmp93785 != 0)
;              |   |      {
;              |   |         %shl173 = 1  <<  -1 * i2 + trunc.i64.i32(%0) + %width;
;              |   |         %shl185 = 2  <<  -1 * i2 + %0;
;              |   |         %5 = freeze(%shl173);
;              |   |         %6 = %5  |  %shl185;
;              |   |         %shl200 = 1  <<  -1 * i2 + %0;
; CHECK:       |   |         %lb_max = (0 <= i1) ? i1 : 0;
; CHECK:       |   |         %ub_min = (%wide.trip.count821 + -1 <= %tile_e_min) ? %wide.trip.count821 + -1 : %tile_e_min;
;              |   |
; CHECK:       |   |         + DO i3 = 0, -1 * %lb_max + %ub_min, 1   <DO_LOOP>
;              |   |         |   %xor178 = (%1)[i3 + %lb_max].1  ^  %5;
;              |   |         |   %8 = %xor178  &  %6;
;              |   |         |   %xor205 = (%8 == %6) ? %shl200 : 0;
;              |   |         |   %9 = %xor178  ^  %xor205;
;              |   |         |   (%1)[i3 + %lb_max].1 = %9;
;              |   |         + END LOOP
;              |   |      }
;              |   |   }
;              |   |   else
;              |   |   {
;              |   |      if (%cmp93785 != 0)
;              |   |      {
;              |   |         %shl102 = 2  <<  -1 * i2 + %0;
;              |   |         %shl127 = 1  <<  -1 * i2 + trunc.i64.i32(%0) + %width;
;              |   |         %3 = freeze(%shl127);
; CHECK:       |   |         %lb_max[[V3:[0-9]*]] = (0 <= i1) ? i1 : 0;
; CHECK:       |   |         %ub_min[[V4:[0-9]*]] = (%wide.trip.count821 + -1 <= %tile_e_min) ? %wide.trip.count821 + -1 : %tile_e_min;
;              |   |
; CHECK:       |   |         + DO i3 = 0, -1 * %lb_max[[V3]] + %ub_min[[V4]], 1   <DO_LOOP>
;              |   |         |   %xor132 = (%1)[i3 + %lb_max[[V3]]].1  ^  %3;
;              |   |         |   %and140 = %xor132  &  %shl102;
;              |   |         |   %tobool141.not = %and140 != 0;
;              |   |         |   %and150 = %xor132  &  %3;
;              |   |         |   %tobool151.not = %and150 != 0;
;              |   |         |   %or.cond.not = %tobool141.not  &  %tobool151.not;
;              |   |         |   %spec.select774 = %xor132  ^  %or.cond.not;
;              |   |         |   (%1)[i3 + %lb_max[[V3]]].1 = %spec.select774;
;              |   |         + END LOOP
;              |   |      }
;              |   |   }
;              |   + END LOOP
;              + END LOOP
;        END REGION
;

; ModuleID = 'test_sum_fused.ll'
source_filename = "oaddn-1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS23quantum_reg_node_struct.quantum_reg_node_struct = type { { float, float }, i64 }

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @test_sum.for.body85(i64 %0, i32 %compare, i1 %cmp93785, i32 %width, ptr %1, i64 %wide.trip.count821) {
newFuncRoot:
  br label %for.body85

for.body85:                                       ; preds = %newFuncRoot, %for.inc212
  %indvars.iv827 = phi i64 [ %0, %newFuncRoot ], [ %indvars.iv.next828, %for.inc212 ]
  %2 = trunc i64 %indvars.iv827 to i32
  %shl86 = shl nuw i32 1, %2
  %and87 = and i32 %shl86, %compare
  %tobool88.not = icmp eq i32 %and87, 0
  br i1 %tobool88.not, label %for.cond165.preheader, label %for.cond91.preheader

for.cond91.preheader:                             ; preds = %for.body85
  br i1 %cmp93785, label %for.body96.lr.ph, label %for.inc212

for.body96.lr.ph:                                 ; preds = %for.cond91.preheader
  %shl102 = shl i64 2, %indvars.iv827
  %add125 = add nsw i32 %2, %width
  %sh_prom126 = zext i32 %add125 to i64
  %shl127 = shl nuw i64 1, %sh_prom126
  %3 = freeze i64 %shl127
  br label %for.body96

for.body96:                                       ; preds = %for.body96, %for.body96.lr.ph
  %indvars.iv819 = phi i64 [ 0, %for.body96.lr.ph ], [ %indvars.iv.next820, %for.body96 ]
  %state100 = getelementptr inbounds %struct._ZTS23quantum_reg_node_struct.quantum_reg_node_struct, ptr %1, i64 %indvars.iv819, i32 1
  %4 = load i64, ptr %state100, align 8
  %xor132 = xor i64 %4, %3
  %and140 = and i64 %xor132, %shl102
  %tobool141.not = icmp ne i64 %and140, 0
  %and150 = and i64 %xor132, %3
  %tobool151.not = icmp ne i64 %and150, 0
  %or.cond.not = and i1 %tobool141.not, %tobool151.not
  %xor157 = zext i1 %or.cond.not to i64
  %spec.select774 = xor i64 %xor132, %xor157
  store i64 %spec.select774, ptr %state100, align 8
  %indvars.iv.next820 = add nuw nsw i64 %indvars.iv819, 1
  %exitcond822.not = icmp eq i64 %indvars.iv.next820, %wide.trip.count821
  br i1 %exitcond822.not, label %for.inc212.loopexit878, label %for.body96

for.inc212.loopexit878:                           ; preds = %for.body96
  br label %for.inc212

for.cond165.preheader:                            ; preds = %for.body85
  br i1 %cmp93785, label %for.body170.lr.ph, label %for.inc212

for.body170.lr.ph:                                ; preds = %for.cond165.preheader
  %add171 = add nsw i32 %2, %width
  %sh_prom172 = zext i32 %add171 to i64
  %shl173 = shl nuw i64 1, %sh_prom172
  %shl185 = shl i64 2, %indvars.iv827
  %5 = freeze i64 %shl173
  %6 = or i64 %5, %shl185
  %shl200 = shl nuw i64 1, %indvars.iv827
  br label %for.body170

for.body170:                                      ; preds = %for.body170, %for.body170.lr.ph
  %indvars.iv823 = phi i64 [ 0, %for.body170.lr.ph ], [ %indvars.iv.next824, %for.body170 ]
  %state177 = getelementptr inbounds %struct._ZTS23quantum_reg_node_struct.quantum_reg_node_struct, ptr %1, i64 %indvars.iv823, i32 1
  %7 = load i64, ptr %state177, align 8
  %xor178 = xor i64 %7, %5
  %8 = and i64 %xor178, %6
  %or.cond759.not = icmp eq i64 %8, %6
  %xor205 = select i1 %or.cond759.not, i64 %shl200, i64 0
  %9 = xor i64 %xor178, %xor205
  store i64 %9, ptr %state177, align 8
  %indvars.iv.next824 = add nuw nsw i64 %indvars.iv823, 1
  %exitcond826.not = icmp eq i64 %indvars.iv.next824, %wide.trip.count821
  br i1 %exitcond826.not, label %for.inc212.loopexit, label %for.body170

for.inc212.loopexit:                              ; preds = %for.body170
  br label %for.inc212

for.inc212:                                       ; preds = %for.inc212.loopexit878, %for.inc212.loopexit, %for.cond165.preheader, %for.cond91.preheader
  %cmp83 = icmp sgt i64 %indvars.iv827, 1
  %indvars.iv.next828 = add nsw i64 %indvars.iv827, -1
  br i1 %cmp83, label %for.body85, label %for.end213.loopexit.exitStub

for.end213.loopexit.exitStub:                     ; preds = %for.inc212
  ret void
}
