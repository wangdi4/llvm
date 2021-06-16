; RUN: %oclopt -internalize-nonkernel-functions -globaldce -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -internalize-nonkernel-functions -globaldce -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; clean up functions not preserving !gdb Metadata

; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32-msvc-elf"

; Function Attrs: nounwind
define i32 @helper1() local_unnamed_addr #0 !dbg !12 {
entry:
  ret i32 40
}

; CHECK-NOT: declare !dbg{{.*}}helper1

define void @test() {
  ret void
}

!opencl.kernels = !{!4}

!4 = !{void ()* @test}
!12 = distinct !DISubprogram(name: "helper1", scope: !13, file: !13, line: 1, type: !14, isLocal: false, isDefinition: true, scopeLine: 2, isOptimized: true)
!13 = !DIFile(filename: "2", directory: "")
!14 = !DISubroutineType(types: !15)
!15 = !{!16}
!16 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY-NOT: WARNING
