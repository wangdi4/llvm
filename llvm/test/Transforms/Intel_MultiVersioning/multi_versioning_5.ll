; RUN: opt < %s -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -passes=multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s

; This test case checks that the profiling information was preserved after
; multiversioning when there are freeze instructions.

; CHECK: br i1 %2, label %entry, label %entry.clone, !prof !10
; CHECK: %frz = freeze i1 true
; CHECK: br i1 %frz, label %if.then, label %if.else, !prof !11
; CHECK: %frz.clone = freeze i1 false
; CHECK: br i1 %frz.clone, label %if.then.clone, label %if.else.clone, !prof !12

; CHECK: !10 = !{!"branch_weights", i32 12000, i32 600}
; CHECK: !11 = !{!"branch_weights", i32 12000, i32 0}
; CHECK: !12 = !{!"branch_weights", i32 0, i32 600}

%struct._ZTS1S.S = type { i8 }

define i32 @foo1(ptr "intel_dtrans_func_index"="1" %arg) !prof !10 !intel.dtrans.func.type !8 {
entry:
  %addr = getelementptr inbounds %struct._ZTS1S.S, ptr %arg, i64 0, i32 0
  %value = load i8, ptr %addr, align 1
  %cmp = icmp ne i8 %value, 0
  %frz = freeze i1 %cmp
  br i1 %frz, label %if.then, label %if.else, !prof !11

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  %result = select i1 %frz, i32 33, i32 22
  ret i32 %result
}

!intel.dtrans.types = !{!5}

!llvm.module.flags = !{!0, !1, !2, !3, !4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS1S.S zeroinitializer, i32 1, !6}
!6 = !{i8 0, i32 0}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!8 = distinct !{!9}
!9 = !{%struct._ZTS1S.S zeroinitializer, i32 1}
!10 = !{!"function_entry_count", i64 12600}
!11 = !{!"branch_weights", i32 12000, i32 600 }
!12 = !{!"function_entry_count", i64 5700}
!13 = !{!"branch_weights", i32 700, i32 5000 }
