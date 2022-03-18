; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

;<0>          BEGIN REGION { }
;<31>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<2:3>              |   if (%n != 0)
;<2:3>              |   {
;<7>                |      (%p)[i1] = i1;
;<2:3>              |   }
;<11>               |   (%q)[i1] = i1;
;<12>               |   if (%m != 0)
;<12>               |   {
;<17>               |      (%p)[i1] = i1;
;<18:9>             |      if (%n != 0)
;<18:9>             |      {
;<22>               |         (%q)[i1] = 0;
;<18:9>             |      }
;<12>               |   }
;<31>               + END LOOP
;<0>          END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: if (%n != 0)
; CHECK: {
; CHECK-NOT: if
; CHECK:   if (%m != 0)
; CHECK:   {
; CHECK-NOT: if
; CHECK:     + DO i1
; CHECK:     + END LOOP
; CHECK:   }
; CHECK:   else
; CHECK:   {
; CHECK-NOT: if
; CHECK:     + DO i1
; CHECK:     + END LOOP
; CHECK:   }
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:   if (%m != 0)
; CHECK:   {
; CHECK-NOT: if
; CHECK:     + DO i1
; CHECK:     + END LOOP
; CHECK:   }
; CHECK:   else
; CHECK:   {
; CHECK-NOT: if
; CHECK:     + DO i1
; CHECK:     + END LOOP
; CHECK:   }
; CHECK: }
; CHECK: END REGION

; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter -disable-output 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low -disable-output 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT: Global optimization report for : foo
; OPTREPORT: LOOP BEGIN at test.c (9, 0)
; OPTREPORT: <Predicate Optimized v4>
; OPTREPORT: LOOP END
; OPTREPORT: LOOP BEGIN at test.c (9, 0)
; OPTREPORT: <Predicate Optimized v2>
; OPTREPORT:     remark #25423: Invariant If condition at line 0 hoisted out of this loop
; OPTREPORT: LOOP END
; OPTREPORT: LOOP BEGIN at test.c (9, 0)
; OPTREPORT: <Predicate Optimized v3>
; OPTREPORT: LOOP END
; OPTREPORT: LOOP BEGIN at test.c (9, 0)
; OPTREPORT: <Predicate Optimized v1>
; OPTREPORT:     remark #25423: Invariant If condition at lines 9 and 3 hoisted out of this loop
; OPTREPORT:     remark #25423: Invariant If condition at line 0 hoisted out of this loop
; OPTREPORT: LOOP END

;Module Before HIR; ModuleID = '7.c'
source_filename = "7.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i64* nocapture %p, i64* nocapture %q, i64 %n, i64 %m) local_unnamed_addr #0 {
entry:
  %tobool = icmp ne i64 %n, 0
  %tobool2 = icmp eq i64 %m, 0
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %i.024 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  br i1 %tobool, label %if.then, label %if.end, !dbg !14

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i64, i64* %p, i64 %i.024
  store i64 %i.024, i64* %arrayidx, align 8
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %arrayidx1 = getelementptr inbounds i64, i64* %q, i64 %i.024
  store i64 %i.024, i64* %arrayidx1, align 8
  br i1 %tobool2, label %for.inc, label %if.then3

if.then3:                                         ; preds = %if.end
  %arrayidx5 = getelementptr inbounds i64, i64* %p, i64 %i.024
  store i64 %i.024, i64* %arrayidx5, align 8
  br i1 %tobool, label %if.then7, label %for.inc, !dbg !15

if.then7:                                         ; preds = %if.then3
  store i64 0, i64* %arrayidx1, align 8
  br label %for.inc

for.inc:                                          ; preds = %if.end, %if.then7, %if.then3
  %inc = add nuw nsw i64 %i.024, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

; attributes #0 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based icx (ICX) dev.8.x.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "/export/iusers/pgprokof/xmain-ws5/llvm/test/Transforms/Intel_LoopTransforms/HIROptPredicate")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"icx (ICX) dev.8.x.0"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!14 = !DILocation(line: 3, scope: !8)
!15 = !DILocation(line: 9, scope: !8)
