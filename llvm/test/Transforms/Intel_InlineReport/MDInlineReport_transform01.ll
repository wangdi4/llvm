; RUN: opt -inline -inline-report=128 < %s -S 2>&1 | FileCheck %s
; RUN: opt -passes='cgscc(inline)' -inline-report=128 < %s -S 2>&1 | FileCheck %s

; This test checks that metadata corresponding to the inlining report was
; updated in accordance with function inlining that happened.

; 1) Function y() should not present in the IR.
; 2) Function a() remains in the IR.
; 3) Check that metadata was updated correctly:
;    a) a() was inlined into main() resulting in two new call sites: x() and z().
;    b) b() was not inlined into main().
;    c) x() was not inlined into a().
;    d) y() was inlined into a() resulting in a new call site: z().
;    e) y() was marked 'dead'.

; CHECK-NOT: define internal fastcc void @y()

; CHECK: define dso_local void @a()

; CHECK: !intel.module.inlining.report = !{[[MAIN_FIR:![0-9]+]], [[B_FIR:![0-9]+]], [[A_FIR:![0-9]+]], [[X_FIR:![0-9]+]], [[Y_FIR:![0-9]+]], [[Z_FIR:![0-9]+]]}
; CHECK: [[MAIN_FIR]] = distinct !{!"intel.function.inlining.report", !"name: main", [[MAIN_CSs:![0-9]+]], !"moduleName: test4.c", !"isDead: 0", !"isDeclaration: 0", !"linkage: A"}
; CHECK-NEXT: [[MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[A_MAIN_CS:![0-9]+]], [[B_MAIN_CS:![0-9]+]]}
; CHECK-NEXT: [[A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", !"name: a", [[A_MAIN_CSs:![0-9]+]], !"isInlined: 1"{{.*}}
; CHECK-NEXT: [[A_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_A_MAIN_CS:![0-9]+]], [[Y_A_MAIN_CS:![0-9]+]]}
; CHECK-NEXT: [[X_A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", !"name: x", [[X_A_MAIN_CSs:![0-9]+]], !"isInlined: 0"{{.*}}
; CHECK-NEXT: [[X_A_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report"}
; CHECK-NEXT: [[Y_A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", !"name: y", [[Y_A_MAIN_CSs:![0-9]+]], !"isInlined: 1"{{.*}}
; CHECK-NEXT: [[Y_A_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_Y_A_MAIN_CS:![0-9]+]]}
; CHECK-NEXT: [[Z_Y_A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", !"name: z", [[Z_Y_A_MAIN_CSs:![0-9]+]], !"isInlined: 0"{{.*}}
; CHECK-NEXT: [[Z_Y_A_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report"}
; CHECK-NEXT: [[B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", !"name: b", [[B_MAIN_CSs:![0-9]+]], !"isInlined: 0"{{.*}}
; CHECK-NEXT: [[B_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report"}
; CHECK-NEXT: [[B_FIR]] = distinct !{!"intel.function.inlining.report", !"name: b", [[B_CSs:![0-9]+]], !"moduleName: test4.c", !"isDead: 0", !"isDeclaration: 1", !"linkage: A"}
; CHECK-NEXT: [[B_CSs]] = distinct !{!"intel.callsites.inlining.report"}
; CHECK-NEXT: [[A_FIR]] = distinct !{!"intel.function.inlining.report", !"name: a", [[A_CSs:![0-9]+]], !"moduleName: test4.c", !"isDead: 0", !"isDeclaration: 0", !"linkage: A"}
; CHECK-NEXT: [[A_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_A_CS:![0-9]+]], [[Y_A_CS:![0-9]+]]}
; CHECK-NEXT: [[X_A_CS]] = distinct !{!"intel.callsite.inlining.report", !"name: x", [[X_A_CSs:![0-9]+]], !"isInlined: 0"{{.*}}
; CHECK-NEXT: [[X_A_CSs]] = distinct !{!"intel.callsites.inlining.report"}
; CHECK-NEXT: [[Y_A_CS]] = distinct !{!"intel.callsite.inlining.report", !"name: y", [[Y_A_CSs:![0-9]+]], !"isInlined: 1"{{.*}}
; CHECK-NEXT: [[Y_A_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_Y_A_CS:![0-9]+]]}
; CHECK-NEXT: [[Z_Y_A_CS]] = distinct !{!"intel.callsite.inlining.report", !"name: z", [[Z_Y_A_CSs:![0-9]+]], !"isInlined: 0"{{.*}}
; CHECK-NEXT: [[Z_Y_A_CSs]] = distinct !{!"intel.callsites.inlining.report"}
; CHECK-NEXT: [[X_FIR]] = distinct !{!"intel.function.inlining.report", !"name: x", [[X_CSs:![0-9]+]], !"moduleName: test4.c", !"isDead: 0", !"isDeclaration: 1", !"linkage: A"}
; CHECK-NEXT: [[X_CSs]] = distinct !{!"intel.callsites.inlining.report"}
; CHECK-NEXT: [[Y_FIR]] = distinct !{!"intel.function.inlining.report", !"name: y", [[Y_CSs:![0-9]+]], !"moduleName: test4.c", !"isDead: 1", !"isDeclaration: 0", !"linkage: L"}
; CHECK-NEXT: [[Y_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_Y_CS:![0-9]+]]}
; CHECK-NEXT: [[Z_Y_CS]] = distinct !{!"intel.callsite.inlining.report", !"name: z", [[Z_Y_CSs:![0-9]+]], !"isInlined: 0"{{.*}}
; CHECK-NEXT: [[Z_Y_CSs]] = distinct !{!"intel.callsites.inlining.report"}
; CHECK-NEXT: [[Z_FIR]] = distinct !{!"intel.function.inlining.report", !"name: z", [[Z_CSs:![0-9]+]], !"moduleName: test4.c", !"isDead: 0", !"isDeclaration: 1", !"linkage: A"}
; CHECK-NEXT: [[Z_CSs]] = distinct !{!"intel.callsites.inlining.report"}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr !dbg !30 !intel.function.inlining.report !8 {
entry:
  call void @a(), !dbg !32, !intel.callsite.inlining.report !10
  call void (...) @b(), !dbg !33, !intel.callsite.inlining.report !12
  ret i32 0, !dbg !34
}

declare !intel.function.inlining.report !14 dso_local void @b(...) local_unnamed_addr

; Function Attrs: nounwind uwtable
define dso_local void @a() local_unnamed_addr !dbg !35 !intel.function.inlining.report !16 {
entry:
  call void (...) @x(), !dbg !36, !intel.callsite.inlining.report !18
  call fastcc void @y(), !dbg !37, !intel.callsite.inlining.report !20
  ret void, !dbg !38
}

declare !intel.function.inlining.report !22 dso_local void @x(...) local_unnamed_addr

; Function Attrs: nounwind uwtable
define internal fastcc void @y() unnamed_addr !dbg !39 !intel.function.inlining.report !24 {
entry:
  call void (...) @z(), !dbg !40, !intel.callsite.inlining.report !26
  ret void, !dbg !41
}

declare !intel.function.inlining.report !28 dso_local void @z(...) local_unnamed_addr

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}
!intel.module.inlining.report = !{!8, !14, !16, !22, !24, !28}

!0 = distinct !DICompileUnit(language: DW_LANG_C89, file: !1, producer: "clang version 9.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 1a36a11c0229ca96a210a267540f8918e2fd7323) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm ad08f08eac9951eba9638f42bbb55db19a1c00a6)", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test4.c", directory: ".")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"clang version 9.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 1a36a11c0229ca96a210a267540f8918e2fd7323) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm ad08f08eac9951eba9638f42bbb55db19a1c00a6)"}
!8 = distinct !{!"intel.function.inlining.report", !"name: main", !9, !"moduleName: test4.c", !"isDead: 0", !"isDeclaration: 0", !"linkage: A"}
!9 = distinct !{!"intel.callsites.inlining.report", !10, !12}
!10 = distinct !{!"intel.callsite.inlining.report", !"name: a", !11, !"isInlined: 0", !"reason: 23", !"inlineCost: -1", !"outerInlineCost: -1", !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 11 col: 3", !"moduleName: test4.c"}
!11 = distinct !{!"intel.callsites.inlining.report"}
!12 = distinct !{!"intel.callsite.inlining.report", !"name: b", !13, !"isInlined: 0", !"reason: 29", !"inlineCost: -1", !"outerInlineCost: -1", !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 12 col: 3", !"moduleName: test4.c"}
!13 = distinct !{!"intel.callsites.inlining.report"}
!14 = distinct !{!"intel.function.inlining.report", !"name: b", !15, !"moduleName: test4.c", !"isDead: 0", !"isDeclaration: 1", !"linkage: A"}
!15 = distinct !{!"intel.callsites.inlining.report"}
!16 = distinct !{!"intel.function.inlining.report", !"name: a", !17, !"moduleName: test4.c", !"isDead: 0", !"isDeclaration: 0", !"linkage: A"}
!17 = distinct !{!"intel.callsites.inlining.report", !18, !20}
!18 = distinct !{!"intel.callsite.inlining.report", !"name: x", !19, !"isInlined: 0", !"reason: 29", !"inlineCost: -1", !"outerInlineCost: -1", !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 17 col: 3", !"moduleName: test4.c"}
!19 = distinct !{!"intel.callsites.inlining.report"}
!20 = distinct !{!"intel.callsite.inlining.report", !"name: y", !21, !"isInlined: 0", !"reason: 23", !"inlineCost: -1", !"outerInlineCost: -1", !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 18 col: 3", !"moduleName: test4.c"}
!21 = distinct !{!"intel.callsites.inlining.report"}
!22 = distinct !{!"intel.function.inlining.report", !"name: x", !23, !"moduleName: test4.c", !"isDead: 0", !"isDeclaration: 1", !"linkage: A"}
!23 = distinct !{!"intel.callsites.inlining.report"}
!24 = distinct !{!"intel.function.inlining.report", !"name: y", !25, !"moduleName: test4.c", !"isDead: 0", !"isDeclaration: 0", !"linkage: L"}
!25 = distinct !{!"intel.callsites.inlining.report", !26}
!26 = distinct !{!"intel.callsite.inlining.report", !"name: z", !27, !"isInlined: 0", !"reason: 29", !"inlineCost: -1", !"outerInlineCost: -1", !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 22 col: 3", !"moduleName: test4.c"}
!27 = distinct !{!"intel.callsites.inlining.report"}
!28 = distinct !{!"intel.function.inlining.report", !"name: z", !29, !"moduleName: test4.c", !"isDead: 0", !"isDeclaration: 1", !"linkage: A"}
!29 = distinct !{!"intel.callsites.inlining.report"}
!30 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 10, type: !31, scopeLine: 10, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!31 = !DISubroutineType(types: !2)
!32 = !DILocation(line: 11, column: 3, scope: !30)
!33 = !DILocation(line: 12, column: 3, scope: !30)
!34 = !DILocation(line: 13, column: 3, scope: !30)
!35 = distinct !DISubprogram(name: "a", scope: !1, file: !1, line: 16, type: !31, scopeLine: 16, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!36 = !DILocation(line: 17, column: 3, scope: !35)
!37 = !DILocation(line: 18, column: 3, scope: !35)
!38 = !DILocation(line: 19, column: 1, scope: !35)
!39 = distinct !DISubprogram(name: "y", scope: !1, file: !1, line: 21, type: !31, scopeLine: 21, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!40 = !DILocation(line: 22, column: 3, scope: !39)
!41 = !DILocation(line: 23, column: 1, scope: !39)
