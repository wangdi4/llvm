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

; Function Attrs: nounwind uwtable
define dso_local i32 @main() !dbg !42 !intel.function.inlining.report !8 {
entry:
  call void @a(), !dbg !46, !intel.callsite.inlining.report !11
  call void (...) @b(), !dbg !47, !intel.callsite.inlining.report !21
  ret i32 0, !dbg !48
}

declare !intel.function.inlining.report !27 dso_local void @b(...)

; Function Attrs: nounwind uwtable
define dso_local void @a() !dbg !49 !intel.function.inlining.report !29 {
entry:
  call void (...) @x(), !dbg !52, !intel.callsite.inlining.report !31
  call void @y(), !dbg !53, !intel.callsite.inlining.report !33
  ret void, !dbg !54
}

declare !intel.function.inlining.report !35 dso_local void @x(...)

; Function Attrs: nounwind uwtable
define internal void @y() !dbg !55 !intel.function.inlining.report !36 {
entry:
  call void (...) @z(), !dbg !56, !intel.callsite.inlining.report !38
  ret void, !dbg !57
}

declare !intel.function.inlining.report !41 dso_local void @z(...)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}
!intel.module.inlining.report = !{!8, !27, !29, !35, !36, !41}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based icx (ICX) 2019.8.2.0", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test1.c", directory: "/export/iusers/ochupina/inl_report")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"icx (ICX) 2019.8.2.0"}
!8 = distinct !{!"intel.function.inlining.report", !9, !10, !20, !24, !25, !26, !59, !58}
!9 = !{!"name: main"}
!10 = distinct !{!"intel.callsites.inlining.report", !11, !21}
!11 = distinct !{!"intel.callsite.inlining.report", !12, null, !13, !14, !15, !16, !17, !18, !19, !"line: 10 col: 3", !20, !58}
!12 = !{!"name: a"}
!13 = !{!"isInlined: 0"}
!14 = !{!"reason: 24"}
!15 = !{!"inlineCost: -1"}
!16 = !{!"outerInlineCost: -1"}
!17 = !{!"inlineThreshold: -1"}
!18 = !{!"earlyExitCost: 2147483647"}
!19 = !{!"earlyExitThreshold: 2147483647"}
!20 = !{!"moduleName: test1.c"}
!21 = distinct !{!"intel.callsite.inlining.report", !22, null, !13, !23, !15, !16, !17, !18, !19, !"line: 11 col: 3", !20, !58}
!22 = !{!"name: b"}
!23 = !{!"reason: 39"}
!24 = !{!"isDead: 0"}
!25 = !{!"isDeclaration: 0"}
!26 = !{!"linkage: A"}
!27 = distinct !{!"intel.function.inlining.report", !22, null, !20, !24, !28, !26, !59, !58}
!28 = !{!"isDeclaration: 1"}
!29 = distinct !{!"intel.function.inlining.report", !12, !30, !20, !24, !25, !26, !59, !58}
!30 = distinct !{!"intel.callsites.inlining.report", !31, !33}
!31 = distinct !{!"intel.callsite.inlining.report", !32, null, !13, !23, !15, !16, !17, !18, !19, !"line: 16 col: 3", !20, !58}
!32 = !{!"name: x"}
!33 = distinct !{!"intel.callsite.inlining.report", !34, null, !13, !14, !15, !16, !17, !18, !19, !"line: 17 col: 3", !20, !58}
!34 = !{!"name: y"}
!35 = distinct !{!"intel.function.inlining.report", !32, null, !20, !24, !28, !26, !59, !58}
!36 = distinct !{!"intel.function.inlining.report", !34, !37, !20, !24, !25, !40, !59, !58}
!37 = distinct !{!"intel.callsites.inlining.report", !38}
!38 = distinct !{!"intel.callsite.inlining.report", !39, null, !13, !23, !15, !16, !17, !18, !19, !"line: 21 col: 3", !20, !58}
!39 = !{!"name: z"}
!40 = !{!"linkage: L"}
!59 = !{!"language: C"}
!58 = !{!"isSuppressPrint: 0"}
!41 = distinct !{!"intel.function.inlining.report", !39, null, !20, !24, !28, !26, !59}
!42 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 9, type: !43, scopeLine: 9, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!43 = !DISubroutineType(types: !44)
!44 = !{!45}
!45 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!46 = !DILocation(line: 10, column: 3, scope: !42)
!47 = !DILocation(line: 11, column: 3, scope: !42)
!48 = !DILocation(line: 12, column: 3, scope: !42)
!49 = distinct !DISubprogram(name: "a", scope: !1, file: !1, line: 15, type: !50, scopeLine: 15, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!50 = !DISubroutineType(types: !51)
!51 = !{null}
!52 = !DILocation(line: 16, column: 3, scope: !49)
!53 = !DILocation(line: 17, column: 3, scope: !49)
!54 = !DILocation(line: 18, column: 1, scope: !49)
!55 = distinct !DISubprogram(name: "y", scope: !1, file: !1, line: 20, type: !50, scopeLine: 20, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!56 = !DILocation(line: 21, column: 3, scope: !55)
!57 = !DILocation(line: 22, column: 1, scope: !55)
