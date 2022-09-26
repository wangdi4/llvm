; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -hir-vec-dir-insert -hir-vplan-vec -hir-cg -intel-ir-optreport-emitter -intel-opt-report=high 2>&1 -disable-output -vplan-force-vf=4 < %s | FileCheck %s
;
; Check that optreport functions properly when a remark is added to a loop after
; removing sibling optreport from it. Before the fix, removeOptReportField
; routine used to create a non-distinct OptReportImpl node, which was not
; possible (safe) to modify (e.g. add another remark to it).
;
; typedef struct MyStruct {
;     int *offset_global;
;     int *size_global;
;     int *size_local;
; } my_struct;
;
;
; void foo(int * restrict offset_global, int * restrict size_global, int * restrict size_local, int NumDims) {
;     int dim;
;     my_struct *obj = NULL;
;     obj = malloc (sizeof(my_struct));
;     obj->offset_global = malloc (sizeof(int) * NumDims);
;     obj->size_global = malloc (sizeof(int) * NumDims);
;     obj->size_local = malloc (sizeof(int) * NumDims);
;
;     for (dim = 0; dim < NumDims; dim++) {
;         if (offset_global) {
;             offset_global[dim] = obj->offset_global[dim];
;         }
;         if (size_global) {
;             size_global[dim] = obj->size_global[dim];
;         }
;         if (size_local) {
;             size_local[dim] = obj->size_local[dim];
;         }
;     }
; }
;
; CHECK:      LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Predicate Optimized v6>
; CHECK-NEXT:     remark #25423: Invariant If condition at line 25 hoisted out of this loop
; CHECK-NEXT:     remark #15300: LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-COUNT-25: remark{{.*}}:
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Remainder loop for vectorization>
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Predicate Optimized v7>
; CHECK-NEXT:     remark #15300: LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-COUNT-25: remark{{.*}}:
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Remainder loop for vectorization>
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Predicate Optimized v2>
; CHECK-NEXT:     remark #25423: Invariant If condition at line 22 hoisted out of this loop
; CHECK-NEXT:     remark #25423: Invariant If condition at line 25 hoisted out of this loop
; CHECK-NEXT:     remark #15300: LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-COUNT-25: remark{{.*}}:
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Remainder loop for vectorization>
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Predicate Optimized v5>
; CHECK-NEXT:     remark #15300: LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-COUNT-25: remark{{.*}}:
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Remainder loop for vectorization>
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Predicate Optimized v3>
; CHECK-NEXT:     remark #25423: Invariant If condition at line 25 hoisted out of this loop
; CHECK-NEXT:     remark #15300: LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-COUNT-25: remark{{.*}}:
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Remainder loop for vectorization>
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Predicate Optimized v4>
; CHECK-NEXT:     remark #15300: LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-COUNT-25: remark{{.*}}:
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Remainder loop for vectorization>
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Predicate Optimized v1>
; CHECK-NEXT:     remark #25423: Invariant If condition at line 19 hoisted out of this loop
; CHECK-NEXT:     remark #25423: Invariant If condition at line 22 hoisted out of this loop
; CHECK-NEXT:     remark #25423: Invariant If condition at line 25 hoisted out of this loop
; CHECK-NEXT:     remark #15300: LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-COUNT-25: remark{{.*}}:
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN at opt_report_crash_reproducer.c (18, 5)
; CHECK-NEXT: <Remainder loop for vectorization>
; CHECK-NEXT: LOOP END
;
;
target triple = "x86_64-unknown-linux-gnu"

%struct.MyStruct = type { i32*, i32*, i32* }

