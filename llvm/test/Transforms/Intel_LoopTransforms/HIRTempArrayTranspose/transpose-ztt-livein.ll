; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir>,hir-temp-array-transpose,print<hir>" -hir-details -disable-output 2>&1 | FileCheck %s

; Check that transpose succeeds where the copy loop creates a new const-dim i1 loop
; and adds the ztt live-in temp from the non-const i2 loop. Previously verifier
; would assert due to missing live-in temp.


; HIR Before
; CHECK:  BEGIN REGION { }
;               + DO i1 = 0, zext.i32.i64(%nNumSamps) + -1, 1
;               |   + DO i2 = 0, %wide.trip.count77 + -1, 1
;               |   |   %nTempFl.0.lcssa.us = 0.000000e+00;
;               |   |
;               |   |      %nTempFl.067.us = 0.000000e+00;
;               |   |   + Ztt: if (%spec.store.select46 != 0)
;               |   |   + LiveIn symbases: 3, 10, 11, 13
; CHECK:        |   |   | <ZTT-REG> LINEAR i32 %spec.store.select46 {sb:[[ZTTLIVEIN:[0-9]+]]}
;               |   |   + DO i3 = 0, %wide.trip.count + -1, 1
;               |   |   |   %4 = (%pInput)[i3][i2];
;               |   |   |   %conv15.us = sitofp.i16.float((%4)[i1]);
;               |   |   |   %nTempFl.067.us = %nTempFl.067.us  +  %conv15.us;
;               |   |   + END LOOP
;               |   |      %nTempFl.0.lcssa.us = %nTempFl.067.us;
;               |   |
;               |   |   %1 = %nTempFl.0.lcssa.us  *  %0;
;               |   |   %conv23.us = fpext.float.double(%1);
;               |   |   %.v.us = (%1 <u 0.000000e+00) ? -5.000000e-01 : 5.000000e-01;
;               |   |   %2 = %.v.us  +  %conv23.us;
;               |   |   %nTempFx.0.us = fptosi.double.i32(%2);
;               |   |   %3 = (%pOutput)[i2];
;               |   |   (%3)[i1] = smax(-32768, smin(32767, %nTempFx.0.us));
;               |   + END LOOP
;               + END LOOP
;         END REGION

; HIR After
; CHECK:  BEGIN REGION { modified }
;               %call = @llvm.stacksave.p0();
; CHECK:        %TranspTmpArr = alloca 128 * %wide.trip.count;
;
; CHECK:        + LiveIn symbases:
; CHECK-SAME: [[ZTTLIVEIN]]
; CHECK:        + DO i64 i1 = 0, 15, 1
; CHECK:        |   + LiveIn symbases:
; CHECK-SAME: [[ZTTLIVEIN]]
; CHECK:        |   + DO i64 i2 = 0, %wide.trip.count + -1, 1
; CHECK:        |   | <ZTT-REG> LINEAR i32 %spec.store.select46 {sb:[[ZTTLIVEIN]]}

