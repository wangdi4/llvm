; The test checks the line numbers in HIR

; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework -hir-cost-model-throttling=0 | FileCheck %s
; RUN: opt < %s -opaque-pointers -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework -hir-cost-model-throttling=0 | FileCheck %s --check-prefix=CHECK-OPAQUE
; RUN: opt < %s -enable-new-pm=0 -hir-ssa-deconstruction -hir-cg -force-hir-cg -S -hir-cost-model-throttling=0 | FileCheck %s --check-prefix=CHECK-CG

; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -opaque-pointers -hir-cost-model-throttling=0 -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s --check-prefix=CHECK-OPAQUE
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-cost-model-throttling=0 -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cg" -hir-cost-model-throttling=0 -force-hir-cg -S 2>&1 | FileCheck %s -check-prefix=CHECK-CG

; Here is a source code for the test:
; <line num>
;
; 1 extern float bar();
; 2 extern int c1();
; 3 extern int c2();
; 4
; 5 void foo(float *a, float *c) {
; 6
; 7 float x = 0;
; 8
; 9 for (int i=0;i<100;i++) {
;10
;11 x += i * bar();
;12
;13 if (x > 10 || i < 10) {
;14   a[i] += x;
;15
;16   if (c1() && i > 0) {
;17     a[i] += x;
;18   }
;19 }
;20
;21 float b = a[i+1] + 2.0;
;22 float g = a[i+1] ? 10.0 : -10.0;
;23
;24 c[i] = b + g;
;25
;26 switch (c1()) {
;27 case 1:
;28   c[i] += c2();
;29   break;
;30 case 2:
;31   c[i] += c1();
;32   break;
;33 default:
;34   break;
;35 }
;36
;37 }
;38
;39 }

; The bitcast operator on function pointers goes away with opaque pointers.

; CHECK-OPAQUE: :9>        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-OPAQUE: :11>       |    %call = @bar();
; CHECK-OPAQUE: :9>        + END LOOP

; CHECK: <0>          BEGIN REGION { }
; CHECK: :9>             + DO i1 = 0, 99, 1   <DO_LOOP>
;
; CHECK: :11>            |   %conv = sitofp.i32.float(i1);
; CHECK: :11>            |   %call = bitcast (float (...)* @bar to float ()*)();
; CHECK: :11>            |   %mul = %conv  *  %call;
; CHECK: :11>            |   %x.060 = %x.060  +  %mul;
;
; CHECK: :13>            |   %cmp1 = %x.060 > 1.000000e+01;
; CHECK: :13>            |   %cmp3 = i1 < 10;
; CHECK: :13>            |   if (umax(%cmp3, %cmp1) != 0)
; CHECK: :13>            |   {
;
; CHECK: :14>            |      %0 = (%a)[i1];
; CHECK: :14>            |      %add5 = %x.060  +  %0;
; CHECK: :14>            |      (%a)[i1] = %add5;
;
; CHECK: :16>            |      %call6 = bitcast (i32 (...)* @c1 to i32 ()*)();
; CHECK: :16>            |      if (i1 > 0 && %call6 != 0)
; CHECK: :16>            |      {
;
; CHECK: :17>            |         %1 = (%a)[i1];
; CHECK: :17>            |         %add11 = %x.060  +  %1;
; CHECK: :17>            |         (%a)[i1] = %add11;
;
; CHECK: :16>            |      }
; CHECK: :13>            |   }
;
; CHECK: :21>            |   %2 = (%a)[i1 + 1];
; CHECK: :21>            |   %conv17 = %2  +  2.000000e+00;
;
; CHECK: :22>            |   %conv21 = (%2 !=u 0.000000e+00) ? 1.000000e+01 : -1.000000e+01;
;
; CHECK: :24>            |   %add22 = %conv17  +  %conv21;
; CHECK: :24>            |   (%c)[i1] = %add22;
;
; CHECK: :26>            |   %call24 = bitcast (i32 (...)* @c1 to i32 ()*)();
; CHECK: :26>            |   switch(%call24)
; CHECK: :26>            |   {
; CHECK: :26>            |   case 1:
;
; CHECK: :28>            |      %call25 = bitcast (i32 (...)* @c2 to i32 ()*)();
;
; CHECK:                 |      %conv31.sink.in = %call25;
; CHECK: :26>            |      break;
; CHECK: :26>            |   case 2:
;
; CHECK: :31>            |      %call30 = bitcast (i32 (...)* @c1 to i32 ()*)();
;
; CHECK:                 |      %conv31.sink.in = %call30;
; CHECK: :26>            |      break;
; CHECK: :26>            |   default:
; CHECK: :26>            |      goto for.cond.backedge;
; CHECK: :26>            |   }
;
; CHECK: :31>            |   %conv31.sink = sitofp.i32.float(%conv31.sink.in);
; CHECK: :31>            |   %3 = (%c)[i1];
; CHECK: :31>            |   %add33 = %3  +  %conv31.sink;
; CHECK: :31>            |   (%c)[i1] = %add33;
;
; CHECK:                 |   for.cond.backedge:
; CHECK: :9>             + END LOOP
; CHECK: <0>          END REGION

