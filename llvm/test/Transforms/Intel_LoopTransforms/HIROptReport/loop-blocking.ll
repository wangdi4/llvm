; Check that proper optreport format and metadata are emitted for Completely Unrolled loop.

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-blocking -hir-cg -intel-loop-optreport=low -intel-ir-optreport-emitter -simplifycfg 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT 

; OPTREPORT:      LOOP BEGIN at m.c (8, 3)

; OPTREPORT:         LOOP BEGIN at m.c (9, 5)

; OPTREPORT:            LOOP BEGIN at m.c (10, 7)

; OPTREPORT:                 LOOP BEGIN at m.c (8, 3)
; OPTREPORT:                 Remark: Loop is blocked

; OPTREPORT:                    LOOP BEGIN at m.c (9, 5)
; OPTREPORT:                   Remark: Loop is blocked

; OPTREPORT:                        LOOP BEGIN at m.c (10, 7)
; OPTREPORT:                       Remark: Loop is blocked
; OPTREPORT:                   LOOP END
; OPTREPORT:               LOOP END
; OPTREPORT:            LOOP END
; OPTREPORT:        LOOP END
; OPTREPORT:    LOOP END
; OPTREPORT: LOOP END

; RUN : opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-blocking -hir-cg -intel-loop-optreport=low -simplifycfg < %s -S | FileCheck %s

;CHECK:  {{![0-9]+}} = distinct !{!"llvm.loop.optreport", [[M3:!.*]]}
;CHECK:  [[M3]] = distinct !{!"intel.loop.optreport", [[M4:!.*]]
;CHECK:  {{![0-9]+}} = !{!"intel.optreport.remarks", [[M8:!.*]]}
;CHECK:  [[M8]] = !{!"intel.optreport.remark", !"Loop is blocked"}

;Module Before HIR
; ModuleID = 'm.c'
source_filename = "m.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16, !dbg !0
@b = common dso_local local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16, !dbg !12
@c = common dso_local local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16, !dbg !6

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @matmul() local_unnamed_addr #0 !dbg !18 {
entry:
  call void @llvm.dbg.value(metadata i32 0, metadata !24, metadata !DIExpression()), !dbg !26
  br label %for.cond1.preheader, !dbg !27

for.cond1.preheader:                              ; preds = %for.inc20, %entry
  %indvars.iv40 = phi i64 [ 0, %entry ], [ %indvars.iv.next41, %for.inc20 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv40, metadata !24, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32 0, metadata !22, metadata !DIExpression()), !dbg !26
  br label %for.cond4.preheader, !dbg !29

for.cond4.preheader:                              ; preds = %for.inc17, %for.cond1.preheader
  %indvars.iv37 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next38, %for.inc17 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv37, metadata !22, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32 0, metadata !25, metadata !DIExpression()), !dbg !26
  %arrayidx16 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @c, i64 0, i64 %indvars.iv37, i64 %indvars.iv40, !dbg !33, !intel-tbaa !39
  %arrayidx16.promoted = load double, double* %arrayidx16, align 8, !dbg !45, !tbaa !39
  br label %for.body6, !dbg !46

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ], !dbg !26
  %0 = phi double [ %arrayidx16.promoted, %for.cond4.preheader ], [ %add, %for.body6 ], !dbg !26
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !25, metadata !DIExpression()), !dbg !26
  %arrayidx8 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @a, i64 0, i64 %indvars.iv37, i64 %indvars.iv, !dbg !47, !intel-tbaa !39
  %1 = load double, double* %arrayidx8, align 8, !dbg !47, !tbaa !39
  %arrayidx12 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @b, i64 0, i64 %indvars.iv, i64 %indvars.iv40, !dbg !48, !intel-tbaa !39
  %2 = load double, double* %arrayidx12, align 8, !dbg !48, !tbaa !39
  %mul = fmul double %1, %2, !dbg !49
  %add = fadd double %0, %mul, !dbg !45
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !50
  call void @llvm.dbg.value(metadata i32 undef, metadata !25, metadata !DIExpression(DW_OP_plus_uconst, 1, DW_OP_stack_value)), !dbg !26
  %exitcond = icmp eq i64 %indvars.iv.next, 1024, !dbg !51
  br i1 %exitcond, label %for.inc17, label %for.body6, !dbg !46, !llvm.loop !52

for.inc17:                                        ; preds = %for.body6
  %add.lcssa = phi double [ %add, %for.body6 ], !dbg !45
  store double %add.lcssa, double* %arrayidx16, align 8, !dbg !45, !tbaa !39
  %indvars.iv.next38 = add nuw nsw i64 %indvars.iv37, 1, !dbg !54
  call void @llvm.dbg.value(metadata i32 undef, metadata !22, metadata !DIExpression(DW_OP_plus_uconst, 1, DW_OP_stack_value)), !dbg !26
  %exitcond39 = icmp eq i64 %indvars.iv.next38, 1024, !dbg !55
  br i1 %exitcond39, label %for.inc20, label %for.cond4.preheader, !dbg !29, !llvm.loop !56

