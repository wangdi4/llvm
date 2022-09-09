; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that "init" routine is qualified as InitRoutine
; for DynClone transformation even though it has two user calls (
; @safecall and @unsafecall). Only unsafecall is considered as unsafe
; because 2nd field of @glob, where pointer of %struct.test.01 is
; saved in init routine, is accessed. Also, tests that debug intrinsics
; are ignored to qualify InitRoutine.

;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i32.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

%struct.netw = type { %struct.test.01*, %struct.test.01* }
@glob = internal global %struct.netw zeroinitializer, align 8

; CHECK: Verified InitRoutine

; This is selected as InitRoutine. Pointer of %struct.test.01 is saved
; in 2nd field of @glob.
define void @init() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  call void @llvm.dbg.value(metadata %struct.test.01* %tp1, metadata !13, metadata !DIExpression()), !dbg !14
  tail call void @safecall()
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  tail call void @unsafecall()
  store %struct.test.01* %tp1, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  ret void
}

; 1st field of @glob is accessed.
define void @safecall() {
  call void @llvm.dbg.value(metadata i32 0, metadata !13, metadata !DIExpression()), !dbg !14
  store %struct.test.01* null, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 0)
  ret void
}

; 2nd field of @glob is accessed.
define void @unsafecall() {
  store %struct.test.01* null, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  ret void
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 0, i64* %F6, align 8
  store i64 1, i64* %F6, align 8
  ret void
}

define i32 @main() {
  call void @init()
  call void @proc1()
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare void @llvm.dbg.value(metadata, metadata, metadata)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test", directory: ".")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!""}
!8 = distinct !DISubprogram(name: "na", linkageName: "na", scope: !1, file: !1, line: 1, type: !9, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, retainedNodes: !2)
; int(void*) type.
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!13 = !DILocalVariable(name: "na", arg: 1, scope: !8, file: !1, line: 1, type: !12)
!14 = !DILocation(line: 1, column: 1, scope: !8)
!15 = !DILocation(line: 1, column: 1, scope: !8)
!16 = !DILocation(line: 1, column: 1, scope: !8)
