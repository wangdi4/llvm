; RUN: opt -disable-hir-opt-predicate-region-simd=false -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -S < %s 2>&1 | FileCheck %s

; This test checks that the inner If condition is hoisted out even if the outer
; If can't be hoisted, and the SIMD directives are set correctly.

; HIR before transformation

; BEGIN REGION { }
;       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
;
;       + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
;       |   if ((%p)[i1] == 8)
;       |   {
;       |      if (%n == 5)
;       |      {
;       |         (%p)[i1] = 1;
;       |      }
;       |   }
;       + END LOOP
;
;       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;       ret ;
; END REGION

; HIR after transformation

; TODO: The extra code in the else branch needs to be removed.

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%n == 5)
; CHECK:       {
; CHECK:          %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
; CHECK:          |   if ((%p)[i1] == 8)
; CHECK:          |   {
; CHECK:          |      (%p)[i1] = 1;
; CHECK:          |   }
; CHECK:          + END LOOP
; CHECK:          @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK:       }
; CHECK:       else
; CHECK-NEXT:       {
; CHECK-NEXT:          %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; CHECK-NEXT:          @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK-NEXT:       }
; CHECK:       ret ;
; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i64 %n,ptr %p) {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  %1 = trunc i64 %n to i32
  br label %for.body

for.body:
  %i = phi i32 [ 0, %entry ], [ %ip, %for.inc ]
  %idxprom = sext i32 %i to i64
  %arrayidx2 = getelementptr inbounds i32, ptr %p, i64 %idxprom
  %2 = load i32, ptr %arrayidx2, align 4
  %cmp1 = icmp eq i32 %2, 8
  br i1 %cmp1, label %if.then, label %for.inc

if.then:
  %cmp2 = icmp eq i32 %1, 5
  br i1 %cmp2, label %if.inner, label %for.inc

if.inner:
  store i32 1, ptr %arrayidx2
  br label %for.inc

for.inc:
  %ip = add nsw i32 %i, 1
  %cmp = icmp slt i32 %i, 99
  br i1 %cmp, label %for.body, label %for.end

for.end:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() #1

declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
