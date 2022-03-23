; RUN: opt -S -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt %s | FileCheck %s
; CHECK: %non_pod_of_A.priv ={{.*}} !dbg [[DBG:![0-9]+]]
; CHECK: call {{.*}}omp.def_constr{{.*}} !dbg [[DBG]]
; CHECK: call {{.*}}omp.destr{{.*}} !dbg [[DBG]]

; The constructor and destructor calls that are created by privatization of the
; structure, did not have debug position info.

;struct NP
;{
;  NP();
;  NP(const NP &);
;  NP &operator=(const NP &);
;  ~NP();
;};
;
;struct A
;{
;  NP non_pod_of_A;
;  int func_of_A();
;};
;
;int A::func_of_A() {
;#pragma omp parallel private(non_pod_of_A)
;  {}
;  return 0;
;}

 
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.A = type { %struct.NP }
%struct.NP = type { i8 }

; Function Attrs: nounwind uwtable
define dso_local i32 @_ZN1A9func_of_AEv(%struct.A* %this) #0 align 2 !dbg !7 {
entry:
  %this.addr = alloca %struct.A*, align 8
  store %struct.A* %this, %struct.A** %this.addr, align 8, !tbaa !35
  call void @llvm.dbg.declare(metadata %struct.A** %this.addr, metadata !33, metadata !DIExpression()), !dbg !39
  %this1 = load %struct.A*, %struct.A** %this.addr, align 8
  %non_pod_of_A = getelementptr inbounds %struct.A, %struct.A* %this1, i32 0, i32 0, !dbg !40, !intel-tbaa !42
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE:NONPOD"(%struct.NP* %non_pod_of_A, %struct.NP* (%struct.NP*)* @_ZTS2NP.omp.def_constr, void (%struct.NP*)* @_ZTS2NP.omp.destr) ], !dbg !45
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ], !dbg !45
  ret i32 0, !dbg !46
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: uwtable
define internal %struct.NP* @_ZTS2NP.omp.def_constr(%struct.NP* %0) #3 section ".text.startup" !dbg !47 {
entry:
  %.addr = alloca %struct.NP*, align 8
  store %struct.NP* %0, %struct.NP** %.addr, align 8, !tbaa !52
  call void @llvm.dbg.declare(metadata %struct.NP** %.addr, metadata !50, metadata !DIExpression()), !dbg !54
  %1 = load %struct.NP*, %struct.NP** %.addr, align 8
  call void @_ZN2NPC1Ev(%struct.NP* %1), !dbg !55
  ret %struct.NP* %1, !dbg !55
}

declare dso_local void @_ZN2NPC1Ev(%struct.NP*) unnamed_addr #4

; Function Attrs: uwtable
define internal void @_ZTS2NP.omp.destr(%struct.NP* %0) #3 section ".text.startup" !dbg !56 {
entry:
  %.addr = alloca %struct.NP*, align 8
  store %struct.NP* %0, %struct.NP** %.addr, align 8, !tbaa !52
  call void @llvm.dbg.declare(metadata %struct.NP** %.addr, metadata !58, metadata !DIExpression()), !dbg !59
  %1 = load %struct.NP*, %struct.NP** %.addr, align 8
  call void @_ZN2NPD1Ev(%struct.NP* %1) #2
  ret void
}

; Function Attrs: nounwind
declare dso_local void @_ZN2NPD1Ev(%struct.NP*) unnamed_addr #5

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }
attributes #3 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 10.0.0", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test.cpp", directory: "/home/users/clin1/test/18968")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 10.0.0"}
!7 = distinct !DISubprogram(name: "func_of_A", linkageName: "_ZN1A9func_of_AEv", scope: !8, file: !1, line: 15, type: !28, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, declaration: !27, retainedNodes: !32)
!8 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "A", file: !1, line: 9, size: 8, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !9, identifier: "_ZTS1A")
!9 = !{!10, !27}
!10 = !DIDerivedType(tag: DW_TAG_member, name: "non_pod_of_A", scope: !8, file: !1, line: 11, baseType: !11, size: 8)
!11 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "NP", file: !1, line: 1, size: 8, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !12, identifier: "_ZTS2NP")
!12 = !{!13, !17, !22, !26}
!13 = !DISubprogram(name: "NP", scope: !11, file: !1, line: 3, type: !14, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!14 = !DISubroutineType(types: !15)
!15 = !{null, !16}
!16 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!17 = !DISubprogram(name: "NP", scope: !11, file: !1, line: 4, type: !18, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!18 = !DISubroutineType(types: !19)
!19 = !{null, !16, !20}
!20 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !21, size: 64)
!21 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !11)
!22 = !DISubprogram(name: "operator=", linkageName: "_ZN2NPaSERKS_", scope: !11, file: !1, line: 5, type: !23, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!23 = !DISubroutineType(types: !24)
!24 = !{!25, !16, !20}
!25 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !11, size: 64)
!26 = !DISubprogram(name: "~NP", scope: !11, file: !1, line: 6, type: !14, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!27 = !DISubprogram(name: "func_of_A", linkageName: "_ZN1A9func_of_AEv", scope: !8, file: !1, line: 12, type: !28, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!28 = !DISubroutineType(types: !29)
!29 = !{!30, !31}
!30 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!31 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !8, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!32 = !{!33}
!33 = !DILocalVariable(name: "this", arg: 1, scope: !7, type: !34, flags: DIFlagArtificial | DIFlagObjectPointer)
!34 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !8, size: 64)
!35 = !{!36, !36, i64 0}
!36 = !{!"pointer@_ZTSP1A", !37, i64 0}
!37 = !{!"omnipotent char", !38, i64 0}
!38 = !{!"Simple C++ TBAA"}
!39 = !DILocation(line: 0, scope: !7)
!40 = !DILocation(line: 16, column: 30, scope: !41)
!41 = distinct !DILexicalBlock(scope: !7, file: !1, line: 16, column: 1)
!42 = !{!43, !44, i64 0}
!43 = !{!"struct@_ZTS1A", !44, i64 0}
!44 = !{!"struct@_ZTS2NP"}
!45 = !DILocation(line: 16, column: 1, scope: !7)
!46 = !DILocation(line: 18, column: 3, scope: !7)
!47 = distinct !DISubprogram(linkageName: "_ZTS2NP.omp.def_constr", scope: !1, file: !1, type: !48, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !49)
!48 = !DISubroutineType(types: !2)
!49 = !{!50}
!50 = !DILocalVariable(arg: 1, scope: !47, type: !51, flags: DIFlagArtificial)
!51 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!52 = !{!53, !53, i64 0}
!53 = !{!"pointer@_ZTSP2NP", !37, i64 0}
!54 = !DILocation(line: 0, scope: !47)
!55 = !DILocation(line: 16, column: 30, scope: !47)
!56 = distinct !DISubprogram(linkageName: "_ZTS2NP.omp.destr", scope: !1, file: !1, type: !48, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !57)
!57 = !{!58}
!58 = !DILocalVariable(arg: 1, scope: !56, type: !51, flags: DIFlagArtificial)
!59 = !DILocation(line: 0, scope: !56)