;               |   |   (%TranspTmpArr)[i1][i2] = (%pInput)[i2][i1];
;               |   + END LOOP
;               + END LOOP
;
;
; CHECK:        + DO i64 i1 = 0, zext.i32.i64(%nNumSamps) + -1, 1
; CHECK:        |   + DO i64 i2 = 0, %wide.trip.count77 + -1, 1
;               |   |   %nTempFl.0.lcssa.us = 0.000000e+00;
;               |   |
;               |   |      %nTempFl.067.us = 0.000000e+00;
; CHECK:        |   |   + DO i64 i3 = 0, %wide.trip.count + -1, 1
; CHECK:        |   |   |   %4 = (%TranspTmpArr)[i2][i3];
;               |   |   |   %conv15.us = sitofp.i16.float((%4)[i1]);
;               |   |   |   %nTempFl.067.us = %nTempFl.067.us  +  %conv15.us;
;               |   |   + END LOOP
;               |   |      %nTempFl.0.lcssa.us = %nTempFl.067.us;
;               |   |
;               |   |   %1 = %nTempFl.0.lcssa.us  *  %0;
;               |   |   %conv23.us = fpext.float.double(%1);
;               |   |   %.v.us = (%1 <u 0.000000e+00) ? -5.000000e-01 : 5.000000e-01;
;               |   |   %2 = %.v.us  +  %conv23.us;
;               |   |   %nTempFx.0.us = fptosi.double.i32(%2);
;               |   |   %3 = (%pOutput)[i2];
;               |   |   (%3)[i1] = smax(-32768, smin(32767, %nTempFx.0.us));
;               |   + END LOOP
;               + END LOOP
;
;               @llvm.stackrestore.p0(&((%call)[0]));
;         END REGION



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define dso_local void @nr5g_test_combine_iq(ptr nocapture noundef readonly %pInput, ptr nocapture noundef readonly %pOutput, i32 noundef %nNumStreams, i32 noundef %nNumPorts, i32 noundef %nNumSamps) local_unnamed_addr #0 !dbg !40 {
entry:
  call void @llvm.dbg.value(metadata ptr %pInput, metadata !54, metadata !DIExpression()), !dbg !64
  call void @llvm.dbg.value(metadata ptr %pOutput, metadata !55, metadata !DIExpression()), !dbg !64
  call void @llvm.dbg.value(metadata i32 %nNumStreams, metadata !56, metadata !DIExpression()), !dbg !64
  call void @llvm.dbg.value(metadata i32 %nNumPorts, metadata !57, metadata !DIExpression()), !dbg !64
  call void @llvm.dbg.value(metadata i32 %nNumSamps, metadata !58, metadata !DIExpression()), !dbg !64
  %spec.store.select = tail call i32 @llvm.umin.i32(i32 %nNumPorts, i32 16), !dbg !65
  call void @llvm.dbg.value(metadata i32 %spec.store.select, metadata !57, metadata !DIExpression()), !dbg !64
  %spec.store.select46 = tail call i32 @llvm.umin.i32(i32 %nNumStreams, i32 16), !dbg !66
  call void @llvm.dbg.value(metadata i32 %spec.store.select46, metadata !56, metadata !DIExpression()), !dbg !64
  call void @llvm.dbg.value(metadata i32 0, metadata !61, metadata !DIExpression()), !dbg !64
  %cmp472.not = icmp eq i32 %nNumSamps, 0, !dbg !67
  br i1 %cmp472.not, label %for.end44, label %for.cond5.preheader.lr.ph, !dbg !70

for.cond5.preheader.lr.ph:                        ; preds = %entry
  %cmp670.not = icmp eq i32 %spec.store.select, 0, !dbg !71
  %cmp965.not = icmp eq i32 %spec.store.select46, 0, !dbg !75
  %conv16 = uitofp i32 %spec.store.select46 to float, !dbg !79
  %0 = fdiv fast float 1.000000e+00, %conv16
  br i1 %cmp670.not, label %for.end44, label %for.cond5.preheader.us.preheader, !dbg !80

for.cond5.preheader.us.preheader:                 ; preds = %for.cond5.preheader.lr.ph
  %wide.trip.count81 = zext i32 %nNumSamps to i64, !dbg !67
  %wide.trip.count77 = zext i32 %spec.store.select to i64
  %wide.trip.count = zext i32 %spec.store.select46 to i64
  br label %for.cond5.preheader.us, !dbg !70

for.cond5.preheader.us:                           ; preds = %for.cond5.for.inc42_crit_edge.us, %for.cond5.preheader.us.preheader
  %indvars.iv79 = phi i64 [ 0, %for.cond5.preheader.us.preheader ], [ %indvars.iv.next80, %for.cond5.for.inc42_crit_edge.us ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv79, metadata !61, metadata !DIExpression()), !dbg !64
  call void @llvm.dbg.value(metadata i32 0, metadata !60, metadata !DIExpression()), !dbg !64
  br label %for.cond8.preheader.us, !dbg !80

for.end.us.loopexit:                              ; preds = %for.body10.us
  %add.us.lcssa = phi float [ %add.us, %for.body10.us ], !dbg !81
  br label %for.end.us

