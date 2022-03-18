; RUN: opt -hir-ssa-deconstruction -hir-opt-var-predicate -S -print-after=hir-opt-var-predicate -disable-output  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s

; Check that both equal candidates (i1 == 20) will be handled at once.

;<0>          BEGIN REGION { }
;<43>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<3:3>              |   if (i1 == 20)
;<3:3>              |   {
;<7>                |      (%p)[20] = 21;
;<3:3>              |   }
;<3:3>              |   else
;<3:3>              |   {
;<14>               |      (%p)[i1 + 1] = i1 + -1;
;<3:3>              |   }
;<18>               |   %3 = (%a)[i1];
;<20>               |   (%a)[i1] = %3 + 1;
;<22>               |   %4 = (%q)[i1];
;<23:9>             |   if (i1 == 20)
;<23:9>             |   {
;<28>               |      (%q)[i1] = %4 + 1;
;<23:9>             |   }
;<23:9>             |   else
;<23:9>             |   {
;<34>               |      (%q)[i1 + 1] = %4 + -1;
;<23:9>             |   }
;<43>               + END LOOP
;<0>          END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 19, 1
; CHECK:       |   (%p)[i1 + 1] = i1 + -1;
; CHECK:       |   %3 = (%a)[i1];
; CHECK:       |   (%a)[i1] = %3 + 1;
; CHECK:       |   %4 = (%q)[i1];
; CHECK:       |   (%q)[i1 + 1] = %4 + -1;
; CHECK:       + END LOOP
;
; CHECK:       (%p)[20] = 21;
; CHECK:       %3 = (%a)[20];
; CHECK:       (%a)[20] = %3 + 1;
; CHECK:       %4 = (%q)[20];
; CHECK:       (%q)[20] = %4 + 1;
;
; CHECK:       + DO i1 = 0, 78, 1
; CHECK:       |   (%p)[i1 + 22] = i1 + 20;
; CHECK:       |   %3 = (%a)[i1 + 21];
; CHECK:       |   (%a)[i1 + 21] = %3 + 1;
; CHECK:       |   %4 = (%q)[i1 + 21];
; CHECK:       |   (%q)[i1 + 22] = %4 + -1;
; CHECK:       + END LOOP
; CHECK: END REGION

; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-var-predicate -disable-output -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-opt-var-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -disable-output -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT: Global optimization report for : foo
; OPTREPORT: LOOP BEGIN
; OPTREPORT: <Predicate Optimized v1>
; OPTREPORT:     remark #25580: Induction variable range split using condition at lines 3 and 9
; OPTREPORT: LOOP END
; OPTREPORT: LOOP BEGIN
; OPTREPORT: <Predicate Optimized v2>
; OPTREPORT: LOOP END

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %p, i32* nocapture %q, i32* nocapture %a) local_unnamed_addr #0 {
entry:
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 20
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %cmp1 = icmp eq i64 %indvars.iv, 20
  br i1 %cmp1, label %if.then, label %if.else, !dbg !14

if.then:                                          ; preds = %for.body
  store i32 21, i32* %arrayidx, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %0 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds i32, i32* %p, i64 %0
  %1 = trunc i64 %indvars.iv to i32
  %2 = add i32 %1, -1
  store i32 %2, i32* %arrayidx4, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %arrayidx6 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx6, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, i32* %arrayidx6, align 4
  %arrayidx10 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx10, align 4
  br i1 %cmp1, label %if.then8, label %if.else14, !dbg !15

if.then8:                                         ; preds = %if.end
  %add11 = add nsw i32 %4, 1
  store i32 %add11, i32* %arrayidx10, align 4
  br label %for.inc

if.else14:                                        ; preds = %if.end
  %sub17 = add nsw i32 %4, -1
  %5 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx20 = getelementptr inbounds i32, i32* %q, i64 %5
  store i32 %sub17, i32* %arrayidx20, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then8, %if.else14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

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
