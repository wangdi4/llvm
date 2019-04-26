; RUN: opt -inline -inline-report=128 < %s -S 2>&1 | FileCheck %s
; RUN: opt -passes='cgscc(inline)' -inline-report=128 < %s -S 2>&1 | FileCheck %s

; This test checks that metadata corresponding to the inlining report was
; updated in accordance with function inlining that happened. a() should
; be inlined in main() twice, resulting in z() call sites in the code.
; b() should be inlined in main() resulting in the x() callsites in the code,
; y() call site from b() should be optimized out by inliner and corresponding
; IR node in the main() inline report tree should be marked deleted.

; CHECK: define dso_local void @main() local_unnamed_addr !intel.function.inlining.report [[MAIN_FIR:![0-9]+]] {
; CHECK-NEXT: entry:
; CHECK-NEXT:   call void (...) @z(), !intel.callsite.inlining.report [[Z_A1_MAIN_CS:![0-9]+]]
; CHECK-NEXT:   call void (...) @x(), !intel.callsite.inlining.report [[X_B_MAIN_CS:![0-9]+]]
; CHECK-NEXT:   call void (...) @z(), !intel.callsite.inlining.report [[Z_A2_MAIN_CS:![0-9]+]]
; CHECK-NEXT:   ret void

; CHECK: !intel.module.inlining.report = !{[[A_FIR:![0-9]+]], [[Z_FIR:![0-9]+]], [[B_FIR:![0-9]+]], [[X_FIR:![0-9]+]], [[Y_FIR:![0-9]+]], [[MAIN_FIR]]}
; CHECK: [[A_FIR]] = distinct !{!"intel.function.inlining.report", [[A_NAME:![0-9]+]], [[A_CSs:![0-9]+]], [[MODULE_NAME:![0-9]+]], [[IS_DEAD_0:![0-9]+]], [[IS_DECL_0:![0-9]+]], [[LINK_A:![0-9]+]]}
; CHECK-NEXT: [[A_NAME]] = !{!"name: a"}
; CHECK-NEXT: [[A_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_A_CS:![0-9]+]]}
; CHECK-NEXT: [[Z_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME:![0-9]+]], null, [[IS_INL_0:![0-9]+]], [[REASON_EXTRN:![0-9]+]]{{.*}}
; CHECK-NEXT: [[Z_NAME]] = !{!"name: z"}
; CHECK-NEXT: [[IS_INL_0]] = !{!"isInlined: 0"}
; CHECK-NEXT: [[REASON_EXTRN]] = !{!"reason:{{.*}}
; CHECK-NEXT: {{.*}}inlineCost
; CHECK-NEXT: {{.*}}outerInlineCost
; CHECK-NEXT: {{.*}}inlineThreshold
; CHECK-NEXT: {{.*}}earlyExitCost
; CHECK-NEXT: {{.*}}earlyExitThreshold
; CHECK-NEXT: [[MODULE_NAME]] = !{!"moduleName: test4.c"}
; CHECK-NEXT: [[IS_DEAD_0]] = !{!"isDead: 0"}
; CHECK-NEXT: [[IS_DECL_0]] = !{!"isDeclaration: 0"}
; CHECK-NEXT: [[LINK_A]] = !{!"linkage: A"}
; CHECK-NEXT: [[Z_FIR]] = distinct !{!"intel.function.inlining.report", [[Z_NAME]], null, [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_1:![0-9]+]], [[LINK_A]]}
; CHECK-NEXT: [[IS_DECL_1]] = !{!"isDeclaration: 1"}
; CHECK-NEXT: [[B_FIR]] = distinct !{!"intel.function.inlining.report", [[B_NAME:![0-9]+]], [[B_CSs:![0-9]+]], [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_0]], [[LINK_A]]}
; CHECK-NEXT: [[B_NAME]] = !{!"name: b"}
; CHECK-NEXT: [[B_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_B_CS:![0-9]+]], [[Y_B_CS:![0-9]+]]}
; CHECK-NEXT: [[X_B_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME:![0-9]+]], null, [[IS_INL_0]], [[REASON_EXTRN]]{{.*}}
; CHECK-NEXT: [[X_NAME]] = !{!"name: x"}
; CHECK-NEXT: [[Y_B_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME:![0-9]+]], null, [[IS_INL_0]], [[REASON_EXTRN]]{{.*}}
; CHECK-NEXT: [[Y_NAME]] = !{!"name: y"}
; CHECK-NEXT: [[X_FIR]] = distinct !{!"intel.function.inlining.report", [[X_NAME]], null, [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_1]], [[LINK_A]]}
; CHECK-NEXT: [[Y_FIR]] = distinct !{!"intel.function.inlining.report", [[Y_NAME]], null, [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_1]], [[LINK_A]]}
; CHECK-NEXT: [[MAIN_FIR]] = distinct !{!"intel.function.inlining.report", [[MAIN_NAME:![0-9]+]], [[MAIN_CSs:![0-9]+]], [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_0]], [[LINK_A]]}
; CHECK-NEXT: [[MAIN_NAME]] = !{!"name: main"}
; CHECK-NEXT: [[MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[A1_MAIN_CS:![0-9]+]], [[B_MAIN_CS:![0-9]+]], [[A2_MAIN_CS:![0-9]+]]}
; CHECK-NEXT: [[A1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[A_NAME]], [[A1_MAIN_CSs:![0-9]+]], [[IS_INL_1:![0-9]+]], [[REASON:![0-9]+]]{{.*}}
; CHECK-NEXT: [[A1_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_A1_MAIN_CS]]}
; CHECK-NEXT: [[Z_A1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME]], null, [[IS_INL_0]], [[REASON_EXTRN]]{{.*}}
; CHECK-NEXT: [[IS_INL_1]] = !{!"isInlined: 1"}
; CHECK-NEXT: [[REASON]] = !{!"reason{{.*}}
; CHECK-NEXT: {{.*}}inlineCost
; CHECK-NEXT: {{.*}}inlineThreshold
; CHECK-NEXT: [[B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[B_NAME]], [[B_MAIN_CSs:![0-9]+]], [[IS_INL_1]], [[REASON]]{{.*}}
; CHECK-NEXT: [[B_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_B_MAIN_CS]], [[Y_B_MAIN_CS:![0-9]+]]}
; CHECK-NEXT: [[X_B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME]], null, [[IS_INL_0]], [[REASON_EXTRN]]{{.*}}
; CHECK-NEXT: [[Y_B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME]], null, [[IS_INL_0]], [[REASON_DELETED:![0-9]+]]{{.*}}
; CHECK-NEXT: [[REASON_DELETED]] = !{!"reason:{{.*}}
; CHECK-NEXT: {{.*}}inlineCost
; CHECK-NEXT: [[A2_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[A_NAME]], [[A2_MAIN_CSs:![0-9]+]], [[IS_INL_1]], [[REASON]]{{.*}}
; CHECK-NEXT: [[A2_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_A2_MAIN_CS]]}
; CHECK-NEXT: [[Z_A2_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME]], null, [[IS_INL_0]], [[REASON_EXTRN]]{{.*}}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @a() local_unnamed_addr !intel.function.inlining.report !2 {
entry:
  call void (...) @z(), !intel.callsite.inlining.report !5
  ret void
}

declare !intel.function.inlining.report !18 dso_local void @z(...) local_unnamed_addr

; Function Attrs: nounwind uwtable
define dso_local void @b(i32 %n) local_unnamed_addr !intel.function.inlining.report !20 {
entry:
  %cmp = icmp slt i32 %n, 3
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  call void (...) @x(), !intel.callsite.inlining.report !23
  br label %if.end

if.else:                                          ; preds = %entry
  call void (...) @y(), !intel.callsite.inlining.report !25
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare !intel.function.inlining.report !27 dso_local void @x(...) local_unnamed_addr

declare !intel.function.inlining.report !28 dso_local void @y(...) local_unnamed_addr

; Function Attrs: nounwind uwtable
define dso_local void @main() local_unnamed_addr  !intel.function.inlining.report !29 {
entry:
  call void @a(), !intel.callsite.inlining.report !32
  call void @b(i32 2), !intel.callsite.inlining.report !34
  call void @a(), !intel.callsite.inlining.report !35
  ret void
}

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!intel.module.inlining.report = !{!2, !18, !20, !27, !28, !29}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = distinct !{!"intel.function.inlining.report", !3, !4, !14, !15, !16, !17}
!3 = !{!"name: a"}
!4 = distinct !{!"intel.callsites.inlining.report", !5}
!5 = distinct !{!"intel.callsite.inlining.report", !6, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!6 = !{!"name: z"}
!7 = !{!"isInlined: 0"}
!8 = !{!"reason: 32"} ;NinlrExtern
!9 = !{!"inlineCost: -1"}
!10 = !{!"outerInlineCost: -1"}
!11 = !{!"inlineThreshold: -1"}
!12 = !{!"earlyExitCost: 2147483647"}
!13 = !{!"earlyExitThreshold: 2147483647"}
!14 = !{!"moduleName: test4.c"}
!15 = !{!"isDead: 0"}
!16 = !{!"isDeclaration: 0"}
!17 = !{!"linkage: A"}
!18 = distinct !{!"intel.function.inlining.report", !6, null, !14, !15, !19, !17}
!19 = !{!"isDeclaration: 1"}
!20 = distinct !{!"intel.function.inlining.report", !21, !22, !14, !15, !16, !17}
!21 = !{!"name: b"}
!22 = distinct !{!"intel.callsites.inlining.report", !23, !25}
!23 = distinct !{!"intel.callsite.inlining.report", !24, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!24 = !{!"name: x"}
!25 = distinct !{!"intel.callsite.inlining.report", !26, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!26 = !{!"name: y"}
!27 = distinct !{!"intel.function.inlining.report", !24, null, !14, !15, !19, !17}
!28 = distinct !{!"intel.function.inlining.report", !26, null, !14, !15, !19, !17}
!29 = distinct !{!"intel.function.inlining.report", !30, !31, !14, !15, !16, !17}
!30 = !{!"name: main"}
!31 = distinct !{!"intel.callsites.inlining.report", !32, !34, !35}
!32 = distinct !{!"intel.callsite.inlining.report", !3, null, !7, !33, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!33 = !{!"reason: 25"} ;NinlrNoReason
!34 = distinct !{!"intel.callsite.inlining.report", !21, null, !7, !33, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!35 = distinct !{!"intel.callsite.inlining.report", !3, null, !7, !33, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}