; CG:
; CHECK-CG: Module
; CHECK-CG-DAG: call void @llvm.dbg.declare(metadata float* {{.*}}, metadata !22, metadata {{.*}}), !dbg ![[m24:.*]]
; CHECK-CG-DAG: call void @llvm.dbg.declare(metadata float* {{.*}}, metadata !19, metadata {{.*}}), !dbg ![[m26:.*]]
; CHECK-CG-DAG: call void @llvm.dbg.declare(metadata float* {{.*}}, metadata !15, metadata {{.*}}), !dbg ![[m27:.*]]
; CHECK-CG: region.0:

; Call + float instructions
; CHECK-CG:  {{.*}} = sitofp i32 {{.*}} to float, !dbg ![[m33:.*]]
; CHECK-CG:  {{.*}} = tail call float bitcast (float (...)* @bar to float ()*)(), !dbg ![[m33]]
; CHECK-CG:  {{.*}} = fmul float {{.*}}, {{.*}}, !dbg ![[m33]]
; CHECK-CG:  {{.*}} = fadd float {{.*}}, {{.*}}, !dbg ![[m33]]
; CHECK-CG:  store float {{.*}}, float* {{.*}}, !dbg ![[m33]]

; IF + complex predicate
; CHECK-CG:  %hir.cmp.7 = fcmp ogt float {{.*}}, 1.000000e+01, !dbg ![[m35:.*]]
; CHECK-CG:  %hir.cmp.8 = icmp slt i32 {{.*}}, 10, !dbg ![[m37:.*]]
; CHECK-CG:  %hir.cmp.10 = icmp ne i1 {{.*}}, false, !dbg ![[m35]]
; CHECK-CG:  br i1 %hir.cmp.10, label %then.10, label %ifmerge.10, !dbg ![[m35]]

; IF + multiple predicates
; CHECK-CG:  %hir.cmp.22 = icmp sgt i32 {{.*}}, 0, !dbg ![[m47:.*]]
; CHECK-CG:  {{.*}} = load i32, i32* {{.*}}, !dbg ![[m45:.*]]
; CHECK-CG:  %hir.cmp.224 = icmp ne i32 {{.*}}, 0, !dbg ![[m45]]
; CHECK-CG:  {{.*}} = and i1 %hir.cmp.22, %hir.cmp.224, !dbg ![[m45]]
; CHECK-CG:  br i1 {{.*}}, label %then.22, label %ifmerge.22, !dbg ![[m45]]

; Load ref + CE
; CHECK-CG:  {{.*}} = load i32, i32* %i1.i32, align 4, !dbg ![[m26]]
; CHECK-CG:  {{.*}} = add i32 {{.*}}, 1, !dbg ![[m26]]
; CHECK-CG:  %[[ptr:.*]] = getelementptr inbounds float, float* %a, i32 {{.*}}, !dbg ![[m26]]
; CHECK-CG:  load float, float* %[[ptr]], align 4, !dbg ![[m26]]

; Scalar store
; CHECK-CG:  store float {{.*}}, float* {{.*}}, align 4, !dbg ![[m26]]

; Select
; CHECK-CG:  %hir.selcmp.37 = fcmp une float {{.*}}, 0.000000e+00, !dbg ![[m25:.*]]
; CHECK-CG:  {{.*}} = select i1 %hir.selcmp.37, float 1.000000e+01, float -1.000000e+01, !dbg ![[m25]]

; Store ref
; CHECK-CG:  %[[ptr2:.*]] = getelementptr inbounds float, float* %c, i32 {{.*}}, !dbg ![[m52:.*]]
; CHECK-CG:  {{.*}} = load float, float* {{.*}}, !dbg ![[m52]]
; CHECK-CG:  store float {{.*}}, float* %[[ptr2]], align 4, !dbg ![[m52]]

; Switch
; CHECK-CG:  switch i32 {{.*}}, label %hir.sw.43.default [
; CHECK-CG:  ], !dbg ![[m53:.*]]


;Module Before HIR; ModuleID = 'debug-nodes.c'
source_filename = "debug-nodes.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nounwind
define void @foo(float* nocapture %a, float* nocapture %c) local_unnamed_addr !dbg !7 {
entry:
  tail call void @llvm.dbg.value(metadata float* %a, i64 0, metadata !13, metadata !23), !dbg !24
  tail call void @llvm.dbg.value(metadata float* %c, i64 0, metadata !14, metadata !23), !dbg !24
  tail call void @llvm.dbg.value(metadata float 0.000000e+00, i64 0, metadata !15, metadata !23), !dbg !25
  tail call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !16, metadata !23), !dbg !26
  tail call void @llvm.dbg.value(metadata float 0.000000e+00, i64 0, metadata !15, metadata !23), !dbg !25
  tail call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !16, metadata !23), !dbg !26
  br label %for.body, !dbg !27

