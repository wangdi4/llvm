; RUN: opt -passes=sample-profile -sample-profile-file=%S/Inputs/Intel_loopcount_average.prof -S < %s | FileCheck %s --check-prefixes=CHECK,AVERAGE --implicit-check-not='"llvm.intel.loopcount"'
; RUN: opt -passes=sample-profile -sample-profile-file=%S/Inputs/Intel_loopcount_average.prof -sample-profile-intel-loopcount-average=false -S < %s | FileCheck %s --implicit-check-not='llvm.intel.loopcount'
; RUN: opt -passes=sample-profile -sample-profile-file=%S/Inputs/Intel_loopcount_average.prof -sample-profile-intel-loopcount-average=false -sample-profile-intel-loopcount-average-common -S < %s | FileCheck %s --check-prefixes=CHECK,COMMON --implicit-check-not=llvm.intel.loopcount_average
; RUN: opt -passes=sample-profile -sample-profile-file=%S/Inputs/Intel_loopcount_average.prof -sample-profile-intel-loopcount-average-common -S < %s | FileCheck %s --check-prefixes=CHECK,AVERAGE,COMMON

; This test checks that we can automatically generate
; llvm.loop.intel.loopcount_average and/or llvm.loop.intel.loopcount metadata
; when loading profile data. The original source uses a macro to define five
; identical functions with both top and bottom-tested loops:

; #define LOOP_COUNT_N(N)       \
; void loop_count_##N(int N) {  \
;   for (int i = 0; i < N; ++i) \
;     foo();                    \
;                               \
;   int i = 0;                  \
;   do foo();                   \
;   while (++i < N);            \
; }                             \
;
; LOOP_COUNT_N(four)
; LOOP_COUNT_N(one_hundred)
; LOOP_COUNT_N(one)
; LOOP_COUNT_N(zero)
; LOOP_COUNT_N(one_or_two)

; These are then run with different (generally constant) trip counts passed in:

; for (int i = 0; i < N; ++i) {
;   loop_count_four(4);
;   loop_count_one_hundred(100);
;   loop_count_one(1);
;   loop_count_zero(0);
;   loop_count_one_or_two(1 + i%2);
; }

declare void @foo()

