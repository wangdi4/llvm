; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s

; Verify that the trip count of the two loops in two different regions is parsed succesfully. After parsing the first region, we need to invalidate the HIR cache before processing the second region because the HIR SCEV may not be valid across regions. The issue is exposed here since the formed regions are not in lexical order in the function. The first region formed lies lexically after the second one. The IV of the first lexical loop is parsed as a blob while parsing the first region. When we get to the seond region, the backedge computation fails because the cache is reused and the IV is treated as a blob.

; CHECK: BEGIN REGION
; CHECK: + DO i1 = 0, zext.i32.i64((-2 + %indvars.iv266)), 1   <DO_LOOP>
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: BEGIN REGION
; CHECK: + DO i1 = 0, zext.i32.i64((trunc.i64.i32(%indvars.iv282) + umax(-2, (-1 * trunc.i64.i32(%indvars.iv282))))), 1   <DO_LOOP>
; CHECK: + END LOOP
; CHECK: END REGION


; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-2061caf.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @main() #0 {
entry:
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  br i1 undef, label %for.body.i230, label %for.body.i

for.body.i230:                                    ; preds = %for.body.i230, %for.body.i
  br i1 undef, label %for.body.i219, label %for.body.i230

for.body.i219:                                    ; preds = %for.body.i219, %for.body.i230
  br i1 undef, label %for.body.i208, label %for.body.i219

for.body.i208:                                    ; preds = %for.body.i208, %for.body.i219
  br i1 undef, label %for.body.i197, label %for.body.i208

for.body.i197:                                    ; preds = %for.body.i197, %for.body.i208
  br i1 undef, label %for.body.i186, label %for.body.i197

for.body.i186:                                    ; preds = %for.body.i186, %for.body.i197
  br i1 undef, label %init.exit187, label %for.body.i186

init.exit187:                                     ; preds = %for.body.i186
  br label %for.body11.lr.ph

for.body11.lr.ph:                                 ; preds = %for.inc72, %init.exit187
  %indvars.iv266 = phi i32 [ 79, %init.exit187 ], [ %indvars.iv.next267, %for.inc72 ]
  br label %for.body11

for.cond13.preheader:                             ; preds = %for.body11
  br i1 undef, label %for.body15.lr.ph, label %for.cond13.preheader.for.inc72_crit_edge

for.cond13.preheader.for.inc72_crit_edge:         ; preds = %for.cond13.preheader
  br label %for.inc72

for.body15.lr.ph:                                 ; preds = %for.cond13.preheader
  br label %for.body15

for.body11:                                       ; preds = %for.body11, %for.body11.lr.ph
  %indvars.iv = phi i64 [ 1, %for.body11.lr.ph ], [ %indvars.iv.next, %for.body11 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond268 = icmp eq i32 %lftr.wideiv, %indvars.iv266
  br i1 %exitcond268, label %for.cond13.preheader, label %for.body11

for.body15:                                       ; preds = %for.end44, %for.body15.lr.ph
  %indvars.iv282 = phi i64 [ undef, %for.body15.lr.ph ], [ %.pre305, %for.end44 ]
  br label %for.cond24.preheader.preheader

for.cond24.preheader.preheader:                   ; preds = %for.end39, %for.body15
  br label %for.cond24.preheader

for.cond24.preheader:                             ; preds = %for.cond24.preheader, %for.cond24.preheader.preheader
  %indvars.iv276 = phi i64 [ %indvars.iv282, %for.cond24.preheader.preheader ], [ %indvars.iv.next277, %for.cond24.preheader ]
  %0 = trunc i64 %indvars.iv276 to i32
  %sub28 = add nsw i32 %0, -1
  %cmp22 = icmp ugt i32 %sub28, 1
  %indvars.iv.next277 = add nsw i64 %indvars.iv276, -1
  br i1 %cmp22, label %for.cond24.preheader, label %for.end39

for.end39:                                        ; preds = %for.cond24.preheader
  br i1 undef, label %for.cond24.preheader.preheader, label %for.end44

for.end44:                                        ; preds = %for.end39
  %.pre305 = add nuw nsw i64 %indvars.iv282, 1
  br i1 undef, label %for.inc72, label %for.body15

for.inc72:                                        ; preds = %for.end44, %for.cond13.preheader.for.inc72_crit_edge
  %indvars.iv.next267 = add nsw i32 %indvars.iv266, -1
  br i1 undef, label %for.end74, label %for.body11.lr.ph

for.end74:                                        ; preds = %for.inc72
  br label %for.body.i175

for.body.i175:                                    ; preds = %for.body.i175, %for.end74
  br i1 undef, label %for.body.i163.preheader, label %for.body.i175

for.body.i163.preheader:                          ; preds = %for.body.i175
  br label %for.body.i163

for.body.i163:                                    ; preds = %for.body.i163, %for.body.i163.preheader
  br i1 undef, label %for.body.i151.preheader, label %for.body.i163

for.body.i151.preheader:                          ; preds = %for.body.i163
  br label %for.body.i151

for.body.i151:                                    ; preds = %for.body.i151, %for.body.i151.preheader
  br i1 undef, label %for.body.i139.preheader, label %for.body.i151

for.body.i139.preheader:                          ; preds = %for.body.i151
  br label %for.body.i139

for.body.i139:                                    ; preds = %for.body.i139, %for.body.i139.preheader
  br i1 undef, label %for.body.i127.preheader, label %for.body.i139

for.body.i127.preheader:                          ; preds = %for.body.i139
  br label %for.body.i127

for.body.i127:                                    ; preds = %for.body.i127, %for.body.i127.preheader
  br i1 undef, label %for.body.i116.preheader, label %for.body.i127

for.body.i116.preheader:                          ; preds = %for.body.i127
  br label %for.body.i116

for.body.i116:                                    ; preds = %for.body.i116, %for.body.i116.preheader
  br i1 undef, label %checkSum.exit, label %for.body.i116

checkSum.exit:                                    ; preds = %for.body.i116
  ret void
}