for.cond.cleanup:                                 ; preds = %for.cond.backedge
  ret void, !dbg !29

for.body:                                         ; preds = %for.cond.backedge, %entry
  %x.060 = phi float [ 0.000000e+00, %entry ], [ %add, %for.cond.backedge ]
  %i.059 = phi i32 [ 0, %entry ], [ %add13, %for.cond.backedge ]
  %conv = sitofp i32 %i.059 to float, !dbg !30
  %call = tail call float bitcast (float (...)* @bar to float ()*)(), !dbg !30
  %mul = fmul float %conv, %call, !dbg !30
  %add = fadd float %x.060, %mul, !dbg !30
  tail call void @llvm.dbg.value(metadata float %add, i64 0, metadata !15, metadata !23), !dbg !25
  %cmp1 = fcmp ogt float %add, 1.000000e+01, !dbg !31
  %cmp3 = icmp slt i32 %i.059, 10, !dbg !33
  %or.cond = or i1 %cmp3, %cmp1, !dbg !31
  br i1 %or.cond, label %if.then, label %if.end12, !dbg !31

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds float, float* %a, i32 %i.059, !dbg !35
  %0 = load float, float* %arrayidx, align 4, !dbg !35, !tbaa !37
  %add5 = fadd float %add, %0, !dbg !35
  store float %add5, float* %arrayidx, align 4, !dbg !35, !tbaa !37
  %call6 = tail call i32 bitcast (i32 (...)* @c1 to i32 ()*)(), !dbg !41
  %tobool = icmp ne i32 %call6, 0, !dbg !41
  %cmp7 = icmp sgt i32 %i.059, 0, !dbg !43
  %or.cond34 = and i1 %cmp7, %tobool, !dbg !41
  br i1 %or.cond34, label %if.then9, label %if.end12, !dbg !41

if.then9:                                         ; preds = %if.then
  %1 = load float, float* %arrayidx, align 4, !dbg !45, !tbaa !37
  %add11 = fadd float %add, %1, !dbg !45
  store float %add11, float* %arrayidx, align 4, !dbg !45, !tbaa !37
  br label %if.end12, !dbg !47

if.end12:                                         ; preds = %if.then, %if.then9, %for.body
  %add13 = add nuw nsw i32 %i.059, 1, !dbg !48
  %arrayidx14 = getelementptr inbounds float, float* %a, i32 %add13, !dbg !48
  %2 = load float, float* %arrayidx14, align 4, !dbg !48, !tbaa !37
  %conv17 = fadd float %2, 2.000000e+00, !dbg !48
  tail call void @llvm.dbg.value(metadata float %conv17, i64 0, metadata !19, metadata !23), !dbg !48
  %tobool20 = fcmp une float %2, 0.000000e+00, !dbg !49
  %conv21 = select i1 %tobool20, float 1.000000e+01, float -1.000000e+01, !dbg !49
  tail call void @llvm.dbg.value(metadata float %conv21, i64 0, metadata !22, metadata !23), !dbg !49
  %add22 = fadd float %conv17, %conv21, !dbg !50
  %arrayidx23 = getelementptr inbounds float, float* %c, i32 %i.059, !dbg !50
  store float %add22, float* %arrayidx23, align 4, !dbg !50, !tbaa !37
  %call24 = tail call i32 bitcast (i32 (...)* @c1 to i32 ()*)(), !dbg !51
  switch i32 %call24, label %for.cond.backedge [
    i32 1, label %sw.bb
    i32 2, label %sw.bb29
  ], !dbg !51

sw.bb:                                            ; preds = %if.end12
  %call25 = tail call i32 bitcast (i32 (...)* @c2 to i32 ()*)(), !dbg !52
  br label %sw.epilog.sink.split, !dbg !54

sw.bb29:                                          ; preds = %if.end12
  %call30 = tail call i32 bitcast (i32 (...)* @c1 to i32 ()*)(), !dbg !55
  br label %sw.epilog.sink.split, !dbg !56

sw.epilog.sink.split:                             ; preds = %sw.bb, %sw.bb29
  %conv31.sink.in = phi i32 [ %call30, %sw.bb29 ], [ %call25, %sw.bb ]
  %conv31.sink = sitofp i32 %conv31.sink.in to float, !dbg !55
  %3 = load float, float* %arrayidx23, align 4, !dbg !55, !tbaa !37
  %add33 = fadd float %3, %conv31.sink, !dbg !55
  store float %add33, float* %arrayidx23, align 4, !dbg !55, !tbaa !37
  br label %for.cond.backedge, !dbg !57

