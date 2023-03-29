; RUN: llc -filetype=obj --use-line-zero-locations=true %s -o - \
; RUN:   | llvm-dwarfdump --debug-info - \
; RUN:   | FileCheck %s --check-prefixes=CHECK,ENABLE
; RUN: llc -filetype=obj --use-line-zero-locations=false %s -o - \
; RUN:   | llvm-dwarfdump --debug-info - \
; RUN:   | FileCheck %s --check-prefixes=CHECK,DISABLE

; CHECK:       DW_TAG_subprogram
; CHECK:         DW_AT_name      ("test")
; CHECK-NOT:   NULL

; CHECK:       DW_TAG_lexical_block
; ENABLE:        DW_AT_ranges (0x{{[0-9a-f]+}}
; ENABLE:          [0x{{[0-9a-f]+}}, 0x{{[0-9a-f]+}})
; ENABLE:          [0x{{[0-9a-f]+}}, 0x{{[0-9a-f]+}})
; ENABLE:          [0x{{[0-9a-f]+}}, 0x{{[0-9a-f]+}})
; ENABLE:          [0x{{[0-9a-f]+}}, 0x{{[0-9a-f]+}}))

; DISABLE-NOT:   DW_AT_ranges
; DISABLE:       DW_AT_low_pc  (0x{{[0-9a-f]+}})
; DISABLE:       DW_AT_high_pc (0x{{[0-9a-f]+}})

;
; ModuleID = 'test.ll'

source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"


; Function Attrs: convergent noinline nounwind optnone
define i32 @test(i32 %0) #0 !dbg !5 {
entry:
  %1 = add nsw i32 %0, 1, !dbg !14
  %2 = add nsw i32 %1, 1, !dbg !15
  %3 = add nsw i32 %2, 1, !dbg !18
  %4 = add nsw i32 %3, 1, !dbg !15
  %5 = add nsw i32 %4, 1, !dbg !22
  %6 = add nsw i32 %5, 1, !dbg !15
  %7 = add nsw i32 %6, 1, !dbg !23
  ret i32 %7
}


attributes #0 = { convergent noinline nounwind optnone }
!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 7, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C11, file: !3, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4)
!3 = !DIFile(filename: "test.c", directory: "/path/to")
!4 = !{}
!5 = distinct !DISubprogram(name: "test", scope: null, file: !3, line: 34, type: !6, scopeLine: 34, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagMainSubprogram, unit: !2, templateParams: !4, retainedNodes: !8)
!6 = !DISubroutineType(types: !7)
!7 = !{null}
!8 = !{!9}
!9 = !DILocalVariable(name: "i", scope: !10, file: !3, line: 17, type: !13)
!10 = distinct !DILexicalBlock(scope: !11, file: !3, line: 34, column: 1)
!11 = distinct !DILexicalBlock(scope: !12, file: !3, line: 30, column: 3)
!12 = distinct !DILexicalBlock(scope: !5, file: !3, line: 29, column: 1)
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !DILocation(line: 34, column: 1, scope: !10)
!15 = !DILocation(line: 0, scope: !16)
!16 = !DILexicalBlockFile(scope: !5, file: !17, discriminator: 0)
!17 = !DIFile(filename: "CPU_DEVICE_RT", directory: "/")
!18 = !DILocation(line: 36, column: 21, scope: !19)
!19 = distinct !DILexicalBlock(scope: !20, file: !3, line: 36, column: 7)
!20 = distinct !DILexicalBlock(scope: !21, file: !3, line: 36, column: 7)
!21 = distinct !DILexicalBlock(scope: !10, file: !3, line: 35, column: 5)
!22 = !DILocation(line: 37, column: 23, scope: !19)
!23 = !DILocation(line: 36, column: 27, scope: !19)
