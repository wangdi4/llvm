; RUN: opt -passes=vplan-vec -vplan-force-vf=2 -vplan-dump-debug-loc -vplan-print-after-vpentity-instrs -disable-output < %s | FileCheck %s

; Test setting of debug location information for phis associated with an
; induction.  This exercises a couple of less common cases, wherein the
; induction operation occurs in more than one block, and where the result
; of the induction is live-out from the loop.

; Test created from following C++ code, with the IR modified to force a
; live-out from the confluence of values of "c" at the end of the if/else.
;   1  int foo(int* iptr, int step, int n) {
;   2    int s = 0;
;   3    char c = step;
;   4  #pragma omp simd linear(iptr) linear(c:1) reduction(+:s)
;   5    for (int i = 0; i < n; i++) {
;   6      iptr++;
;   7      if (*iptr) {
;   8        c++;
;   9        *iptr += c;
;  10      } else {
;  11        s += *iptr * c;
;  12        c++;
;  13      }
;  14    }
;  15    return s+c;
;  16  }

; CHECK: VPlan after insertion of VPEntities instructions:
; CHECK-NEXT: VPlan IR for:  _Z3fooPiii:omp.inner.for.body.#1

; Preheader
; CHECK: BB1:
; CHECK:  i8 [[INDINIT:%.*]] = induction-init{add} i8 %c.linear.promoted i8 1
; CHECK-NEXT:   DbgLoc: linear4.cpp:4:1
; CHECK:  i8 [[INITSTEP:%.*]] = induction-init-step{add} i8 1
; CHECK-NEXT:   DbgLoc: linear4.cpp:4:1

; Header
; CHECK: BB2:
; CHECK:  i8 [[PHI1:%.*]] = phi [ i8 [[INDINIT]], BB1 ], [ i8 [[ADD3:%.*]], BB5 ]
; CHECK-NEXT:   DbgLoc: linear4.cpp:12:8

; If-block
; CHECK: BB4:
; CHECK:  i8 [[ADD1:%.*]] = add i8 [[PHI1]] i8 1
; CHECK-NEXT:   DbgLoc: linear4.cpp:8:8

; Then-block
; CHECK: BB3:
; CHECK:  i8 [[ADD2:%.*]] = add i8 [[PHI1]] i8 1
; CHECK-NEXT:   DbgLoc: linear4.cpp:12:8

; Confluence/exit block
; CHECK: BB5:
; CHECK:  i8 [[PHI2:%.*]] = phi [ i8 [[ADD1]], BB4 ],  [ i8 [[ADD2]], BB3 ]
; CHECK-NEXT:   DbgLoc: linear4.cpp:12:8
; CHECK-EMPTY:
; CHECK-NEXT:  i8 [[ADD3]] = add i8 [[PHI1]] i8 [[INITSTEP]]
; CHECK-NEXT:   DbgLoc: linear4.cpp:12:8

; Post-exit
; CHECK: BB6:
; CHECK:  i8 [[FINAL:%.*]] = induction-final{add} i8 %c.linear.promoted i8 1
; CHECK-NEXT:   DbgLoc: linear4.cpp:12:8

; CHECK: External Uses:
; CHECK-NEXT: Id: 0     %wjs = phi i8 [ [[EXTUSE:%.*]], %omp.body.continue ] i8 [[FINAL]] -> i8 [[EXTUSE]];

define i32 @_Z3fooPiii(ptr %iptr, i32 %step, i32 %n) !dbg !8 {
entry:
  %s.red = alloca i32, align 4
  %iptr.addr.linear = alloca ptr, align 8
  %c.linear = alloca i8, align 1
  %i.linear.iv = alloca i32, align 4
  %conv = trunc i32 %step to i8, !dbg !27
  %cmp = icmp sgt i32 %n, 0, !dbg !29
  br i1 %cmp, label %DIR.OMP.SIMD.220, label %omp.precond.end, !dbg !30

DIR.OMP.SIMD.220:
  store ptr %iptr, ptr %iptr.addr.linear, align 8, !dbg !31
  store i8 %conv, ptr %c.linear, align 1, !dbg !31
  store i32 0, ptr %s.red, align 4, !dbg !31
  br label %omp.inner.for.body.lr.ph, !dbg !30

omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(ptr %iptr.addr.linear, i32 0, i32 1, i32 1), "QUAL.OMP.LINEAR:TYPED"(ptr %c.linear, i8 0, i32 1, i32 1), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %s.red, i32 0, i32 1), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ], !dbg !31
  br label %DIR.OMP.SIMD.1, !dbg !31

DIR.OMP.SIMD.1:
  %iptr.addr.linear.promoted = load ptr, ptr %iptr.addr.linear, align 8
  %c.linear.promoted = load i8, ptr %c.linear, align 1
  %1 = trunc i32 %n to i8, !dbg !30
  %2 = add i32 %n, -1, !dbg !30
  %3 = zext i32 %2 to i64, !dbg !30
  %4 = shl nuw nsw i64 %3, 2, !dbg !30
  %5 = add nuw nsw i64 %4, 4, !dbg !30
  br label %omp.inner.for.body, !dbg !30

