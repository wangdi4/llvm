; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the inner condition is not hoisted out of the
; inner loop since the target loop is outside of SIMD directives. These
; directives are inside a loop.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %simd = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
;       |
;       |   + DO i2 = 0, 99, 1   <DO_LOOP> <simd>
;       |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;       |   |   |   if (i1 == 5)
;       |   |   |   {
;       |   |   |      (%a)[5 * i2 + i3] = i2 + i3 + 5;
;       |   |   |   }
;       |   |   + END LOOP
;       |   + END LOOP
;       |
;       |   @llvm.directive.region.exit(%simd); [ DIR.OMP.END.SIMD() ]
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   %simd = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; CHECK:       |
; CHECK:       |   + DO i2 = 0, 99, 1   <DO_LOOP> <simd>
; CHECK:       |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   |   |   if (i1 == 5)
; CHECK:       |   |   |   {
; CHECK:       |   |   |      (%a)[5 * i2 + i3] = i2 + i3 + 5;
; CHECK:       |   |   |   }
; CHECK:       |   |   + END LOOP
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |   @llvm.directive.region.exit(%simd); [ DIR.OMP.END.SIMD() ]
; CHECK:       + END LOOP
; CHECK: END REGION


;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPii(ptr nocapture noundef writeonly %a, i32 noundef %n) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %i.030 = phi i32 [ 0, %entry ], [ %inc16, %for.cond.cleanup3 ]
  %simd = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  %cmp9 = icmp eq i32 %i.030, 5
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond5.preheader:                              ; preds = %for.cond1.preheader, %for.cond.cleanup7
  %indvars.iv33 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next34, %for.cond.cleanup7 ]
  %0 = add nuw nsw i64 %indvars.iv33, 5
  %1 = mul nuw nsw i64 %indvars.iv33, 5
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %inc16 = add nuw nsw i32 %i.030, 1
  %exitcond38.not = icmp eq i32 %inc16, 100
  call void @llvm.directive.region.exit(token %simd) [ "DIR.OMP.END.SIMD"() ]
  br i1 %exitcond38.not, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.inc
  %indvars.iv.next34 = add nuw nsw i64 %indvars.iv33, 1
  %exitcond37.not = icmp eq i64 %indvars.iv.next34, 100
  br i1 %exitcond37.not, label %for.cond.cleanup3, label %for.cond5.preheader

for.body8:                                        ; preds = %for.cond5.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp9, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body8
  %2 = add nuw nsw i64 %0, %indvars.iv
  %3 = add nuw nsw i64 %indvars.iv, %1
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %3
  %4 = trunc i64 %2 to i32
  store i32 %4, ptr %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body8, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup7, label %for.body8
}

declare token @llvm.directive.region.entry() #1

declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
