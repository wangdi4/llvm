; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,hir-optreport-emitter" -intel-opt-report=low -aa-pipeline="basic-aa"  < %s -disable-output 2>&1 | FileCheck %s

; This test case checks that the opt report was generated correctly and
; the line number for the debug information is correct. It was created from
; the following test case:

; int foo(int *a, int n, int m) {
; 
;   int res = 0;
;   for(int i = 0; i < n; i++) {
;     if (m + i == 2) {
;       res += (0 + a[i]);
;       a[i] = a[m];
;     } else {
;       res += (1 + a[i]);
;       a[i] = a[m] + n;
;     }
;   }
; 
;   return res;
; }

; The loop is at line 4 and the If condition should be line 5.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if (i1 + sext.i32.i64(%m) == 2)
;       |   {
;       |      %2 = (%a)[i1];
;       |      (%a)[i1] = (%a)[%m];
;       |      %.pn = %2;
;       |   }
;       |   else
;       |   {
;       |      %4 = (%a)[i1];
;       |      %5 = (%a)[%m];
;       |      (%a)[i1] = %n + %5;
;       |      %.pn = %4 + 1;
;       |   }
;       |   %res.034 = %.pn  +  %res.034;
;       + END LOOP
; END REGION

; HIR after transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   %cldst.motioned = (%a)[i1];
;       |   %cldst.hoisted3 = (%a)[%m];
;       |   if (i1 + sext.i32.i64(%m) == 2)
;       |   {
;       |      %2 = %cldst.motioned;
;       |      %cldst.motioned = %cldst.hoisted3;
;       |      %.pn = %2;
;       |   }
;       |   else
;       |   {
;       |      %4 = %cldst.motioned;
;       |      %5 = %cldst.hoisted3;
;       |      %cldst.motioned = %n + %5;
;       |      %.pn = %4 + 1;
;       |   }
;       |   (%a)[i1] = %cldst.motioned;
;       |   %res.034 = %.pn  +  %res.034;
;       + END LOOP
; END REGION


; CHECK: Report from: HIR Loop optimizations framework for : _Z3fooPiii
; CHECK: LOOP BEGIN at simple.cpp (4, 3)
; CHECK:     remark #25589: 4 loads hoisted out of If at line 5 to make them unconditional in loop
; CHECK:     remark #25590: 2 stores sunk out of If at line 5 to make them unconditional in loop
; CHECK: LOOP END


;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local noundef i32 @_Z3fooPiii(ptr nocapture noundef %a, i32 noundef %n, i32 noundef %m) local_unnamed_addr #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata ptr %a, metadata !13, metadata !DIExpression()), !dbg !19
  call void @llvm.dbg.value(metadata i32 %n, metadata !14, metadata !DIExpression()), !dbg !19
  call void @llvm.dbg.value(metadata i32 %m, metadata !15, metadata !DIExpression()), !dbg !19
  call void @llvm.dbg.value(metadata i32 0, metadata !16, metadata !DIExpression()), !dbg !19
  call void @llvm.dbg.value(metadata i32 0, metadata !17, metadata !DIExpression()), !dbg !20
  %cmp33 = icmp sgt i32 %n, 0, !dbg !21
  br i1 %cmp33, label %for.body.lr.ph, label %for.cond.cleanup, !dbg !23

for.body.lr.ph:                                   ; preds = %entry
  %idxprom12 = sext i32 %m to i64, !dbg !24
  %arrayidx13 = getelementptr inbounds i32, ptr %a, i64 %idxprom12, !dbg !24
  %wide.trip.count = zext i32 %n to i64, !dbg !21
  br label %for.body, !dbg !23

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %res.1.lcssa = phi i32 [ %res.1, %for.inc ], !dbg !28
  br label %for.cond.cleanup, !dbg !29

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %res.0.lcssa = phi i32 [ 0, %entry ], [ %res.1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %res.0.lcssa, !dbg !29

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %res.034 = phi i32 [ 0, %for.body.lr.ph ], [ %res.1, %for.inc ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !17, metadata !DIExpression()), !dbg !20
  call void @llvm.dbg.value(metadata i32 %res.034, metadata !16, metadata !DIExpression()), !dbg !19
  %0 = add nsw i64 %indvars.iv, %idxprom12, !dbg !30
  %1 = icmp eq i64 %0, 2
  br i1 %1, label %if.then, label %if.else, !dbg !31

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv, !dbg !32
  %2 = load i32, ptr %arrayidx, align 4, !dbg !32, !tbaa !34
  call void @llvm.dbg.value(metadata !DIArgList(i32 %res.034, i32 %2), metadata !16, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value)), !dbg !19
  %3 = load i32, ptr %arrayidx13, align 4, !dbg !38, !tbaa !34
  store i32 %3, ptr %arrayidx, align 4, !dbg !39, !tbaa !34
  br label %for.inc, !dbg !40