omp.inner.for.body:
  %6 = phi i8 [ %c.linear.promoted, %DIR.OMP.SIMD.1 ], [ %9, %omp.body.continue ]
  %7 = phi ptr [ %iptr.addr.linear.promoted, %DIR.OMP.SIMD.1 ], [ %incdec.ptr, %omp.body.continue ], !dbg !28
  %.omp.iv.local.024 = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %add11, %omp.body.continue ]
  store i32 %.omp.iv.local.024, ptr %i.linear.iv, align 4, !dbg !38
  %incdec.ptr = getelementptr inbounds i32, ptr %7, i64 1, !dbg !41
  %8 = load i32, ptr %incdec.ptr, align 4, !dbg !43
  %tobool.not = icmp eq i32 %8, 0, !dbg !43
  br i1 %tobool.not, label %if.else, label %if.then, !dbg !45

if.then:
  %inc = add i8 %6, 1, !dbg !46
  %conv5 = sext i8 %inc to i32, !dbg !48
  %add6 = add nsw i32 %8, %conv5, !dbg !49
  store i32 %add6, ptr %incdec.ptr, align 4, !dbg !49
  br label %omp.body.continue, !dbg !50

if.else:
  %inc10 = add i8 %6, 1, !dbg !51
  br label %omp.body.continue

omp.body.continue:
  %9 = phi i8 [ %inc, %if.then ], [ %inc10, %if.else ]
  %add11 = add nuw nsw i32 %.omp.iv.local.024, 1, !dbg !29
  %exitcond.not = icmp eq i32 %add11, %n, !dbg !29
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, label %omp.inner.for.body, !dbg !30

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge:
  %wjs = phi i8 [ %9, %omp.body.continue ]
  %10 = add i8 %c.linear.promoted, %1, !dbg !30
  %scevgep = getelementptr i8, ptr %iptr.addr.linear.promoted, i64 %5, !dbg !30
  store ptr %scevgep, ptr %iptr.addr.linear, align 8, !dbg !41
  store i8 %10, ptr %c.linear, align 1, !dbg !59
  br label %DIR.OMP.END.SIMD.2, !dbg !59

DIR.OMP.END.SIMD.2:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ], !dbg !31
  br label %DIR.OMP.END.SIMD.231, !dbg !31

DIR.OMP.END.SIMD.231:
  %11 = load i32, ptr %s.red, align 4, !dbg !30
  %12 = load i8, ptr %c.linear, align 1, !dbg !30
  br label %omp.precond.end, !dbg !30

omp.precond.end:
  %c.1 = phi i8 [ %conv, %entry ], [ %wjs, %DIR.OMP.END.SIMD.231 ]
  %s.1 = phi i32 [ 0, %entry ], [ %11, %DIR.OMP.END.SIMD.231 ]
  %conv12 = sext i8 %c.1 to i32, !dbg !60
  %add13 = add nsw i32 %s.1, %conv12, !dbg !61
  ret i32 %add13, !dbg !62
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)", isOptimized: true, flags: " --driver-mode=g++ --intel -fiopenmp-simd -O1 -fiopenmp -g linear4.cpp -mllvm -print-before=vplan-vec -mllvm -print-module-scope -fveclib=SVML -fheinous-gnu-extensions", runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "linear4.cpp", directory: "/here")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"openmp", i32 50}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!8 = distinct !DISubprogram(name: "foo", linkageName: "_Z3fooPiii", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !13)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !12, !11, !11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!13 = !{}
!21 = distinct !DILexicalBlock(scope: !8, file: !1, line: 4, column: 1)
!26 = !DILocation(line: 0, scope: !8)
!27 = !DILocation(line: 3, column: 12, scope: !8)
!28 = !DILocation(line: 0, scope: !21)
!29 = !DILocation(line: 5, column: 3, scope: !21)
!30 = !DILocation(line: 4, column: 1, scope: !8)
!31 = !DILocation(line: 4, column: 1, scope: !21)
!38 = !DILocation(line: 5, column: 26, scope: !21)
!41 = !DILocation(line: 6, column: 9, scope: !42)
!42 = distinct !DILexicalBlock(scope: !21, file: !1, line: 5, column: 31)
!43 = !DILocation(line: 7, column: 9, scope: !44)
!44 = distinct !DILexicalBlock(scope: !42, file: !1, line: 7, column: 9)
!45 = !DILocation(line: 7, column: 9, scope: !42)
!46 = !DILocation(line: 8, column: 8, scope: !47)
!47 = distinct !DILexicalBlock(scope: !44, file: !1, line: 7, column: 16)
!48 = !DILocation(line: 9, column: 16, scope: !47)
!49 = !DILocation(line: 9, column: 13, scope: !47)
!50 = !DILocation(line: 10, column: 5, scope: !47)
!51 = !DILocation(line: 12, column: 8, scope: !52)
!52 = distinct !DILexicalBlock(scope: !44, file: !1, line: 10, column: 12)
!53 = !DILocation(line: 14, column: 3, scope: !42)
!59 = !DILocation(line: 0, scope: !44)
!60 = !DILocation(line: 15, column: 12, scope: !8)
!61 = !DILocation(line: 15, column: 11, scope: !8)
!62 = !DILocation(line: 15, column: 3, scope: !8)
