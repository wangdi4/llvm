; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-opt-predicate -print-before=hir-opt-predicate -print-after=hir-opt-predicate %s 2>&1 | FileCheck %s
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,print<hir>,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" %s 2>&1 | FileCheck %s

; Check the unswitching of two sibling switch statements with equivalent conditions but different set of cases.

;<0>          BEGIN REGION { }
;<30>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<2:2>              |   switch(%n)
;<2:2>              |   {
;<2:2>              |   case 1:
;<9>                |      @bar1();
;<2:2>              |      break;
;<2:2>              |   default:
;<6>                |      @bar2();
;<2:2>              |      break;
;<2:2>              |   }
;<12:11>            |   switch(%n)
;<12:11>            |   {
;<12:11>            |   case 2:
;<19>               |      @bar3();
;<12:11>            |      break;
;<12:11>            |   default:
;<16>               |      @bar4();
;<12:11>            |      break;
;<12:11>            |   }
;<30>               + END LOOP
;<0>          END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:      switch(%n)
; CHECK:      {
; CHECK:      case 1:
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   @bar1();
; CHECK:         |   @bar4();
; CHECK:         + END LOOP
; CHECK:         break;
; CHECK:      case 2:
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   @bar2();
; CHECK:         |   @bar3();
; CHECK:         + END LOOP
; CHECK:         break;
; CHECK:      default:
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   @bar2();
; CHECK:         |   @bar4();
; CHECK:         + END LOOP
; CHECK:         break;
; CHECK:      }
; CHECK: END REGION

; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-opt-predicate -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter -disable-output 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-opt-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low -disable-output 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT: Global optimization report for : foo
; OPTREPORT: LOOP BEGIN
; OPTREPORT: <Predicate Optimized v2>
; OPTREPORT: LOOP END
; OPTREPORT: LOOP BEGIN
; OPTREPORT: <Predicate Optimized v1>
; OPTREPORT:     remark #25424: Invariant Switch condition at lines 11 and 2 hoisted out of this loop
; OPTREPORT: LOOP END
; OPTREPORT: LOOP BEGIN
; OPTREPORT: <Predicate Optimized v3>
; OPTREPORT: LOOP END

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* %a, i32 %n) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  switch i32 %n, label %sw.default [
    i32 1, label %sw.bb
  ], !dbg !14

sw.bb:                                            ; preds = %for.body
  call void (...) @bar1()
  br label %sw.epilog

sw.default:                                       ; preds = %for.body
  call void (...) @bar2()
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb
  switch i32 %n, label %sw.default2 [
    i32 2, label %sw.bb1
  ], !dbg !15

sw.bb1:                                           ; preds = %sw.epilog
  call void (...) @bar3()
  br label %sw.epilog3

sw.default2:                                      ; preds = %sw.epilog
  call void (...) @bar4()
  br label %sw.epilog3

sw.epilog3:                                       ; preds = %sw.default2, %sw.bb1
  br label %for.inc

for.inc:                                          ; preds = %sw.epilog3
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

declare dso_local void @bar1(...)
declare dso_local void @bar2(...)
declare dso_local void @bar3(...)
declare dso_local void @bar4(...)

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
!14 = !DILocation(line: 2, scope: !8)
!15 = !DILocation(line: 11, scope: !8)
