; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that %liveout is marked as liveout of i2 loop.

; + DO i1 = 0, 5, 1   <DO_LOOP>
; |   %.pre.i3.i = (@g_1089)[0][-1 * i1 + 6];
; |   %t32 = %.pre.i3.i;
; |
; |   + DO i2 = 0, 6, 1   <DO_MULTI_EXIT_LOOP>
; |   |   %storemerge12248.out = i2;
; |   |   %t32.out = %t32;
; |   |   %liveout = %storemerge12248.out;
; |   |   if (%t32.out != 0)
; |   |   {
; |   |      goto for.end846;
; |   |   }
; |   |   %t32 = 0;
; |   |   %liveout = 7;
; |   + END LOOP
; |
; |   for.end846:
; + END LOOP

; Get symbase of %liveout from region liveout list.
; CHECK: LiveOuts: %liveout(sym:[[LIVEOUT:.*]])

; Verify that %liveout is marked as liveout of both i1 and i2 loop.
; CHECK: + LiveOut symbases: [[LIVEOUT]]
; CHECK: + DO i64 i1 = 0, 5, 1   <DO_LOOP>

; CHECK: |   + LiveOut symbases: [[LIVEOUT]]
; CHECK: |   + DO i8 i2 = 0, 6, 1   <DO_MULTI_EXIT_LOOP>


; ModuleID = 'func.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


@g_1089 = external hidden unnamed_addr global [8 x i16], align 16

define void @func() {
entry:
  br label %for.body570

for.body570:                                ; preds = %for.end846, %entry
  %storemerge1252 = phi i64 [ 6, %entry ], [ %sub.i154.i.i, %for.end846 ]
  %arrayidx830 = getelementptr inbounds [8 x i16], ptr @g_1089, i64 0, i64 %storemerge1252
  %.pre.i3.i = load i16, ptr %arrayidx830, align 2
  br label %for.body575

for.cond572:                                ; preds = %for.body575
  %cmp573 = icmp ult i8 %add844, 7
  br i1 %cmp573, label %for.body575, label %for.end846

for.body575:                                ; preds = %for.cond572, %for.body570
  %t32 = phi i16 [ %.pre.i3.i, %for.body570 ], [ 0, %for.cond572 ]
  %storemerge12248 = phi i8 [ 0, %for.body570 ], [ %add844, %for.cond572 ]
  %tobool831 = icmp eq i16 %t32, 0
  %add844 = add nuw nsw i8 %storemerge12248, 1
  br i1 %tobool831, label %for.cond572, label %for.end846

for.end846:                                 ; preds = %for.body575, %for.cond572
  %liveout = phi i8 [ 7, %for.cond572 ], [ %storemerge12248, %for.body575 ]
  %sub.i154.i.i = add nsw i64 %storemerge1252, -1
  %cmp569 = icmp ugt i64 %storemerge1252, 1
  br i1 %cmp569, label %for.body570, label %for.end899

for.end899:                                 ; preds = %for.end846
  %.lcssa3636 = phi i8 [ %liveout, %for.end846 ]
  ret void
}

