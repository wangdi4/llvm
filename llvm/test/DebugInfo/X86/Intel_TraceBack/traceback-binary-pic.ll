; RUN: llc -O0 -mtriple x86_64-linux-gnu %s --filetype=obj --relocation-model=pic -o %t1.o
; RUN: llvm-readobj -S %t1.o | FileCheck %s --check-prefixes=ELF

;     ELF-LABEL: Section
;     ELF:    Name: .trace
;     ELF:    Type: SHT_PROGBITS
;     ELF:    Flags
;     ELF:      SHF_ALLOC
;     ELF:      SHF_WRITE

define float @Square(float %x) local_unnamed_addr !dbg !9 {
entry:
  %mul = fmul fast float %x, %x, !dbg !11
  ret float %mul, !dbg !12
}


!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang", isOptimized: true, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-binary-pic.c", directory: "/temp")
!2 = !{}
!3 = !{i32 2, !"TraceBack", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 1}
!8 = !{!"clang"}
!9 = distinct !DISubprogram(name: "Square", scope: !1, file: !1, line: 1, type: !10, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!10 = !DISubroutineType(types: !2)
!11 = !DILocation(line: 5, column: 10, scope: !9)
!12 = !DILocation(line: 6, column: 4, scope: !9)
