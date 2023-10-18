; In HIR code generation, Verify that we attach the correct line number to
; selects and horizontal reductions associated with a reduction operation.

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -mcpu=skylake-avx512 -S -disable-output -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s

; RUN: opt -passes=hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -mcpu=skylake-avx512 -disable-output -intel-opt-report=high < %s 2>&1 | FileCheck --check-prefix=OPTRPT %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; The reduction instruction is associated with !dbg !26764, pointing to
; line 255 ofo the source file.  We check that all adds, selects, and
; horizontal reductions associated with that original instruction are
; assigned to line 255 after HIR code gen.  Formerly these were assigned
; to line 238 (!dbg !25941), which is the #pragma omp simd directive.

; Test reduced from customer code.

; CHECK: *** IR Dump After vpo::VPlanDriverHIRPass ***
; CHECK: <{{.*}}> BEGIN REGION { modified }
; CHECK: <{{.*}}> [[INIT1:%.*]] = 0.000000e+00;
; CHECK: <{{.*}}> [[PHITMP1:%.*]] = [[INIT1]];
; CHECK: <{{.*}}> + DO i1 = 0, %loop.ub, 16
; CHECK: <{{.*}}:255> |   [[ADD1:%.*]] = [[PHITMP1]] + {{%.*}}
; CHECK: <{{.*}}:255> |   [[SELECT1:%.*]] = ({{%.*}} == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? [[ADD1]] : [[PHITMP1]];
; CHECK: <{{.*}}> |   [[PHITMP1]] = [[SELECT1]];
; CHECK: <{{.*}}> + END LOOP
; CHECK: <{{.*}}:255> [[RED1:%.*]] = @llvm.vector.reduce.fadd.v16f64([[RED1]],  [[SELECT1]]);
; CHECK: <{{.*}}> [[PHITMP2:%.*]] = [[RED1]];
; CHECK: <{{.*}}> merge.blk{{.*}}:
; CHECK: <{{.*}}> [[INIT2:%.*]] = 0.000000e+00;
; CHECK: <{{.*}}> [[PHITMP3:%.*]] = [[INIT2]];
; CHECK: <{{.*}}:255> [[ADD2:%.*]] = [[PHITMP3]] + {{%.*}};
; CHECK: <{{.*}}:255> [[SELECT2:%.*]] = ({{%.*}} == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? [[ADD2]] : [[PHITMP3]];
; CHECK: <{{.*}}:255> [[SELECT3:%.*]] = ({{%.*}} == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? [[SELECT2]] : [[PHITMP3]];
; CHECK: <{{.*}}> [[PHITMP3]] = [[SELECT3]];
; CHECK: <{{.*}}:255> [[RED1]] = @llvm.vector.reduce.fadd.v16f64([[PHITMP2]], [[SELECT3]]);
; CHECK: <{{.*}}> END REGION

; OPTRPT: remark #25588: Loop has SIMD reduction
; OPTRPT-NEXT: remark #15590: vectorization support: add reduction with value type double [redacted.hpp:255:28]

define double @foo(double %K, i32 %size, double %w4, i64 %w39, ptr %w40, i64 %w45, ptr %w44) {
entry:
  %b0.red = alloca double, align 8
  %i.linear.iv = alloca i64, align 8
  %cmp3 = icmp sgt i32 %size, 0
  %conv = zext i32 %size to i64
  br i1 %cmp3, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:
  store double 0.000000e+00, ptr %b0.red, align 8, !dbg !25941
  br label %DIR.OMP.SIMD.1386

DIR.OMP.SIMD.1386:
  %w5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %b0.red, double 0.000000e+00, i32 1), "QUAL.OMP.SIMDLEN"(i32 16), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i64 0, i32 1, i32 1) ], !dbg !25941
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:
  %b0.red.promoted = load double, ptr %b0.red, align 8
  br label %omp.inner.for.body

omp.inner.for.body:
  %w13 = phi double [ %b0.red.promoted, %DIR.OMP.SIMD.2 ], [ %w23, %if.end ]
  %w18 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %add29, %if.end ]
  %w46 = add nsw i64 %w18, %w39
  %w47 = getelementptr inbounds double, ptr %w40, i64 %w46
  %w19 = load double, ptr %w47, align 8
  %cmp7 = fcmp fast ogt double %w19, %K
  br i1 %cmp7, label %if.then, label %if.end