; Function Attrs: nofree nounwind uwtable
define dso_local void @foo(i32* noalias %offset_global, i32* noalias %size_global, i32* noalias %size_local, i32 %NumDims) local_unnamed_addr #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32* %offset_global, metadata !15, metadata !DIExpression()), !dbg !28
  call void @llvm.dbg.value(metadata i32* %size_global, metadata !16, metadata !DIExpression()), !dbg !28
  call void @llvm.dbg.value(metadata i32* %size_local, metadata !17, metadata !DIExpression()), !dbg !28
  call void @llvm.dbg.value(metadata i32 %NumDims, metadata !18, metadata !DIExpression()), !dbg !28
  call void @llvm.dbg.value(metadata %struct.MyStruct* null, metadata !20, metadata !DIExpression()), !dbg !28
  call void @llvm.dbg.value(metadata i8* undef, metadata !20, metadata !DIExpression()), !dbg !28
  %conv = sext i32 %NumDims to i64, !dbg !29
  %mul = shl nsw i64 %conv, 2, !dbg !30
  %call1 = tail call i8* @malloc(i64 %mul), !dbg !31
  %call5 = tail call i8* @malloc(i64 %mul), !dbg !32
  %call9 = tail call i8* @malloc(i64 %mul), !dbg !33
  call void @llvm.dbg.value(metadata i32 0, metadata !19, metadata !DIExpression()), !dbg !28
  %cmp51 = icmp sgt i32 %NumDims, 0, !dbg !34
  %0 = bitcast i8* %call1 to i32*, !dbg !37
  %1 = bitcast i8* %call5 to i32*, !dbg !37
  %2 = bitcast i8* %call9 to i32*, !dbg !37
  br i1 %cmp51, label %for.body.lr.ph, label %for.end, !dbg !37

for.body.lr.ph:                                   ; preds = %entry
  %tobool = icmp eq i32* %offset_global, null, !dbg !38
  %tobool15 = icmp eq i32* %size_global, null, !dbg !41
  %tobool23 = icmp eq i32* %size_local, null, !dbg !43
  br label %for.body, !dbg !37

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !19, metadata !DIExpression()), !dbg !28
  br i1 %tobool, label %if.end, label %if.then, !dbg !45

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 %indvars.iv, !dbg !46
  %3 = load i32, i32* %arrayidx, align 4, !dbg !46, !tbaa !48
  %arrayidx14 = getelementptr inbounds i32, i32* %offset_global, i64 %indvars.iv, !dbg !52
  store i32 %3, i32* %arrayidx14, align 4, !dbg !53, !tbaa !48
  br label %if.end, !dbg !54

if.end:                                           ; preds = %for.body, %if.then
  br i1 %tobool15, label %if.end22, label %if.then16, !dbg !55

if.then16:                                        ; preds = %if.end
  %arrayidx19 = getelementptr inbounds i32, i32* %1, i64 %indvars.iv, !dbg !56
  %4 = load i32, i32* %arrayidx19, align 4, !dbg !56, !tbaa !48
  %arrayidx21 = getelementptr inbounds i32, i32* %size_global, i64 %indvars.iv, !dbg !58
  store i32 %4, i32* %arrayidx21, align 4, !dbg !59, !tbaa !48
  br label %if.end22, !dbg !60

if.end22:                                         ; preds = %if.end, %if.then16
  br i1 %tobool23, label %for.inc, label %if.then24, !dbg !61

if.then24:                                        ; preds = %if.end22
  %arrayidx27 = getelementptr inbounds i32, i32* %2, i64 %indvars.iv, !dbg !62
  %5 = load i32, i32* %arrayidx27, align 4, !dbg !62, !tbaa !48
  %arrayidx29 = getelementptr inbounds i32, i32* %size_local, i64 %indvars.iv, !dbg !64
  store i32 %5, i32* %arrayidx29, align 4, !dbg !65, !tbaa !48
  br label %for.inc, !dbg !66

for.inc:                                          ; preds = %if.end22, %if.then24
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !67
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next, metadata !19, metadata !DIExpression()), !dbg !28
  %exitcond = icmp eq i64 %indvars.iv.next, %conv, !dbg !34
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !dbg !37, !llvm.loop !68

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end, !dbg !70

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void, !dbg !70
}

; Function Attrs: nofree nounwind
declare dso_local noalias i8* @malloc(i64) local_unnamed_addr #1
; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