define void @loop_count_four(i32 noundef %four) "use-sample-profile" !dbg !3 {
; CHECK-LABEL: define void @loop_count_four
entry:
  br label %for.cond, !dbg !6

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %four, !dbg !7
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !dbg !7

for.cond.cleanup:                                 ; preds = %for.cond
  br label %do.body, !dbg !9

for.body:                                         ; preds = %for.cond
  call void @foo(), !dbg !11
  %inc = add nsw i32 %i.0, 1, !dbg !13
  br label %for.cond, !dbg !13, !llvm.loop !15

; CHECK: br label %for.cond{{.*}} !llvm.loop ![[#FOUR_TOP_LOOP:]]

do.body:                                          ; preds = %do.body, %for.cond.cleanup
  %i1.0 = phi i32 [ 0, %for.cond.cleanup ], [ %inc2, %do.body ]
  call void @foo(), !dbg !16
  %inc2 = add nsw i32 %i1.0, 1, !dbg !18
  %cmp3 = icmp slt i32 %inc2, %four, !dbg !18
  br i1 %cmp3, label %do.body, label %do.end, !dbg !18, !llvm.loop !20

; CHECK: br i1 %cmp3, label %do.body, label %do.end{{.*}} !llvm.loop ![[#FOUR_BOTTOM_LOOP:]]

do.end:                                           ; preds = %do.body
  ret void, !dbg !21
}

define void @loop_count_one_hundred(i32 noundef %one_hundred) "use-sample-profile" !dbg !23 {
; CHECK-LABEL: define void @loop_count_one_hundred
entry:
  br label %for.cond, !dbg !24

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %one_hundred, !dbg !25
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !dbg !25

for.cond.cleanup:                                 ; preds = %for.cond
  br label %do.body, !dbg !27

for.body:                                         ; preds = %for.cond
  call void @foo(), !dbg !29
  %inc = add nsw i32 %i.0, 1, !dbg !31
  br label %for.cond, !dbg !31, !llvm.loop !33

; CHECK: br label %for.cond{{.*}} !llvm.loop ![[#ONE_HUNDRED_TOP_LOOP:]]

do.body:                                          ; preds = %do.body, %for.cond.cleanup
  %i1.0 = phi i32 [ 0, %for.cond.cleanup ], [ %inc2, %do.body ]
  call void @foo(), !dbg !34
  %inc2 = add nsw i32 %i1.0, 1, !dbg !36
  %cmp3 = icmp slt i32 %inc2, %one_hundred, !dbg !36
  br i1 %cmp3, label %do.body, label %do.end, !dbg !36, !llvm.loop !38

; CHECK: br i1 %cmp3, label %do.body, label %do.end{{.*}} !llvm.loop ![[#ONE_HUNDRED_BOTTOM_LOOP:]]

do.end:                                           ; preds = %do.body
  ret void, !dbg !39
}

define void @loop_count_one(i32 noundef %one) "use-sample-profile" !dbg !41 {
; CHECK-LABEL: define void @loop_count_one
entry:
  br label %for.cond, !dbg !42

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %one, !dbg !43
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !dbg !43

for.cond.cleanup:                                 ; preds = %for.cond
  br label %do.body, !dbg !45

for.body:                                         ; preds = %for.cond
  call void @foo(), !dbg !47
  %inc = add nsw i32 %i.0, 1, !dbg !49
  br label %for.cond, !dbg !49, !llvm.loop !51

; CHECK: br label %for.cond{{.*}} !llvm.loop ![[#ONE_TOP_LOOP:]]

do.body:                                          ; preds = %do.body, %for.cond.cleanup
  %i1.0 = phi i32 [ 0, %for.cond.cleanup ], [ %inc2, %do.body ]
  call void @foo(), !dbg !52
  %inc2 = add nsw i32 %i1.0, 1, !dbg !54
  %cmp3 = icmp slt i32 %inc2, %one, !dbg !54
  br i1 %cmp3, label %do.body, label %do.end, !dbg !54, !llvm.loop !56

; CHECK: br i1 %cmp3, label %do.body, label %do.end{{.*}} !llvm.loop ![[#ONE_BOTTOM_LOOP:]]

do.end:                                           ; preds = %do.body
  ret void, !dbg !57
}

define void @loop_count_zero(i32 noundef %zero) "use-sample-profile" !dbg !59 {
; CHECK-LABEL: define void @loop_count_zero
entry:
  br label %for.cond, !dbg !60

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %zero, !dbg !61
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !dbg !61

for.cond.cleanup:                                 ; preds = %for.cond
  br label %do.body, !dbg !63

for.body:                                         ; preds = %for.cond
  call void @foo(), !dbg !65
  %inc = add nsw i32 %i.0, 1, !dbg !67
  br label %for.cond, !dbg !67, !llvm.loop !69

; CHECK: br label %for.cond{{.*}} !llvm.loop ![[#ZERO_TOP_LOOP:]]

do.body:                                          ; preds = %do.body, %for.cond.cleanup
  %i1.0 = phi i32 [ 0, %for.cond.cleanup ], [ %inc2, %do.body ]
  call void @foo(), !dbg !70
  %inc2 = add nsw i32 %i1.0, 1, !dbg !72
  %cmp3 = icmp slt i32 %inc2, %zero, !dbg !72
  br i1 %cmp3, label %do.body, label %do.end, !dbg !72, !llvm.loop !74

; CHECK: br i1 %cmp3, label %do.body, label %do.end{{.*}} !llvm.loop ![[#ZERO_BOTTOM_LOOP:]]

do.end:                                           ; preds = %do.body
  ret void, !dbg !75
}

define void @loop_count_one_or_two(i32 noundef %one_or_two) "use-sample-profile" !dbg !77 {
; CHECK-LABEL: define void @loop_count_one_or_two
entry:
  br label %for.cond, !dbg !78

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %one_or_two, !dbg !79
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !dbg !79

; CHECK: br label %for.cond{{.*}} !llvm.loop ![[#ONE_OR_TWO_TOP_LOOP:]]

for.cond.cleanup:                                 ; preds = %for.cond
  br label %do.body, !dbg !81

for.body:                                         ; preds = %for.cond
  call void @foo(), !dbg !83
  %inc = add nsw i32 %i.0, 1, !dbg !85
  br label %for.cond, !dbg !85, !llvm.loop !87

do.body:                                          ; preds = %do.body, %for.cond.cleanup
  %i1.0 = phi i32 [ 0, %for.cond.cleanup ], [ %inc2, %do.body ]
  call void @foo(), !dbg !88
  %inc2 = add nsw i32 %i1.0, 1, !dbg !90
  %cmp3 = icmp slt i32 %inc2, %one_or_two, !dbg !90
  br i1 %cmp3, label %do.body, label %do.end, !dbg !90, !llvm.loop !92

; CHECK: br i1 %cmp3, label %do.body, label %do.end{{.*}} !llvm.loop ![[#ONE_OR_TWO_BOTTOM_LOOP:]]

do.end:                                           ; preds = %do.body
  ret void, !dbg !93
}

; CHECK-DAG: ![[#FOUR_TOP_LOOP]] = distinct !{![[#FOUR_TOP_LOOP]], ![[#]], ![[#]]
; AVERAGE-SAME: ![[#LOOPCOUNT_AVG_4:]]
; COMMON-SAME: ![[#LOOPCOUNT_COMMON_4:]]
; AVERAGE-DAG: ![[#LOOPCOUNT_AVG_4]] = !{!"llvm.loop.intel.loopcount_average", i64 4}
; COMMON-DAG: ![[#LOOPCOUNT_COMMON_4]] = !{!"llvm.loop.intel.loopcount", i64 4}
; CHECK-DAG: ![[#FOUR_BOTTOM_LOOP]] = distinct !{![[#FOUR_BOTTOM_LOOP]], ![[#]], ![[#]]
; AVERAGE-SAME: ![[#LOOPCOUNT_AVG_4:]]
; COMMON-SAME: ![[#LOOPCOUNT_COMMON_4:]]

; NOTE: Averages may be imprecise for larger numbers due to sampling error; in
; this case it's measured to be 98 or 97 when the true value is 100.
; CHECK-DAG: ![[#ONE_HUNDRED_TOP_LOOP]] = distinct !{![[#ONE_HUNDRED_TOP_LOOP]], ![[#]], ![[#]]
; AVERAGE-SAME: ![[#LOOPCOUNT_AVG_98:]]
; COMMON-SAME: ![[#LOOPCOUNT_COMMON_98:]]
; AVERAGE-DAG: ![[#LOOPCOUNT_AVG_98]] = !{!"llvm.loop.intel.loopcount_average", i64 98}
; COMMON-DAG: ![[#LOOPCOUNT_COMMON_98]] = !{!"llvm.loop.intel.loopcount", i64 98}
; CHECK-DAG: ![[#ONE_HUNDRED_BOTTOM_LOOP]] = distinct !{![[#ONE_HUNDRED_BOTTOM_LOOP]], ![[#]], ![[#]]
; AVERAGE-SAME: ![[#LOOPCOUNT_AVG_97:]]
; COMMON-SAME: ![[#LOOPCOUNT_COMMON_97:]]
; AVERAGE-DAG: ![[#LOOPCOUNT_AVG_97]] = !{!"llvm.loop.intel.loopcount_average", i64 97}
; COMMON-DAG: ![[#LOOPCOUNT_COMMON_97]] = !{!"llvm.loop.intel.loopcount", i64 97}

; CHECK-DAG: ![[#ONE_TOP_LOOP]] = distinct !{![[#ONE_TOP_LOOP]], ![[#]], ![[#]]
; AVERAGE-SAME: ![[#LOOPCOUNT_AVG_1:]]
; COMMON-SAME: ![[#LOOPCOUNT_COMMON_1:]]
; AVERAGE-DAG: ![[#LOOPCOUNT_AVG_1]] = !{!"llvm.loop.intel.loopcount_average", i64 1}
; COMMON-DAG: ![[#LOOPCOUNT_COMMON_1]] = !{!"llvm.loop.intel.loopcount", i64 1}
; CHECK-DAG: ![[#ONE_BOTTOM_LOOP]] = distinct !{![[#ONE_BOTTOM_LOOP]], ![[#]], ![[#]]
; AVERAGE-SAME: ![[#LOOPCOUNT_AVG_1:]]
; COMMON-SAME: ![[#LOOPCOUNT_COMMON_1:]]

; CHECK-DAG: ![[#ZERO_TOP_LOOP]] = distinct !{![[#ZERO_TOP_LOOP]], ![[#]], ![[#]]
; AVERAGE-SAME: ![[#LOOPCOUNT_AVG_0:]]
; COMMON-SAME: ![[#LOOPCOUNT_COMMON_0:]]
; AVERAGE-DAG: ![[#LOOPCOUNT_AVG_0]] = !{!"llvm.loop.intel.loopcount_average", i64 0}
; COMMON-DAG: ![[#LOOPCOUNT_COMMON_0]] = !{!"llvm.loop.intel.loopcount", i64 0}
; CHECK-DAG: ![[#ZERO_BOTTOM_LOOP]] = distinct !{![[#ZERO_BOTTOM_LOOP]], ![[#]], ![[#]]
; AVERAGE-SAME: ![[#LOOPCOUNT_AVG_1:]]
; COMMON-SAME: ![[#LOOPCOUNT_COMMON_1:]]

; NOTE: Average trip counts aren't necessarily integer, but the metadata
; requires an integer value. In this case the true value for both loops is 1.5,
; which is rounded down to 1 or up to 2 based on which side the sampling error
; falls on for the loop (here, rounding 1.48 down for the top-tested loop and 1.68 up for the bottom-tested one).
; CHECK-DAG: ![[#ONE_OR_TWO_TOP_LOOP]] = distinct !{![[#ONE_OR_TWO_TOP_LOOP]], ![[#]], ![[#]]
; AVERAGE-SAME: ![[#LOOPCOUNT_AVG_1:]]
; COMMON-SAME: ![[#LOOPCOUNT_COMMON_1:]]
; CHECK-DAG: ![[#ONE_OR_TWO_BOTTOM_LOOP]] = distinct !{![[#ONE_OR_TWO_BOTTOM_LOOP]], ![[#]], ![[#]]
; AVERAGE-SAME: ![[#LOOPCOUNT_AVG_2:]]
; COMMON-SAME: ![[#LOOPCOUNT_COMMON_2:]]
; AVERAGE-DAG: ![[#LOOPCOUNT_AVG_2]] = !{!"llvm.loop.intel.loopcount_average", i64 2}
; COMMON-DAG: ![[#LOOPCOUNT_COMMON_2]] = !{!"llvm.loop.intel.loopcount", i64 2}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: NoDebug, splitDebugInlining: false, debugInfoForProfiling: true, nameTableKind: None)
!1 = !DIFile(filename: "Intel_loopcount_average.c", directory: "/home/dwoodwor/spgo-loopcountavg/testcase")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = distinct !DISubprogram(name: "loop_count_four", scope: !1, file: !1, line: 13, type: !4, scopeLine: 13, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!4 = !DISubroutineType(types: !5)
!5 = !{}
!6 = !DILocation(line: 13, column: 1, scope: !3)
!7 = !DILocation(line: 13, column: 1, scope: !8)
!8 = !DILexicalBlockFile(scope: !3, file: !1, discriminator: 2)
!9 = !DILocation(line: 13, column: 1, scope: !10)
!10 = !DILexicalBlockFile(scope: !3, file: !1, discriminator: 8)
!11 = !DILocation(line: 13, column: 1, scope: !12)
!12 = !DILexicalBlockFile(scope: !3, file: !1, discriminator: 4)
!13 = !DILocation(line: 13, column: 1, scope: !14)
!14 = !DILexicalBlockFile(scope: !3, file: !1, discriminator: 6)
!15 = distinct !{!15, !6, !6}
!16 = !DILocation(line: 13, column: 1, scope: !17)
!17 = !DILexicalBlockFile(scope: !3, file: !1, discriminator: 10)
!18 = !DILocation(line: 13, column: 1, scope: !19)
!19 = !DILexicalBlockFile(scope: !3, file: !1, discriminator: 12)
!20 = distinct !{!20, !6, !6}
!21 = !DILocation(line: 13, column: 1, scope: !22)
!22 = !DILexicalBlockFile(scope: !3, file: !1, discriminator: 14)
!23 = distinct !DISubprogram(name: "loop_count_one_hundred", scope: !1, file: !1, line: 14, type: !4, scopeLine: 14, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!24 = !DILocation(line: 14, column: 1, scope: !23)
!25 = !DILocation(line: 14, column: 1, scope: !26)
!26 = !DILexicalBlockFile(scope: !23, file: !1, discriminator: 2)
!27 = !DILocation(line: 14, column: 1, scope: !28)
!28 = !DILexicalBlockFile(scope: !23, file: !1, discriminator: 8)
!29 = !DILocation(line: 14, column: 1, scope: !30)
!30 = !DILexicalBlockFile(scope: !23, file: !1, discriminator: 4)
!31 = !DILocation(line: 14, column: 1, scope: !32)
!32 = !DILexicalBlockFile(scope: !23, file: !1, discriminator: 6)
!33 = distinct !{!33, !24, !24}
!34 = !DILocation(line: 14, column: 1, scope: !35)
!35 = !DILexicalBlockFile(scope: !23, file: !1, discriminator: 10)
!36 = !DILocation(line: 14, column: 1, scope: !37)
!37 = !DILexicalBlockFile(scope: !23, file: !1, discriminator: 12)
!38 = distinct !{!38, !24, !24}
!39 = !DILocation(line: 14, column: 1, scope: !40)
!40 = !DILexicalBlockFile(scope: !23, file: !1, discriminator: 14)
!41 = distinct !DISubprogram(name: "loop_count_one", scope: !1, file: !1, line: 15, type: !4, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!42 = !DILocation(line: 15, column: 1, scope: !41)
!43 = !DILocation(line: 15, column: 1, scope: !44)
!44 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 2)
!45 = !DILocation(line: 15, column: 1, scope: !46)
!46 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 8)
!47 = !DILocation(line: 15, column: 1, scope: !48)
!48 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 4)
!49 = !DILocation(line: 15, column: 1, scope: !50)
!50 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 6)
!51 = distinct !{!51, !42, !42}
!52 = !DILocation(line: 15, column: 1, scope: !53)
!53 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 10)
!54 = !DILocation(line: 15, column: 1, scope: !55)
!55 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 12)
!56 = distinct !{!56, !42, !42}
!57 = !DILocation(line: 15, column: 1, scope: !58)
!58 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 14)
!59 = distinct !DISubprogram(name: "loop_count_zero", scope: !1, file: !1, line: 16, type: !4, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!60 = !DILocation(line: 16, column: 1, scope: !59)
!61 = !DILocation(line: 16, column: 1, scope: !62)
!62 = !DILexicalBlockFile(scope: !59, file: !1, discriminator: 2)
!63 = !DILocation(line: 16, column: 1, scope: !64)
!64 = !DILexicalBlockFile(scope: !59, file: !1, discriminator: 8)
!65 = !DILocation(line: 16, column: 1, scope: !66)
!66 = !DILexicalBlockFile(scope: !59, file: !1, discriminator: 4)
!67 = !DILocation(line: 16, column: 1, scope: !68)
!68 = !DILexicalBlockFile(scope: !59, file: !1, discriminator: 6)
!69 = distinct !{!69, !60, !60}
!70 = !DILocation(line: 16, column: 1, scope: !71)
!71 = !DILexicalBlockFile(scope: !59, file: !1, discriminator: 10)
!72 = !DILocation(line: 16, column: 1, scope: !73)
!73 = !DILexicalBlockFile(scope: !59, file: !1, discriminator: 12)
!74 = distinct !{!74, !60, !60}
!75 = !DILocation(line: 16, column: 1, scope: !76)
!76 = !DILexicalBlockFile(scope: !59, file: !1, discriminator: 14)
!77 = distinct !DISubprogram(name: "loop_count_one_or_two", scope: !1, file: !1, line: 17, type: !4, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!78 = !DILocation(line: 17, column: 1, scope: !77)
!79 = !DILocation(line: 17, column: 1, scope: !80)
!80 = !DILexicalBlockFile(scope: !77, file: !1, discriminator: 2)
!81 = !DILocation(line: 17, column: 1, scope: !82)
!82 = !DILexicalBlockFile(scope: !77, file: !1, discriminator: 8)
!83 = !DILocation(line: 17, column: 1, scope: !84)
!84 = !DILexicalBlockFile(scope: !77, file: !1, discriminator: 4)
!85 = !DILocation(line: 17, column: 1, scope: !86)
!86 = !DILexicalBlockFile(scope: !77, file: !1, discriminator: 6)
!87 = distinct !{!87, !78, !78}
!88 = !DILocation(line: 17, column: 1, scope: !89)
!89 = !DILexicalBlockFile(scope: !77, file: !1, discriminator: 10)
!90 = !DILocation(line: 17, column: 1, scope: !91)
!91 = !DILexicalBlockFile(scope: !77, file: !1, discriminator: 12)
!92 = distinct !{!92, !78, !78}
!93 = !DILocation(line: 17, column: 1, scope: !94)
!94 = !DILexicalBlockFile(scope: !77, file: !1, discriminator: 14)