for.inc20:                                        ; preds = %for.inc17
  %indvars.iv.next41 = add nuw nsw i64 %indvars.iv40, 1, !dbg !58
  call void @llvm.dbg.value(metadata i32 undef, metadata !24, metadata !DIExpression(DW_OP_plus_uconst, 1, DW_OP_stack_value)), !dbg !26
  %exitcond42 = icmp eq i64 %indvars.iv.next41, 1024, !dbg !59
  br i1 %exitcond42, label %for.end22, label %for.cond1.preheader, !dbg !27, !llvm.loop !60

for.end22:                                        ; preds = %for.inc20
  ret void, !dbg !62
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!14, !15, !16}
!llvm.ident = !{!17}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 2, type: !8, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang based oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: None)
!3 = !DIFile(filename: "m.c", directory: "/nfs/site/home/yoonseoc/temp/blocking-optreport")
!4 = !{}
!5 = !{!6, !0, !12}
!6 = !DIGlobalVariableExpression(var: !7, expr: !DIExpression())
!7 = distinct !DIGlobalVariable(name: "c", scope: !2, file: !3, line: 1, type: !8, isLocal: false, isDefinition: true)
!8 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 67108864, elements: !10)
!9 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!10 = !{!11, !11}
!11 = !DISubrange(count: 1024)
!12 = !DIGlobalVariableExpression(var: !13, expr: !DIExpression())
!13 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !3, line: 3, type: !8, isLocal: false, isDefinition: true)
!14 = !{i32 2, !"Dwarf Version", i32 4}
!15 = !{i32 2, !"Debug Info Version", i32 3}
!16 = !{i32 1, !"wchar_size", i32 4}
!17 = !{!"oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!18 = distinct !DISubprogram(name: "matmul", scope: !3, file: !3, line: 5, type: !19, scopeLine: 5, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !21)
!19 = !DISubroutineType(types: !20)
!20 = !{null}
!21 = !{!22, !24, !25}
!22 = !DILocalVariable(name: "i", scope: !18, file: !3, line: 6, type: !23)
!23 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!24 = !DILocalVariable(name: "j", scope: !18, file: !3, line: 6, type: !23)
!25 = !DILocalVariable(name: "k", scope: !18, file: !3, line: 6, type: !23)
!26 = !DILocation(line: 0, scope: !18)
!27 = !DILocation(line: 8, column: 3, scope: !28)
!28 = distinct !DILexicalBlock(scope: !18, file: !3, line: 8, column: 3)
!29 = !DILocation(line: 9, column: 5, scope: !30)
!30 = distinct !DILexicalBlock(scope: !31, file: !3, line: 9, column: 5)
!31 = distinct !DILexicalBlock(scope: !32, file: !3, line: 8, column: 25)
!32 = distinct !DILexicalBlock(scope: !28, file: !3, line: 8, column: 3)
!33 = !DILocation(line: 11, column: 3, scope: !34)
!34 = distinct !DILexicalBlock(scope: !35, file: !3, line: 10, column: 29)
!35 = distinct !DILexicalBlock(scope: !36, file: !3, line: 10, column: 7)
!36 = distinct !DILexicalBlock(scope: !37, file: !3, line: 10, column: 7)
!37 = distinct !DILexicalBlock(scope: !38, file: !3, line: 9, column: 27)
!38 = distinct !DILexicalBlock(scope: !30, file: !3, line: 9, column: 5)
!39 = !{!40, !42, i64 0}
!40 = !{!"array@_ZTSA1024_A1024_d", !41, i64 0}
!41 = !{!"array@_ZTSA1024_d", !42, i64 0}
!42 = !{!"double", !43, i64 0}
!43 = !{!"omnipotent char", !44, i64 0}
!44 = !{!"Simple C/C++ TBAA"}
!45 = !DILocation(line: 11, column: 11, scope: !34)
!46 = !DILocation(line: 10, column: 7, scope: !36)
!47 = !DILocation(line: 11, column: 14, scope: !34)
!48 = !DILocation(line: 11, column: 24, scope: !34)
!49 = !DILocation(line: 11, column: 22, scope: !34)
!50 = !DILocation(line: 10, column: 25, scope: !35)
!51 = !DILocation(line: 10, column: 17, scope: !35)
!52 = distinct !{!52, !46, !53}
!53 = !DILocation(line: 12, column: 7, scope: !36)
!54 = !DILocation(line: 9, column: 23, scope: !38)
!55 = !DILocation(line: 9, column: 15, scope: !38)
!56 = distinct !{!56, !29, !57}
!57 = !DILocation(line: 13, column: 5, scope: !30)
!58 = !DILocation(line: 8, column: 21, scope: !32)
!59 = !DILocation(line: 8, column: 13, scope: !32)
!60 = distinct !{!60, !27, !61}
!61 = !DILocation(line: 14, column: 3, scope: !28)
!62 = !DILocation(line: 16, column: 1, scope: !18)
