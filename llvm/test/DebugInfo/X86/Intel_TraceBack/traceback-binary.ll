; Check the header format of section .trace
; RUN: llc -O0 -mtriple x86_64-linux-gnu %s --filetype=obj -o %t1.o
; RUN: llvm-readobj -S %t1.o | FileCheck %s --check-prefixes=HEADER,ELF,ELF64
; RUN: llc -O0 -mtriple i386-linux-gnu %s --filetype=obj -o %t2.o
; RUN: llvm-readobj -S %t2.o | FileCheck %s --check-prefixes=HEADER,ELF,ELF32
; RUN: llc -O0 -mtriple x86_64-windows-msvc %s --filetype=obj -o %t3.o
; RUN: llvm-readobj -S %t3.o | FileCheck %s --check-prefixes=HEADER,COFF,COFF64
; RUN: llc -O0 -mtriple i386-windows-msvc %s --filetype=obj -o %t4.o
; RUN: llvm-readobj -S %t4.o | FileCheck %s --check-prefixes=HEADER,COFF,COFF32

;  HEADER-LABEL:  Section
;  HEADER:    Name: .trace
;     ELF:    Type: SHT_PROGBITS
;     ELF:    Flags
;     ELF:      SHF_ALLOC
;   ELF64:    AddressAlignment: 8
;   ELF32:    AddressAlignment: 4
;    COFF:    Characteristics
;  COFF64:      IMAGE_SCN_ALIGN_8BYTES
;  COFF32:      IMAGE_SCN_ALIGN_4BYTES
;    COFF:      IMAGE_SCN_CNT_INITIALIZED_DATA
;    COFF:      IMAGE_SCN_MEM_READ

; Check the relocations of section .trace, there should be 4 relocations,
; 1 for .text begin and 3 function entries.
; RUN: llvm-readelf -r  %t1.o | FileCheck --check-prefixes=RELOCATION %s

; RELOCATION-LABEL: Relocation section '.rela.trace'
; RELOCATION:          Type             Symbol's Value
; RELOCATION-COUNT-4:  R_X86_64_64      0000000000000000

; To regenerate the test file traceback-binary.ll
; clang -traceback -S -emit-llvm traceback-binary.c

@i = dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @subr2() !dbg !7 {
entry:
  %0 = load i32, i32* @i, align 4, !dbg !9
  %inc = add nsw i32 %0, 1, !dbg !9
  store i32 %inc, i32* @i, align 4, !dbg !9
  ret void, !dbg !10
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @subr1() !dbg !11 {
entry:
  call void @subr2(), !dbg !12
  ret void, !dbg !13
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() !dbg !14 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  call void @subr1(), !dbg !15
  ret i32 0, !dbg !16
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-binary.c", directory: "/temp")
!2 = !{}
!3 = !{i32 2, !"TraceBack", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 11.0.0"}
!7 = distinct !DISubprogram(name: "subr2", scope: !1, file: !1, line: 3, type: !8, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !2)
!9 = !DILocation(line: 4, column: 3, scope: !7)
!10 = !DILocation(line: 5, column: 1, scope: !7)
!11 = distinct !DISubprogram(name: "subr1", scope: !1, file: !1, line: 7, type: !8, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!12 = !DILocation(line: 8, column: 3, scope: !11)
!13 = !DILocation(line: 9, column: 1, scope: !11)
!14 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 11, type: !8, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!15 = !DILocation(line: 12, column: 3, scope: !14)
!16 = !DILocation(line: 13, column: 3, scope: !14)