for.end.us:                                       ; preds = %for.cond8.preheader.us, %for.end.us.loopexit
  %nTempFl.0.lcssa.us = phi float [ 0.000000e+00, %for.cond8.preheader.us ], [ %add.us.lcssa, %for.end.us.loopexit ]
  %1 = fmul fast float %nTempFl.0.lcssa.us, %0
  call void @llvm.dbg.value(metadata float %1, metadata !63, metadata !DIExpression()), !dbg !64
  %cmp17.us = fcmp fast ult float %1, 0.000000e+00, !dbg !83
  %conv23.us = fpext float %1 to double, !dbg !85
  %.v.us = select i1 %cmp17.us, double -5.000000e-01, double 5.000000e-01, !dbg !85
  %2 = fadd fast double %.v.us, %conv23.us, !dbg !85
  %nTempFx.0.us = fptosi double %2 to i32, !dbg !86
  call void @llvm.dbg.value(metadata i32 %nTempFx.0.us, metadata !62, metadata !DIExpression()), !dbg !64
  %spec.store.select45.us = tail call i32 @llvm.smin.i32(i32 %nTempFx.0.us, i32 32767), !dbg !87
  call void @llvm.dbg.value(metadata i32 %spec.store.select45.us, metadata !62, metadata !DIExpression()), !dbg !64
  %spec.store.select47.us = tail call i32 @llvm.smax.i32(i32 %spec.store.select45.us, i32 -32768), !dbg !88
  call void @llvm.dbg.value(metadata i32 %spec.store.select47.us, metadata !62, metadata !DIExpression()), !dbg !64
  %conv34.us = trunc i32 %spec.store.select47.us to i16, !dbg !89
  %arrayidx36.us = getelementptr inbounds ptr, ptr %pOutput, i64 %indvars.iv75, !dbg !90
  %3 = load ptr, ptr %arrayidx36.us, align 8, !dbg !90, !tbaa !91
  %arrayidx38.us = getelementptr inbounds i16, ptr %3, i64 %indvars.iv79, !dbg !90
  store i16 %conv34.us, ptr %arrayidx38.us, align 2, !dbg !95, !tbaa !96
  %indvars.iv.next76 = add nuw nsw i64 %indvars.iv75, 1, !dbg !98
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next76, metadata !60, metadata !DIExpression()), !dbg !64
  %exitcond78.not = icmp eq i64 %indvars.iv.next76, %wide.trip.count77, !dbg !71
  br i1 %exitcond78.not, label %for.cond5.for.inc42_crit_edge.us, label %for.cond8.preheader.us, !dbg !80, !llvm.loop !99

for.body10.us:                                    ; preds = %for.body10.us.preheader, %for.body10.us
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body10.us ], [ 0, %for.body10.us.preheader ]
  %nTempFl.067.us = phi float [ %add.us, %for.body10.us ], [ 0.000000e+00, %for.body10.us.preheader ]
  call void @llvm.dbg.value(metadata float %nTempFl.067.us, metadata !63, metadata !DIExpression()), !dbg !64
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !59, metadata !DIExpression()), !dbg !64
  %arrayidx12.us = getelementptr inbounds [16 x ptr], ptr %pInput, i64 %indvars.iv, i64 %indvars.iv75, !dbg !102
  %4 = load ptr, ptr %arrayidx12.us, align 8, !dbg !102, !tbaa !103
  %arrayidx14.us = getelementptr inbounds i16, ptr %4, i64 %indvars.iv79, !dbg !102
  %5 = load i16, ptr %arrayidx14.us, align 2, !dbg !102, !tbaa !96
  %conv15.us = sitofp i16 %5 to float, !dbg !102
  %add.us = fadd fast float %nTempFl.067.us, %conv15.us, !dbg !81
  call void @llvm.dbg.value(metadata float %add.us, metadata !63, metadata !DIExpression()), !dbg !64
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !105
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next, metadata !59, metadata !DIExpression()), !dbg !64
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count, !dbg !75
  br i1 %exitcond.not, label %for.end.us.loopexit, label %for.body10.us, !dbg !106, !llvm.loop !107

