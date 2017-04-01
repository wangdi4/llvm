; REQUIRES: asserts
; RUN: opt < %s -analyze -hir-region-identification -debug-only=hir-region-identification  2>&1 | FileCheck %s

; Verify that we detect and skip irreducible cfg. The bblocks for.cond49.sink.split, for.cond49 and for.body51 constitute a multi-entry loop.
; CHECK: Irreducible CFG not supported


; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-59f419b.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @main() local_unnamed_addr {
entry:
  br label %for.cond12.preheader

for.cond12.preheader:                             ; preds = %entry
  br label %for.body20

for.body20:                                       ; preds = %for.cond92.preheader, %for.cond12.preheader
  %indvars.iv501 = phi i64 [ 1, %for.cond12.preheader ], [ %indvars.iv.next502, %for.cond92.preheader ]
  br i1 undef, label %for.cond49, label %if.then

if.then:                                          ; preds = %for.body20
  br label %for.cond49.sink.split

for.cond49.sink.split:                            ; preds = %for.body51, %if.then
  br label %for.cond49

for.cond49:                                       ; preds = %for.cond49.sink.split, %for.body20
  br i1 undef, label %for.body51, label %for.cond92.preheader

for.body51:                                       ; preds = %for.cond49
  br label %for.cond49.sink.split

for.cond92.preheader:                             ; preds = %for.cond49
  %indvars.iv.next502 = add nsw i64 %indvars.iv501, 1
  %exitcond503 = icmp eq i64 %indvars.iv.next502, 15
  br i1 %exitcond503, label %for.cond18.for.inc140_crit_edge, label %for.body20

for.cond18.for.inc140_crit_edge:                  ; preds = %for.cond92.preheader
  unreachable
}

