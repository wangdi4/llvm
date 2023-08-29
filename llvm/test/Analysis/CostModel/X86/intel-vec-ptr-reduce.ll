; RUN: opt -passes="loop-unroll" -S %s | FileCheck %s
; CHECK: llvm.vector.reduce.umin.v16p0

; 24495: Make sure the cost modeller does not crash when it sees a vector
; reduction of pointer type.

target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

define dso_local void @SyFgets() local_unnamed_addr #0 {
entry:
  switch i32 undef, label %if.end21 [
    i32 0, label %if.then3
    i32 1, label %land.lhs.true11
  ]

if.then3:                                         ; preds = %entry
  unreachable

land.lhs.true11:                                  ; preds = %entry
  unreachable

if.end21:                                         ; preds = %entry
  br label %while.cond

while.cond:                                       ; preds = %while.cond1790.preheader, %if.end21
  br label %while.cond199

if.end170:                                        ; No predecessors!
  unreachable

while.cond199:                                    ; preds = %sw.bb1029, %while.cond199, %while.cond
  switch i32 undef, label %sw.default [
    i32 1, label %while.cond199.thread3917.preheader
    i32 373, label %sw.bb1029
    i32 -1, label %while.cond199
    i32 15, label %while.cond1244.preheader
    i32 18, label %sw.bb1006.preheader
    i32 268, label %sw.bb1214.preheader
    i32 341, label %sw.bb1029
  ]

sw.bb1214.preheader:                              ; preds = %while.cond199
  unreachable

sw.bb1006.preheader:                              ; preds = %while.cond199
  unreachable

while.cond1244.preheader:                         ; preds = %while.cond199
  unreachable

while.cond199.thread3917.preheader:               ; preds = %while.cond199
  %0 = call ptr @llvm.vector.reduce.umin.v16p0(<16 x ptr> undef)
  br label %for.body1756

sw.bb1029:                                        ; preds = %while.cond199, %while.cond199
  br label %while.cond199

sw.default:                                       ; preds = %while.cond199
  unreachable

while.cond1790.preheader:                         ; preds = %for.body1756
  br label %while.cond

for.body1756:                                     ; preds = %for.body1756, %while.cond199.thread3917.preheader
  br i1 undef, label %for.body1756, label %while.cond1790.preheader
}

; Function Attrs: nofree nosync nounwind readnone willreturn
declare ptr @llvm.vector.reduce.umin.v16p0(<16 x ptr>) #1

attributes #0 = { "use-soft-float"="false" }
attributes #1 = { nofree nosync nounwind readnone willreturn }

