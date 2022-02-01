; RUN: opt -enable-new-pm=0 -o %t < %s
; RUN: llvm-lto --use-new-pm=0 %t -o %r -exported-symbol=main -O3 -disable-verify -print-after=hir-temp-cleanup -hir-details 2>&1 | FileCheck %s --check-prefixes="CHECK,AA"
; RUN: llvm-lto --use-new-pm=0 %t -o %r -exported-symbol=main -O3 -disable-verify -print-after=hir-temp-cleanup -hir-details -enable-andersen=false 2>&1 | FileCheck %s --check-prefixes="CHECK,NOAA"
; RUN: opt %s -passes='lto<O3>' -whole-program-assume -disable-verify -loopopt -print-after=hir-temp-cleanup -hir-details 2>&1 | FileCheck %s --check-prefixes="CHECK,AA"
; RUN: opt %s -passes='lto<O3>' -whole-program-assume -disable-verify -loopopt -print-after=hir-temp-cleanup -hir-details -enable-npm-andersen=false 2>&1 | FileCheck %s --check-prefixes="CHECK,NOAA"

; This test is checking that Andersen's AA results are available for the loopopt.
;
; Loopopt is a function pass while the Andersen's AA is a module analysis.
; We should carefully preserve module level AA results because they are
; expensive to re-compute and it's very easy to unintentionally invalidate them.
;
; The test is written with the assumption that no analysis except Andersen's AA
; are able to resolve dependencies between %a and %b. If other analysis improve
; the test may become not indicative.
;
; Function: foo
;
;     BEGIN REGION { }
;           + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;           |   %conv = sitofp.i32.float(i1);
;           |   %add = (%b)[i1]  +  %conv;
;           |   (%a)[i1] = %add;
;           + END LOOP
;     END REGION
;
; Function: bar
;
;     BEGIN REGION { }
;           + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;           |   %add = (%a)[i1]  +  (%b)[i1];
;           |   (%a)[i1] = %add;
;           + END LOOP
;     END REGION

; CHECK-LABEL: Function: foo

; CHECK: float* %b)[
; CHECK-SAME: {sb:[[SB:.*]]}

; CHECK: float* %a)[
; AA-NOT: {sb:[[SB]]}
; NOAA-SAME: {sb:[[SB]]}

; CHECK-LABEL: Function: bar

; CHECK: float* %a)[
; CHECK-SAME: {sb:[[SB:.*]]}

; CHECK: float* %b)[
; AA-NOT: {sb:[[SB]]}
; NOAA-SAME: {sb:[[SB]]}

; CHECK: float* %a)[
; CHECK-SAME: {sb:[[SB]]}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local noalias float* @fmalloc(i32 %count) local_unnamed_addr #0 {
entry:
  %conv = sext i32 %count to i64
  %mul = shl nsw i64 %conv, 2
  %call = tail call noalias i8* @malloc(i64 %mul) #3
  %0 = bitcast i8* %call to float*
  ret float* %0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) local_unnamed_addr #1

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local i32 @foo(float* nocapture %a, float* nocapture readonly %b, i32 %n) local_unnamed_addr #2 {
entry:
  %cmp13 = icmp sgt i32 %n, 0
  br i1 %cmp13, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %sub = add nsw i32 %n, -1
  %idxprom3 = sext i32 %sub to i64
  %arrayidx4 = getelementptr inbounds float, float* %a, i64 %idxprom3
  %0 = load float, float* %arrayidx4, align 4, !tbaa !4
  %conv5 = fptosi float %0 to i32
  ret i32 %conv5

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4, !tbaa !4
  %2 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %2 to float
  %add = fadd fast float %1, %conv
  %arrayidx2 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  store float %add, float* %arrayidx2, align 4, !tbaa !4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local i32 @bar(float* nocapture %a, float* nocapture readonly %b, i32 %n) local_unnamed_addr #2 {
entry:
  %cmp11 = icmp sgt i32 %n, 0
  br i1 %cmp11, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %sub = add nsw i32 %n, -1
  %idxprom3 = sext i32 %sub to i64
  %arrayidx4 = getelementptr inbounds float, float* %a, i64 %idxprom3
  %0 = load float, float* %arrayidx4, align 4, !tbaa !4
  %conv = fptosi float %0 to i32
  ret i32 %conv

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4, !tbaa !4
  %arrayidx2 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  %2 = load float, float* %arrayidx2, align 4, !tbaa !4
  %add = fadd fast float %2, %1
  store float %add, float* %arrayidx2, align 4, !tbaa !4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32 %argc, i8** nocapture readnone %argv) local_unnamed_addr #0 {
entry:
  %conv.i = sext i32 %argc to i64
  %mul.i = shl nsw i64 %conv.i, 2
  %call.i = tail call noalias i8* @malloc(i64 %mul.i) #3
  %0 = bitcast i8* %call.i to float*
  %call.i13 = tail call noalias i8* @malloc(i64 %mul.i) #3
  %1 = bitcast i8* %call.i13 to float*
  %call2 = tail call i32 @foo(float* %0, float* %1, i32 %argc)
  %call3 = tail call i32 @bar(float* %0, float* %1, i32 %argc)
  %call4 = tail call i32 @foo(float* null, float* null, i32 %argc)
  %call5 = tail call i32 @bar(float* null, float* null, i32 %argc)
  %add = add nsw i32 %call3, %call2
  ret i32 %add
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" "loopopt-pipeline"="full" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" "loopopt-pipeline"="full" }
attributes #2 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" "loopopt-pipeline"="full" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ThinLTO", i32 0}
!2 = !{i32 1, !"EnableSplitLTOUnit", i32 0}
!3 = !{!"icx (ICX) 2019.8.1.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

^0 = module: (path: "mem.o", hash: (0, 0, 0, 0, 0))
^1 = gv: (name: "malloc") ; guid = 2336192559129972258
^2 = gv: (name: "fmalloc", summaries: (function: (module: ^0, flags: (linkage: external, notEligibleToImport: 1, live: 0, dsoLocal: 1), insts: 5, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 0, returnDoesNotAlias: 1, noInline: 0), calls: ((callee: ^1))))) ; guid = 5680171629252751931
^3 = gv: (name: "foo", summaries: (function: (module: ^0, flags: (linkage: external, notEligibleToImport: 1, live: 0, dsoLocal: 1), insts: 21, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 1)))) ; guid = 6699318081062747564
^4 = gv: (name: "main", summaries: (function: (module: ^0, flags: (linkage: external, notEligibleToImport: 1, live: 0, dsoLocal: 1), insts: 10, calls: ((callee: ^1), (callee: ^3), (callee: ^5))))) ; guid = 15822663052811949562
^5 = gv: (name: "bar", summaries: (function: (module: ^0, flags: (linkage: external, notEligibleToImport: 1, live: 0, dsoLocal: 1), insts: 20, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 1)))) ; guid = 16434608426314478903
