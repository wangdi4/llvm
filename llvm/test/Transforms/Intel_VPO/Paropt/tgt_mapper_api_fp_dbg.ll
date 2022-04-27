; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:

; int main() {
;   short y = 111;
; #pragma omp target firstprivate(y)
;   ;
; }

; Since the module has debug information, check that a map-name struct is
; created for the firstprivate operand, %y.ir.
; CHECK: @.mapname = private unnamed_addr constant [{{[0-9]+}} x i8] c";y.ir{{[^ ;]*}};unknown;0;0;;\00", align 1
; CHECK: @.offload_mapnames = private constant [1 x i8*] [i8* getelementptr inbounds ([{{[0-9]+}} x i8], [{{[0-9]+}} x i8]* @.mapname, i32 0, i32 0)]

; Check that tgt_mapper is called using the map-names struct.
; CHECK:  %{{[^ ]+}} = call i32 @__tgt_target_mapper(%struct.ident_t* @{{[^ ,]+}}, i64 %{{[^ ,]+}}, i8* @{{[^ ,]+}}, i32 1, i8** %{{[^ ,]}}, i8** %{{[^ ,]}}, i64* getelementptr inbounds ([1 x i64], [1 x i64]* @.offload_sizes, i32 0, i32 0), i64* getelementptr inbounds ([1 x i64], [1 x i64]* @.offload_maptypes, i32 0, i32 0), i8** getelementptr inbounds ([1 x i8*], [1 x i8*]* @.offload_mapnames, i32 0, i32 0), i8** null)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define hidden i32 @main() #0 !dbg !9 {
entry:
  %y.ir = alloca i16, align 2
  call void @llvm.dbg.declare(metadata i16* %y.ir, metadata !13, metadata !DIExpression()), !dbg !15
  store i16 111, i16* %y.ir, align 2, !dbg !15

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i16* %y.ir) ], !dbg !16
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !16

  ret i32 0, !dbg !18
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline norecurse nounwind optnone uwtable mustprogress "contains-openmp-target"="true" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!3}
!llvm.module.flags = !{!4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "tgt_fp.c", directory: "/path/to/file")
!2 = !{}
!3 = !{i32 0, i32 2055, i32 151396820, !"_Z4main", i32 3, i32 0, i32 0}
!4 = !{i32 7, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{i32 7, !"PIC Level", i32 2}
!8 = !{!"clang 10.0.0"}
!9 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !10, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!10 = !DISubroutineType(types: !11)
!11 = !{!12}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DILocalVariable(name: "y", scope: !9, file: !1, line: 2, type: !14)
!14 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!15 = !DILocation(line: 2, column: 9, scope: !9)
!16 = !DILocation(line: 3, column: 1, scope: !17)
!17 = distinct !DILexicalBlock(scope: !9, file: !1, line: 3, column: 1)
!18 = !DILocation(line: 5, column: 1, scope: !9)
