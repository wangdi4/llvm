; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; // C++ test:
; struct NP
; {
;   int data;
;   NP() { data = 0; };
;   NP(const NP &);
;   NP &operator=(const NP &);
;   ~NP();
; };
; void foo() {
;   NP aaa;
;   #pragma omp parallel for firstprivate(aaa) lastprivate(aaa)
;   for(int i=0; i<1000; i++) { }
; }

; firstprivate(aaa) and lastprivate(aaa) result in calls to copy-constructor, copy-assign, and destructor.
; Check that the debug info associated with these calls are correct.
;
; CHECK: %aaa.lpriv = alloca %struct.NP, align 4, !dbg [[DBG:![0-9]+]]
; CHECK: call void {{.*}}omp.copy_constr{{.*}} !dbg [[DBG]]
; CHECK: call void {{.*}}omp.copy_assign{{.*}} !dbg [[DBG]]
; CHECK: call void {{.*}}omp.destr{{.*}} !dbg [[DBG]]

; ModuleID = 'lit2.cpp'
source_filename = "lit2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.NP = type { i32 }

$_ZN2NPC2Ev = comdat any

; Function Attrs: noinline optnone uwtable
define dso_local void @_Z3foov() #0 !dbg !7 {
entry:
  %aaa = alloca %struct.NP, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  call void @llvm.dbg.declare(metadata %struct.NP* %aaa, metadata !10, metadata !DIExpression()), !dbg !29
  call void @_ZN2NPC2Ev(%struct.NP* %aaa), !dbg !29
  call void @llvm.dbg.declare(metadata i32* %.omp.iv, metadata !30, metadata !DIExpression()), !dbg !32
  call void @llvm.dbg.declare(metadata i32* %.omp.lb, metadata !33, metadata !DIExpression()), !dbg !32
  store i32 0, i32* %.omp.lb, align 4, !dbg !34
  call void @llvm.dbg.declare(metadata i32* %.omp.ub, metadata !35, metadata !DIExpression()), !dbg !32
  store i32 999, i32* %.omp.ub, align 4, !dbg !34
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%struct.NP* %aaa, void (%struct.NP*, %struct.NP*)* @_ZTS2NP.omp.copy_constr, void (%struct.NP*)* @_ZTS2NP.omp.destr), "QUAL.OMP.LASTPRIVATE:NONPOD"(%struct.NP* %aaa, i8* null, void (%struct.NP*, %struct.NP*)* @_ZTS2NP.omp.copy_assign, void (%struct.NP*)* @_ZTS2NP.omp.destr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ], !dbg !36
  %1 = load i32, i32* %.omp.lb, align 4, !dbg !34
  store i32 %1, i32* %.omp.iv, align 4, !dbg !34
  br label %omp.inner.for.cond, !dbg !36

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4, !dbg !34
  %3 = load i32, i32* %.omp.ub, align 4, !dbg !34
  %cmp = icmp sle i32 %2, %3, !dbg !37
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !36

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.dbg.declare(metadata i32* %i, metadata !38, metadata !DIExpression()), !dbg !39
  %4 = load i32, i32* %.omp.iv, align 4, !dbg !34
  %mul = mul nsw i32 %4, 1, !dbg !40
  %add = add nsw i32 0, %mul, !dbg !40
  store i32 %add, i32* %i, align 4, !dbg !40
  br label %omp.body.continue, !dbg !41

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !43

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, i32* %.omp.iv, align 4, !dbg !34
  %add1 = add nsw i32 %5, 1, !dbg !37
  store i32 %add1, i32* %.omp.iv, align 4, !dbg !37
  br label %omp.inner.for.cond, !dbg !43, !llvm.loop !44

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !43

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !36
  call void @_ZN2NPD1Ev(%struct.NP* %aaa) #3, !dbg !46
  ret void, !dbg !46
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN2NPC2Ev(%struct.NP* %this) unnamed_addr #2 comdat align 2 !dbg !47 {
entry:
  %this.addr = alloca %struct.NP*, align 8
  store %struct.NP* %this, %struct.NP** %this.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.NP** %this.addr, metadata !48, metadata !DIExpression()), !dbg !50
  %this1 = load %struct.NP*, %struct.NP** %this.addr, align 8
  %data = getelementptr inbounds %struct.NP, %struct.NP* %this1, i32 0, i32 0, !dbg !51
  store i32 0, i32* %data, align 4, !dbg !53
  ret void, !dbg !54
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: noinline uwtable
define internal void @_ZTS2NP.omp.copy_constr(%struct.NP* %0, %struct.NP* %1) #4 !dbg !55 {
entry:
  %.addr = alloca %struct.NP*, align 8
  %.addr1 = alloca %struct.NP*, align 8
  store %struct.NP* %0, %struct.NP** %.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.NP** %.addr, metadata !58, metadata !DIExpression()), !dbg !59
  store %struct.NP* %1, %struct.NP** %.addr1, align 8
  call void @llvm.dbg.declare(metadata %struct.NP** %.addr1, metadata !60, metadata !DIExpression()), !dbg !59
  %2 = load %struct.NP*, %struct.NP** %.addr, align 8, !dbg !61
  %3 = load %struct.NP*, %struct.NP** %.addr1, align 8, !dbg !59
  call void @_ZN2NPC1ERKS_(%struct.NP* %2, %struct.NP* nonnull align 4 dereferenceable(4) %3), !dbg !61
  ret void, !dbg !61
}

