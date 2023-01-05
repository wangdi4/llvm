; RUN: opt -opaque-pointers -passes=inlinereportsetup -inline-report=0x180 < %s -S 2>&1 | FileCheck %s

; This test checks that setup pass correctly adds new call sites and functions to the inline report.


; CHECK: define dso_local void @main() local_unnamed_addr !intel.function.inlining.report [[MAIN_FIR:![0-9]+]] {
; CHECK:   call void (...) @foo1(), !intel.callsite.inlining.report [[FOO1_MAIN_CS:![0-9]+]]
; CHECK:   tail call void (...) @k(), !intel.callsite.inlining.report [[K_A_MAIN_CS:![0-9]+]]
; CHECK:   call void (...) @foo2(), !intel.callsite.inlining.report [[FOO2_A_MAIN_CS:![0-9]+]]
; CHECK:   tail call void (...) @l(), !intel.callsite.inlining.report [[L_A_MAIN_CS:![0-9]+]]
; CHECK:   call void (...) @foo3(), !intel.callsite.inlining.report [[FOO3_MAIN_CS:![0-9]+]]
; CHECK:   tail call void (...) @x(), !intel.callsite.inlining.report [[X_B_MAIN_CS:![0-9]+]]
; CHECK:   call void (...) @foo4(), !intel.callsite.inlining.report [[FOO4_MAIN_CS:![0-9]+]]

; CHECK: declare !intel.function.inlining.report [[FOO1_FIR:![0-9]+]] dso_local void @foo1(...) local_unnamed_addr

; CHECK: declare !intel.function.inlining.report [[FOO2_FIR:![0-9]+]] dso_local void @foo2(...) local_unnamed_addr

; CHECK: declare !intel.function.inlining.report [[FOO3_FIR:![0-9]+]] dso_local void @foo3(...) local_unnamed_addr

; CHECK: declare !intel.function.inlining.report [[FOO4_FIR:![0-9]+]] dso_local void @foo4(...) local_unnamed_addr

; CHECK: !intel.module.inlining.report = !{[[A_FIR:![0-9]+]], [[K_FIR:![0-9]+]], [[L_FIR:![0-9]+]], [[B_FIR:![0-9]+]], [[X_FIR:![0-9]+]], [[Y_FIR:![0-9]+]], [[MAIN_FIR]], [[FOO1_FIR]], [[FOO2_FIR]], [[FOO3_FIR]], [[FOO4_FIR]]}
; CHECK: [[A_FIR]] = distinct !{!"intel.function.inlining.report", [[A_NAME:![0-9]+]], [[A_CSs:![0-9]+]]{{.*}}
; CHECK: [[A_NAME]] = !{!"name: a"}
; CHECK: [[A_CSs]] = distinct !{!"intel.callsites.inlining.report", [[K_A_CS:![0-9]+]], [[L_A_CS:![0-9]+]]}
; CHECK: [[K_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[K_NAME:![0-9]+]], null, [[IS_INL_0:![0-9]+]]{{.*}}
; CHECK: [[K_NAME]] = !{!"name: k"}
; CHECK: [[IS_INL_0]] = !{!"isInlined: 0"}
; CHECK: [[L_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[L_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[L_NAME]] = !{!"name: l"}
; CHECK: [[K_FIR]] = distinct !{!"intel.function.inlining.report", [[K_NAME]], null, {{.*}}
; CHECK: [[L_FIR]] = distinct !{!"intel.function.inlining.report", [[L_NAME]], null, {{.*}}
; CHECK: [[B_FIR]] = distinct !{!"intel.function.inlining.report", [[B_NAME:![0-9]+]], [[B_CSs:![0-9]+]]{{.*}}
; CHECK: [[B_NAME]] = !{!"name: b"}
; CHECK: [[B_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_B_CS:![0-9]+]], [[Y_B_CS:![0-9]+]]}
; CHECK: [[X_B_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[X_NAME]] = !{!"name: x"}
; CHECK: [[Y_B_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[Y_NAME]] = !{!"name: y"}
; CHECK: [[X_FIR]] = distinct !{!"intel.function.inlining.report", [[X_NAME]], null, {{.*}}
; CHECK: [[Y_FIR]] = distinct !{!"intel.function.inlining.report", [[Y_NAME]], null, {{.*}}
; CHECK: [[MAIN_FIR]] = distinct !{!"intel.function.inlining.report", [[MAIN_NAME:![0-9]+]], [[MAIN_CSs:![0-9]+]], {{.*}}
; CHECK: [[MAIN_NAME]] = !{!"name: main"}
; CHECK: [[MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[FOO1_MAIN_CS]], [[A_MAIN_CS:![0-9]+]], [[FOO3_MAIN_CS]], [[B_MAIN_CS:![0-9]+]], [[FOO4_MAIN_CS]]}
; CHECK: [[FOO1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[FOO1_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[FOO1_NAME]] = !{!"name: foo1"}
; CHECK: [[A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[A_NAME]], [[A_MAIN_CSs:![0-9]+]], [[IS_INL_1:![0-9]+]]{{.*}}
; CHECK: [[A_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[K_A_MAIN_CS]], [[FOO2_A_MAIN_CS]], [[L_A_MAIN_CS]]}
; CHECK: [[K_A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[K_NAME]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[FOO2_A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[FOO2_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[FOO2_NAME]] = !{!"name: foo2"}
; CHECK: [[L_A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[L_NAME]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[IS_INL_1]] = !{!"isInlined: 1"}
; CHECK: [[FOO3_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[FOO3_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[FOO3_NAME]] = !{!"name: foo3"}
; CHECK: [[B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[B_NAME]], [[B_MAIN_CSs:![0-9]+]], [[IS_INL_1]]{{.*}}
; CHECK: [[B_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_B_MAIN_CS]]}
; CHECK: [[X_B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[FOO4_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[FOO4_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[FOO4_NAME]] = !{!"name: foo4"}
; CHECK: [[FOO1_FIR]] = distinct !{!"intel.function.inlining.report", [[FOO1_NAME]], null, {{.*}}
; CHECK: [[FOO2_FIR]] = distinct !{!"intel.function.inlining.report", [[FOO2_NAME]], null, {{.*}}
; CHECK: [[FOO3_FIR]] = distinct !{!"intel.function.inlining.report", [[FOO3_NAME]], null, {{.*}}
; CHECK: [[FOO4_FIR]] = distinct !{!"intel.function.inlining.report", [[FOO4_NAME]], null, {{.*}}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @a() local_unnamed_addr !intel.function.inlining.report !2 {
entry:
  tail call void (...) @k(), !intel.callsite.inlining.report !5
  tail call void (...) @l(), !intel.callsite.inlining.report !15
  ret void
}

declare !intel.function.inlining.report !20 dso_local void @k(...) local_unnamed_addr

declare !intel.function.inlining.report !22 dso_local void @l(...) local_unnamed_addr

define dso_local void @b(i32 %n) local_unnamed_addr !intel.function.inlining.report !23 {
entry:
  %cmp = icmp slt i32 %n, 3
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  tail call void (...) @x(), !intel.callsite.inlining.report !26
  br label %if.end

if.else:                                          ; preds = %entry
  tail call void (...) @y(), !intel.callsite.inlining.report !28
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare !intel.function.inlining.report !30 dso_local void @x(...) local_unnamed_addr

declare !intel.function.inlining.report !31 dso_local void @y(...) local_unnamed_addr

define dso_local void @main() local_unnamed_addr !intel.function.inlining.report !32 {
entry:
  call void (...) @foo1()
  tail call void (...) @k(), !intel.callsite.inlining.report !37
  call void (...) @foo2()
  tail call void (...) @l(), !intel.callsite.inlining.report !38
  call void (...) @foo3()
  tail call void (...) @x(), !intel.callsite.inlining.report !45
  call void (...) @foo4()
  ret void
}

declare dso_local void @foo1(...) local_unnamed_addr

declare dso_local void @foo2(...) local_unnamed_addr

declare dso_local void @foo3(...) local_unnamed_addr

declare dso_local void @foo4(...) local_unnamed_addr

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!intel.module.inlining.report = !{!2, !20, !22, !23, !30, !31, !32}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = distinct !{!"intel.function.inlining.report", !3, !4, !14, !17, !18, !19}
!3 = !{!"name: a"}
!4 = distinct !{!"intel.callsites.inlining.report", !5, !15}
!5 = distinct !{!"intel.callsite.inlining.report", !6, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!6 = !{!"name: k"}
!7 = !{!"isInlined: 0"}
!8 = !{!"reason: 34"}
!9 = !{!"inlineCost: -1"}
!10 = !{!"outerInlineCost: -1"}
!11 = !{!"inlineThreshold: -1"}
!12 = !{!"earlyExitCost: 2147483647"}
!13 = !{!"earlyExitThreshold: 2147483647"}
!14 = !{!"moduleName: test4.c"}
!15 = distinct !{!"intel.callsite.inlining.report", !16, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!16 = !{!"name: l"}
!17 = !{!"isDead: 0"}
!18 = !{!"isDeclaration: 0"}
!19 = !{!"linkage: A"}
!20 = distinct !{!"intel.function.inlining.report", !6, null, !14, !17, !21, !19}
!21 = !{!"isDeclaration: 1"}
!22 = distinct !{!"intel.function.inlining.report", !16, null, !14, !17, !21, !19}
!23 = distinct !{!"intel.function.inlining.report", !24, !25, !14, !17, !18, !19}
!24 = !{!"name: b"}
!25 = distinct !{!"intel.callsites.inlining.report", !26, !28}
!26 = distinct !{!"intel.callsite.inlining.report", !27, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!27 = !{!"name: x"}
!28 = distinct !{!"intel.callsite.inlining.report", !29, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!29 = !{!"name: y"}
!30 = distinct !{!"intel.function.inlining.report", !27, null, !14, !17, !21, !19}
!31 = distinct !{!"intel.function.inlining.report", !29, null, !14, !17, !21, !19}
!32 = distinct !{!"intel.function.inlining.report", !33, !34, !14, !17, !18, !19}
!33 = !{!"name: main"}
!34 = distinct !{!"intel.callsites.inlining.report", !35, !43}
!35 = distinct !{!"intel.callsite.inlining.report", !3, !36, !39, !40, !41, !10, !42, !12, !13, !"line: 0 col: 0", !14}
!36 = distinct !{!"intel.callsites.inlining.report", !37, !38}
!37 = distinct !{!"intel.callsite.inlining.report", !6, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!38 = distinct !{!"intel.callsite.inlining.report", !16, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!39 = !{!"isInlined: 1"}
!40 = !{!"reason: 9"}
!41 = !{!"inlineCost: 30"}
!42 = !{!"inlineThreshold: 337"}
!43 = distinct !{!"intel.callsite.inlining.report", !24, !44, !39, !40, !46, !10, !42, !12, !13, !"line: 0 col: 0", !14}
!44 = distinct !{!"intel.callsites.inlining.report", !45}
!45 = distinct !{!"intel.callsite.inlining.report", !27, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!46 = !{!"inlineCost: -5"}
