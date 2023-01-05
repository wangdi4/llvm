; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -S -ip-manyreccalls-cloning-min-rec-callsites=2 -passes='module(ip-cloning)' -ip-manyreccalls-splitting=false 2>&1 | FileCheck %s

; Check that foo is selected for cloning as a "many recursive calls" cloning
; candidate.

; Check also that all calls, including the calls to the clone, have dbg info.

; This is the same test as ip_cloning_mrc07-opaque-ptr.ll, but checks for IR
; without requiring asserts.

; Check changes to the IR

%struct.MYSTRUCT = type { i32, i32 }

@myglobal = dso_local global i32 45, align 4, !dbg !0
@cache = dso_local global %struct.MYSTRUCT zeroinitializer, align 4

define dso_local i32 @goo(ptr %cacheptr) !dbg !18 {
entry:
  %i = load i32, ptr @myglobal, align 4
  %tobool = icmp ne i32 %i, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %field2 = getelementptr inbounds %struct.MYSTRUCT, ptr %cacheptr, i32 0, i32 1
  %i1 = load i32, ptr %field2, align 4
  %call = call i32 @foo(i32 1, i32 1, i32 %i1), !dbg !25
  br label %return

if.end:                                           ; preds = %entry
  %i2 = load i32, ptr @myglobal, align 4
  %call1 = call i32 @foo(i32 0, i32 %i2, i32 0), !dbg !27
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ %call, %if.then ], [ %call1, %if.end ]
  ret i32 %retval.0
}

; Check that a test is inserted for the switch variable, which results in
; either a call to foo or the clone foo.1.

; CHECK: define dso_local i32 @goo{{.*}}
; CHECK: [[V1:%[A-Za-z0-9.]+]] = getelementptr inbounds %struct.MYSTRUCT, ptr %cacheptr, i32 0, i32 1
; CHECK: [[V2:%[A-Za-z0-9.]+]] = load i32, ptr [[V1]], align 4
; CHECK: [[V3:%[A-Za-z0-9.]+]] = icmp eq i32 [[V2]], 0
; CHECK: br i1 [[V3]], label %[[V4:[A-Za-z0-9.]+]], label %[[V5:[A-Za-z0-9.]+]]
; CHECK: [[V5]]:
; CHECK: {{.*}}call i32 @foo(i32 1, i32 1, i32 %i1), !dbg
; CHECK: br label %[[V6:[A-Za-z0-9.]+]]
; CHECK: [[V4]]:
; CHECK: {{.*}}call i32 @foo.1(i32 1, i32 1, i32 0), !dbg
; CHECK: br label %[[V6]]
; CHECK: [[V6]]:

define internal i32 @foo(i32 %arg0, i32 %arg1, i32 %arg2) !dbg !28 {
entry:
  %tobool = icmp ne i32 %arg0, 0
  br i1 %tobool, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %tobool1 = icmp ne i32 %arg1, 0
  br i1 %tobool1, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  br label %return

if.end:                                           ; preds = %land.lhs.true, %entry
  switch i32 %arg2, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb
  ]

sw.default:                                       ; preds = %if.end
  %call = call i32 @foo(i32 1, i32 1, i32 %arg2), !dbg !41
  br label %return

sw.bb:                                            ; preds = %if.end, %if.end
  %sub2 = sub nsw i32 %arg2, 2
  %call3 = call i32 @foo(i32 0, i32 1, i32 %sub2), !dbg !42
  br label %return

return:                                           ; preds = %sw.bb, %sw.default, %if.then
  %call4 = call i32 @foo(i32 %arg2, i32 1, i32 0), !dbg !43
  ret i32 %call4
}

; CHECK: define internal i32 @foo{{.*}}
; CHECK: entry:
; CHECK: [[V1:%[A-Za-z0-9.]+]] = icmp eq i32 %arg0, 1
; CHECK: [[V2:%[A-Za-z0-9.]+]] = icmp eq i32 %arg1, 1
; CHECK: [[V3:%[A-Za-z0-9.]+]] = and i1 [[V1]], [[V2]]
; CHECK: [[V4:%[A-Za-z0-9.]+]] = icmp eq i32 %arg2, 0
; CHECK: [[V5:%[A-Za-z0-9.]+]] = and i1 [[V3]], [[V4]]
; CHECK: br i1 [[V5]], label %[[V6:[0-9]+]], label %[[V7:[0-9]+]]
; CHECK: [[V6]]:
; CHECK: [[V7:%[A-Za-z0-9.]+]] = call i32 @foo.1{{.*}}, !dbg
; CHECK:  ret i32 [[V7]]

; Check that the clone foo.1 is generated

; CHECK: define internal i32 @foo.1{{.*}}
; CHECK: entry:

; Check that the IF-tests get their constant values of 1.
; CHECK: {{.*}} = icmp ne i32 1, 0
; CHECK: {{.*}} = icmp ne i32 1, 0

; Check that the SWITCH-test gets its constant value of 0.
; CHECK: switch i32 0, label %sw.default
; CHECK: sw.default:

