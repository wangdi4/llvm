; RUN: opt < %s -S -O2 -intel-opt-report=high | FileCheck %s

; Verify that Loop OptReport was attached from loop(s) to the function.
; CHECK: define {{.*}} @doit() {{.*}} !intel.optreport.rootnode !{{[0-9]+}}

; CMPLRLLVM-7153: doit() must have 'void' return type.

; In incorrect case the type had self-reference:
;   !17 = distinct !DISubprogram(name: "doit", scope: !3, file: !3, line: 11, type: !18, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !20)
;   !18 = !DISubroutineType(types: !19)
;   !19 = distinct !{!19}

; CHECK: !DISubprogram(name: "doit", {{.*}}, type: ![[TYPEMD:[0-9]+]]
; CHECK: ![[TYPEMD]] = !DISubroutineType(types: ![[TYPESMD:[0-9]+]])
; CHECK: ![[TYPESMD]] = !{null}

; ModuleID = 'revp.c'
source_filename = "revp.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@aa = internal unnamed_addr global [6 x [5 x [4 x i32]]] zeroinitializer, align 16, !dbg !0
@.str = private unnamed_addr constant [33 x i8] c"failed %d@%d,%d,%d != %d revp.c\0A\00", align 1
@str = private unnamed_addr constant [14 x i8] c"passed revp.c\00", align 1

; Function Attrs: noinline norecurse nounwind uwtable writeonly
define dso_local void @doit() local_unnamed_addr #0 !dbg !17 {
entry:
  call void @llvm.dbg.value(metadata i32 0, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 5, metadata !21, metadata !DIExpression()), !dbg !26
  br label %for.body3.lr.ph, !dbg !27

for.body3.lr.ph:                                  ; preds = %for.inc15, %entry
  %indvars.iv50 = phi i64 [ 5, %entry ], [ %indvars.iv.next51, %for.inc15 ]
  %indvars.iv46 = phi i64 [ 4, %entry ], [ %indvars.iv.next47, %for.inc15 ]
  %indvars.iv = phi i64 [ 3, %entry ], [ %indvars.iv.next, %for.inc15 ]
  %index.041 = phi i32 [ 0, %entry ], [ %index.2.lcssa, %for.inc15 ]
  call void @llvm.dbg.value(metadata i32 %index.041, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i64 %indvars.iv50, metadata !21, metadata !DIExpression()), !dbg !26
  %indvars.iv.next51 = add nsw i64 %indvars.iv50, -1, !dbg !29
  call void @llvm.dbg.value(metadata i32 %index.041, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 undef, metadata !22, metadata !DIExpression(DW_OP_constu, 1, DW_OP_minus, DW_OP_stack_value)), !dbg !31
  %0 = trunc i64 %indvars.iv50 to i32, !dbg !32
  %1 = trunc i64 %indvars.iv.next51 to i32, !dbg !32
  br label %for.body3, !dbg !32

for.cond1.loopexit:                               ; preds = %for.body7, %for.body3
  %index.2.lcssa = phi i32 [ %index.136, %for.body3 ], [ %inc, %for.body7 ], !dbg !34
  %indvars.iv.next49 = add nsw i64 %indvars.iv48, -1, !dbg !35
  %b.0 = add nsw i32 %b.037, -1, !dbg !35
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 %b.0, metadata !22, metadata !DIExpression()), !dbg !31
  %cmp2 = icmp sgt i64 %indvars.iv.next49, 0, !dbg !36
  %indvars.iv.next43 = add nsw i64 %indvars.iv42, -1, !dbg !32
  br i1 %cmp2, label %for.body3, label %for.inc15, !dbg !32