declare dso_local void @_ZN2NPC1ERKS_(%struct.NP*, %struct.NP* nonnull align 4 dereferenceable(4)) unnamed_addr #5

; Function Attrs: noinline uwtable
define internal void @_ZTS2NP.omp.destr(%struct.NP* %0) #4 section ".text.startup" !dbg !62 {
entry:
  %.addr = alloca %struct.NP*, align 8
  store %struct.NP* %0, %struct.NP** %.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.NP** %.addr, metadata !64, metadata !DIExpression()), !dbg !65
  %1 = load %struct.NP*, %struct.NP** %.addr, align 8
  call void @_ZN2NPD1Ev(%struct.NP* %1) #3
  ret void
}

; Function Attrs: nounwind
declare dso_local void @_ZN2NPD1Ev(%struct.NP*) unnamed_addr #6

; Function Attrs: noinline uwtable
define internal void @_ZTS2NP.omp.copy_assign(%struct.NP* %0, %struct.NP* %1) #4 !dbg !66 {
entry:
  %.addr = alloca %struct.NP*, align 8
  %.addr1 = alloca %struct.NP*, align 8
  store %struct.NP* %0, %struct.NP** %.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.NP** %.addr, metadata !67, metadata !DIExpression()), !dbg !68
  store %struct.NP* %1, %struct.NP** %.addr1, align 8
  call void @llvm.dbg.declare(metadata %struct.NP** %.addr1, metadata !69, metadata !DIExpression()), !dbg !68
  %2 = load %struct.NP*, %struct.NP** %.addr, align 8, !dbg !70
  %3 = load %struct.NP*, %struct.NP** %.addr1, align 8, !dbg !70
  %call = call nonnull align 4 dereferenceable(4) %struct.NP* @_ZN2NPaSERKS_(%struct.NP* %2, %struct.NP* nonnull align 4 dereferenceable(4) %3), !dbg !70
  ret void, !dbg !70
}

declare dso_local nonnull align 4 dereferenceable(4) %struct.NP* @_ZN2NPaSERKS_(%struct.NP*, %struct.NP* nonnull align 4 dereferenceable(4)) #5

