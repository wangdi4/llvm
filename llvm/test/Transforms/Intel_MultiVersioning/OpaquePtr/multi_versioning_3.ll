; RUN: opt < %s -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -passes=multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s

; Test multi versioning setting of profile weights on the conditional
; created that controls the version selection.

%struct._ZTS1S.S = type { i8 }

; In this case, the profile information should be maintained on the branch
; that controls the version selection.
define i32 @foo1(ptr "intel_dtrans_func_index"="1" %arg) !prof !10 !intel.dtrans.func.type !8 {
entry:
  %addr = getelementptr inbounds %struct._ZTS1S.S, ptr %arg, i64 0, i32 0
  %value = load i8, ptr %addr, align 1
  %cmp = icmp ne i8 %value, 0
  br i1 %cmp, label %if.then, label %if.else, !prof !11

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  %result = select i1 %cmp, i32 33, i32 22
  ret i32 %result
}
; CHECK-LABEL: define i32 @foo1(ptr {{.*}} %arg)
; The initial test should get profile weights based on the original counts
; CHECK: br i1 %2, label %entry, label %entry.clone, !prof !10

; The multi-versioned statements should maintain the profile counts.
; CHECK: br i1 true, label %if.then, label %if.else, !prof !11
; CHECK: br i1 false, label %if.then.clone, label %if.else.clone, !prof !12


; In this case, the profile weights need to be reversed on the updated
; instructions because the multi-versioning pass always creates the version
; selection test as a non-equality test.
define i32 @foo2(ptr "intel_dtrans_func_index"="1" %arg) !prof !12 !intel.dtrans.func.type !8 {
entry:
  %addr = getelementptr inbounds %struct._ZTS1S.S, ptr %arg, i64 0, i32 0
  %value = load i8, ptr %addr, align 1
  %cmp = icmp eq i8 %value, 0
  br i1 %cmp, label %if.then, label %if.else, !prof !13

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  %result = select i1 %cmp, i32 33, i32 22
  ret i32 %result
}
; CHECK-LABEL: define i32 @foo2(ptr {{.*}} %arg)
; CHECK: br i1 %2, label %entry, label %entry.clone, !prof !14
; CHECK: br i1 false, label %if.then, label %if.else, !prof !15
; CHECK: br i1 true, label %if.then.clone, label %if.else.clone, !prof !16

; No profile information should be set when there wasn't profile information
; available.
define i32 @foo3(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !8 {
entry:
  %addr = getelementptr inbounds %struct._ZTS1S.S, ptr %arg, i64 0, i32 0
  %value = load i8, ptr %addr, align 1
  %cmp1 = icmp ne i8 %value, 0
  br i1 %cmp1, label %if.then, label %if.else

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  %result = select i1 %cmp1, i32 33, i32 22
  ret i32 %result
}
; Verify that there is no !prof metadata attached to the branches.
; CHECK-LABEL: define i32 @foo3(ptr {{.*}} %arg)
; CHECK: br i1 %2, label %entry, label %entry.clone{{$}}
; CHECK: br i1 true, label %if.then, label %if.else{{$}}
; CHECK: br i1 false, label %if.then.clone, label %if.else.clone{{$}}

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

; CHECK: !10 = !{!"branch_weights", i32 12000, i32 600}
; CHECK: !11 = !{!"branch_weights", i32 12000, i32 0}
; CHECK: !12 = !{!"branch_weights", i32 0, i32 600}
; CHECK: !14 = !{!"branch_weights", i32 5000, i32 700}
; CHECK: !15 = !{!"branch_weights", i32 0, i32 5000}
; CHECK: !16 = !{!"branch_weights", i32 700, i32 0}