for.body3:                                        ; preds = %for.body3.lr.ph, %for.cond1.loopexit
  %indvars.iv48 = phi i64 [ %indvars.iv46, %for.body3.lr.ph ], [ %indvars.iv.next49, %for.cond1.loopexit ]
  %indvars.iv42 = phi i64 [ %indvars.iv, %for.body3.lr.ph ], [ %indvars.iv.next43, %for.cond1.loopexit ]
  %b.037 = phi i32 [ %1, %for.body3.lr.ph ], [ %b.0, %for.cond1.loopexit ]
  %index.136 = phi i32 [ %index.041, %for.body3.lr.ph ], [ %index.2.lcssa, %for.cond1.loopexit ]
  %b.0.in35 = phi i32 [ %0, %for.body3.lr.ph ], [ %b.037, %for.cond1.loopexit ]
  call void @llvm.dbg.value(metadata i32 %index.136, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 %b.0.in35, metadata !23, metadata !DIExpression(DW_OP_constu, 2, DW_OP_minus, DW_OP_stack_value)), !dbg !38
  call void @llvm.dbg.value(metadata i32 %index.136, metadata !24, metadata !DIExpression()), !dbg !25
  %cmp630 = icmp sgt i32 %b.0.in35, 1, !dbg !39
  br i1 %cmp630, label %for.body7, label %for.cond1.loopexit, !dbg !43, !llvm.loop !44

for.body7:                                        ; preds = %for.body3, %for.body7
  %indvars.iv44 = phi i64 [ %indvars.iv.next45, %for.body7 ], [ %indvars.iv42, %for.body3 ]
  %index.232 = phi i32 [ %inc, %for.body7 ], [ %index.136, %for.body3 ]
  call void @llvm.dbg.value(metadata i32 %index.232, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i64 %indvars.iv44, metadata !23, metadata !DIExpression()), !dbg !38
  %inc = add nsw i32 %index.232, 1, !dbg !46
  %arrayidx11 = getelementptr inbounds [6 x [5 x [4 x i32]]], [6 x [5 x [4 x i32]]]* @aa, i64 0, i64 %indvars.iv50, i64 %indvars.iv48, i64 %indvars.iv44, !dbg !47, !intel-tbaa !48
  store i32 %index.232, i32* %arrayidx11, align 4, !dbg !55, !tbaa !48
  %indvars.iv.next45 = add nsw i64 %indvars.iv44, -1, !dbg !56
  call void @llvm.dbg.value(metadata i32 %inc, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 undef, metadata !23, metadata !DIExpression(DW_OP_constu, 1, DW_OP_minus, DW_OP_stack_value)), !dbg !38
  %cmp6 = icmp sgt i64 %indvars.iv44, 0, !dbg !39
  br i1 %cmp6, label %for.body7, label %for.cond1.loopexit, !dbg !43, !llvm.loop !57

for.inc15:                                        ; preds = %for.cond1.loopexit
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 undef, metadata !21, metadata !DIExpression(DW_OP_constu, 1, DW_OP_minus, DW_OP_stack_value)), !dbg !26
  %cmp = icmp ugt i64 %indvars.iv.next51, 1, !dbg !59
  %indvars.iv.next = add nsw i64 %indvars.iv, -1, !dbg !27
  %indvars.iv.next47 = add nsw i64 %indvars.iv46, -1, !dbg !27
  br i1 %cmp, label %for.body3.lr.ph, label %for.end17, !dbg !27, !llvm.loop !60