attributes #0 = { noinline optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 9.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "lit2.cpp", directory: "llvm/llvm/test/Transforms/Intel_VPO/Paropt")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 9.0.0"}
!7 = distinct !DISubprogram(name: "foo", linkageName: "_Z3foov", scope: !1, file: !1, line: 9, type: !8, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{null}
!10 = !DILocalVariable(name: "aaa", scope: !7, file: !1, line: 10, type: !11)
!11 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "NP", file: !1, line: 1, size: 32, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !12, identifier: "_ZTS2NP")
!12 = !{!13, !15, !19, !24, !28}
!13 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !11, file: !1, line: 3, baseType: !14, size: 32)
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !DISubprogram(name: "NP", scope: !11, file: !1, line: 4, type: !16, scopeLine: 4, flags: DIFlagPrototyped, spFlags: 0)
!16 = !DISubroutineType(types: !17)
!17 = !{null, !18}
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!19 = !DISubprogram(name: "NP", scope: !11, file: !1, line: 5, type: !20, scopeLine: 5, flags: DIFlagPrototyped, spFlags: 0)
!20 = !DISubroutineType(types: !21)
!21 = !{null, !18, !22}
!22 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !23, size: 64)
!23 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !11)
!24 = !DISubprogram(name: "operator=", linkageName: "_ZN2NPaSERKS_", scope: !11, file: !1, line: 6, type: !25, scopeLine: 6, flags: DIFlagPrototyped, spFlags: 0)
!25 = !DISubroutineType(types: !26)
!26 = !{!27, !18, !22}
!27 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !11, size: 64)
!28 = !DISubprogram(name: "~NP", scope: !11, file: !1, line: 7, type: !16, scopeLine: 7, flags: DIFlagPrototyped, spFlags: 0)
!29 = !DILocation(line: 10, column: 6, scope: !7)
!30 = !DILocalVariable(name: ".omp.iv", scope: !31, type: !14, flags: DIFlagArtificial)
!31 = distinct !DILexicalBlock(scope: !7, file: !1, line: 11, column: 3)
!32 = !DILocation(line: 0, scope: !31)
!33 = !DILocalVariable(name: ".omp.lb", scope: !31, type: !14, flags: DIFlagArtificial)
!34 = !DILocation(line: 12, column: 7, scope: !31)
!35 = !DILocalVariable(name: ".omp.ub", scope: !31, type: !14, flags: DIFlagArtificial)
!36 = !DILocation(line: 11, column: 3, scope: !7)
!37 = !DILocation(line: 12, column: 3, scope: !31)
!38 = !DILocalVariable(name: "i", scope: !31, file: !1, line: 12, type: !14)
!39 = !DILocation(line: 12, column: 11, scope: !31)
!40 = !DILocation(line: 12, column: 24, scope: !31)
!41 = !DILocation(line: 12, column: 31, scope: !42)
!42 = distinct !DILexicalBlock(scope: !31, file: !1, line: 12, column: 29)
!43 = !DILocation(line: 11, column: 3, scope: !31)
!44 = distinct !{!44, !43, !45}
!45 = !DILocation(line: 11, column: 62, scope: !31)
!46 = !DILocation(line: 13, column: 1, scope: !7)
!47 = distinct !DISubprogram(name: "NP", linkageName: "_ZN2NPC2Ev", scope: !11, file: !1, line: 4, type: !16, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !15, retainedNodes: !2)
!48 = !DILocalVariable(name: "this", arg: 1, scope: !47, type: !49, flags: DIFlagArtificial | DIFlagObjectPointer)
!49 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!50 = !DILocation(line: 0, scope: !47)
!51 = !DILocation(line: 4, column: 10, scope: !52)
!52 = distinct !DILexicalBlock(scope: !47, file: !1, line: 4, column: 8)
!53 = !DILocation(line: 4, column: 15, scope: !52)
!54 = !DILocation(line: 4, column: 20, scope: !47)
!55 = distinct !DISubprogram(name: "_ZTS2NP.omp.copy_constr", linkageName: "_ZL23_ZTS2NP.omp.copy_constrv", scope: !1, file: !1, line: 11, type: !56, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !0, retainedNodes: !2)
!56 = !DISubroutineType(types: !57)
!57 = !{null, !49, !49}
!58 = !DILocalVariable(arg: 1, scope: !55, type: !49, flags: DIFlagArtificial)
!59 = !DILocation(line: 0, scope: !55)
!60 = !DILocalVariable(arg: 2, scope: !55, type: !49, flags: DIFlagArtificial)
!61 = !DILocation(line: 11, column: 41, scope: !55)
!62 = distinct !DISubprogram(linkageName: "_ZTS2NP.omp.destr", scope: !1, file: !1, type: !63, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !0, retainedNodes: !2)
!63 = !DISubroutineType(types: !2)
!64 = !DILocalVariable(arg: 1, scope: !62, type: !49, flags: DIFlagArtificial)
!65 = !DILocation(line: 0, scope: !62)
!66 = distinct !DISubprogram(name: "_ZTS2NP.omp.copy_assign", linkageName: "_ZL23_ZTS2NP.omp.copy_assignv", scope: !1, file: !1, line: 11, type: !56, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !0, retainedNodes: !2)
!67 = !DILocalVariable(arg: 1, scope: !66, type: !49, flags: DIFlagArtificial)
!68 = !DILocation(line: 0, scope: !66)
!69 = !DILocalVariable(arg: 2, scope: !66, type: !49, flags: DIFlagArtificial)
!70 = !DILocation(line: 11, column: 58, scope: !66)