for.cond8.preheader.us:                           ; preds = %for.end.us, %for.cond5.preheader.us
  %indvars.iv75 = phi i64 [ 0, %for.cond5.preheader.us ], [ %indvars.iv.next76, %for.end.us ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv75, metadata !60, metadata !DIExpression()), !dbg !64
  call void @llvm.dbg.value(metadata float 0.000000e+00, metadata !63, metadata !DIExpression()), !dbg !64
  call void @llvm.dbg.value(metadata i32 0, metadata !59, metadata !DIExpression()), !dbg !64
  br i1 %cmp965.not, label %for.end.us, label %for.body10.us.preheader, !dbg !106

for.body10.us.preheader:                          ; preds = %for.cond8.preheader.us
  br label %for.body10.us, !dbg !106

for.cond5.for.inc42_crit_edge.us:                 ; preds = %for.end.us
  %indvars.iv.next80 = add nuw nsw i64 %indvars.iv79, 1, !dbg !109
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next80, metadata !61, metadata !DIExpression()), !dbg !64
  %exitcond82.not = icmp eq i64 %indvars.iv.next80, %wide.trip.count81, !dbg !67
  br i1 %exitcond82.not, label %for.end44.loopexit, label %for.cond5.preheader.us, !dbg !70, !llvm.loop !110

for.end44.loopexit:                               ; preds = %for.cond5.for.inc42_crit_edge.us
  br label %for.end44, !dbg !112

for.end44:                                        ; preds = %for.end44.loopexit, %for.cond5.preheader.lr.ph, %entry
  ret void, !dbg !112
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.umin.i32(i32, i32) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smin.i32(i32, i32) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smax.i32(i32, i32) #1