if.else:                                          ; preds = %for.body
  %arrayidx9 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv, !dbg !41
  %4 = load i32, ptr %arrayidx9, align 4, !dbg !41, !tbaa !34
  %add10 = add nsw i32 %4, 1, !dbg !42
  call void @llvm.dbg.value(metadata !DIArgList(i32 %res.034, i32 %add10), metadata !16, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value)), !dbg !19
  %5 = load i32, ptr %arrayidx13, align 4, !dbg !24, !tbaa !34
  %add14 = add nsw i32 %5, %n, !dbg !43
  store i32 %add14, ptr %arrayidx9, align 4, !dbg !44, !tbaa !34
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %.pn = phi i32 [ %2, %if.then ], [ %add10, %if.else ]
  %res.1 = add nsw i32 %.pn, %res.034, !dbg !28
  call void @llvm.dbg.value(metadata i32 %res.1, metadata !16, metadata !DIExpression()), !dbg !19
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !45
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next, metadata !17, metadata !DIExpression()), !dbg !20
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count, !dbg !21
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !dbg !23, !llvm.loop !46
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)", isOptimized: true, flags: " --intel -O2 -g -c simple.cpp -mllvm -print-module-before-loopopt -fveclib=SVML -fheinous-gnu-extensions", runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "simple.cpp", directory: "/localdisk2/ayrivera/dev-cond-ld-str-opt-report/llvm/llvm/test/Transforms/Intel_LoopTransforms/HIRConditionalLoadStoreMotion")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"uwtable", i32 2}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!7 = distinct !DISubprogram(name: "foo", linkageName: "_Z3fooPiii", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !12)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11, !10, !10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!12 = !{!13, !14, !15, !16, !17}
!13 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 1, type: !11)
!14 = !DILocalVariable(name: "n", arg: 2, scope: !7, file: !1, line: 1, type: !10)
!15 = !DILocalVariable(name: "m", arg: 3, scope: !7, file: !1, line: 1, type: !10)
!16 = !DILocalVariable(name: "res", scope: !7, file: !1, line: 3, type: !10)
!17 = !DILocalVariable(name: "i", scope: !18, file: !1, line: 4, type: !10)
!18 = distinct !DILexicalBlock(scope: !7, file: !1, line: 4, column: 3)
!19 = !DILocation(line: 0, scope: !7)
!20 = !DILocation(line: 0, scope: !18)
!21 = !DILocation(line: 4, column: 20, scope: !22)
!22 = distinct !DILexicalBlock(scope: !18, file: !1, line: 4, column: 3)
!23 = !DILocation(line: 4, column: 3, scope: !18)
!24 = !DILocation(line: 10, column: 14, scope: !25)
!25 = distinct !DILexicalBlock(scope: !26, file: !1, line: 8, column: 12)
!26 = distinct !DILexicalBlock(scope: !27, file: !1, line: 5, column: 9)
!27 = distinct !DILexicalBlock(scope: !22, file: !1, line: 4, column: 30)
!28 = !DILocation(line: 0, scope: !26)
!29 = !DILocation(line: 14, column: 3, scope: !7)
!30 = !DILocation(line: 5, column: 11, scope: !26)
!31 = !DILocation(line: 5, column: 9, scope: !27)
!32 = !DILocation(line: 6, column: 19, scope: !33)
!33 = distinct !DILexicalBlock(scope: !26, file: !1, line: 5, column: 21)
!34 = !{!35, !35, i64 0}
!35 = !{!"int", !36, i64 0}
!36 = !{!"omnipotent char", !37, i64 0}
!37 = !{!"Simple C++ TBAA"}
!38 = !DILocation(line: 7, column: 14, scope: !33)
!39 = !DILocation(line: 7, column: 12, scope: !33)
!40 = !DILocation(line: 8, column: 5, scope: !33)
!41 = !DILocation(line: 9, column: 19, scope: !25)
!42 = !DILocation(line: 9, column: 17, scope: !25)
!43 = !DILocation(line: 10, column: 19, scope: !25)
!44 = !DILocation(line: 10, column: 12, scope: !25)
!45 = !DILocation(line: 4, column: 26, scope: !22)
!46 = distinct !{!46, !23, !47, !48}
!47 = !DILocation(line: 12, column: 3, scope: !18)
!48 = !{!"llvm.loop.mustprogress"}
