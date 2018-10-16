; RUN: opt < %s  -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -scoped-noalias -aa-eval -evaluate-aa-metadata -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
;
; It test whether the compiler can generate the scope alias metadata
; to indicate that a[i][j-1] or a[i][j+1] are overlapped with a[i][j].
; #pragma omp parallel for
;  for (int i=0; i<M; i++) {
;    for (int j=1; j<N-1; j++)
;      a[i][j] = a[i][j] + a[i][j-1] + a[i][j+1];
;  }

target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind uwtable
define dso_local void @BNL_hotloop(double %x, double %y, [512 x double]* %a) #0 !dbg !9 {
entry:
  %a.addr = alloca [512 x double]*, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  call void @llvm.dbg.value(metadata double %x, metadata !18, metadata !DIExpression()), !dbg !32
  call void @llvm.dbg.value(metadata double %y, metadata !19, metadata !DIExpression()), !dbg !33
  store [512 x double]* %a, [512 x double]** %a.addr, align 8, !tbaa !34
  call void @llvm.dbg.declare(metadata [512 x double]** %a.addr, metadata !20, metadata !DIExpression()), !dbg !38
  %0 = bitcast i32* %.omp.iv to i8*, !dbg !39
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3, !dbg !39
  call void @llvm.dbg.declare(metadata i32* %.omp.iv, metadata !21, metadata !DIExpression()), !dbg !40
  %1 = bitcast i32* %.omp.lb to i8*, !dbg !39
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3, !dbg !39
  call void @llvm.dbg.declare(metadata i32* %.omp.lb, metadata !24, metadata !DIExpression()), !dbg !40
  store i32 0, i32* %.omp.lb, align 4, !dbg !41, !tbaa !42
  %2 = bitcast i32* %.omp.ub to i8*, !dbg !39
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #3, !dbg !39
  call void @llvm.dbg.declare(metadata i32* %.omp.ub, metadata !25, metadata !DIExpression()), !dbg !40
  store volatile i32 511, i32* %.omp.ub, align 4, !dbg !41, !tbaa !42
  call void @llvm.dbg.value(metadata i32 1, metadata !26, metadata !DIExpression()), !dbg !40
  call void @llvm.dbg.value(metadata i32 0, metadata !27, metadata !DIExpression()), !dbg !40
  br label %DIR.OMP.PARALLEL.LOOP.1, !dbg !39

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %entry
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"([512 x double]** %a.addr) ], !dbg !39
  %4 = load i32, i32* %.omp.lb, align 4, !dbg !41, !tbaa !42
  store volatile i32 %4, i32* %.omp.iv, align 4, !dbg !41, !tbaa !42
  %5 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !42
  %6 = bitcast i32* %i to i8*
  %7 = bitcast i32* %j to i8*
  %8 = load [512 x double]*, [512 x double]** %a.addr, align 8
  br label %omp.inner.for.cond, !dbg !39

omp.inner.for.cond:                               ; preds = %for.cond.cleanup, %DIR.OMP.PARALLEL.LOOP.1
  %9 = load volatile i32, i32* %.omp.iv, align 4, !dbg !41, !tbaa !42
  %cmp = icmp sle i32 %9, %5, !dbg !44
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit, !dbg !39

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #3, !dbg !39
  call void @llvm.dbg.declare(metadata i32* %i, metadata !28, metadata !DIExpression()), !dbg !45
  %10 = load volatile i32, i32* %.omp.iv, align 4, !dbg !41, !tbaa !42
  store i32 %10, i32* %i, align 4, !dbg !46, !tbaa !42
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #3, !dbg !47
  call void @llvm.dbg.declare(metadata i32* %j, metadata !29, metadata !DIExpression()), !dbg !48
  store i32 1, i32* %j, align 4, !dbg !48, !tbaa !42
  %11 = load i32, i32* %i, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds [512 x double], [512 x double]* %8, i64 %idxprom
  br label %for.cond, !dbg !47

