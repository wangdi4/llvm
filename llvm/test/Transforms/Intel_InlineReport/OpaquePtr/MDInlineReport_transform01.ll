; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=128 < %s -S 2>&1 | FileCheck %s

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
; CHECK: [[MAIN_FIR]] = distinct !{!"intel.function.inlining.report", [[MAIN_NAME:![0-9]+]], [[MAIN_CSs:![0-9]+]], [[MODULE_NAME:![0-9]+]], [[IS_DEAD_0:![0-9]+]], [[IS_DECL_0:![0-9]+]], [[LINK_A:![0-9]+]], [[LANG_C:![0-9]+]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK-NEXT: [[MAIN_NAME]] = !{!"name: main"}
; CHECK-NEXT: [[MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[A_MAIN_CS:![0-9]+]], [[B_MAIN_CS:![0-9]+]]}
; CHECK-NEXT: [[A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[A_NAME:![0-9]+]], [[A_MAIN_CSs:![0-9]+]], [[IS_INL_1:![0-9]+]]{{.*}}
; CHECK-NEXT: [[A_NAME]] = !{!"name: a"}
; CHECK-NEXT: [[A_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_A_MAIN_CS:![0-9]+]], [[Y_A_MAIN_CS:![0-9]+]]}
; CHECK-NEXT: [[X_A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME:![0-9]+]], null, [[IS_INL_0:![0-9]+]]{{.*}}
; CHECK-NEXT: [[X_NAME]] = !{!"name: x"}
; CHECK-NEXT: [[IS_INL_0]] = !{!"isInlined: 0"}
; CHECK: [[MODULE_NAME]] = !{!"moduleName: test1.c"}
; CHECK-NEXT: [[SUPPRESS_PRINT]] = !{!"isSuppressPrint: 0"}
; CHECK-NEXT: [[Y_A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME:![0-9]+]], [[Y_A_MAIN_CSs:![0-9]+]], [[IS_INL_1]]{{.*}}
; CHECK-NEXT: [[Y_NAME]]  = !{!"name: y"}
; CHECK-NEXT: [[Y_A_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_Y_A_MAIN_CS:![0-9]+]]}
; CHECK-NEXT: [[Z_Y_A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK-NEXT: [[Z_NAME]] = !{!"name: z"}
; CHECK-NEXT: [[IS_INL_1]] = !{!"isInlined: 1"}
; CHECK: [[B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[B_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK-NEXT: [[B_NAME]] = !{!"name: b"}
; CHECK: [[IS_DEAD_0]] = !{!"isDead: 0"}
; CHECK-NEXT: [[IS_DECL_0]] = !{!"isDeclaration: 0"}
; CHECK-NEXT: [[LINK_A]] = !{!"linkage: A"}
; CHECK-NEXT: [[LANG_C]] = !{!"language: C"}
; CHECK-NEXT: [[B_FIR]] = distinct !{!"intel.function.inlining.report", [[B_NAME]], null, [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_1:![0-9]+]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK-NEXT: [[IS_DECL_1]] = !{!"isDeclaration: 1"}
; CHECK-NEXT: [[A_FIR]] = distinct !{!"intel.function.inlining.report", [[A_NAME]], [[A_CSs:![0-9]+]], [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_0]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK-NEXT: [[A_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_A_CS:![0-9]+]], [[Y_A_CS:![0-9]+]]}
; CHECK-NEXT: [[X_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME]], null, [[IS_INL_0]]{{.*}}
; CHECK-NEXT: [[Y_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME]], [[Y_A_CSs:![0-9]+]], [[IS_INL_1]]{{.*}}
; CHECK-NEXT: [[Y_A_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_Y_A_CS:![0-9]+]]}
; CHECK-NEXT: [[Z_Y_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME]], null, [[IS_INL_0]]{{.*}}
; CHECK-NEXT: [[X_FIR]] = distinct !{!"intel.function.inlining.report", [[X_NAME]], null, [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_1]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK-NEXT: [[Y_FIR]] = distinct !{!"intel.function.inlining.report", [[Y_NAME]], [[Y_CSs:![0-9]+]], [[MODULE_NAME]], [[IS_DEAD_1:![0-9]+]], [[IS_DECL_0]], [[LINK_L:![0-9]+]], [[LANG_C]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK-NEXT: [[Y_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_Y_CS:![0-9]+]]}
; CHECK-NEXT: [[Z_Y_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[IS_DEAD_1]] = !{!"isDead: 1"}
; CHECK-NEXT: [[LINK_L]] = !{!"linkage: L"}
; CHECK-NEXT: [[Z_FIR]] = distinct !{!"intel.function.inlining.report", [[Z_NAME]], null, [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_1]], [[LINK_A]], [[LANG_C]]}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() !dbg !44 !intel.function.inlining.report !8 {
entry:
  call void @a(), !dbg !48, !intel.callsite.inlining.report !11
  call void (...) @b(), !dbg !49, !intel.callsite.inlining.report !22
  ret i32 0, !dbg !50
}

declare !intel.function.inlining.report !29 dso_local void @b(...)

define dso_local void @a() !dbg !51 !intel.function.inlining.report !31 {
entry:
  call void (...) @x(), !dbg !54, !intel.callsite.inlining.report !33
  call void @y(), !dbg !55, !intel.callsite.inlining.report !35
  ret void, !dbg !56
}

declare !intel.function.inlining.report !37 dso_local void @x(...)

define internal void @y() !dbg !57 !intel.function.inlining.report !38 {
entry:
  call void (...) @z(), !dbg !58, !intel.callsite.inlining.report !40
  ret void, !dbg !59
}

declare !intel.function.inlining.report !43 dso_local void @z(...)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}
!intel.module.inlining.report = !{!8, !29, !31, !37, !38, !43}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based icx (ICX) 2019.8.2.0", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test1.c", directory: "/export/iusers/ochupina/inl_report")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"icx (ICX) 2019.8.2.0"}
!8 = distinct !{!"intel.function.inlining.report", !9, !10, !20, !25, !26, !27, !28, !21}
!9 = !{!"name: main"}
!10 = distinct !{!"intel.callsites.inlining.report", !11, !22}
!11 = distinct !{!"intel.callsite.inlining.report", !12, null, !13, !14, !15, !16, !17, !18, !19, !"line: 10 col: 3", !20, !21}
!12 = !{!"name: a"}
!13 = !{!"isInlined: 0"}
!14 = !{!"reason: 24"}
!15 = !{!"inlineCost: -1"}
!16 = !{!"outerInlineCost: -1"}
!17 = !{!"inlineThreshold: -1"}
!18 = !{!"earlyExitCost: 2147483647"}
!19 = !{!"earlyExitThreshold: 2147483647"}
!20 = !{!"moduleName: test1.c"}
!21 = !{!"isSuppressPrint: 0"}
!22 = distinct !{!"intel.callsite.inlining.report", !23, null, !13, !24, !15, !16, !17, !18, !19, !"line: 11 col: 3", !20, !21}
!23 = !{!"name: b"}
!24 = !{!"reason: 39"}
!25 = !{!"isDead: 0"}
!26 = !{!"isDeclaration: 0"}
!27 = !{!"linkage: A"}
!28 = !{!"language: C"}
!29 = distinct !{!"intel.function.inlining.report", !23, null, !20, !25, !30, !27, !28, !21}
!30 = !{!"isDeclaration: 1"}
!31 = distinct !{!"intel.function.inlining.report", !12, !32, !20, !25, !26, !27, !28, !21}
!32 = distinct !{!"intel.callsites.inlining.report", !33, !35}
!33 = distinct !{!"intel.callsite.inlining.report", !34, null, !13, !24, !15, !16, !17, !18, !19, !"line: 16 col: 3", !20, !21}
!34 = !{!"name: x"}
!35 = distinct !{!"intel.callsite.inlining.report", !36, null, !13, !14, !15, !16, !17, !18, !19, !"line: 17 col: 3", !20, !21}
!36 = !{!"name: y"}
!37 = distinct !{!"intel.function.inlining.report", !34, null, !20, !25, !30, !27, !28, !21}
!38 = distinct !{!"intel.function.inlining.report", !36, !39, !20, !25, !26, !42, !28, !21}
!39 = distinct !{!"intel.callsites.inlining.report", !40}
!40 = distinct !{!"intel.callsite.inlining.report", !41, null, !13, !24, !15, !16, !17, !18, !19, !"line: 21 col: 3", !20, !21}
!41 = !{!"name: z"}
!42 = !{!"linkage: L"}
!43 = distinct !{!"intel.function.inlining.report", !41, null, !20, !25, !30, !27, !28}
!44 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 9, type: !45, scopeLine: 9, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!45 = !DISubroutineType(types: !46)
!46 = !{!47}
!47 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!48 = !DILocation(line: 10, column: 3, scope: !44)
!49 = !DILocation(line: 11, column: 3, scope: !44)
!50 = !DILocation(line: 12, column: 3, scope: !44)
!51 = distinct !DISubprogram(name: "a", scope: !1, file: !1, line: 15, type: !52, scopeLine: 15, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!52 = !DISubroutineType(types: !53)
!53 = !{null}
!54 = !DILocation(line: 16, column: 3, scope: !51)
!55 = !DILocation(line: 17, column: 3, scope: !51)
!56 = !DILocation(line: 18, column: 1, scope: !51)
!57 = distinct !DISubprogram(name: "y", scope: !1, file: !1, line: 20, type: !52, scopeLine: 20, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!58 = !DILocation(line: 21, column: 3, scope: !57)
!59 = !DILocation(line: 22, column: 1, scope: !57)