if.then:
  %w48 = add nsw i64 %w18, %w45
  %w49 = getelementptr inbounds double, ptr %w44, i64 %w48
  %w20 = load double, ptr %w49, align 8
  %mul.i.i = fmul fast double %w20, %w4
  %add24 = fadd fast double %w13, %mul.i.i, !dbg !26764
  br label %if.end

if.end:
  %w23 = phi double [ %add24, %if.then ], [ %w13, %omp.inner.for.body ]
  %add29 = add nuw nsw i64 %w18, 1
  %exitcond.not = icmp eq i64 %add29, %conv
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2379, label %omp.inner.for.body

DIR.OMP.END.SIMD.2379:
  %.lcssa396 = phi double [ %w23, %if.end ]
  store double %.lcssa396, ptr %b0.red, align 8, !dbg !26764
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:
  call void @llvm.directive.region.exit(token %w5) [ "DIR.OMP.END.SIMD"() ], !dbg !25941
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:
  %w32 = fadd double %.lcssa396, 0.000000e+00, !dbg !25938
  br label %omp.precond.end

omp.precond.end:
  %b0.1 = phi double [ 0.000000e+00, %entry ], [ %w32, %DIR.OMP.END.SIMD.4 ]
  ret double %b0.1
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!llvm.dbg.cu = !{!56}
!llvm.module.flags = !{!13605, !13606, !13607, !13608}

!2 = !DIFile(filename: "redacted.cpp", directory: "/nfs/site/home/redacted")
!56 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !2, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)", isOptimized: true, flags: " --redacted", runtimeVersion: 0, emissionKind: FullDebug, enums: !57, retainedTypes: !2804, globals: !10030, imports: !10290, splitDebugInlining: false, nameTableKind: None)
!57 = !{}
!2804 = !{}
!6900 = !DIFile(filename: "redacted.hpp", directory: "/nfs/site/home/redacted")
!6902 = !DINamespace(name: "full", scope: !6903)
!6903 = !DINamespace(scope: null)
!10030 = !{}
!10290 = !{}
!13605 = !{i32 7, !"Dwarf Version", i32 4}
!13606 = !{i32 2, !"Debug Info Version", i32 3}
!13607 = !{i32 1, !"wchar_size", i32 4}
!13608 = !{i32 7, !"uwtable", i32 2}
!25801 = distinct !DISubprogram(name: "foo", linkageName: "linkagefoo", scope: !6902, file: !6900, line: 216, type: !25802, scopeLine: 217, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !56, templateParams: !25837, retainedNodes: !25804)
!25802 = !DISubroutineType(types: !25803)
!25803 = !{}
!25804 = !{}
!25812 = distinct !DILexicalBlock(scope: !25801, file: !6900, line: 218, column: 13)
!25814 = distinct !DILexicalBlock(scope: !25815, file: !6900, line: 218, column: 56)
!25815 = distinct !DILexicalBlock(scope: !25812, file: !6900, line: 218, column: 13)
!25825 = distinct !DILexicalBlock(scope: !25814, file: !6900, line: 238, column: 1)
!25831 = distinct !DILexicalBlock(scope: !25825, file: !6900, line: 240, column: 54)
!25833 = distinct !DILexicalBlock(scope: !25834, file: !6900, line: 243, column: 58)
!25834 = distinct !DILexicalBlock(scope: !25831, file: !6900, line: 243, column: 25)
!25837 = !{}
!25938 = !DILocation(line: 238, column: 1, scope: !25814)
!25941 = !DILocation(line: 238, column: 1, scope: !25825)
!26764 = !DILocation(line: 255, column: 28, scope: !25833)
