; CMPLRLLVM-33683

; SCEV is incorrectly using the result of cmp162.not514, to simplify
; cmp162.not509 as always-true.
; Both comparisons have the equivalent LHS and RHS, but the ugt in not514 is
; being changed to ult due to incorrect range sharpening.

; RUN: opt -S -passes="indvars" %s | FileCheck %s
; CHECK: br{{.*}}cmp162.not509{{.*}}for.body164

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


define dso_local void @main() local_unnamed_addr {
entry:
  br label %cond.end28

cond.end28:                                       ; preds = %for.end373, %entry
  %storemerge502558 = phi i64 [ 0, %entry ], [ %add375, %for.end373 ]
  br label %for.body42

for.body42:                                       ; preds = %for.inc371, %cond.end28
  %conv39534 = phi i32 [ 0, %cond.end28 ], [ %conv39, %for.inc371 ]
  %v.0533 = phi i16 [ 0, %cond.end28 ], [ %inc372, %for.inc371 ]
  %idxprom45 = zext i16 %v.0533 to i64
  %conv161508 = zext i32 %conv39534 to i64
  %cmp162.not509 = icmp ult i64 %storemerge502558, %conv161508
  %cmp53.not514 = icmp ugt i64 %storemerge502558, %idxprom45
  br i1 %cmp53.not514, label %for.inc371, label %for.cond57.preheader.preheader

for.cond57.preheader.preheader:                   ; preds = %for.body42
  br label %for.cond57.preheader

for.cond57.preheader:                             ; preds = %if.end367, %for.cond57.preheader.preheader
  %h.0516 = phi i64 [ %inc369, %if.end367 ], [ %storemerge502558, %for.cond57.preheader.preheader ]
  br label %for.cond160.preheader

for.cond160.preheader:                            ; preds = %for.cond57.preheader
  br i1 %cmp162.not509, label %if.end367, label %for.body164.preheader

for.body164.preheader:                            ; preds = %for.cond160.preheader
  unreachable

if.end367:                                        ; preds = %for.cond160.preheader
  %inc369 = add i64 %h.0516, 1
  br i1 undef, label %for.inc371.loopexit, label %for.cond57.preheader

for.inc371.loopexit:                              ; preds = %if.end367
  br label %for.inc371

for.inc371:                                       ; preds = %for.inc371.loopexit, %for.body42
  %inc372 = add i16 %v.0533, 1
  %conv39 = zext i16 %inc372 to i32
  %cmp40 = icmp ult i16 %inc372, 9
  br i1 %cmp40, label %for.body42, label %for.end373

for.end373:                                       ; preds = %for.inc371
  %add375 = add i64 %storemerge502558, 9
  br i1 undef, label %cond.end28, label %for.end376

for.end376:                                       ; preds = %for.end373
  unreachable
}