for.cond.backedge:                                ; preds = %sw.epilog.sink.split, %if.end12
  tail call void @llvm.dbg.value(metadata float %add, i64 0, metadata !15, metadata !23), !dbg !25
  tail call void @llvm.dbg.value(metadata i32 %add13, i64 0, metadata !16, metadata !23), !dbg !26
  %exitcond = icmp eq i32 %add13, 100, !dbg !58
  br i1 %exitcond, label %for.cond.cleanup, label %for.body, !dbg !27, !llvm.loop !60
}

declare float @bar(...) local_unnamed_addr

declare i32 @c1(...) local_unnamed_addr

declare i32 @c2(...) local_unnamed_addr

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!llvm.dbg.intel.emit_class_debug_always = !{!5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C89, file: !1, producer: "clang version 4.0.0 (trunk 20848) (llvm/branches/loopopt 20990)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "debug-nodes.c", directory: "/export/iusers/pgprokof/loopopt-3/llvm/test/Analysis/Intel_LoopAnalysis/HIRParser")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{!"true"}
!6 = !{!"clang version 4.0.0 (trunk 20848) (llvm/branches/loopopt 20990)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 5, type: !8, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !12)
!8 = !DISubroutineType(types: !9)
!9 = !{null, !10, !10}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 32)
!11 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!12 = !{!13, !14, !15, !16, !19, !22}
!13 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 5, type: !10)
!14 = !DILocalVariable(name: "c", arg: 2, scope: !7, file: !1, line: 5, type: !10)
!15 = !DILocalVariable(name: "x", scope: !7, file: !1, line: 7, type: !11)
!16 = !DILocalVariable(name: "i", scope: !17, file: !1, line: 9, type: !18)
!17 = distinct !DILexicalBlock(scope: !7, file: !1, line: 9)
!18 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!19 = !DILocalVariable(name: "b", scope: !20, file: !1, line: 21, type: !11)
!20 = distinct !DILexicalBlock(scope: !21, file: !1, line: 9)
!21 = distinct !DILexicalBlock(scope: !17, file: !1, line: 9)
!22 = !DILocalVariable(name: "g", scope: !20, file: !1, line: 22, type: !11)
!23 = !DIExpression()
!24 = !DILocation(line: 5, scope: !7)
!25 = !DILocation(line: 7, scope: !7)
!26 = !DILocation(line: 9, scope: !17)
!27 = !DILocation(line: 9, scope: !28)
!28 = !DILexicalBlockFile(scope: !17, file: !1, discriminator: 1)
!29 = !DILocation(line: 39, scope: !7)
!30 = !DILocation(line: 11, scope: !20)
!31 = !DILocation(line: 13, scope: !32)
!32 = distinct !DILexicalBlock(scope: !20, file: !1, line: 13)
!33 = !DILocation(line: 13, scope: !34)
!34 = !DILexicalBlockFile(scope: !32, file: !1, discriminator: 1)
!35 = !DILocation(line: 14, scope: !36)
!36 = distinct !DILexicalBlock(scope: !32, file: !1, line: 13)
!37 = !{!38, !38, i64 0}
!38 = !{!"float", !39, i64 0}
!39 = !{!"omnipotent char", !40, i64 0}
!40 = !{!"Simple C/C++ TBAA"}
!41 = !DILocation(line: 16, scope: !42)
!42 = distinct !DILexicalBlock(scope: !36, file: !1, line: 16)
!43 = !DILocation(line: 16, scope: !44)
!44 = !DILexicalBlockFile(scope: !42, file: !1, discriminator: 1)
!45 = !DILocation(line: 17, scope: !46)
!46 = distinct !DILexicalBlock(scope: !42, file: !1, line: 16)
!47 = !DILocation(line: 18, scope: !46)
!48 = !DILocation(line: 21, scope: !20)
!49 = !DILocation(line: 22, scope: !20)
!50 = !DILocation(line: 24, scope: !20)
!51 = !DILocation(line: 26, scope: !20)
!52 = !DILocation(line: 28, scope: !53)
!53 = distinct !DILexicalBlock(scope: !20, file: !1, line: 26)
!54 = !DILocation(line: 29, scope: !53)
!55 = !DILocation(line: 31, scope: !53)
!56 = !DILocation(line: 32, scope: !53)
!57 = !DILocation(line: 37, scope: !21)
!58 = !DILocation(line: 9, scope: !59)
!59 = !DILexicalBlockFile(scope: !21, file: !1, discriminator: 1)
!60 = distinct !{!60, !26, !61}
!61 = !DILocation(line: 37, scope: !17)
