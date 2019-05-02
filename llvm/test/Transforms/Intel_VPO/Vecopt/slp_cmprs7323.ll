; RUN: opt < %s -slp-vectorizer -S -enable-intel-advanced-opts -mattr=+avx2
; ModuleID = 'creduce-meow.cpp'
source_filename = "creduce-meow.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.B = type { %class.A, %class.A }
%class.A = type { [3 x i32] }

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @_ZNK1B8longsideERi(%class.B* nocapture readonly %this, i32* nocapture dereferenceable(4) %dir) local_unnamed_addr #0 align 2 !dbg !8 {
entry:
  call void @llvm.dbg.value(metadata %class.B* %this, metadata !35, metadata !DIExpression()), !dbg !44
  call void @llvm.dbg.value(metadata i32* %dir, metadata !37, metadata !DIExpression()), !dbg !45
  call void @llvm.dbg.value(metadata %class.B* %this, metadata !46, metadata !DIExpression(DW_OP_plus_uconst, 12, DW_OP_stack_value)), !dbg !51
  call void @llvm.dbg.value(metadata i32 undef, metadata !49, metadata !DIExpression()), !dbg !53
  %arrayidx.i = getelementptr inbounds %class.B, %class.B* %this, i64 0, i32 1, i32 0, i64 0, !dbg !54, !intel-tbaa !55
  %0 = load i32, i32* %arrayidx.i, align 4, !dbg !54, !tbaa !55
  call void @llvm.dbg.value(metadata %class.B* %this, metadata !46, metadata !DIExpression()), !dbg !62
  call void @llvm.dbg.value(metadata i32 undef, metadata !49, metadata !DIExpression()), !dbg !64
  %arrayidx.i14 = getelementptr inbounds %class.B, %class.B* %this, i64 0, i32 0, i32 0, i64 0, !dbg !65, !intel-tbaa !66
  %1 = load i32, i32* %arrayidx.i14, align 4, !dbg !65, !tbaa !66
  %sub = add i32 %0, 1, !dbg !67
  %add = sub i32 %sub, %1, !dbg !68
  call void @llvm.dbg.value(metadata i32 %add, metadata !38, metadata !DIExpression()), !dbg !69
  call void @llvm.dbg.value(metadata i32 %add, metadata !41, metadata !DIExpression()), !dbg !70
  call void @llvm.dbg.value(metadata i32 1, metadata !42, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i32 %add, metadata !41, metadata !DIExpression()), !dbg !70
  %arrayIdx = getelementptr inbounds %class.B, %class.B* %this, i64 0, i32 1, i32 0, i64 1, !dbg !72
  %gepload = load i32, i32* %arrayIdx, align 4, !dbg !72, !tbaa !55
  %arrayIdx19 = getelementptr inbounds %class.B, %class.B* %this, i64 0, i32 0, i32 0, i64 1, !dbg !81
  %gepload20 = load i32, i32* %arrayIdx19, align 4, !dbg !81, !tbaa !66
  %2 = sub i32 %gepload, %gepload20, !dbg !83
  %3 = add i32 %2, 1, !dbg !83
  %hir.cmp.39 = icmp sgt i32 %3, %add, !dbg !84
  br i1 %hir.cmp.39, label %then.39, label %ifmerge.39, !dbg !85

then.39:                                          ; preds = %entry
  store i32 1, i32* %dir, align 4, !dbg !86, !tbaa !87
  call void @llvm.dbg.value(metadata i32 1, metadata !41, metadata !DIExpression()), !dbg !70
  br label %ifmerge.39, !dbg !85

