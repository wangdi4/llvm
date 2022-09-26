; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -disable-output -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s
;
; Test to check vector code generated for input HIR with memrefs in an HLIF.
; Incoming HIR into the vectorizer has two loops that look like the following:
; Function: foo
;     DO i1 = 0, 99, 1   <DO_LOOP>
;       if ((@sarr)[0][i1].0 == (@sarr)[0][i1].1)
;       {
;          (@arr)[0][i1] = i1;
;       }
;     END LOOP
;
; Function: foo2
;     DO i1 = 0, 99, 1   <DO_LOOP>
;       if ((@sarr)[0][i1].0 == 10)
;       {
;          (@arr)[0][i1] = (@sarr)[0][i1].1;
;       }
;     END LOOP
;
; The test verifies that we generate VLS loads that are 8-wide with
; appropriate shuffles for the first loop. VLS analysis currently does
; not handle accesses under a mask. As a result, we test for gathers
; being generated for the second loop.
;
; CHECK: Function: foo
; CHECK:          DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:       %.vls.load = (<8 x i64>*)(@sarr)[0][i1].0;
; CHECK-NEXT:       %vls.extract = shufflevector %.vls.load, %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK-NEXT:       %vls.extract1 = shufflevector %.vls.load, %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK-NEXT:       %.vec = %vls.extract == %vls.extract1;
; CHECK-NEXT:       (<4 x i64>*)(@arr)[0][i1] = i1 + <i64 0, i64 1, i64 2, i64 3>, Mask = @{%.vec};
; CHECK-NEXT:     END LOOP
;
; CHECK: Function: foo2
; CHECK:          DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:       %.vec2 = undef;
; CHECK-NEXT:       %.vec = (<4 x i64>*)(@sarr)[0][i1 + <i64 0, i64 1, i64 2, i64 3>].0;
; CHECK-NEXT:       %.vec1 = %.vec == 10;
; CHECK-NEXT:       %.vec2 = (<4 x i64>*)(@sarr)[0][i1 + <i64 0, i64 1, i64 2, i64 3>].1, Mask = @{%.vec1};
; CHECK-NEXT:       (<4 x i64>*)(@arr)[0][i1] = %.vec2, Mask = @{%.vec1};
; CHECK-NEXT:     END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S1 = type { i64, i64 }

@sarr = dso_local local_unnamed_addr global [100 x %struct.S1] zeroinitializer, align 16
@arr = dso_local local_unnamed_addr global [100 x i64] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i64 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %l1.014 = phi i64 [ 0, %entry ], [ %add, %for.inc ]
  %a = getelementptr inbounds [100 x %struct.S1], [100 x %struct.S1]* @sarr, i64 0, i64 %l1.014, i32 0
  %0 = load i64, i64* %a, align 16
  %b = getelementptr inbounds [100 x %struct.S1], [100 x %struct.S1]* @sarr, i64 0, i64 %l1.014, i32 1
  %1 = load i64, i64* %b, align 8
  %cmp2 = icmp eq i64 %0, %1
  br i1 %cmp2, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds [100 x i64], [100 x i64]* @arr, i64 0, i64 %l1.014
  store i64 %l1.014, i64* %arrayidx3, align 8
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %add = add nuw nsw i64 %l1.014, 1
  %exitcond.not = icmp eq i64 %add, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret i64 0
}

define dso_local i64 @foo2() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %l1.014 = phi i64 [ 0, %entry ], [ %add, %for.inc ]
  %a = getelementptr inbounds [100 x %struct.S1], [100 x %struct.S1]* @sarr, i64 0, i64 %l1.014, i32 0
  %0 = load i64, i64* %a, align 16
  %cmp2 = icmp eq i64 %0, 10
  br i1 %cmp2, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %b = getelementptr inbounds [100 x %struct.S1], [100 x %struct.S1]* @sarr, i64 0, i64 %l1.014, i32 1
  %1 = load i64, i64* %b, align 8
  %arrayidx3 = getelementptr inbounds [100 x i64], [100 x i64]* @arr, i64 0, i64 %l1.014
  store i64 %1, i64* %arrayidx3, align 8
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %add = add nuw nsw i64 %l1.014, 1
  %exitcond.not = icmp eq i64 %add, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret i64 0
}