for.end17:                                        ; preds = %for.inc15
  ret void, !dbg !62
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 !dbg !63 {
entry:
  call void @llvm.dbg.value(metadata i32 0, metadata !70, metadata !DIExpression()), !dbg !71
  tail call void @doit(), !dbg !72
  call void @llvm.dbg.value(metadata i32 5, metadata !67, metadata !DIExpression()), !dbg !73
  call void @llvm.dbg.value(metadata i32 0, metadata !70, metadata !DIExpression()), !dbg !71
  br label %for.body3.lr.ph, !dbg !74

for.body3.lr.ph:                                  ; preds = %for.inc22, %entry
  %indvars.iv77 = phi i64 [ 5, %entry ], [ %indvars.iv.next78, %for.inc22 ]
  %indvars.iv73 = phi i64 [ 4, %entry ], [ %indvars.iv.next74, %for.inc22 ]
  %indvars.iv = phi i64 [ 3, %entry ], [ %indvars.iv.next, %for.inc22 ]
  %index.063 = phi i32 [ 0, %entry ], [ %index.2.lcssa, %for.inc22 ]
  call void @llvm.dbg.value(metadata i32 %index.063, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i64 %indvars.iv77, metadata !67, metadata !DIExpression()), !dbg !73
  %indvars.iv.next78 = add nsw i64 %indvars.iv77, -1, !dbg !76
  call void @llvm.dbg.value(metadata i32 %index.063, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i32 undef, metadata !68, metadata !DIExpression(DW_OP_constu, 1, DW_OP_minus, DW_OP_stack_value)), !dbg !78
  %0 = trunc i64 %indvars.iv77 to i32, !dbg !79
  %1 = trunc i64 %indvars.iv.next78 to i32, !dbg !79
  br label %for.body3, !dbg !79

for.cond1.loopexit:                               ; preds = %if.end, %for.body3
  %index.2.lcssa = phi i32 [ %index.157, %for.body3 ], [ %inc, %if.end ], !dbg !81
  %indvars.iv.next76 = add nsw i64 %indvars.iv75, -1, !dbg !82
  %b.0 = add nsw i32 %b.058, -1, !dbg !82
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i32 %b.0, metadata !68, metadata !DIExpression()), !dbg !78
  %cmp2 = icmp sgt i64 %indvars.iv.next76, 0, !dbg !83
  %indvars.iv.next70 = add nsw i64 %indvars.iv69, -1, !dbg !79
  br i1 %cmp2, label %for.body3, label %for.inc22, !dbg !79

for.body3:                                        ; preds = %for.body3.lr.ph, %for.cond1.loopexit
  %indvars.iv75 = phi i64 [ %indvars.iv73, %for.body3.lr.ph ], [ %indvars.iv.next76, %for.cond1.loopexit ]
  %indvars.iv69 = phi i64 [ %indvars.iv, %for.body3.lr.ph ], [ %indvars.iv.next70, %for.cond1.loopexit ]
  %b.058 = phi i32 [ %1, %for.body3.lr.ph ], [ %b.0, %for.cond1.loopexit ]
  %index.157 = phi i32 [ %index.063, %for.body3.lr.ph ], [ %index.2.lcssa, %for.cond1.loopexit ]
  %b.0.in56 = phi i32 [ %0, %for.body3.lr.ph ], [ %b.058, %for.cond1.loopexit ]
  call void @llvm.dbg.value(metadata i32 %index.157, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i32 %b.0.in56, metadata !69, metadata !DIExpression(DW_OP_constu, 2, DW_OP_minus, DW_OP_stack_value)), !dbg !85
  call void @llvm.dbg.value(metadata i32 %index.157, metadata !70, metadata !DIExpression()), !dbg !71
  %cmp651 = icmp sgt i32 %b.0.in56, 1, !dbg !86
  br i1 %cmp651, label %for.body7, label %for.cond1.loopexit, !dbg !89, !llvm.loop !90

for.body7:                                        ; preds = %for.body3, %if.end
  %indvars.iv71 = phi i64 [ %indvars.iv.next72, %if.end ], [ %indvars.iv69, %for.body3 ]
  %index.253 = phi i32 [ %inc, %if.end ], [ %index.157, %for.body3 ]
  call void @llvm.dbg.value(metadata i32 %index.253, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i64 %indvars.iv71, metadata !69, metadata !DIExpression()), !dbg !85
  %arrayidx11 = getelementptr inbounds [6 x [5 x [4 x i32]]], [6 x [5 x [4 x i32]]]* @aa, i64 0, i64 %indvars.iv77, i64 %indvars.iv75, i64 %indvars.iv71, !dbg !92, !intel-tbaa !48
  %2 = load i32, i32* %arrayidx11, align 4, !dbg !92, !tbaa !48
  %cmp12 = icmp eq i32 %2, %index.253, !dbg !95
  br i1 %cmp12, label %if.end, label %if.then, !dbg !96

