; RUN: opt -passes="dse" -S %s | FileCheck %s
; The store of or117 postdominates or104, but the address depends on
; iteration count.
; CHECK: store{{.*}}%or104
; CHECK: store{{.*}}%or117

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@mask_pawn_protected_w = external dso_local local_unnamed_addr global [64 x i64], align 16

define dso_local void @InitializeMasks() local_unnamed_addr #0 {
entry:
  br label %for.body98

for.body98:                                       ; preds = %for.inc140, %for.inc140.thread, %entry
  %indvars.iv377 = phi i64 [ 8, %entry ], [ undef, %for.inc140 ], [ undef, %for.inc140.thread ]
  %or104 = or i64 undef, undef
  %arrayidx106 = getelementptr inbounds [64 x i64], [64 x i64]* @mask_pawn_protected_w, i64 0, i64 %indvars.iv377
  store i64 %or104, i64* %arrayidx106, align 8
  %cmp107 = icmp ugt i64 %indvars.iv377, 15
  br i1 %cmp107, label %if.end, label %if.end.thread

if.end.thread:                                    ; preds = %for.body98
  br label %for.inc140.thread

if.end:                                           ; preds = %for.body98
  %or117 = or i64 undef, undef
  store i64 %or117, i64* %arrayidx106, align 8
  %cmp127 = icmp ult i64 %indvars.iv377, 48
  br i1 %cmp127, label %for.inc140.thread, label %for.inc140

for.inc140.thread:                                ; preds = %if.end, %if.end.thread
  br label %for.body98

for.inc140:                                       ; preds = %if.end
  %exitcond384.not = icmp eq i64 undef, 56
  br i1 %exitcond384.not, label %for.body145.preheader, label %for.body98

for.body145.preheader:                            ; preds = %for.inc140
  br label %for.body167

for.body167:                                      ; preds = %for.body167, %for.body145.preheader
  br i1 undef, label %for.body167, label %for.inc177

for.inc177:                                       ; preds = %for.body167
  ret void
}

attributes #0 = { "unsafe-fp-math"="true" }


