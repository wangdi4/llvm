; Test preservation of loop start locations for a simple loop nest.

; RUN: opt -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-general-unroll,hir-cg,simplifycfg,intel-ir-optreport-emitter -disable-output -intel-opt-report=low < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,HIR

; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -disable-output -intel-opt-report=low < %s 2>&1 | FileCheck %s

; CHECK:      LOOP BEGIN at 50802.c (7, 1)
; CHECK-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-EMPTY:
; CHECK-NEXT:     LOOP BEGIN at 50802.c (9, 5)
; HIR-NEXT:           remark #25438: Loop unrolled without remainder by 8
; CHECK-NEXT:     LOOP END
; CHECK-NEXT: LOOP END

target triple = "x86_64-unknown-linux-gnu"

@arr = global [1024 x [1024 x i64]] zeroinitializer, align 16

define void @foo() #0 {
DIR.OMP.SIMD.1:
  %l1.linear.iv = alloca i64, align 8
  br label %DIR.OMP.SIMD.120

DIR.OMP.SIMD.120:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %l1.linear.iv, i64 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.local.015 = phi i64 [ 0, %DIR.OMP.SIMD.120 ], [ %add4, %omp.inner.for.inc ]
  br label %for.body

for.body:
  %l2.017 = phi i64 [ 0, %omp.inner.for.body ], [ %inc, %for.body ]
  %add2 = add nuw nsw i64 %l2.017, %.omp.iv.local.015
  %arrayidx3 = getelementptr inbounds [1024 x [1024 x i64]], ptr @arr, i64 0, i64 %l2.017, i64 %.omp.iv.local.015
  store i64 %add2, ptr %arrayidx3, align 8
  %inc = add nuw nsw i64 %l2.017, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %omp.inner.for.inc, label %for.body, !llvm.loop !45

omp.inner.for.inc:
  %add4 = add nuw nsw i64 %.omp.iv.local.015, 1
  %exitcond18.not = icmp eq i64 %add4, 1024
  br i1 %exitcond18.not, label %DIR.OMP.END.SIMD.119, label %omp.inner.for.body, !llvm.loop !49

DIR.OMP.END.SIMD.119:
  store i64 1024, ptr %l1.linear.iv, align 8
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:
  ret void
}


declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)


attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!9, !10, !11, !12, !13}
!llvm.ident = !{!14}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "arr", scope: !2, file: !3, line: 1, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C11, file: !3, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: true, flags: " --intel -g -O2 -S -x core-avx2 -fiopenmp 50802.c -qopt-report-file=stdout -mllvm -print-module-before-loopopt -fveclib=SVML -fheinous-gnu-extensions", runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "50802.c", directory: "/nfs/site/home/schmidtw/d2/src/50802")
!4 = !{!0}
!5 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 67108864, elements: !7)
!6 = !DIBasicType(name: "long", size: 64, encoding: DW_ATE_signed)
!7 = !{!8, !8}
!8 = !DISubrange(count: 1024)
!9 = !{i32 7, !"Dwarf Version", i32 4}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{i32 7, !"openmp", i32 51}
!13 = !{i32 7, !"uwtable", i32 2}
!14 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!15 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 3, type: !16, scopeLine: 4, flags: DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !18)
!16 = !DISubroutineType(types: !17)
!17 = !{null}
!18 = !{!19, !20, !21, !23}
!19 = !DILocalVariable(name: "l1", scope: !15, file: !3, line: 5, type: !6)
!20 = !DILocalVariable(name: "l2", scope: !15, file: !3, line: 5, type: !6)
!21 = !DILocalVariable(name: ".omp.iv", scope: !22, type: !6, flags: DIFlagArtificial)
!22 = distinct !DILexicalBlock(scope: !15, file: !3, line: 7, column: 1)
!23 = !DILocalVariable(name: ".omp.ub", scope: !22, type: !6, flags: DIFlagArtificial)
!25 = !DILocation(line: 7, column: 1, scope: !22)
!28 = !DILocation(line: 9, column: 5, scope: !29)
!29 = distinct !DILexicalBlock(scope: !30, file: !3, line: 9, column: 5)
!30 = distinct !DILexicalBlock(scope: !22, file: !3, line: 8, column: 33)
!42 = distinct !{}
!45 = distinct !{!45, !28, !46, !47}
!46 = !DILocation(line: 11, column: 5, scope: !29)
!47 = !{!"llvm.loop.mustprogress"}
!49 = distinct !{!49, !25, !50, !51}
!50 = !DILocation(line: 7, column: 28, scope: !22)
!51 = !{!"llvm.loop.vectorize.enable", i1 true}
