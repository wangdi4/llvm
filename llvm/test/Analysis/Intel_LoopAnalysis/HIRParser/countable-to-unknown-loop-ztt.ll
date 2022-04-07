; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -enable-new-pm=0 -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that ZTT is extracted for the countable inner loop which is converted to unknown loop when parsing for the upper bound fails.

; CHECK: + DO i1 = 0, -1 * %.pr + -1, 1   <DO_LOOP>
; CHECK: |   %3 = (@g)[0];
; CHECK: |   if (%3 > %m.sroa.0.0.copyload * i1 + (%.pr * %m.sroa.0.0.copyload) + smin(%m.sroa.0.0.copyload, %inc18))
; CHECK: |   {
; CHECK: |         %5 = sext.i32.i64(%m.sroa.0.0.copyload * i1 + (%.pr * %m.sroa.0.0.copyload) + smin(%m.sroa.0.0.copyload, %inc18));
; CHECK: |      + UNKNOWN LOOP i2
; CHECK: |      |   <i2 = 0>
; CHECK: |      |   for.body8:
; CHECK: |      |   (%1)[-1 * i2 + sext.i32.i64(%3)] = %conv;
; CHECK: |      |   %indvars.iv.next = -1 * i2 + sext.i32.i64(%3)  +  -1;
; CHECK: |      |   if (-1 * i2 + sext.i32.i64(%3) + -1 > %5)
; CHECK: |      |   {
; CHECK: |      |      <i2 = i2 + 1>
; CHECK: |      |      goto for.body8;
; CHECK: |      |   }
; CHECK: |      + END LOOP
; CHECK: |         (@g)[0] = %indvars.iv.next;
; CHECK: |   }
; CHECK: |   %inc18 = i1 + %.pr + 1;
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.struct_d = type { %struct.struct_b }
%struct.struct_b = type { i32 }

@e = dso_local local_unnamed_addr global %struct.struct_d zeroinitializer, align 4
@h = dso_local local_unnamed_addr global i32 0, align 4
@g = dso_local local_unnamed_addr global i32 0, align 4
@k = dso_local local_unnamed_addr global i32 0, align 4
@f = dso_local local_unnamed_addr global double* null, align 8

define dso_local void @foo() {
entry:
  %m.sroa.0.0.copyload = load i32, i32* getelementptr inbounds (%struct.struct_d, %struct.struct_d* @e, i64 0, i32 0, i32 0), align 4
  %.pr = load i32, i32* @h, align 4
  %tobool17 = icmp eq i32 %.pr, 0
  br i1 %tobool17, label %for.end10, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %0 = load i32, i32* @k, align 4
  %conv = sitofp i32 %0 to double
  %1 = load double*, double** @f, align 8
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc9
  %inc18 = phi i32 [ %.pr, %for.body.lr.ph ], [ %inc, %for.inc9 ]
  %cmp = icmp slt i32 %inc18, %m.sroa.0.0.copyload
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %mul = mul nsw i32 %inc18, %m.sroa.0.0.copyload
  %add = add nsw i32 %mul, %inc18
  br label %if.end

if.else:                                          ; preds = %for.body
  %mul3 = mul nsw i32 %inc18, %m.sroa.0.0.copyload
  %add5 = add nsw i32 %mul3, %m.sroa.0.0.copyload
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %2 = phi i32 [ %add5, %if.else ], [ %add, %if.then ]
  %3 = load i32, i32* @g, align 4
  %cmp716 = icmp sgt i32 %3, %2
  br i1 %cmp716, label %for.body8.lr.ph, label %for.inc9

for.body8.lr.ph:                                  ; preds = %if.end
  %4 = sext i32 %3 to i64
  %5 = sext i32 %2 to i64
  br label %for.body8

for.body8:                                        ; preds = %for.body8.lr.ph, %for.body8
  %indvars.iv = phi i64 [ %4, %for.body8.lr.ph ], [ %indvars.iv.next, %for.body8 ]
  %ptridx = getelementptr inbounds double, double* %1, i64 %indvars.iv
  store double %conv, double* %ptridx, align 8
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %cmp7 = icmp sgt i64 %indvars.iv.next, %5
  br i1 %cmp7, label %for.body8, label %for.cond6.for.inc9_crit_edge

for.cond6.for.inc9_crit_edge:                     ; preds = %for.body8
  %indvars.iv.next.lcssa = phi i64 [ %indvars.iv.next, %for.body8 ]
  %6 = trunc i64 %indvars.iv.next.lcssa to i32
  store i32 %6, i32* @g, align 4
  br label %for.inc9

for.inc9:                                         ; preds = %for.cond6.for.inc9_crit_edge, %if.end
  %inc = add nsw i32 %inc18, 1
  %tobool = icmp eq i32 %inc, 0
  br i1 %tobool, label %for.cond.for.end10_crit_edge, label %for.body

for.cond.for.end10_crit_edge:                     ; preds = %for.inc9
  store i32 0, i32* @h, align 4
  br label %for.end10

for.end10:                                        ; preds = %for.cond.for.end10_crit_edge, %entry
  ret void
}