if.then:                                          ; preds = %for.body7
  call void @llvm.dbg.value(metadata i64 %indvars.iv77, metadata !67, metadata !DIExpression()), !dbg !73
  call void @llvm.dbg.value(metadata i64 %indvars.iv71, metadata !69, metadata !DIExpression()), !dbg !85
  call void @llvm.dbg.value(metadata i32 %index.253, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i64 %indvars.iv77, metadata !67, metadata !DIExpression()), !dbg !73
  call void @llvm.dbg.value(metadata i64 %indvars.iv71, metadata !69, metadata !DIExpression()), !dbg !85
  call void @llvm.dbg.value(metadata i32 %index.253, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i64 %indvars.iv77, metadata !67, metadata !DIExpression()), !dbg !73
  call void @llvm.dbg.value(metadata i64 %indvars.iv71, metadata !69, metadata !DIExpression()), !dbg !85
  call void @llvm.dbg.value(metadata i32 %index.253, metadata !70, metadata !DIExpression()), !dbg !71
  %3 = trunc i64 %indvars.iv77 to i32, !dbg !73
  %4 = trunc i64 %indvars.iv75 to i32, !dbg !73
  %5 = trunc i64 %indvars.iv71 to i32, !dbg !73
  call void @llvm.dbg.value(metadata i32 %3, metadata !67, metadata !DIExpression()), !dbg !73
  call void @llvm.dbg.value(metadata i32 %index.253, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i32 %5, metadata !69, metadata !DIExpression()), !dbg !85
  call void @llvm.dbg.value(metadata i32 %3, metadata !67, metadata !DIExpression()), !dbg !73
  call void @llvm.dbg.value(metadata i32 %5, metadata !69, metadata !DIExpression()), !dbg !85
  call void @llvm.dbg.value(metadata i32 %index.253, metadata !70, metadata !DIExpression()), !dbg !71
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str, i64 0, i64 0), i32 %2, i32 %3, i32 %4, i32 %5, i32 %index.253), !dbg !97
  tail call void @exit(i32 1) #6, !dbg !99
  unreachable, !dbg !99

if.end:                                           ; preds = %for.body7
  %inc = add nsw i32 %index.253, 1, !dbg !100
  %indvars.iv.next72 = add nsw i64 %indvars.iv71, -1, !dbg !101
  call void @llvm.dbg.value(metadata i32 %inc, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i32 undef, metadata !69, metadata !DIExpression(DW_OP_constu, 1, DW_OP_minus, DW_OP_stack_value)), !dbg !85
  %cmp6 = icmp sgt i64 %indvars.iv71, 0, !dbg !86
  br i1 %cmp6, label %for.body7, label %for.cond1.loopexit, !dbg !89, !llvm.loop !102

for.inc22:                                        ; preds = %for.cond1.loopexit
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i32 %index.2.lcssa, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.value(metadata i32 undef, metadata !67, metadata !DIExpression(DW_OP_constu, 1, DW_OP_minus, DW_OP_stack_value)), !dbg !73
  %cmp = icmp ugt i64 %indvars.iv.next78, 1, !dbg !104
  %indvars.iv.next = add nsw i64 %indvars.iv, -1, !dbg !74
  %indvars.iv.next74 = add nsw i64 %indvars.iv73, -1, !dbg !74
  br i1 %cmp, label %for.body3.lr.ph, label %for.end24, !dbg !74, !llvm.loop !105

for.end24:                                        ; preds = %for.inc22
  %puts = tail call i32 @puts(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @str, i64 0, i64 0)), !dbg !107
  ret i32 0, !dbg !108
}

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #2

; Function Attrs: noreturn nounwind
declare dso_local void @exit(i32) local_unnamed_addr #3

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #4

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #5