for.cond:                                         ; preds = %for.body, %omp.inner.for.body
  %12 = load i32, i32* %j, align 4, !dbg !49, !tbaa !42
  %cmp1 = icmp slt i32 %12, 511, !dbg !51
  br i1 %cmp1, label %for.body, label %for.cond.cleanup, !dbg !52

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %7) #3, !dbg !53
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %6) #3, !dbg !54
  %13 = load volatile i32, i32* %.omp.iv, align 4, !dbg !41, !tbaa !42
  %add19 = add nsw i32 %13, 1, !dbg !44
  store volatile i32 %add19, i32* %.omp.iv, align 4, !dbg !44, !tbaa !42
  br label %omp.inner.for.cond, !dbg !55, !llvm.loop !56

for.body:                                         ; preds = %for.cond
  %idxprom2 = sext i32 %12 to i64, !dbg !58
  %arrayidx3 = getelementptr inbounds [512 x double], [512 x double]* %arrayidx, i64 0, i64 %idxprom2, !dbg !58
  %14 = load double, double* %arrayidx3, align 8, !dbg !58, !tbaa !60
  %sub = sub nsw i32 %12, 1, !dbg !63
  %idxprom6 = sext i32 %sub to i64, !dbg !64
  %arrayidx7 = getelementptr inbounds [512 x double], [512 x double]* %arrayidx, i64 0, i64 %idxprom6, !dbg !64
  %15 = load double, double* %arrayidx7, align 8, !dbg !64, !tbaa !60
  %add8 = fadd double %14, %15, !dbg !65
  %add11 = add nsw i32 %12, 1, !dbg !66
  %idxprom12 = sext i32 %add11 to i64, !dbg !67
  %arrayidx13 = getelementptr inbounds [512 x double], [512 x double]* %arrayidx, i64 0, i64 %idxprom12, !dbg !67
  %16 = load double, double* %arrayidx13, align 8, !dbg !67, !tbaa !60
  %add14 = fadd double %add8, %16, !dbg !68
  store double %add14, double* %arrayidx3, align 8, !dbg !69, !tbaa !60
  %17 = load i32, i32* %j, align 4, !dbg !70, !tbaa !42
  %inc = add nsw i32 %17, 1, !dbg !70
  store i32 %inc, i32* %j, align 4, !dbg !70, !tbaa !42
  br label %for.cond, !dbg !53, !llvm.loop !71

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !39
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #3, !dbg !39
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %1) #3, !dbg !39
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #3, !dbg !39
  ret void, !dbg !74
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 53ef909f6145cef0d136b38ab35f7f0bd8cb7078) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm ae0486bf9c03535134bf6e19cf996cdbdcbe003b)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "kernel.c", directory: "/nfs/site/home/jlin4/test6/166")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 53ef909f6145cef0d136b38ab35f7f0bd8cb7078) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm ae0486bf9c03535134bf6e19cf996cdbdcbe003b)"}
!8 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 53ef909f6145cef0d136b38ab35f7f0bd8cb7078) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d26bb6e3270385a47365721fcd88679b193b71e6)"}
!9 = distinct !DISubprogram(name: "BNL_hotloop", scope: !1, file: !1, line: 13, type: !10, isLocal: false, isDefinition: true, scopeLine: 14, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !17)
!10 = !DISubroutineType(types: !11)
!11 = !{null, !12, !12, !13}
!12 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 32768, elements: !15)
!15 = !{!16}
!16 = !DISubrange(count: 512)
!17 = !{!18, !19, !20, !21, !24, !25, !26, !27, !28, !29}
!18 = !DILocalVariable(name: "x", arg: 1, scope: !9, file: !1, line: 13, type: !12)
!19 = !DILocalVariable(name: "y", arg: 2, scope: !9, file: !1, line: 13, type: !12)
!20 = !DILocalVariable(name: "a", arg: 3, scope: !9, file: !1, line: 13, type: !13)
!21 = !DILocalVariable(name: ".omp.iv", scope: !22, type: !23, flags: DIFlagArtificial)
!22 = distinct !DILexicalBlock(scope: !9, file: !1, line: 17, column: 11)
!23 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!24 = !DILocalVariable(name: ".omp.lb", scope: !22, type: !23, flags: DIFlagArtificial)
!25 = !DILocalVariable(name: ".omp.ub", scope: !22, type: !23, flags: DIFlagArtificial)
!26 = !DILocalVariable(name: ".omp.stride", scope: !22, type: !23, flags: DIFlagArtificial)
!27 = !DILocalVariable(name: ".omp.is_last", scope: !22, type: !23, flags: DIFlagArtificial)
!28 = !DILocalVariable(name: "i", scope: !22, file: !1, line: 21, type: !23)
!29 = !DILocalVariable(name: "j", scope: !30, file: !1, line: 23, type: !23)
!30 = distinct !DILexicalBlock(scope: !31, file: !1, line: 23, column: 5)
!31 = distinct !DILexicalBlock(scope: !22, file: !1, line: 21, column: 27)
!32 = !DILocation(line: 13, column: 25, scope: !9)
!33 = !DILocation(line: 13, column: 35, scope: !9)
!34 = !{!35, !35, i64 0}
!35 = !{!"pointer@_ZTSPA512_d", !36, i64 0}
!36 = !{!"omnipotent char", !37, i64 0}
!37 = !{!"Simple C/C++ TBAA"}
!38 = !DILocation(line: 13, column: 45, scope: !9)
!39 = !DILocation(line: 17, column: 11, scope: !9)
!40 = !DILocation(line: 0, scope: !22)
!41 = !DILocation(line: 21, column: 8, scope: !22)
!42 = !{!43, !43, i64 0}
!43 = !{!"int", !36, i64 0}
!44 = !DILocation(line: 21, column: 3, scope: !22)
!45 = !DILocation(line: 21, column: 12, scope: !22)
!46 = !DILocation(line: 21, column: 22, scope: !22)
!47 = !DILocation(line: 23, column: 10, scope: !30)
!48 = !DILocation(line: 23, column: 14, scope: !30)
!49 = !DILocation(line: 23, column: 19, scope: !50)
!50 = distinct !DILexicalBlock(scope: !30, file: !1, line: 23, column: 5)
!51 = !DILocation(line: 23, column: 20, scope: !50)
!52 = !DILocation(line: 23, column: 5, scope: !30)
!53 = !DILocation(line: 23, column: 5, scope: !50)
!54 = !DILocation(line: 26, column: 3, scope: !31)
!55 = !DILocation(line: 17, column: 11, scope: !22)
!56 = distinct !{!56, !55, !57}
!57 = !DILocation(line: 17, column: 27, scope: !22)
!58 = !DILocation(line: 24, column: 17, scope: !59)
!59 = distinct !DILexicalBlock(scope: !50, file: !1, line: 23, column: 31)
!60 = !{!61, !62, i64 0}
!61 = !{!"array@_ZTSA512_d", !62, i64 0}
!62 = !{!"double", !36, i64 0}
!63 = !DILocation(line: 24, column: 33, scope: !59)
!64 = !DILocation(line: 24, column: 27, scope: !59)
!65 = !DILocation(line: 24, column: 25, scope: !59)
!66 = !DILocation(line: 24, column: 45, scope: !59)
!67 = !DILocation(line: 24, column: 39, scope: !59)
!68 = !DILocation(line: 24, column: 37, scope: !59)
!69 = !DILocation(line: 24, column: 15, scope: !59)
!70 = !DILocation(line: 23, column: 27, scope: !50)
!71 = distinct !{!71, !52, !72, !73}
!72 = !DILocation(line: 25, column: 5, scope: !30)
!73 = !{!"llvm.loop.unroll.disable"}
!74 = !DILocation(line: 27, column: 1, scope: !9)
; CHECK: MayAlias:     [512 x double]* %arrayidx, double* %arrayidx3
; CHECK: MayAlias:     [512 x double]* %arrayidx, double* %arrayidx7
