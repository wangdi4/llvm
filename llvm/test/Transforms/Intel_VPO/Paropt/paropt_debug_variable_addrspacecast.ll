; RUN: opt -S -switch-to-offload -vpo-paropt %s | FileCheck %s
;
; Variable "V" has an llvm.dbg.declare, but the addrspace casted version of "V"
; is passed into the kernel. When the kernel is outlined, an llvm.dbg.declare
; should be created for the address space casted version of the variable.
;
; CHECK: define {{.*}} void @__omp_{{.*}}main{{.*}} !dbg [[OFFLOAD:![0-9]+]] {
; CHECK-LABEL: DIR.OMP.TARGET.ENTRY.split
; CHECK: call void @llvm.dbg.declare(metadata i32* %v.ascast.priv{{.*}}, metadata [[V_TARGET:![0-9]+]], metadata !DIExpression())
; CHECK: }
;
; [[OFFLOAD]] = distinct !DISubprogram(name: "main.DIR.OMP.TARGET.ENTRY.split"
; [[TARGET:![0-9]+]] = distinct !DILexicalBlock(scope: [[OFFLOAD]]
; [[V_TARGET]] = !DILocalVariable(name: "V", scope: [[TARGET]]
;
; ModuleID = 'test.ll'
source_filename = "test.ll"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline norecurse nounwind optnone
define hidden i32 @main(i32 %argc, i8 addrspace(4)* addrspace(4)* %argv) #2 !dbg !6 {
entry:
  %v = alloca i32, align 4
  call void @llvm.dbg.declare(metadata i32 *%v, metadata !15, metadata !DIExpression()), !dbg !17
  %v.ascast = addrspacecast i32* %v to i32 addrspace(4)*
  br label %DIR.OMP.TARGET.ENTRY

DIR.OMP.TARGET.ENTRY:                     ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %v.ascast) ], !dbg !13
  br label %KERNEL

KERNEL:                                   ; preds = %DIR.OMP.TEAMS.ENTRY
  store i32 0, i32 addrspace(4)* %v.ascast, align 4, !dbg !17
  br label %DIR.OMP.TARGET.EXIT

DIR.OMP.TARGET.EXIT:                      ; preds = %DIR.OMP.TEAMS.EXIT
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !13
  ret i32 0, !dbg !18
}

attributes #0 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #1 = { nounwind }
attributes #2 = { noinline norecurse nounwind optnone "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!3}
!llvm.module.flags = !{!4, !5}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, imports: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cpp", directory: "/path/to")
!2 = !{}
!3 = !{i32 0, i32 2055, i32 48505783, !"_Z4main", i32 57, i32 0, i32 0}
!4 = !{i32 7, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 52, type: !7, scopeLine: 53, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!7 = !DISubroutineType(types: !8)
!8 = !{!9, !9, !10}
!9 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!13 = !DILocation(line: 57, column: 1, scope: !14)
!14 = distinct !DILexicalBlock(scope: !6, file: !1, line: 57, column: 1)
!15 = !DILocalVariable(name: "V", scope: !16, file: !1, line: 61, type: !9)
!16 = distinct !DILexicalBlock(scope: !14, file: !1, line: 58, column: 3)
!17 = !DILocation(line: 61, column: 9, scope: !16)
!18 = !DILocation(line: 74, column: 3, scope: !6)