ifmerge.39:                                       ; preds = %then.39, %entry
  %t3.0 = phi i32 [ 1, %then.39 ], [ %add, %entry ]
  call void @llvm.dbg.value(metadata i32 %t3.0, metadata !41, metadata !DIExpression()), !dbg !70
  %arrayIdx21 = getelementptr inbounds %class.B, %class.B* %this, i64 0, i32 1, i32 0, i64 2, !dbg !72
  %gepload22 = load i32, i32* %arrayIdx21, align 4, !dbg !72, !tbaa !55
  %arrayIdx23 = getelementptr inbounds %class.B, %class.B* %this, i64 0, i32 0, i32 0, i64 2, !dbg !81
  %gepload24 = load i32, i32* %arrayIdx23, align 4, !dbg !81, !tbaa !66
  %4 = sub i32 %gepload22, %gepload24, !dbg !83
  %5 = add i32 %4, 1, !dbg !83
  %hir.cmp.17 = icmp sgt i32 %5, %t3.0, !dbg !84
  br i1 %hir.cmp.17, label %then.17, label %ifmerge.17, !dbg !85

then.17:                                          ; preds = %ifmerge.39
  store i32 2, i32* %dir, align 4, !dbg !86, !tbaa !87
  call void @llvm.dbg.value(metadata i32 2, metadata !41, metadata !DIExpression()), !dbg !70
  br label %ifmerge.17, !dbg !85

