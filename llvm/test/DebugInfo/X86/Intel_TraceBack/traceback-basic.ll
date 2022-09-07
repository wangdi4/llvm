; Check the appropriate routine tag is selected for different platforms
; RUN: llc -O0 -mtriple x86_64-linux-gnu %s -o %t1
; RUN: FileCheck < %t1 %s --check-prefixes=ALIGN-64
; ALIGN-64:      .p2align    3, 0x0          # Align to boundary 8
; ALIGN-64-NEXT: .byte      12               # TB_TAG_RTN64
; RUN: llc -O0 -mtriple i386-linux-gnu %s -o  %t2
; RUN: FileCheck < %t2 %s --check-prefixes=ALIGN-32
; ALIGN-32:      .p2align    2, 0x0          # Align to boundary 4
; ALIGN-32-NEXT: .byte       2               # TB_TAG_RTN32

; Check the entry's size of the function
; RUN: FileCheck < %t1 %s --check-prefixes=ENTRY-64
; ENTRY-64: .quad{{.*}}  # TB_AT_RoutineBegin
; RUN: FileCheck < %t2 %s --check-prefixes=ENTRY-32
; ENTRY-32: .long{{.*}}  # TB_AT_RoutineBegin

; Check the section header is of correct fields on different platforms.
; RUN: FileCheck < %t1 %s --check-prefixes=LINUX
; LINUX: .section    .trace,"a",@progbits
; RUN: llc -mtriple x86_64-windows-msvc %s -o %t3
; RUN: FileCheck < %t3 %s --check-prefixes=WIN
; WIN: .section    .trace,"dr"

; Check the records about filename omit the path to the file.
; RUN: FileCheck < %t1 %s --check-prefixes=FILENAME
; RUN: FileCheck < %t3 %s --check-prefixes=FILENAME
; FILENAME:      .short 17                      # TB_AT_NameLength
; FILENAME-NEXT: .ascii "traceback-basic.c"     # TB_AT_FileName

; Check the record for module name is emitted before the record for filename.
; RUN: FileCheck < %t1 %s --check-prefixes=MODULENAME
; MODULENAME:      .short  0                    # TB_AT_NameLength
; MODULENAME:      .short  {{[0-9]+}}           # TB_AT_NameLength
; MODULENAME-NEXT: .ascii  {{.*}}               # TB_AT_FileName

; Check the records about ranges have correct format.
; RUN: FileCheck < %t1 %s --check-prefixes=RANGE
; RUN: FileCheck < %t3 %s --check-prefixes=RANGE
; RANGE: .long .{{.*}}-.{{.*}}       # TB_AT_ModuleSize
; RANGE: .long .{{.*}}-{{.*}}        # TB_AT_CodeSize
; RANGE: .long (.{{.*}}-{{.*}})-1    # TB_AT_PC4

; Check the version of .trace format.
; RUN: FileCheck < %t1 %s --check-prefixes=VERSION
; VERSION:      .short 2   # TB_AT_MajorV
; VERSION-NEXT: .byte 0    # TB_AT_MinorV

; Check only one initial line record LN1, and widest PC record PC4 for function main.
; RUN: FileCheck < %t1 %s --check-prefixes=RECORD
; RECORD:      .byte {{[0-9]+}}        # TB_TAG_LN1
; RECORD-NEXT: .byte {{[0-9]+}}        # TB_AT_LN1
; RECORD-NEXT: .byte {{[0-9]+}}        # TB_TAG_PC4
; RECORD-NEXT: .long (.{{.*}}-main)-1  # TB_AT_PC4
; RECORD-NOT:  .byte {{[0-9]+}}        # TB_TAG_LN1

; Check the DWARF section is not emitted when there is a "TraceBack" flag but
; no "Dwarf Version" flag.
; RUN: FileCheck < %t1 %s --check-prefixes=DWARF
; DWARF-NOT: .section   .debug_{{.*}}

; To regenerate the test file traceback-basic.ll
; clang -traceback -S -emit-llvm traceback-basic.c

define dso_local i32 @main() !dbg !7 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  ret i32 0, !dbg !9
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/traceback-basic.c", directory: "/temp")
!2 = !{}
!3 = !{i32 2, !"TraceBack", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 11.0.0"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !2)
!9 = !DILocation(line: 1, column: 14, scope: !7)
