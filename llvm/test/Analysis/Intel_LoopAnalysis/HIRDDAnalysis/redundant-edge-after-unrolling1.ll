; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -hir-dd-analysis -analyze -enable-new-pm=0 -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s

; Verify that we do not create redundant flow edge for %0 from <4> to <22>
; because the definition on <21> kills it.

; CHECK-NOT: 4:22 %0 --> %0 FLOW

; HIR after unrolling inner loop-
; <38>               + DO i1 = 0, 24, 1   <DO_LOOP> <nounroll>
; <3>                |   (@reg_class_contents)[0][i1] = 0;
; <4>                |   %0 = 0;
; <41>               |   %shl = 1  <<  0;
; <42>               |   %and = (@int_reg_class_contents)[0][i1][0]  &&  %shl;
; <43>               |   if (%and != 0)
; <43>               |   {
; <44>               |      %shl8 = 1  <<  0;
; <45>               |      %0 = %0  ||  %shl8;
; <46>               |      (@reg_class_contents)[0][i1] = %0;
; <43>               |   }
; <47>               |   %shl = 1  <<  1;
; <48>               |   %and = (@int_reg_class_contents)[0][i1][0]  &&  %shl;
; <49>               |   if (%and != 0)
; <49>               |   {
; <50>               |      %shl8 = 1  <<  1;
; <51>               |      %0 = %0  ||  %shl8;
; <52>               |      (@reg_class_contents)[0][i1] = %0;
; <49>               |   }
; <53>               |   %shl = 1  <<  2;
; <54>               |   %and = (@int_reg_class_contents)[0][i1][0]  &&  %shl;
; <55>               |   if (%and != 0)
; <55>               |   {
; <56>               |      %shl8 = 1  <<  2;
; <57>               |      %0 = %0  ||  %shl8;
; <58>               |      (@reg_class_contents)[0][i1] = %0;
; <55>               |   }
; <13>               |   %shl = 1  <<  3;
; <14>               |   %and = (@int_reg_class_contents)[0][i1][0]  &&  %shl;
; <16>               |   if (%and != 0)
; <16>               |   {
; <20>               |      %shl8 = 1  <<  3;
; <21>               |      %0 = %0  ||  %shl8;
; <22>               |      (@reg_class_contents)[0][i1] = %0;
; <16>               |   }
; <38>               + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@reg_class_contents = common dso_local local_unnamed_addr global [25 x i64] zeroinitializer, align 16
@int_reg_class_contents = internal unnamed_addr constant [25 x [2 x i32]] [[2 x i32] zeroinitializer, [2 x i32] [i32 1, i32 0], [2 x i32] [i32 2, i32 0], [2 x i32] [i32 4, i32 0], [2 x i32] [i32 8, i32 0], [2 x i32] [i32 16, i32 0], [2 x i32] [i32 32, i32 0], [2 x i32] [i32 3, i32 0], [2 x i32] [i32 15, i32 0], [2 x i32] [i32 1114352, i32 8160], [2 x i32] [i32 127, i32 8160], [2 x i32] [i32 1114367, i32 0], [2 x i32] [i32 1114367, i32 8160], [2 x i32] [i32 256, i32 0], [2 x i32] [i32 512, i32 0], [2 x i32] [i32 65280, i32 0], [2 x i32] [i32 534773760, i32 2088960], [2 x i32] [i32 -536870912, i32 31], [2 x i32] [i32 534774016, i32 2088960], [2 x i32] [i32 534774272, i32 2088960], [2 x i32] [i32 534839040, i32 2088960], [2 x i32] [i32 131071, i32 8160], [2 x i32] [i32 534839551, i32 2097120], [2 x i32] [i32 534904831, i32 2097120], [2 x i32] [i32 -1, i32 2097151]], align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @init_reg_sets() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc11, %entry
  %indvars.iv26 = phi i64 [ 0, %entry ], [ %indvars.iv.next27, %for.inc11 ]
  %arrayidx = getelementptr inbounds [25 x i64], [25 x i64]* @reg_class_contents, i64 0, i64 %indvars.iv26
  store i64 0, i64* %arrayidx, align 8
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.body
  %0 = phi i64 [ 0, %for.body ], [ %3, %for.inc ]
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.inc ]
  %1 = trunc i64 %indvars.iv to i32
  %div = lshr i64 %indvars.iv, 5
  %idxprom6 = and i64 %div, 134217727
  %arrayidx7 = getelementptr inbounds [25 x [2 x i32]], [25 x [2 x i32]]* @int_reg_class_contents, i64 0, i64 %indvars.iv26, i64 %idxprom6
  %2 = load i32, i32* %arrayidx7, align 4
  %shl = shl nuw nsw i32 1, %1
  %and = and i32 %2, %shl
  %tobool = icmp eq i32 %and, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body3
  %shl8 = shl nuw nsw i64 1, %indvars.iv
  %or = or i64 %0, %shl8
  store i64 %or, i64* %arrayidx, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %3 = phi i64 [ %0, %for.body3 ], [ %or, %if.then ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc11, label %for.body3

for.inc11:                                        ; preds = %for.inc
  %indvars.iv.next27 = add nuw nsw i64 %indvars.iv26, 1
  %exitcond28 = icmp eq i64 %indvars.iv.next27, 25
  br i1 %exitcond28, label %for.end13, label %for.body, !llvm.loop !0

for.end13:                                        ; preds = %for.inc11
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.disable"}