ifmerge.17:                                       ; preds = %then.17, %ifmerge.39
  ret i32 undef, !dbg !88
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang ac851cfbcd4a1eacb0869ed900c16428bfa067b4) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm de318ab56e50813085f3b1ebd5e801fd7213bf40)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "creduce-meow.cpp", directory: "/export/iusers/ebrevnov/tmp")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang ac851cfbcd4a1eacb0869ed900c16428bfa067b4) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm de318ab56e50813085f3b1ebd5e801fd7213bf40)"}
!8 = distinct !DISubprogram(name: "longside", linkageName: "_ZNK1B8longsideERi", scope: !9, file: !1, line: 12, type: !31, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, declaration: !30, retainedNodes: !34)
!9 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "B", file: !1, line: 6, size: 192, flags: DIFlagTypePassByValue, elements: !10, identifier: "_ZTS1B")
!10 = !{!11, !24, !25, !30}
!11 = !DIDerivedType(tag: DW_TAG_member, name: "smallend", scope: !9, file: !1, line: 9, baseType: !12, size: 96)
!12 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "A", file: !1, line: 1, size: 96, flags: DIFlagTypePassByValue, elements: !13, identifier: "_ZTS1A")
!13 = !{!14, !19}
!14 = !DIDerivedType(tag: DW_TAG_member, name: "vect", scope: !12, file: !1, line: 4, baseType: !15, size: 96, flags: DIFlagPublic)
!15 = !DICompositeType(tag: DW_TAG_array_type, baseType: !16, size: 96, elements: !17)
!16 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!17 = !{!18}
!18 = !DISubrange(count: 3)
!19 = !DISubprogram(name: "operator[]", linkageName: "_ZNK1AixEi", scope: !12, file: !1, line: 3, type: !20, scopeLine: 3, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!20 = !DISubroutineType(types: !21)
!21 = !{!16, !22, !16}
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!23 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !12)
!24 = !DIDerivedType(tag: DW_TAG_member, name: "bigend", scope: !9, file: !1, line: 10, baseType: !12, size: 96, offset: 96)
!25 = !DISubprogram(name: "length", linkageName: "_ZNK1B6lengthEi", scope: !9, file: !1, line: 7, type: !26, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!26 = !DISubroutineType(types: !27)
!27 = !{!16, !28, !16}
!28 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !29, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!29 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !9)
!30 = !DISubprogram(name: "longside", linkageName: "_ZNK1B8longsideERi", scope: !9, file: !1, line: 8, type: !31, scopeLine: 8, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!31 = !DISubroutineType(types: !32)
!32 = !{!16, !28, !33}
!33 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !16, size: 64)
!34 = !{!35, !37, !38, !39, !41, !42}
!35 = !DILocalVariable(name: "this", arg: 1, scope: !8, type: !36, flags: DIFlagArtificial | DIFlagObjectPointer)
!36 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !29, size: 64)
!37 = !DILocalVariable(name: "dir", arg: 2, scope: !8, file: !1, line: 12, type: !33)
!38 = !DILocalVariable(name: "__trans_tmp_1", scope: !8, file: !1, line: 13, type: !16)
!39 = !DILocalVariable(name: "dir", scope: !40, file: !1, line: 15, type: !16)
!40 = distinct !DILexicalBlock(scope: !8, file: !1, line: 14, column: 3)
!41 = !DILocalVariable(name: "maxlen", scope: !8, file: !1, line: 18, type: !16)
!42 = !DILocalVariable(name: "i", scope: !43, file: !1, line: 19, type: !16)
!43 = distinct !DILexicalBlock(scope: !8, file: !1, line: 19, column: 3)
!44 = !DILocation(line: 0, scope: !8)
!45 = !DILocation(line: 12, column: 18, scope: !8)
!46 = !DILocalVariable(name: "this", arg: 1, scope: !47, type: !50, flags: DIFlagArtificial | DIFlagObjectPointer)
!47 = distinct !DISubprogram(name: "operator[]", linkageName: "_ZNK1AixEi", scope: !12, file: !1, line: 3, type: !20, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, declaration: !19, retainedNodes: !48)
!48 = !{!46, !49}
!49 = !DILocalVariable(name: "i", arg: 2, scope: !47, file: !1, line: 3, type: !16)
!50 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!51 = !DILocation(line: 0, scope: !47, inlinedAt: !52)
!52 = distinct !DILocation(line: 16, column: 21, scope: !40)
!53 = !DILocation(line: 3, column: 18, scope: !47, inlinedAt: !52)
!54 = !DILocation(line: 3, column: 36, scope: !47, inlinedAt: !52)
!55 = !{!56, !59, i64 12}
!56 = !{!"struct@_ZTS1B", !57, i64 0, !57, i64 12}
!57 = !{!"struct@_ZTS1A", !58, i64 0}
!58 = !{!"array@_ZTSA3_i", !59, i64 0}
!59 = !{!"int", !60, i64 0}
!60 = !{!"omnipotent char", !61, i64 0}
!61 = !{!"Simple C++ TBAA"}
!62 = !DILocation(line: 0, scope: !47, inlinedAt: !63)
!63 = distinct !DILocation(line: 16, column: 35, scope: !40)
!64 = !DILocation(line: 3, column: 18, scope: !47, inlinedAt: !63)
!65 = !DILocation(line: 3, column: 36, scope: !47, inlinedAt: !63)
!66 = !{!56, !59, i64 0}
!67 = !DILocation(line: 16, column: 33, scope: !40)
!68 = !DILocation(line: 16, column: 49, scope: !40)
!69 = !DILocation(line: 13, column: 7, scope: !8)
!70 = !DILocation(line: 18, column: 7, scope: !8)
!71 = !DILocation(line: 19, column: 12, scope: !43)
!72 = !DILocation(line: 3, column: 36, scope: !47, inlinedAt: !73)
!73 = distinct !DILocation(line: 7, column: 34, scope: !74, inlinedAt: !78)
!74 = distinct !DISubprogram(name: "length", linkageName: "_ZNK1B6lengthEi", scope: !9, file: !1, line: 7, type: !26, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, declaration: !25, retainedNodes: !75)
!75 = !{!76, !77}
!76 = !DILocalVariable(name: "this", arg: 1, scope: !74, type: !36, flags: DIFlagArtificial | DIFlagObjectPointer)
!77 = !DILocalVariable(name: "dir", arg: 2, scope: !74, file: !1, line: 7, type: !16)
!78 = distinct !DILocation(line: 20, column: 9, scope: !79)
!79 = distinct !DILexicalBlock(scope: !80, file: !1, line: 20, column: 9)
!80 = distinct !DILexicalBlock(scope: !43, file: !1, line: 19, column: 3)
!81 = !DILocation(line: 3, column: 36, scope: !47, inlinedAt: !82)
!82 = distinct !DILocation(line: 7, column: 48, scope: !74, inlinedAt: !78)
!83 = !DILocation(line: 7, column: 62, scope: !74, inlinedAt: !78)
!84 = !DILocation(line: 20, column: 19, scope: !79)
!85 = !DILocation(line: 20, column: 9, scope: !80)
!86 = !DILocation(line: 21, column: 20, scope: !79)
!87 = !{!59, !59, i64 0}
!88 = !DILocation(line: 22, column: 1, scope: !8)