; Check that the recursive call in the switch is transformed into a call to
; the clone.
; CHECK: {{.*}} call i32 @foo.1(i32 1, i32 1, i32 0), !dbg
; CHECK: sw.bb:

; Check that the recursive call outside the default switch case is not
; transformed.
; CHECK: [[V1:%[A-Za-z0-9.]+]] = sub nsw i32 0, 2
; CHECK: {{.*}} = call i32 @foo(i32 0, i32 1, i32 [[V1]]), !dbg
; CHECK: br label %[[V2:[A-Za-z0-9]+]]
; CHECK: [[V2]]:

; Check that the recursive call outside the SWITCH-test is transformed into
; a conditional call of either the original or the clone.
; CHECK: [[V3:%[A-Za-z0-9.]+]] = icmp eq i32 0, 1
; CHECK: br i1 [[V3]], label %[[V4:[A-Za-z0-9.]+]], label %[[V5:[A-Za-z0-9.]+]]
; CHECK: [[V5]]:
; CHECK: {{.*}} = call i32 @foo(i32 0, i32 1, i32 0), !dbg
; CHECK: br label %[[V6:[0-9]+]]
; CHECK: [[V4]]:
; CHECK: {{.*}} call i32 @foo.1(i32 1, i32 1, i32 0), !dbg
; CHECK: br label %[[V6]]
; CHECK: [[V6]]:

define dso_local i32 @main() !dbg !44 {
entry:
  %call = call i32 @goo(ptr @cache), !dbg !47
  ret i32 %call
}

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!14, !15, !16}
!llvm.ident = !{!17}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "myglobal", scope: !2, file: !3, line: 7, type: !12, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: None)
!3 = !DIFile(filename: "sm.c", directory: "/export/iusers/rcox2/rgCC")
!4 = !{}
!5 = !{!0, !6}
!6 = !DIGlobalVariableExpression(var: !7, expr: !DIExpression())
!7 = distinct !DIGlobalVariable(name: "cache", scope: !2, file: !3, line: 6, type: !8, isLocal: false, isDefinition: true)
!8 = !DIDerivedType(tag: DW_TAG_typedef, name: "MYSTRUCT", file: !3, line: 4, baseType: !9)
!9 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !3, line: 1, size: 64, elements: !10)
!10 = !{!11, !13}
!11 = !DIDerivedType(tag: DW_TAG_member, name: "int1", scope: !9, file: !3, line: 2, baseType: !12, size: 32)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DIDerivedType(tag: DW_TAG_member, name: "int2", scope: !9, file: !3, line: 3, baseType: !12, size: 32, offset: 32)
!14 = !{i32 2, !"Dwarf Version", i32 4}
!15 = !{i32 2, !"Debug Info Version", i32 3}
!16 = !{i32 1, !"wchar_size", i32 4}
!17 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!18 = distinct !DISubprogram(name: "goo", scope: !3, file: !3, line: 37, type: !19, scopeLine: 37, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !22)
!19 = !DISubroutineType(types: !20)
!20 = !{!12, !21}
!21 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !8, size: 64)
!22 = !{!23, !24}
!23 = !DILocalVariable(name: "cacheptr", arg: 1, scope: !18, file: !3, line: 37, type: !21)
!24 = !DILocalVariable(name: "myint", scope: !18, file: !3, line: 38, type: !12)
!25 = !DILocation(line: 40, column: 13, scope: !26)
!26 = distinct !DILexicalBlock(scope: !18, file: !3, line: 39, column: 7)
!27 = !DILocation(line: 43, column: 3, scope: !18)
!28 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 9, type: !29, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !31)
!29 = !DISubroutineType(types: !30)
!30 = !{!12, !12, !12, !12}
!31 = !{!32, !33, !34, !35, !36, !37, !38, !39, !40}
!32 = !DILocalVariable(name: "arg0", arg: 1, scope: !28, file: !3, line: 9, type: !12)
!33 = !DILocalVariable(name: "arg1", arg: 2, scope: !28, file: !3, line: 9, type: !12)
!34 = !DILocalVariable(name: "arg2", arg: 3, scope: !28, file: !3, line: 9, type: !12)
!35 = !DILabel(scope: !28, name: "landtrue", file: !3, line: 14)
!36 = !DILabel(scope: !28, name: "ifthen", file: !3, line: 19)
!37 = !DILabel(scope: !28, name: "ifend", file: !3, line: 21)
!38 = !DILabel(scope: !28, name: "mydefault", file: !3, line: 27)
!39 = !DILabel(scope: !28, name: "bb", file: !3, line: 30)
!40 = !DILabel(scope: !28, name: "myreturn", file: !3, line: 33)
!41 = !DILocation(line: 28, column: 3, scope: !28)
!42 = !DILocation(line: 31, column: 3, scope: !28)
!43 = !DILocation(line: 34, column: 10, scope: !28)
!44 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 46, type: !45, scopeLine: 46, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !4)
!45 = !DISubroutineType(types: !46)
!46 = !{!12}
!47 = !DILocation(line: 47, column: 10, scope: !44)
; end INTEL_FEATURE_SW_ADVANCED
