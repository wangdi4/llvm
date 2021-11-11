; CMPLRLLVM-32528
; The phi in while.cond179 is dead and is removed by LoopStrengthReduce.
; When the SCEVs are salvaged to update the debug MD, the SCEV for the phi
; should not be analyzed, as it is a SCEVUnknown with a null Value.

; RUN: opt -debugify -loop-reduce -S < %s | FileCheck %s

; Function Attrs: nounwind
define void @nopreheader(i8* %end) {

; CHECK-LABEL: @nopreheader(
; CHECK:       while.cond179:
; CHECK-NEXT:    call void @llvm.dbg.value(metadata i8* undef, metadata [[META9:![0-9]+]], metadata !DIExpression()), !dbg [[DBG16:![0-9]+]]

; CHECK:       while.cond238:
; CHECK-NEXT:    [[TMP0:%.*]] = phi i64 {{.*}} !dbg [[DBG25:![0-9]+]]
; CHECK-NEXT:    call void @llvm.dbg.value(metadata i64 [[TMP0]], metadata [[META11:![0-9]+]], metadata !DIExpression()), !dbg [[DBG25]]
; CHECK-NEXT:    [[TMPX:%.*]] = add i64 undef, [[TMP0]], !dbg [[DBG26:![0-9]+]]
; CHECK-NEXT:    call void @llvm.dbg.value(metadata i64 [[TMPX]], metadata [[META12:![0-9]+]], metadata !DIExpression()), !dbg [[DBG26]]

; CHECK:       land.rhs243:
; CHECK-NEXT:    [[INDVAR_NEXT15:%.*]] = add i64 [[TMP0]], 1, !dbg [[DBG28:![0-9]+]]
; CHECK-NEXT:    call void @llvm.dbg.value(metadata i64 [[INDVAR_NEXT15]], metadata [[META13:![0-9]+]], metadata !DIExpression()), !dbg [[DBG28]]

entry:
  br label %while.cond179

while.cond179:                                    ; preds = %if.end348, %entry
  %s.1 = phi i8* [ %incdec.ptr356, %if.end348 ], [ undef, %entry ]
  indirectbr i8* undef, [label %land.rhs184, label %while.end453]

land.rhs184:                                      ; preds = %while.cond179
  indirectbr i8* undef, [label %while.end453, label %while.cond197]

while.cond197:                                    ; preds = %land.rhs202, %land.rhs184
  indirectbr i8* undef, [label %land.rhs202, label %while.end215]

land.rhs202:                                      ; preds = %while.cond197
  indirectbr i8* undef, [label %while.end215, label %while.cond197]

while.end215:                                     ; preds = %land.rhs202, %while.cond197
  indirectbr i8* undef, [label %PREMATURE, label %if.end221]

if.end221:                                        ; preds = %while.end215
  indirectbr i8* undef, [label %while.cond238.preheader, label %lor.lhs.false227]

lor.lhs.false227:                                 ; preds = %if.end221
  indirectbr i8* undef, [label %while.cond238.preheader, label %if.else]

while.cond238.preheader:                          ; preds = %lor.lhs.false227, %if.end221
  indirectbr i8* undef, [label %while.cond238]

while.cond238:                                    ; preds = %land.rhs243, %while.cond238.preheader
  %0 = phi i64 [ %indvar.next15, %land.rhs243 ], [ 0, %while.cond238.preheader ]
  %tmpx = add i64 undef, %0
  indirectbr i8* undef, [label %land.rhs243, label %while.end256]

land.rhs243:                                      ; preds = %while.cond238
  %indvar.next15 = add i64 %0, 1
  indirectbr i8* undef, [label %while.end256, label %while.cond238]

while.end256:                                     ; preds = %land.rhs243, %while.cond238
  unreachable

if.else:                                          ; preds = %lor.lhs.false227
  indirectbr i8* undef, [label %if.then297, label %if.else386]

if.then297:                                       ; preds = %if.else
  indirectbr i8* undef, [label %PREMATURE, label %if.end307]

if.end307:                                        ; preds = %if.then297
  indirectbr i8* undef, [label %if.end314, label %FAIL]

if.end314:                                        ; preds = %if.end307
  indirectbr i8* undef, [label %PREMATURE, label %if.end348]

if.end348:                                        ; preds = %if.end314
  %incdec.ptr356 = getelementptr inbounds i8, i8* undef, i64 2
  indirectbr i8* undef, [label %while.cond179]

if.else386:                                       ; preds = %if.else
  unreachable

while.end453:                                     ; preds = %land.rhs184, %while.cond179
  unreachable

FAIL:                                             ; preds = %if.end307
  unreachable

PREMATURE:                                        ; preds = %if.end314, %if.then297, %while.end215
  unreachable
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