attributes #0 = { nofree nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "target-cpu"="sapphirerapids" "target-features"="+adx,+aes,+amx-bf16,+amx-int8,+amx-tile,+avx,+avx2,+avx512bf16,+avx512bitalg,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512fp16,+avx512ifma,+avx512vbmi,+avx512vbmi2,+avx512vl,+avx512vnni,+avx512vpopcntdq,+avxvnni,+bmi,+bmi2,+cldemote,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+enqcmd,+f16c,+fma,+fsgsbase,+fxsr,+gfni,+invpcid,+lzcnt,+mmx,+movbe,+movdir64b,+movdiri,+pclmul,+pconfig,+pku,+popcnt,+prfchw,+ptwrite,+rdpid,+rdrnd,+rdseed,+sahf,+serialize,+sgx,+sha,+shstk,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+tsxldtrk,+uintr,+vaes,+vpclmulqdq,+waitpkg,+wbnoinvd,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!34, !35, !36, !37, !38}
!llvm.ident = !{!39}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: true, flags: " --intel -Werror -Wextra -march=sapphirerapids -ffp-model=fast -fiopenmp -Wno-error -ffp-model=fast -Wno-error -c -g -O3 -D LINUX -D TEST_APP -D _GNU_SOURCE -D MLOG_ENABLED -D PRINT_ERR_OK -D PRINT_LOG_OK -D LTE_BS_PHY_STATS_ENABLED -D _POSIX_PRIORITY_SCHEDULING -D PHY_AVX512 -D _BBLIB_SPR_ -qopt-zmm-usage=high -D ALLOW_EXPERIMENTAL_API -D NR5G -D NR5G_BS_PHY_STATS_ENABLED -D CPA -D MPSS -D __INTEL_COMPILER=1700 -D X64 -fveclib=SVML -fheinous-gnu-extensions -mllvm -paropt=31 -fopenmp-typed-clauses -D __GCC_HAVE_DWARF2_CFI_ASM=1 -Xclang -fintel-compatibility -Xclang -fintel-libirc-allowed -mllvm -disable-hir-generate-mkl-call -mllvm -loopopt=1 -Xclang -floopopt-pipeline=light -mllvm -intel-abi-compatible=true -mllvm -hir-details -mllvm -print-module-before-loopopt nr5g_test_fft-29da8a.c -fheinous-gnu-extensions", runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !21, globals: !33, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "nr5g_test_fft-29da8a.c", directory: "/localdisk2/liuchen3/ws")
!2 = !{!3}
!3 = !DICompositeType(tag: DW_TAG_enumeration_type, file: !4, line: 185, baseType: !5, size: 32, elements: !6)
!4 = !DIFile(filename: "nr5g/gnb_ul/gnb_ul_phy_structure.h", directory: "/localdisk2/liuchen3/ws")
!5 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!6 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20}
!7 = !DIEnumerator(name: "FORMAT_0", value: 0)
!8 = !DIEnumerator(name: "FORMAT_1", value: 1)
!9 = !DIEnumerator(name: "FORMAT_2", value: 2)
!10 = !DIEnumerator(name: "FORMAT_3", value: 3)
!11 = !DIEnumerator(name: "FORMAT_A1", value: 4)
!12 = !DIEnumerator(name: "FORMAT_A2", value: 5)
!13 = !DIEnumerator(name: "FORMAT_A3", value: 6)
!14 = !DIEnumerator(name: "FORMAT_B1", value: 7)
!15 = !DIEnumerator(name: "FORMAT_B2", value: 8)
!16 = !DIEnumerator(name: "FORMAT_B3", value: 9)
!17 = !DIEnumerator(name: "FORMAT_B4", value: 10)
!18 = !DIEnumerator(name: "FORMAT_C0", value: 11)
!19 = !DIEnumerator(name: "FORMAT_C2", value: 12)
!20 = !DIEnumerator(name: "FORMAT_LAST", value: 13)
!21 = !{!22, !23, !24, !25, !30}
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!23 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!24 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!25 = !DIDerivedType(tag: DW_TAG_typedef, name: "int16_t", file: !26, line: 25, baseType: !27)
!26 = !DIFile(filename: "/usr/include/x86_64-linux-gnu/bits/stdint-intn.h", directory: "")
!27 = !DIDerivedType(tag: DW_TAG_typedef, name: "__int16_t", file: !28, line: 39, baseType: !29)
!28 = !DIFile(filename: "/usr/include/x86_64-linux-gnu/bits/types.h", directory: "")
!29 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!30 = !DIDerivedType(tag: DW_TAG_typedef, name: "int32_t", file: !26, line: 26, baseType: !31)
!31 = !DIDerivedType(tag: DW_TAG_typedef, name: "__int32_t", file: !28, line: 41, baseType: !32)
!32 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!33 = !{}
!34 = !{i32 7, !"Dwarf Version", i32 4}
!35 = !{i32 2, !"Debug Info Version", i32 3}
!36 = !{i32 1, !"wchar_size", i32 4}
!37 = !{i32 7, !"openmp", i32 51}
!38 = !{i32 7, !"uwtable", i32 2}
!39 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!40 = distinct !DISubprogram(name: "nr5g_test_combine_iq", scope: !41, file: !41, line: 1298, type: !42, scopeLine: 1300, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !53)
!41 = !DIFile(filename: "test/utils/nr5g_test_fft.c", directory: "/localdisk2/liuchen3/ws")
!42 = !DISubroutineType(types: !43)
!43 = !{null, !44, !49, !50, !50, !50}
!44 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !45, size: 64)
!45 = !DICompositeType(tag: DW_TAG_array_type, baseType: !46, size: 1024, elements: !47)
!46 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !25, size: 64)
!47 = !{!48}
!48 = !DISubrange(count: 16)
!49 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !46, size: 64)
!50 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint32_t", file: !51, line: 26, baseType: !52)
!51 = !DIFile(filename: "/usr/include/x86_64-linux-gnu/bits/stdint-uintn.h", directory: "")
!52 = !DIDerivedType(tag: DW_TAG_typedef, name: "__uint32_t", file: !28, line: 42, baseType: !5)
!53 = !{!54, !55, !56, !57, !58, !59, !60, !61, !62, !63}
!54 = !DILocalVariable(name: "pInput", arg: 1, scope: !40, file: !41, line: 1298, type: !44)
!55 = !DILocalVariable(name: "pOutput", arg: 2, scope: !40, file: !41, line: 1298, type: !49)
!56 = !DILocalVariable(name: "nNumStreams", arg: 3, scope: !40, file: !41, line: 1299, type: !50)
!57 = !DILocalVariable(name: "nNumPorts", arg: 4, scope: !40, file: !41, line: 1299, type: !50)
!58 = !DILocalVariable(name: "nNumSamps", arg: 5, scope: !40, file: !41, line: 1299, type: !50)
!59 = !DILocalVariable(name: "nIdx", scope: !40, file: !41, line: 1301, type: !50)
!60 = !DILocalVariable(name: "nPorts", scope: !40, file: !41, line: 1301, type: !50)
!61 = !DILocalVariable(name: "nSamps", scope: !40, file: !41, line: 1301, type: !50)
!62 = !DILocalVariable(name: "nTempFx", scope: !40, file: !41, line: 1302, type: !30)
!63 = !DILocalVariable(name: "nTempFl", scope: !40, file: !41, line: 1303, type: !23)
!64 = !DILocation(line: 0, scope: !40)
!65 = !DILocation(line: 1305, column: 9, scope: !40)
!66 = !DILocation(line: 1307, column: 9, scope: !40)
!67 = !DILocation(line: 1310, column: 29, scope: !68)
!68 = distinct !DILexicalBlock(scope: !69, file: !41, line: 1310, column: 5)
!69 = distinct !DILexicalBlock(scope: !40, file: !41, line: 1310, column: 5)
!70 = !DILocation(line: 1310, column: 5, scope: !69)
!71 = !DILocation(line: 1312, column: 33, scope: !72)
!72 = distinct !DILexicalBlock(scope: !73, file: !41, line: 1312, column: 9)
!73 = distinct !DILexicalBlock(scope: !74, file: !41, line: 1312, column: 9)
!74 = distinct !DILexicalBlock(scope: !68, file: !41, line: 1311, column: 5)
!75 = !DILocation(line: 1315, column: 33, scope: !76)
!76 = distinct !DILexicalBlock(scope: !77, file: !41, line: 1315, column: 13)
!77 = distinct !DILexicalBlock(scope: !78, file: !41, line: 1315, column: 13)
!78 = distinct !DILexicalBlock(scope: !72, file: !41, line: 1313, column: 9)
!79 = !DILocation(line: 1319, column: 33, scope: !78)
!80 = !DILocation(line: 1312, column: 9, scope: !73)
!81 = !DILocation(line: 1317, column: 25, scope: !82)
!82 = distinct !DILexicalBlock(scope: !76, file: !41, line: 1316, column: 13)
!83 = !DILocation(line: 1320, column: 25, scope: !84)
!84 = distinct !DILexicalBlock(scope: !78, file: !41, line: 1320, column: 17)
!85 = !DILocation(line: 1320, column: 17, scope: !78)
!86 = !DILocation(line: 0, scope: !84)
!87 = !DILocation(line: 1325, column: 17, scope: !78)
!88 = !DILocation(line: 1327, column: 17, scope: !78)
!89 = !DILocation(line: 1330, column: 39, scope: !78)
!90 = !DILocation(line: 1330, column: 13, scope: !78)
!91 = !{!92, !92, i64 0}
!92 = !{!"pointer@_ZTSPs", !93, i64 0}
!93 = !{!"omnipotent char", !94, i64 0}
!94 = !{!"Simple C/C++ TBAA"}
!95 = !DILocation(line: 1330, column: 37, scope: !78)
!96 = !{!97, !97, i64 0}
!97 = !{!"short", !93, i64 0}
!98 = !DILocation(line: 1312, column: 52, scope: !72)
!99 = distinct !{!99, !80, !100, !101}
!100 = !DILocation(line: 1331, column: 9, scope: !73)
!101 = !{!"llvm.loop.mustprogress"}
!102 = !DILocation(line: 1317, column: 28, scope: !82)
!103 = !{!104, !92, i64 0}
!104 = !{!"array@_ZTSA16_Ps", !92, i64 0}
!105 = !DILocation(line: 1315, column: 52, scope: !76)
!106 = !DILocation(line: 1315, column: 13, scope: !77)
!107 = distinct !{!107, !106, !108, !101}
!108 = !DILocation(line: 1318, column: 13, scope: !77)
!109 = !DILocation(line: 1310, column: 48, scope: !68)
!110 = distinct !{!110, !70, !111, !101}
!111 = !DILocation(line: 1332, column: 5, scope: !69)
!112 = !DILocation(line: 1333, column: 1, scope: !40)
