; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,verify<hir>,print<hir>" < %s 2>&1 | FileCheck %s

; Check that "%t.0 = 0" and "%t.0 = 1" expanded as "(%.TempArray)[0][i1] = 0" and "(%.TempArray)[0][i1] = 1".

; BEGIN REGION { }
;        + DO i1 = 0, 19, 1   <DO_LOOP>
;        |   %mul = i1  *  i1;
;        |   %t.0 = 0;
;        |   if (%mul < 8)
;        |   {
;        |      %t.0 = 1;
;        |   }
;        |   %conv = sitofp.i32.float(%t.0); <distribute_point>
;        |   (%a)[i1] = %conv;
;        + END LOOP
; END REGION

; BEGIN REGION
; CHECK: modified
; CHECK: + DO i1 = 0, 19, 1   <DO_LOOP>
;        |   %mul = i1  *  i1;
;        |   %t.0 = 0;
; CHECK: |   (%.TempArray)[0][i1] = 0;
;        |   if (%mul < 8)
;        |   {
;        |      %t.0 = 1;
; CHECK: |      (%.TempArray)[0][i1] = 1;
;        |   }
; CHECK: + END LOOP
;
;
; CHECK: + DO i1 = 0, 19, 1   <DO_LOOP>
;        |   %t.0 = (%.TempArray)[0][i1];
;        |   %conv = sitofp.i32.float(%t.0);
;        |   (%a)[i1] = %conv;
; CHECK: + END LOOP
; END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(ptr %a) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %mul = mul nsw i32 %i.01, %i.01
  %cmp1 = icmp slt i32 %mul, 8
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %t.0 = phi i32 [ 1, %if.then ], [ 0, %for.body ]
  %p = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %conv = sitofp i32 %t.0 to float
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds float, ptr %a, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  call void @llvm.directive.region.exit(token %p) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 20
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