attributes #0 = { noinline norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readnone speculatable }
attributes #5 = { nounwind }
attributes #6 = { noreturn nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!12, !13, !14}
!llvm.dbg.intel.emit_class_debug_always = !{!15}
!llvm.ident = !{!16}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "aa", scope: !2, file: !3, line: 9, type: !6, isLocal: true, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C89, file: !3, producer: "clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang b97cd1e0ccdf66edc0b2a4aadd0de0874ecd119f) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 5edd51f7507f0fc3d199aae5dd0a15917e51c59e)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: None)
!3 = !DIFile(filename: "revp.c", directory: "/icsmnt/scel67_iusers/vzakhari/workspaces/xmain03/test/tc")
!4 = !{}
!5 = !{!0}
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 3840, elements: !8)
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{!9, !10, !11}
!9 = !DISubrange(count: 6)
!10 = !DISubrange(count: 5)
!11 = !DISubrange(count: 4)
!12 = !{i32 2, !"Dwarf Version", i32 4}
!13 = !{i32 2, !"Debug Info Version", i32 3}
!14 = !{i32 1, !"wchar_size", i32 4}
!15 = !{!"true"}
!16 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang b97cd1e0ccdf66edc0b2a4aadd0de0874ecd119f) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 5edd51f7507f0fc3d199aae5dd0a15917e51c59e)"}
!17 = distinct !DISubprogram(name: "doit", scope: !3, file: !3, line: 11, type: !18, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !20)
!18 = !DISubroutineType(types: !19)
!19 = !{null}
!20 = !{!21, !22, !23, !24}
!21 = !DILocalVariable(name: "a", scope: !17, file: !3, line: 13, type: !7)
!22 = !DILocalVariable(name: "b", scope: !17, file: !3, line: 13, type: !7)
!23 = !DILocalVariable(name: "c", scope: !17, file: !3, line: 13, type: !7)
!24 = !DILocalVariable(name: "index", scope: !17, file: !3, line: 14, type: !7)
!25 = !DILocation(line: 14, column: 7, scope: !17)
!26 = !DILocation(line: 13, column: 7, scope: !17)
!27 = !DILocation(line: 15, column: 3, scope: !28)
!28 = distinct !DILexicalBlock(scope: !17, file: !3, line: 15, column: 3)
!29 = !DILocation(line: 15, column: 22, scope: !30)
!30 = distinct !DILexicalBlock(scope: !28, file: !3, line: 15, column: 3)
!31 = !DILocation(line: 13, column: 10, scope: !17)
!32 = !DILocation(line: 16, column: 5, scope: !33)
!33 = distinct !DILexicalBlock(scope: !30, file: !3, line: 16, column: 5)
!34 = !DILocation(line: 0, scope: !17)
!35 = !DILocation(line: 0, scope: !33)
!36 = !DILocation(line: 16, column: 25, scope: !37)
!37 = distinct !DILexicalBlock(scope: !33, file: !3, line: 16, column: 5)
!38 = !DILocation(line: 13, column: 13, scope: !17)
!39 = !DILocation(line: 19, column: 27, scope: !40)
!40 = distinct !DILexicalBlock(scope: !41, file: !3, line: 19, column: 7)
!41 = distinct !DILexicalBlock(scope: !42, file: !3, line: 19, column: 7)
!42 = distinct !DILexicalBlock(scope: !37, file: !3, line: 17, column: 1)
!43 = !DILocation(line: 19, column: 7, scope: !41)
!44 = distinct !{!44, !32, !45}
!45 = !DILocation(line: 21, column: 1, scope: !33)
!46 = !DILocation(line: 20, column: 28, scope: !40)
!47 = !DILocation(line: 20, column: 9, scope: !40)
!48 = !{!49, !52, i64 0}
!49 = !{!"array@_ZTSA6_A5_A4_i", !50, i64 0}
!50 = !{!"array@_ZTSA5_A4_i", !51, i64 0}
!51 = !{!"array@_ZTSA4_i", !52, i64 0}
!52 = !{!"int", !53, i64 0}
!53 = !{!"omnipotent char", !54, i64 0}
!54 = !{!"Simple C/C++ TBAA"}
!55 = !DILocation(line: 20, column: 21, scope: !40)
!56 = !DILocation(line: 19, column: 33, scope: !40)
!57 = distinct !{!57, !43, !58}
!58 = !DILocation(line: 20, column: 28, scope: !41)
!59 = !DILocation(line: 15, column: 16, scope: !30)
!60 = distinct !{!60, !27, !61}
!61 = !DILocation(line: 21, column: 1, scope: !28)
!62 = !DILocation(line: 22, column: 1, scope: !17)
!63 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 24, type: !64, scopeLine: 24, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !66)
!64 = !DISubroutineType(types: !65)
!65 = !{!7}
!66 = !{!67, !68, !69, !70}
!67 = !DILocalVariable(name: "a", scope: !63, file: !3, line: 25, type: !7)
!68 = !DILocalVariable(name: "b", scope: !63, file: !3, line: 25, type: !7)
!69 = !DILocalVariable(name: "c", scope: !63, file: !3, line: 25, type: !7)
!70 = !DILocalVariable(name: "index", scope: !63, file: !3, line: 26, type: !7)
!71 = !DILocation(line: 26, column: 7, scope: !63)
!72 = !DILocation(line: 28, column: 3, scope: !63)
!73 = !DILocation(line: 25, column: 7, scope: !63)
!74 = !DILocation(line: 29, column: 3, scope: !75)
!75 = distinct !DILexicalBlock(scope: !63, file: !3, line: 29, column: 3)
!76 = !DILocation(line: 29, column: 22, scope: !77)
!77 = distinct !DILexicalBlock(scope: !75, file: !3, line: 29, column: 3)
!78 = !DILocation(line: 25, column: 10, scope: !63)
!79 = !DILocation(line: 30, column: 3, scope: !80)
!80 = distinct !DILexicalBlock(scope: !77, file: !3, line: 30, column: 3)
!81 = !DILocation(line: 0, scope: !63)
!82 = !DILocation(line: 0, scope: !80)
!83 = !DILocation(line: 30, column: 23, scope: !84)
!84 = distinct !DILexicalBlock(scope: !80, file: !3, line: 30, column: 3)
!85 = !DILocation(line: 25, column: 13, scope: !63)
!86 = !DILocation(line: 31, column: 23, scope: !87)
!87 = distinct !DILexicalBlock(scope: !88, file: !3, line: 31, column: 3)
!88 = distinct !DILexicalBlock(scope: !84, file: !3, line: 31, column: 3)
!89 = !DILocation(line: 31, column: 3, scope: !88)
!90 = distinct !{!90, !79, !91}
!91 = !DILocation(line: 37, column: 3, scope: !80)
!92 = !DILocation(line: 32, column: 9, scope: !93)
!93 = distinct !DILexicalBlock(scope: !94, file: !3, line: 32, column: 9)
!94 = distinct !DILexicalBlock(scope: !87, file: !3, line: 31, column: 33)
!95 = !DILocation(line: 32, column: 21, scope: !93)
!96 = !DILocation(line: 32, column: 9, scope: !94)
!97 = !DILocation(line: 33, column: 7, scope: !98)
!98 = distinct !DILexicalBlock(scope: !93, file: !3, line: 32, column: 31)
!99 = !DILocation(line: 34, column: 7, scope: !98)
!100 = !DILocation(line: 36, column: 10, scope: !94)
!101 = !DILocation(line: 31, column: 29, scope: !87)
!102 = distinct !{!102, !89, !103}
!103 = !DILocation(line: 37, column: 3, scope: !88)
!104 = !DILocation(line: 29, column: 16, scope: !77)
!105 = distinct !{!105, !74, !106}
!106 = !DILocation(line: 37, column: 3, scope: !75)
!107 = !DILocation(line: 38, column: 3, scope: !63)
!108 = !DILocation(line: 40, column: 3, scope: !63)