attributes #0 = { nofree nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "opt_report_crash_reproducer.c", directory: "/home/optreport")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!7 = distinct !DISubprogram(name: "foo", scope: !8, file: !8, line: 10, type: !9, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !14)
!8 = !DIFile(filename: "opt_report_crash_reproducer.c", directory: "")
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11, !11, !11, !13}
!11 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !12)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !{!15, !16, !17, !18, !19, !20}
!15 = !DILocalVariable(name: "offset_global", arg: 1, scope: !7, file: !8, line: 10, type: !11)
!16 = !DILocalVariable(name: "size_global", arg: 2, scope: !7, file: !8, line: 10, type: !11)
!17 = !DILocalVariable(name: "size_local", arg: 3, scope: !7, file: !8, line: 10, type: !11)
!18 = !DILocalVariable(name: "NumDims", arg: 4, scope: !7, file: !8, line: 10, type: !13)
!19 = !DILocalVariable(name: "dim", scope: !7, file: !8, line: 11, type: !13)
!20 = !DILocalVariable(name: "obj", scope: !7, file: !8, line: 12, type: !21)
!21 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !22, size: 64)
!22 = !DIDerivedType(tag: DW_TAG_typedef, name: "my_struct", file: !8, line: 7, baseType: !23)
!23 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "MyStruct", file: !8, line: 3, size: 192, elements: !24)
!24 = !{!25, !26, !27}
!25 = !DIDerivedType(tag: DW_TAG_member, name: "offset_global", scope: !23, file: !8, line: 4, baseType: !12, size: 64)
!26 = !DIDerivedType(tag: DW_TAG_member, name: "size_global", scope: !23, file: !8, line: 5, baseType: !12, size: 64, offset: 64)
!27 = !DIDerivedType(tag: DW_TAG_member, name: "size_local", scope: !23, file: !8, line: 6, baseType: !12, size: 64, offset: 128)
!28 = !DILocation(line: 0, scope: !7)
!29 = !DILocation(line: 14, column: 48, scope: !7)
!30 = !DILocation(line: 14, column: 46, scope: !7)
!31 = !DILocation(line: 14, column: 26, scope: !7)
!32 = !DILocation(line: 15, column: 24, scope: !7)
!33 = !DILocation(line: 16, column: 23, scope: !7)
!34 = !DILocation(line: 18, column: 23, scope: !35)
!35 = distinct !DILexicalBlock(scope: !36, file: !8, line: 18, column: 5)
!36 = distinct !DILexicalBlock(scope: !7, file: !8, line: 18, column: 5)
!37 = !DILocation(line: 18, column: 5, scope: !36)
!38 = !DILocation(line: 19, column: 13, scope: !39)
!39 = distinct !DILexicalBlock(scope: !40, file: !8, line: 19, column: 13)
!40 = distinct !DILexicalBlock(scope: !35, file: !8, line: 18, column: 41)
!41 = !DILocation(line: 22, column: 13, scope: !42)
!42 = distinct !DILexicalBlock(scope: !40, file: !8, line: 22, column: 13)
!43 = !DILocation(line: 25, column: 13, scope: !44)
!44 = distinct !DILexicalBlock(scope: !40, file: !8, line: 25, column: 13)
!45 = !DILocation(line: 19, column: 13, scope: !40)
!46 = !DILocation(line: 20, column: 34, scope: !47)
!47 = distinct !DILexicalBlock(scope: !39, file: !8, line: 19, column: 28)
!48 = !{!49, !49, i64 0}
!49 = !{!"int", !50, i64 0}
!50 = !{!"omnipotent char", !51, i64 0}
!51 = !{!"Simple C/C++ TBAA"}
!52 = !DILocation(line: 20, column: 13, scope: !47)
!53 = !DILocation(line: 20, column: 32, scope: !47)
!54 = !DILocation(line: 21, column: 9, scope: !47)
!55 = !DILocation(line: 22, column: 13, scope: !40)
!56 = !DILocation(line: 23, column: 32, scope: !57)
!57 = distinct !DILexicalBlock(scope: !42, file: !8, line: 22, column: 26)
!58 = !DILocation(line: 23, column: 13, scope: !57)
!59 = !DILocation(line: 23, column: 30, scope: !57)
!60 = !DILocation(line: 24, column: 9, scope: !57)
!61 = !DILocation(line: 25, column: 13, scope: !40)
!62 = !DILocation(line: 26, column: 31, scope: !63)
!63 = distinct !DILexicalBlock(scope: !44, file: !8, line: 25, column: 25)
!64 = !DILocation(line: 26, column: 13, scope: !63)
!65 = !DILocation(line: 26, column: 29, scope: !63)
!66 = !DILocation(line: 27, column: 9, scope: !63)
!67 = !DILocation(line: 18, column: 37, scope: !35)
!68 = distinct !{!68, !37, !69}
!69 = !DILocation(line: 28, column: 5, scope: !36)
!70 = !DILocation(line: 29, column: 1, scope: !7)
