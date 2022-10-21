; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -vpo-paropt -intel-opt-report=low -intel-ir-optreport-emitter -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization),vpo-paropt,function(intel-ir-optreport-emitter)' -intel-opt-report=low -S %s 2>&1 | FileCheck %s

; Test src:
;
; void test1(int X, int Y) {
;   int A, B;
; #pragma omp target teams
; #pragma omp distribute firstprivate(A)
;   for (int i = 0; i < X; ++i)
; #pragma omp parallel
; #pragma omp for firstprivate(B)
;     for (int j = 0; j < Y; j++) {}
; }
;
; void test2(int X) {
;   float C = 10;
;   float D = 11;
;   float E;
;   int F, G, H, I;
;
; #pragma omp target map(to: C) map(tofrom: D) map(from: E) \
;                    map(from: F) map(to: G) map(tofrom: H)
;   {
;     E = C + D;
; #pragma omp parallel for firstprivate(F) lastprivate(F) private(G)
;     for (int i = 0; i < X; ++i) {
;       G = 10; (void) H; (void) I;
;     }
;   }
; }
;
; void test3(int X, int Y) {
;   int J, K, L, M;
; #pragma omp target teams distribute
;   for (int i = 0; i < X; ++i) {
; #pragma omp parallel for firstprivate(J)
;     for (int j = 0; j < Y; j++) {}
; #pragma omp parallel for firstprivate(K)
;     for (int j = 0; j < Y; j++) {}
;   }
; #pragma omp target teams distribute
;   for (int i = 0; i < X; ++i) {
; #pragma omp parallel for shared(L)
;     for (int j = 0; j < Y; j++) {}
; #pragma omp parallel for shared(M)
;     for (int j = 0; j < Y; j++) {}
;   }
; }

; CHECK:      Global optimization report for : __omp_offloading_35_d6824d91__Z5test1_l3{{[[:space:]]}}
; CHECK-NEXT: OMP TARGET BEGIN at test.c (3, 1)
; CHECK-NEXT:     remark: FIRSTPRIVATE clause for variable 'A' is redundant
; CHECK-NEXT:     remark: FIRSTPRIVATE clause for variable 'B' is redundant{{[[:space:]]}}
; CHECK-NEXT:     OMP TEAMS BEGIN at test.c (3, 1)
; CHECK-NEXT:         remark: SHARED clause for variable 'A' has been changed to PRIVATE
; CHECK-NEXT:         remark: SHARED clause for variable 'B' has been changed to PRIVATE{{[[:space:]]}}
; CHECK-NEXT:         OMP DISTRIBUTE BEGIN at test.c (4, 1)
; CHECK-NEXT:             remark: FIRSTPRIVATE clause for variable 'A' has been changed to PRIVATE{{[[:space:]]}}
; CHECK-NEXT:             OMP PARALLEL BEGIN at test.c (6, 1)
; CHECK-NEXT:                 remark: SHARED clause for variable 'B' has been changed to PRIVATE{{[[:space:]]}}
; CHECK-NEXT:                 OMP FOR BEGIN at test.c (7, 1)
; CHECK-NEXT:                     remark: FIRSTPRIVATE clause for variable 'B' has been changed to PRIVATE
; CHECK-NEXT:                 OMP FOR END
; CHECK-NEXT:             OMP PARALLEL END
; CHECK-NEXT:         OMP DISTRIBUTE END
; CHECK-NEXT:     OMP TEAMS END
; CHECK-NEXT: OMP TARGET END

; CHECK:      Global optimization report for : __omp_offloading_35_d6824d91__Z5test2_l17{{[[:space:]]}}
; CHECK-NEXT: OMP TARGET BEGIN at test.c (17, 1)
; CHECK-NEXT:     remark: MAP:TO clause for variable 'C' can be changed to FIRSTPRIVATE to reduce mapping overhead
; CHECK-NEXT:     remark: MAP:TOFROM clause for variable 'D' can be changed to FIRSTPRIVATE to reduce mapping overhead
; CHECK-NEXT:     remark: MAP:FROM clause for variable 'F' is redundant
; CHECK-NEXT:     remark: MAP:TOFROM clause for variable 'H' is redundant
; CHECK-NEXT:     remark: MAP:TO clause for variable 'G' is redundant
; CHECK-NEXT:     remark: FIRSTPRIVATE clause for variable 'I' is redundant{{[[:space:]]}}
; CHECK-NEXT:     OMP PARALLEL FOR BEGIN at test.c (21, 1)
; CHECK-NEXT:         remark: FIRSTPRIVATE clause for variable 'F' has been changed to PRIVATE
; CHECK-NEXT:         remark: LASTPRIVATE clause for variable 'F' has been changed to PRIVATE
; CHECK-NEXT:         remark: SHARED clause for variable 'H' has been changed to PRIVATE
; CHECK-NEXT:         remark: SHARED clause for variable 'I' has been changed to PRIVATE
; CHECK-NEXT:     OMP PARALLEL FOR END
; CHECK-NEXT: OMP TARGET END

; CHECK:      Global optimization report for : __omp_offloading_35_d6824d91__Z5test3_l30{{[[:space:]]}}
; CHECK-NEXT: OMP TARGET BEGIN at test.c (30, 1)
; CHECK-NEXT:     remark: FIRSTPRIVATE clause for variable 'J' is redundant
; CHECK-NEXT:     remark: FIRSTPRIVATE clause for variable 'K' is redundant{{[[:space:]]}}
; CHECK-NEXT:     OMP TEAMS BEGIN at test.c (30, 1)
; CHECK-NEXT:         remark: SHARED clause for variable 'J' has been changed to PRIVATE
; CHECK-NEXT:         remark: SHARED clause for variable 'K' has been changed to PRIVATE{{[[:space:]]}}
; CHECK-NEXT:         OMP DISTRIBUTE BEGIN at test.c (30, 1){{[[:space:]]}}
; CHECK-NEXT:             OMP PARALLEL FOR BEGIN at test.c (32, 1)
; CHECK-NEXT:                 remark: FIRSTPRIVATE clause for variable 'J' has been changed to PRIVATE
; CHECK-NEXT:             OMP PARALLEL FOR END{{[[:space:]]}}
; CHECK-NEXT:             OMP PARALLEL FOR BEGIN at test.c (34, 1)
; CHECK-NEXT:                 remark: FIRSTPRIVATE clause for variable 'K' has been changed to PRIVATE
; CHECK-NEXT:             OMP PARALLEL FOR END
; CHECK-NEXT:         OMP DISTRIBUTE END
; CHECK-NEXT:     OMP TEAMS END
; CHECK-NEXT: OMP TARGET END{{[[:space:]]}}

; CHECK:      Global optimization report for : __omp_offloading_35_d6824d91__Z5test3_l37{{[[:space:]]}}
; CHECK-NEXT: OMP TARGET BEGIN at test.c (37, 1)
; CHECK-NEXT:     remark: FIRSTPRIVATE clause for variable 'L' is redundant
; CHECK-NEXT:     remark: FIRSTPRIVATE clause for variable 'M' is redundant{{[[:space:]]}}
; CHECK-NEXT:     OMP TEAMS BEGIN at test.c (37, 1)
; CHECK-NEXT:         remark: SHARED clause for variable 'L' has been changed to PRIVATE
; CHECK-NEXT:         remark: SHARED clause for variable 'M' has been changed to PRIVATE{{[[:space:]]}}
; CHECK-NEXT:         OMP DISTRIBUTE BEGIN at test.c (37, 1){{[[:space:]]}}
; CHECK-NEXT:             OMP PARALLEL FOR BEGIN at test.c (39, 1)
; CHECK-NEXT:                 remark: SHARED clause for variable 'L' has been changed to PRIVATE
; CHECK-NEXT:             OMP PARALLEL FOR END{{[[:space:]]}}
; CHECK-NEXT:             OMP PARALLEL FOR BEGIN at test.c (41, 1)
; CHECK-NEXT:                 remark: SHARED clause for variable 'M' has been changed to PRIVATE
; CHECK-NEXT:             OMP PARALLEL FOR END
; CHECK-NEXT:         OMP DISTRIBUTE END
; CHECK-NEXT:     OMP TEAMS END
; CHECK-NEXT: OMP TARGET END

; CHECK: define internal void @__omp_offloading_35_d6824d91__Z5test1_l3({{.+}} !intel.optreport.rootnode !{{[0-9]+}}
; CHECK: define internal void @__omp_offloading_35_d6824d91__Z5test2_l17({{.+}} !intel.optreport.rootnode !{{[0-9]+}}
; CHECK: define internal void @__omp_offloading_35_d6824d91__Z5test3_l30({{.+}} !intel.optreport.rootnode !{{[0-9]+}}
; CHECK: define internal void @__omp_offloading_35_d6824d91__Z5test3_l37({{.+}} !intel.optreport.rootnode !{{[0-9]+}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@0 = private unnamed_addr constant [17 x i8] c";E;test.c;14;9;;\00", align 1
@1 = private unnamed_addr constant [17 x i8] c";C;test.c;12;9;;\00", align 1
@2 = private unnamed_addr constant [17 x i8] c";D;test.c;13;9;;\00", align 1
@3 = private unnamed_addr constant [17 x i8] c";F;test.c;15;7;;\00", align 1
@4 = private unnamed_addr constant [18 x i8] c";H;test.c;15;13;;\00", align 1
@5 = private unnamed_addr constant [18 x i8] c";G;test.c;15;10;;\00", align 1

define dso_local void @test1(i32 %X, i32 %Y) #0 !dbg !12 {
entry:
  %X.addr = alloca i32, align 4
  %Y.addr = alloca i32, align 4
  %A = alloca i32, align 4
  %B = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp5 = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv13 = alloca i32, align 4
  %.omp.lb14 = alloca i32, align 4
  %.omp.ub15 = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 %X, i32* %X.addr, align 4, !tbaa !14
  store i32 %Y, i32* %Y.addr, align 4, !tbaa !14
  %0 = load i32, i32* %X.addr, align 4, !dbg !18, !tbaa !14
  store i32 %0, i32* %.capture_expr.2, align 4, !dbg !18, !tbaa !14
  %1 = load i32, i32* %.capture_expr.2, align 4, !dbg !18, !tbaa !14
  %sub = sub nsw i32 %1, 0, !dbg !19
  %sub1 = sub nsw i32 %sub, 1, !dbg !19
  %add = add nsw i32 %sub1, 1, !dbg !19
  %div = sdiv i32 %add, 1, !dbg !19
  %sub2 = sub nsw i32 %div, 1, !dbg !19
  store i32 %sub2, i32* %.capture_expr.3, align 4, !dbg !19, !tbaa !14
  store i32 0, i32* %.omp.lb, align 4, !dbg !20, !tbaa !14
  %2 = load i32, i32* %.capture_expr.3, align 4, !dbg !19, !tbaa !14
  store i32 %2, i32* %.omp.ub, align 4, !dbg !20, !tbaa !14
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %Y.addr),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %A),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %B),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.2),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv13),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb14),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub15),
    "QUAL.OMP.PRIVATE"(i32* %j),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0),
    "QUAL.OMP.PRIVATE"(i32* %tmp),
    "QUAL.OMP.PRIVATE"(i32* %tmp5) ], !dbg !21
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED"(i32* %Y.addr),
    "QUAL.OMP.SHARED"(i32* %A),
    "QUAL.OMP.SHARED"(i32* %B),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv),
    "QUAL.OMP.SHARED"(i32* %.omp.lb),
    "QUAL.OMP.SHARED"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i),
    "QUAL.OMP.SHARED"(i32* %.capture_expr.2),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv13),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb14),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub15),
    "QUAL.OMP.PRIVATE"(i32* %j),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0),
    "QUAL.OMP.PRIVATE"(i32* %tmp),
    "QUAL.OMP.PRIVATE"(i32* %tmp5) ], !dbg !21
  %5 = load i32, i32* %.capture_expr.2, align 4, !dbg !18, !tbaa !14
  %cmp = icmp slt i32 0, %5, !dbg !19
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end27, !dbg !22

omp.precond.then:                                 ; preds = %entry
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %A),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv13),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb14),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub15),
    "QUAL.OMP.PRIVATE"(i32* %j),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0),
    "QUAL.OMP.PRIVATE"(i32* %tmp5) ], !dbg !22
  %7 = load i32, i32* %.omp.lb, align 4, !dbg !20, !tbaa !14
  store i32 %7, i32* %.omp.iv, align 4, !dbg !20, !tbaa !14
  br label %omp.inner.for.cond, !dbg !22

omp.inner.for.cond:                               ; preds = %omp.precond.end, %omp.precond.then
  %8 = load i32, i32* %.omp.iv, align 4, !dbg !20, !tbaa !14
  %9 = load i32, i32* %.omp.ub, align 4, !dbg !20, !tbaa !14
  %cmp3 = icmp sle i32 %8, %9, !dbg !19
  br i1 %cmp3, label %omp.inner.for.body, label %omp.loop.exit26, !dbg !22

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, i32* %.omp.iv, align 4, !dbg !20, !tbaa !14
  %mul = mul nsw i32 %10, 1, !dbg !23
  %add4 = add nsw i32 0, %mul, !dbg !23
  store i32 %add4, i32* %i, align 4, !dbg !23, !tbaa !14
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED"(i32* %Y.addr),
    "QUAL.OMP.SHARED"(i32* %B),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv13),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb14),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub15),
    "QUAL.OMP.PRIVATE"(i32* %j),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0),
    "QUAL.OMP.PRIVATE"(i32* %tmp5) ], !dbg !24
  %12 = load i32, i32* %Y.addr, align 4, !dbg !25, !tbaa !14
  store i32 %12, i32* %.capture_expr.0, align 4, !dbg !25, !tbaa !14
  %13 = load i32, i32* %.capture_expr.0, align 4, !dbg !25, !tbaa !14
  %sub6 = sub nsw i32 %13, 0, !dbg !26
  %sub7 = sub nsw i32 %sub6, 1, !dbg !26
  %add8 = add nsw i32 %sub7, 1, !dbg !26
  %div9 = sdiv i32 %add8, 1, !dbg !26
  %sub10 = sub nsw i32 %div9, 1, !dbg !26
  store i32 %sub10, i32* %.capture_expr.1, align 4, !dbg !26, !tbaa !14
  %14 = load i32, i32* %.capture_expr.0, align 4, !dbg !25, !tbaa !14
  %cmp11 = icmp slt i32 0, %14, !dbg !26
  br i1 %cmp11, label %omp.precond.then12, label %omp.precond.end, !dbg !27

omp.precond.then12:                               ; preds = %omp.inner.for.body
  store i32 0, i32* %.omp.lb14, align 4, !dbg !28, !tbaa !14
  %15 = load i32, i32* %.capture_expr.1, align 4, !dbg !26, !tbaa !14
  store i32 %15, i32* %.omp.ub15, align 4, !dbg !28, !tbaa !14
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %B),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv13),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb14),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub15),
    "QUAL.OMP.PRIVATE"(i32* %j) ], !dbg !27
  %17 = load i32, i32* %.omp.lb14, align 4, !dbg !28, !tbaa !14
  store i32 %17, i32* %.omp.iv13, align 4, !dbg !28, !tbaa !14
  br label %omp.inner.for.cond16, !dbg !27

omp.inner.for.cond16:                             ; preds = %omp.inner.for.body18, %omp.precond.then12
  %18 = load i32, i32* %.omp.iv13, align 4, !dbg !28, !tbaa !14
  %19 = load i32, i32* %.omp.ub15, align 4, !dbg !28, !tbaa !14
  %cmp17 = icmp sle i32 %18, %19, !dbg !26
  br i1 %cmp17, label %omp.inner.for.body18, label %omp.loop.exit, !dbg !27

omp.inner.for.body18:                             ; preds = %omp.inner.for.cond16
  %20 = load i32, i32* %.omp.iv13, align 4, !dbg !28, !tbaa !14
  %mul19 = mul nsw i32 %20, 1, !dbg !29
  %add20 = add nsw i32 0, %mul19, !dbg !29
  store i32 %add20, i32* %j, align 4, !dbg !29, !tbaa !14
  %21 = load i32, i32* %.omp.iv13, align 4, !dbg !28, !tbaa !14
  %add21 = add nsw i32 %21, 1, !dbg !26
  store i32 %add21, i32* %.omp.iv13, align 4, !dbg !26, !tbaa !14
  br label %omp.inner.for.cond16, !dbg !27, !llvm.loop !30

omp.loop.exit:                                    ; preds = %omp.inner.for.cond16
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.LOOP"() ], !dbg !27
  br label %omp.precond.end, !dbg !27

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.PARALLEL"() ], !dbg !24
  %22 = load i32, i32* %.omp.iv, align 4, !dbg !20, !tbaa !14
  %add24 = add nsw i32 %22, 1, !dbg !19
  store i32 %add24, i32* %.omp.iv, align 4, !dbg !19, !tbaa !14
  br label %omp.inner.for.cond, !dbg !22, !llvm.loop !32

omp.loop.exit26:                                  ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.DISTRIBUTE"() ], !dbg !22
  br label %omp.precond.end27, !dbg !22

omp.precond.end27:                                ; preds = %omp.loop.exit26, %entry
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ], !dbg !21
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ], !dbg !21
  ret void, !dbg !34
}

define dso_local void @test2(i32 %X) #0 !dbg !35 {
entry:
  %X.addr = alloca i32, align 4
  %C = alloca float, align 4
  %D = alloca float, align 4
  %E = alloca float, align 4
  %F = alloca i32, align 4
  %G = alloca i32, align 4
  %H = alloca i32, align 4
  %I = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.4 = alloca i32, align 4
  %.capture_expr.5 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %X, i32* %X.addr, align 4, !tbaa !14
  store float 1.000000e+01, float* %C, align 4, !dbg !36, !tbaa !37
  store float 1.100000e+01, float* %D, align 4, !dbg !39, !tbaa !37
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.MAP.FROM"(float* %E, float* %E, i64 4, i64 34, [17 x i8]* @0, i8* null),
    "QUAL.OMP.MAP.TO"(float* %C, float* %C, i64 4, i64 33, [17 x i8]* @1, i8* null),
    "QUAL.OMP.MAP.TOFROM"(float* %D, float* %D, i64 4, i64 35, [17 x i8]* @2, i8* null),
    "QUAL.OMP.MAP.FROM"(i32* %F, i32* %F, i64 4, i64 34, [17 x i8]* @3, i8* null),
    "QUAL.OMP.MAP.TOFROM"(i32* %H, i32* %H, i64 4, i64 35, [18 x i8]* @4, i8* null),
    "QUAL.OMP.MAP.TO"(i32* %G, i32* %G, i64 4, i64 1, [18 x i8]* @5, i8* null),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %X.addr),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %I),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.5),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.4),
    "QUAL.OMP.PRIVATE"(i32* %tmp) ], !dbg !40
  %1 = load float, float* %C, align 4, !dbg !41, !tbaa !37
  %2 = load float, float* %D, align 4, !dbg !42, !tbaa !37
  %add = fadd fast float %1, %2, !dbg !43
  store float %add, float* %E, align 4, !dbg !44, !tbaa !37
  %3 = load i32, i32* %X.addr, align 4, !dbg !45, !tbaa !14
  store i32 %3, i32* %.capture_expr.4, align 4, !dbg !45, !tbaa !14
  %4 = load i32, i32* %.capture_expr.4, align 4, !dbg !45, !tbaa !14
  %sub = sub nsw i32 %4, 0, !dbg !46
  %sub1 = sub nsw i32 %sub, 1, !dbg !46
  %add2 = add nsw i32 %sub1, 1, !dbg !46
  %div = sdiv i32 %add2, 1, !dbg !46
  %sub3 = sub nsw i32 %div, 1, !dbg !46
  store i32 %sub3, i32* %.capture_expr.5, align 4, !dbg !46, !tbaa !14
  %5 = load i32, i32* %.capture_expr.4, align 4, !dbg !45, !tbaa !14
  %cmp = icmp slt i32 0, %5, !dbg !46
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end, !dbg !47

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4, !dbg !48, !tbaa !14
  %6 = load i32, i32* %.capture_expr.5, align 4, !dbg !46, !tbaa !14
  store i32 %6, i32* %.omp.ub, align 4, !dbg !48, !tbaa !14
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %F),
    "QUAL.OMP.LASTPRIVATE"(i32* %F),
    "QUAL.OMP.PRIVATE"(i32* %G),
    "QUAL.OMP.SHARED"(i32* %H),
    "QUAL.OMP.SHARED"(i32* %I),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i) ], !dbg !47
  %8 = load i32, i32* %.omp.lb, align 4, !dbg !48, !tbaa !14
  store i32 %8, i32* %.omp.iv, align 4, !dbg !48, !tbaa !14
  br label %omp.inner.for.cond, !dbg !47

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %omp.precond.then
  %9 = load i32, i32* %.omp.iv, align 4, !dbg !48, !tbaa !14
  %10 = load i32, i32* %.omp.ub, align 4, !dbg !48, !tbaa !14
  %cmp4 = icmp sle i32 %9, %10, !dbg !46
  br i1 %cmp4, label %omp.inner.for.body, label %omp.loop.exit, !dbg !47

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = load i32, i32* %.omp.iv, align 4, !dbg !48, !tbaa !14
  %mul = mul nsw i32 %11, 1, !dbg !49
  %add5 = add nsw i32 0, %mul, !dbg !49
  store i32 %add5, i32* %i, align 4, !dbg !49, !tbaa !14
  store i32 10, i32* %G, align 4, !dbg !50, !tbaa !14
  %12 = load i32, i32* %H, align 4, !dbg !51, !tbaa !14
  %13 = load i32, i32* %I, align 4, !dbg !52, !tbaa !14
  %14 = load i32, i32* %.omp.iv, align 4, !dbg !48, !tbaa !14
  %add6 = add nsw i32 %14, 1, !dbg !46
  store i32 %add6, i32* %.omp.iv, align 4, !dbg !46, !tbaa !14
  br label %omp.inner.for.cond, !dbg !47, !llvm.loop !53

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !47
  br label %omp.precond.end, !dbg !47

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !40
  ret void, !dbg !55
}

define dso_local void @test3(i32 %X, i32 %Y) #0 !dbg !56 {
entry:
  %X.addr = alloca i32, align 4
  %Y.addr = alloca i32, align 4
  %J = alloca i32, align 4
  %K = alloca i32, align 4
  %L = alloca i32, align 4
  %M = alloca i32, align 4
  %.capture_expr.10 = alloca i32, align 4
  %.capture_expr.11 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp5 = alloca i32, align 4
  %.capture_expr.6 = alloca i32, align 4
  %.capture_expr.7 = alloca i32, align 4
  %.omp.iv13 = alloca i32, align 4
  %.omp.lb14 = alloca i32, align 4
  %.omp.ub15 = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp22 = alloca i32, align 4
  %.capture_expr.8 = alloca i32, align 4
  %.capture_expr.9 = alloca i32, align 4
  %.omp.iv30 = alloca i32, align 4
  %.omp.lb31 = alloca i32, align 4
  %.omp.ub32 = alloca i32, align 4
  %j36 = alloca i32, align 4
  %.capture_expr.16 = alloca i32, align 4
  %.capture_expr.17 = alloca i32, align 4
  %.omp.lb56 = alloca i32, align 4
  %.omp.ub57 = alloca i32, align 4
  %tmp58 = alloca i32, align 4
  %.omp.iv61 = alloca i32, align 4
  %i65 = alloca i32, align 4
  %tmp68 = alloca i32, align 4
  %.capture_expr.12 = alloca i32, align 4
  %.capture_expr.13 = alloca i32, align 4
  %.omp.iv76 = alloca i32, align 4
  %.omp.lb77 = alloca i32, align 4
  %.omp.ub78 = alloca i32, align 4
  %j82 = alloca i32, align 4
  %tmp91 = alloca i32, align 4
  %.capture_expr.14 = alloca i32, align 4
  %.capture_expr.15 = alloca i32, align 4
  %.omp.iv99 = alloca i32, align 4
  %.omp.lb100 = alloca i32, align 4
  %.omp.ub101 = alloca i32, align 4
  %j105 = alloca i32, align 4
  store i32 %X, i32* %X.addr, align 4, !tbaa !14
  store i32 %Y, i32* %Y.addr, align 4, !tbaa !14
  %0 = load i32, i32* %X.addr, align 4, !dbg !57, !tbaa !14
  store i32 %0, i32* %.capture_expr.10, align 4, !dbg !57, !tbaa !14
  %1 = load i32, i32* %.capture_expr.10, align 4, !dbg !57, !tbaa !14
  %sub = sub nsw i32 %1, 0, !dbg !58
  %sub1 = sub nsw i32 %sub, 1, !dbg !58
  %add = add nsw i32 %sub1, 1, !dbg !58
  %div = sdiv i32 %add, 1, !dbg !58
  %sub2 = sub nsw i32 %div, 1, !dbg !58
  store i32 %sub2, i32* %.capture_expr.11, align 4, !dbg !58, !tbaa !14
  store i32 0, i32* %.omp.lb, align 4, !dbg !59, !tbaa !14
  %2 = load i32, i32* %.capture_expr.11, align 4, !dbg !58, !tbaa !14
  store i32 %2, i32* %.omp.ub, align 4, !dbg !59, !tbaa !14
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %Y.addr),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %J),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %K),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.10),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.7),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv13),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb14),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub15),
    "QUAL.OMP.PRIVATE"(i32* %j),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.6),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.9),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv30),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb31),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub32),
    "QUAL.OMP.PRIVATE"(i32* %j36),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.8),
    "QUAL.OMP.PRIVATE"(i32* %tmp),
    "QUAL.OMP.PRIVATE"(i32* %tmp5),
    "QUAL.OMP.PRIVATE"(i32* %tmp22) ], !dbg !60
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED"(i32* %Y.addr),
    "QUAL.OMP.SHARED"(i32* %J),
    "QUAL.OMP.SHARED"(i32* %K),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv),
    "QUAL.OMP.SHARED"(i32* %.omp.lb),
    "QUAL.OMP.SHARED"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i),
    "QUAL.OMP.SHARED"(i32* %.capture_expr.10),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.7),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv13),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb14),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub15),
    "QUAL.OMP.PRIVATE"(i32* %j),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.6),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.9),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv30),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb31),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub32),
    "QUAL.OMP.PRIVATE"(i32* %j36),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.8),
    "QUAL.OMP.PRIVATE"(i32* %tmp),
    "QUAL.OMP.PRIVATE"(i32* %tmp5),
    "QUAL.OMP.PRIVATE"(i32* %tmp22) ], !dbg !60
  %5 = load i32, i32* %.capture_expr.10, align 4, !dbg !57, !tbaa !14
  %cmp = icmp slt i32 0, %5, !dbg !58
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end50, !dbg !60

omp.precond.then:                                 ; preds = %entry
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.7),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv13),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb14),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub15),
    "QUAL.OMP.PRIVATE"(i32* %j),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.6),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.9),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv30),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb31),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub32),
    "QUAL.OMP.PRIVATE"(i32* %j36),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.8),
    "QUAL.OMP.PRIVATE"(i32* %tmp5),
    "QUAL.OMP.PRIVATE"(i32* %tmp22) ], !dbg !60
  %7 = load i32, i32* %.omp.lb, align 4, !dbg !59, !tbaa !14
  store i32 %7, i32* %.omp.iv, align 4, !dbg !59, !tbaa !14
  br label %omp.inner.for.cond, !dbg !60

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc46, %omp.precond.then
  %8 = load i32, i32* %.omp.iv, align 4, !dbg !59, !tbaa !14
  %9 = load i32, i32* %.omp.ub, align 4, !dbg !59, !tbaa !14
  %cmp3 = icmp sle i32 %8, %9, !dbg !58
  br i1 %cmp3, label %omp.inner.for.body, label %omp.loop.exit49, !dbg !60

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, i32* %.omp.iv, align 4, !dbg !59, !tbaa !14
  %mul = mul nsw i32 %10, 1, !dbg !61
  %add4 = add nsw i32 0, %mul, !dbg !61
  store i32 %add4, i32* %i, align 4, !dbg !61, !tbaa !14
  %11 = load i32, i32* %Y.addr, align 4, !dbg !62, !tbaa !14
  store i32 %11, i32* %.capture_expr.6, align 4, !dbg !62, !tbaa !14
  %12 = load i32, i32* %.capture_expr.6, align 4, !dbg !62, !tbaa !14
  %sub6 = sub nsw i32 %12, 0, !dbg !63
  %sub7 = sub nsw i32 %sub6, 1, !dbg !63
  %add8 = add nsw i32 %sub7, 1, !dbg !63
  %div9 = sdiv i32 %add8, 1, !dbg !63
  %sub10 = sub nsw i32 %div9, 1, !dbg !63
  store i32 %sub10, i32* %.capture_expr.7, align 4, !dbg !63, !tbaa !14
  %13 = load i32, i32* %.capture_expr.6, align 4, !dbg !62, !tbaa !14
  %cmp11 = icmp slt i32 0, %13, !dbg !63
  br i1 %cmp11, label %omp.precond.then12, label %omp.precond.end, !dbg !64

omp.precond.then12:                               ; preds = %omp.inner.for.body
  store i32 0, i32* %.omp.lb14, align 4, !dbg !65, !tbaa !14
  %14 = load i32, i32* %.capture_expr.7, align 4, !dbg !63, !tbaa !14
  store i32 %14, i32* %.omp.ub15, align 4, !dbg !65, !tbaa !14
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %J),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv13),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb14),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub15),
    "QUAL.OMP.PRIVATE"(i32* %j) ], !dbg !64
  %16 = load i32, i32* %.omp.lb14, align 4, !dbg !65, !tbaa !14
  store i32 %16, i32* %.omp.iv13, align 4, !dbg !65, !tbaa !14
  br label %omp.inner.for.cond16, !dbg !64

omp.inner.for.cond16:                             ; preds = %omp.inner.for.body18, %omp.precond.then12
  %17 = load i32, i32* %.omp.iv13, align 4, !dbg !65, !tbaa !14
  %18 = load i32, i32* %.omp.ub15, align 4, !dbg !65, !tbaa !14
  %cmp17 = icmp sle i32 %17, %18, !dbg !63
  br i1 %cmp17, label %omp.inner.for.body18, label %omp.loop.exit, !dbg !64

omp.inner.for.body18:                             ; preds = %omp.inner.for.cond16
  %19 = load i32, i32* %.omp.iv13, align 4, !dbg !65, !tbaa !14
  %mul19 = mul nsw i32 %19, 1, !dbg !66
  %add20 = add nsw i32 0, %mul19, !dbg !66
  store i32 %add20, i32* %j, align 4, !dbg !66, !tbaa !14
  %20 = load i32, i32* %.omp.iv13, align 4, !dbg !65, !tbaa !14
  %add21 = add nsw i32 %20, 1, !dbg !63
  store i32 %add21, i32* %.omp.iv13, align 4, !dbg !63, !tbaa !14
  br label %omp.inner.for.cond16, !dbg !64, !llvm.loop !67

omp.loop.exit:                                    ; preds = %omp.inner.for.cond16
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !64
  br label %omp.precond.end, !dbg !64

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  %21 = load i32, i32* %Y.addr, align 4, !dbg !69, !tbaa !14
  store i32 %21, i32* %.capture_expr.8, align 4, !dbg !69, !tbaa !14
  %22 = load i32, i32* %.capture_expr.8, align 4, !dbg !69, !tbaa !14
  %sub23 = sub nsw i32 %22, 0, !dbg !70
  %sub24 = sub nsw i32 %sub23, 1, !dbg !70
  %add25 = add nsw i32 %sub24, 1, !dbg !70
  %div26 = sdiv i32 %add25, 1, !dbg !70
  %sub27 = sub nsw i32 %div26, 1, !dbg !70
  store i32 %sub27, i32* %.capture_expr.9, align 4, !dbg !70, !tbaa !14
  %23 = load i32, i32* %.capture_expr.8, align 4, !dbg !69, !tbaa !14
  %cmp28 = icmp slt i32 0, %23, !dbg !70
  br i1 %cmp28, label %omp.precond.then29, label %omp.inner.for.inc46, !dbg !71

omp.precond.then29:                               ; preds = %omp.precond.end
  store i32 0, i32* %.omp.lb31, align 4, !dbg !72, !tbaa !14
  %24 = load i32, i32* %.capture_expr.9, align 4, !dbg !70, !tbaa !14
  store i32 %24, i32* %.omp.ub32, align 4, !dbg !72, !tbaa !14
  %25 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %K),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv30),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb31),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub32),
    "QUAL.OMP.PRIVATE"(i32* %j36) ], !dbg !71
  %26 = load i32, i32* %.omp.lb31, align 4, !dbg !72, !tbaa !14
  store i32 %26, i32* %.omp.iv30, align 4, !dbg !72, !tbaa !14
  br label %omp.inner.for.cond33, !dbg !71

omp.inner.for.cond33:                             ; preds = %omp.inner.for.body35, %omp.precond.then29
  %27 = load i32, i32* %.omp.iv30, align 4, !dbg !72, !tbaa !14
  %28 = load i32, i32* %.omp.ub32, align 4, !dbg !72, !tbaa !14
  %cmp34 = icmp sle i32 %27, %28, !dbg !70
  br i1 %cmp34, label %omp.inner.for.body35, label %omp.loop.exit43, !dbg !71

omp.inner.for.body35:                             ; preds = %omp.inner.for.cond33
  %29 = load i32, i32* %.omp.iv30, align 4, !dbg !72, !tbaa !14
  %mul37 = mul nsw i32 %29, 1, !dbg !73
  %add38 = add nsw i32 0, %mul37, !dbg !73
  store i32 %add38, i32* %j36, align 4, !dbg !73, !tbaa !14
  %30 = load i32, i32* %.omp.iv30, align 4, !dbg !72, !tbaa !14
  %add41 = add nsw i32 %30, 1, !dbg !70
  store i32 %add41, i32* %.omp.iv30, align 4, !dbg !70, !tbaa !14
  br label %omp.inner.for.cond33, !dbg !71, !llvm.loop !74

omp.loop.exit43:                                  ; preds = %omp.inner.for.cond33
  call void @llvm.directive.region.exit(token %25) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !71
  br label %omp.inner.for.inc46, !dbg !71

omp.inner.for.inc46:                              ; preds = %omp.loop.exit43, %omp.precond.end
  %31 = load i32, i32* %.omp.iv, align 4, !dbg !59, !tbaa !14
  %add47 = add nsw i32 %31, 1, !dbg !58
  store i32 %add47, i32* %.omp.iv, align 4, !dbg !58, !tbaa !14
  br label %omp.inner.for.cond, !dbg !60, !llvm.loop !76

omp.loop.exit49:                                  ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.DISTRIBUTE"() ], !dbg !60
  br label %omp.precond.end50, !dbg !60

omp.precond.end50:                                ; preds = %omp.loop.exit49, %entry
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ], !dbg !60
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ], !dbg !60
  %32 = load i32, i32* %X.addr, align 4, !dbg !78, !tbaa !14
  store i32 %32, i32* %.capture_expr.16, align 4, !dbg !78, !tbaa !14
  %33 = load i32, i32* %.capture_expr.16, align 4, !dbg !78, !tbaa !14
  %sub51 = sub nsw i32 %33, 0, !dbg !79
  %sub52 = sub nsw i32 %sub51, 1, !dbg !79
  %add53 = add nsw i32 %sub52, 1, !dbg !79
  %div54 = sdiv i32 %add53, 1, !dbg !79
  %sub55 = sub nsw i32 %div54, 1, !dbg !79
  store i32 %sub55, i32* %.capture_expr.17, align 4, !dbg !79, !tbaa !14
  store i32 0, i32* %.omp.lb56, align 4, !dbg !80, !tbaa !14
  %34 = load i32, i32* %.capture_expr.17, align 4, !dbg !79, !tbaa !14
  store i32 %34, i32* %.omp.ub57, align 4, !dbg !80, !tbaa !14
  %35 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %Y.addr),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %L),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %M),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv61),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb56),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.ub57),
    "QUAL.OMP.PRIVATE"(i32* %i65),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.16),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.13),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv76),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb77),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub78),
    "QUAL.OMP.PRIVATE"(i32* %j82),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.12),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.15),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv99),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb100),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub101),
    "QUAL.OMP.PRIVATE"(i32* %j105),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.14),
    "QUAL.OMP.PRIVATE"(i32* %tmp58),
    "QUAL.OMP.PRIVATE"(i32* %tmp68),
    "QUAL.OMP.PRIVATE"(i32* %tmp91) ], !dbg !81
  %36 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED"(i32* %Y.addr),
    "QUAL.OMP.SHARED"(i32* %L),
    "QUAL.OMP.SHARED"(i32* %M),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv61),
    "QUAL.OMP.SHARED"(i32* %.omp.lb56),
    "QUAL.OMP.SHARED"(i32* %.omp.ub57),
    "QUAL.OMP.PRIVATE"(i32* %i65),
    "QUAL.OMP.SHARED"(i32* %.capture_expr.16),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.13),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv76),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb77),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub78),
    "QUAL.OMP.PRIVATE"(i32* %j82),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.12),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.15),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv99),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb100),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub101),
    "QUAL.OMP.PRIVATE"(i32* %j105),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.14),
    "QUAL.OMP.PRIVATE"(i32* %tmp58),
    "QUAL.OMP.PRIVATE"(i32* %tmp68),
    "QUAL.OMP.PRIVATE"(i32* %tmp91) ], !dbg !81
  %37 = load i32, i32* %.capture_expr.16, align 4, !dbg !78, !tbaa !14
  %cmp59 = icmp slt i32 0, %37, !dbg !79
  br i1 %cmp59, label %omp.precond.then60, label %omp.precond.end119, !dbg !81

omp.precond.then60:                               ; preds = %omp.precond.end50
  %38 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv61),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb56),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub57),
    "QUAL.OMP.PRIVATE"(i32* %i65),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.13),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv76),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb77),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub78),
    "QUAL.OMP.PRIVATE"(i32* %j82),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.12),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.15),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv99),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb100),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub101),
    "QUAL.OMP.PRIVATE"(i32* %j105),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.14),
    "QUAL.OMP.PRIVATE"(i32* %tmp68),
    "QUAL.OMP.PRIVATE"(i32* %tmp91) ], !dbg !81
  %39 = load i32, i32* %.omp.lb56, align 4, !dbg !80, !tbaa !14
  store i32 %39, i32* %.omp.iv61, align 4, !dbg !80, !tbaa !14
  br label %omp.inner.for.cond62, !dbg !81

omp.inner.for.cond62:                             ; preds = %omp.inner.for.inc115, %omp.precond.then60
  %40 = load i32, i32* %.omp.iv61, align 4, !dbg !80, !tbaa !14
  %41 = load i32, i32* %.omp.ub57, align 4, !dbg !80, !tbaa !14
  %cmp63 = icmp sle i32 %40, %41, !dbg !79
  br i1 %cmp63, label %omp.inner.for.body64, label %omp.loop.exit118, !dbg !81

omp.inner.for.body64:                             ; preds = %omp.inner.for.cond62
  %42 = load i32, i32* %.omp.iv61, align 4, !dbg !80, !tbaa !14
  %mul66 = mul nsw i32 %42, 1, !dbg !82
  %add67 = add nsw i32 0, %mul66, !dbg !82
  store i32 %add67, i32* %i65, align 4, !dbg !82, !tbaa !14
  %43 = load i32, i32* %Y.addr, align 4, !dbg !83, !tbaa !14
  store i32 %43, i32* %.capture_expr.12, align 4, !dbg !83, !tbaa !14
  %44 = load i32, i32* %.capture_expr.12, align 4, !dbg !83, !tbaa !14
  %sub69 = sub nsw i32 %44, 0, !dbg !84
  %sub70 = sub nsw i32 %sub69, 1, !dbg !84
  %add71 = add nsw i32 %sub70, 1, !dbg !84
  %div72 = sdiv i32 %add71, 1, !dbg !84
  %sub73 = sub nsw i32 %div72, 1, !dbg !84
  store i32 %sub73, i32* %.capture_expr.13, align 4, !dbg !84, !tbaa !14
  %45 = load i32, i32* %.capture_expr.12, align 4, !dbg !83, !tbaa !14
  %cmp74 = icmp slt i32 0, %45, !dbg !84
  br i1 %cmp74, label %omp.precond.then75, label %omp.precond.end90, !dbg !85

omp.precond.then75:                               ; preds = %omp.inner.for.body64
  store i32 0, i32* %.omp.lb77, align 4, !dbg !86, !tbaa !14
  %46 = load i32, i32* %.capture_expr.13, align 4, !dbg !84, !tbaa !14
  store i32 %46, i32* %.omp.ub78, align 4, !dbg !86, !tbaa !14
  %47 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED"(i32* %L),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv76),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb77),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub78),
    "QUAL.OMP.PRIVATE"(i32* %j82) ], !dbg !85
  %48 = load i32, i32* %.omp.lb77, align 4, !dbg !86, !tbaa !14
  store i32 %48, i32* %.omp.iv76, align 4, !dbg !86, !tbaa !14
  br label %omp.inner.for.cond79, !dbg !85

omp.inner.for.cond79:                             ; preds = %omp.inner.for.body81, %omp.precond.then75
  %49 = load i32, i32* %.omp.iv76, align 4, !dbg !86, !tbaa !14
  %50 = load i32, i32* %.omp.ub78, align 4, !dbg !86, !tbaa !14
  %cmp80 = icmp sle i32 %49, %50, !dbg !84
  br i1 %cmp80, label %omp.inner.for.body81, label %omp.loop.exit89, !dbg !85

omp.inner.for.body81:                             ; preds = %omp.inner.for.cond79
  %51 = load i32, i32* %.omp.iv76, align 4, !dbg !86, !tbaa !14
  %mul83 = mul nsw i32 %51, 1, !dbg !87
  %add84 = add nsw i32 0, %mul83, !dbg !87
  store i32 %add84, i32* %j82, align 4, !dbg !87, !tbaa !14
  %52 = load i32, i32* %.omp.iv76, align 4, !dbg !86, !tbaa !14
  %add87 = add nsw i32 %52, 1, !dbg !84
  store i32 %add87, i32* %.omp.iv76, align 4, !dbg !84, !tbaa !14
  br label %omp.inner.for.cond79, !dbg !85, !llvm.loop !88

omp.loop.exit89:                                  ; preds = %omp.inner.for.cond79
  call void @llvm.directive.region.exit(token %47) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !85
  br label %omp.precond.end90, !dbg !85

omp.precond.end90:                                ; preds = %omp.loop.exit89, %omp.inner.for.body64
  %53 = load i32, i32* %Y.addr, align 4, !dbg !90, !tbaa !14
  store i32 %53, i32* %.capture_expr.14, align 4, !dbg !90, !tbaa !14
  %54 = load i32, i32* %.capture_expr.14, align 4, !dbg !90, !tbaa !14
  %sub92 = sub nsw i32 %54, 0, !dbg !91
  %sub93 = sub nsw i32 %sub92, 1, !dbg !91
  %add94 = add nsw i32 %sub93, 1, !dbg !91
  %div95 = sdiv i32 %add94, 1, !dbg !91
  %sub96 = sub nsw i32 %div95, 1, !dbg !91
  store i32 %sub96, i32* %.capture_expr.15, align 4, !dbg !91, !tbaa !14
  %55 = load i32, i32* %.capture_expr.14, align 4, !dbg !90, !tbaa !14
  %cmp97 = icmp slt i32 0, %55, !dbg !91
  br i1 %cmp97, label %omp.precond.then98, label %omp.inner.for.inc115, !dbg !92

omp.precond.then98:                               ; preds = %omp.precond.end90
  store i32 0, i32* %.omp.lb100, align 4, !dbg !93, !tbaa !14
  %56 = load i32, i32* %.capture_expr.15, align 4, !dbg !91, !tbaa !14
  store i32 %56, i32* %.omp.ub101, align 4, !dbg !93, !tbaa !14
  %57 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED"(i32* %M),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv99),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb100),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub101),
    "QUAL.OMP.PRIVATE"(i32* %j105) ], !dbg !92
  %58 = load i32, i32* %.omp.lb100, align 4, !dbg !93, !tbaa !14
  store i32 %58, i32* %.omp.iv99, align 4, !dbg !93, !tbaa !14
  br label %omp.inner.for.cond102, !dbg !92

omp.inner.for.cond102:                            ; preds = %omp.inner.for.body104, %omp.precond.then98
  %59 = load i32, i32* %.omp.iv99, align 4, !dbg !93, !tbaa !14
  %60 = load i32, i32* %.omp.ub101, align 4, !dbg !93, !tbaa !14
  %cmp103 = icmp sle i32 %59, %60, !dbg !91
  br i1 %cmp103, label %omp.inner.for.body104, label %omp.loop.exit112, !dbg !92

omp.inner.for.body104:                            ; preds = %omp.inner.for.cond102
  %61 = load i32, i32* %.omp.iv99, align 4, !dbg !93, !tbaa !14
  %mul106 = mul nsw i32 %61, 1, !dbg !94
  %add107 = add nsw i32 0, %mul106, !dbg !94
  store i32 %add107, i32* %j105, align 4, !dbg !94, !tbaa !14
  %62 = load i32, i32* %.omp.iv99, align 4, !dbg !93, !tbaa !14
  %add110 = add nsw i32 %62, 1, !dbg !91
  store i32 %add110, i32* %.omp.iv99, align 4, !dbg !91, !tbaa !14
  br label %omp.inner.for.cond102, !dbg !92, !llvm.loop !95

omp.loop.exit112:                                 ; preds = %omp.inner.for.cond102
  call void @llvm.directive.region.exit(token %57) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !92
  br label %omp.inner.for.inc115, !dbg !92

omp.inner.for.inc115:                             ; preds = %omp.loop.exit112, %omp.precond.end90
  %63 = load i32, i32* %.omp.iv61, align 4, !dbg !80, !tbaa !14
  %add116 = add nsw i32 %63, 1, !dbg !79
  store i32 %add116, i32* %.omp.iv61, align 4, !dbg !79, !tbaa !14
  br label %omp.inner.for.cond62, !dbg !81, !llvm.loop !97

omp.loop.exit118:                                 ; preds = %omp.inner.for.cond62
  call void @llvm.directive.region.exit(token %38) [ "DIR.OMP.END.DISTRIBUTE"() ], !dbg !81
  br label %omp.precond.end119, !dbg !81

omp.precond.end119:                               ; preds = %omp.loop.exit118, %omp.precond.end50
  call void @llvm.directive.region.exit(token %36) [ "DIR.OMP.END.TEAMS"() ], !dbg !81
  call void @llvm.directive.region.exit(token %35) [ "DIR.OMP.END.TARGET"() ], !dbg !81
  ret void, !dbg !99
}

declare token @llvm.directive.region.entry() #1

declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!3, !4, !5, !6}
!llvm.module.flags = !{!7, !8, !9, !10}
!llvm.ident = !{!11}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "/tmp")
!2 = !{}
!3 = !{i32 0, i32 53, i32 -696103535, !"_Z5test2", i32 17, i32 1, i32 0}
!4 = !{i32 0, i32 53, i32 -696103535, !"_Z5test3", i32 30, i32 2, i32 0}
!5 = !{i32 0, i32 53, i32 -696103535, !"_Z5test3", i32 37, i32 3, i32 0}
!6 = !{i32 0, i32 53, i32 -696103535, !"_Z5test1", i32 3, i32 0, i32 0}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{i32 7, !"openmp", i32 50}
!10 = !{i32 7, !"uwtable", i32 1}
!11 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!12 = distinct !DISubprogram(name: "test1", scope: !1, file: !1, line: 1, type: !13, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!13 = !DISubroutineType(types: !2)
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !DILocation(line: 5, column: 23, scope: !12)
!19 = !DILocation(line: 5, column: 3, scope: !12)
!20 = !DILocation(line: 5, column: 8, scope: !12)
!21 = !DILocation(line: 3, column: 1, scope: !12)
!22 = !DILocation(line: 4, column: 1, scope: !12)
!23 = !DILocation(line: 5, column: 26, scope: !12)
!24 = !DILocation(line: 6, column: 1, scope: !12)
!25 = !DILocation(line: 8, column: 25, scope: !12)
!26 = !DILocation(line: 8, column: 5, scope: !12)
!27 = !DILocation(line: 7, column: 1, scope: !12)
!28 = !DILocation(line: 8, column: 10, scope: !12)
!29 = !DILocation(line: 8, column: 28, scope: !12)
!30 = distinct !{!30, !27, !31}
!31 = !DILocation(line: 7, column: 32, scope: !12)
!32 = distinct !{!32, !22, !33}
!33 = !DILocation(line: 4, column: 39, scope: !12)
!34 = !DILocation(line: 9, column: 1, scope: !12)
!35 = distinct !DISubprogram(name: "test2", scope: !1, file: !1, line: 11, type: !13, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!36 = !DILocation(line: 12, column: 9, scope: !35)
!37 = !{!38, !38, i64 0}
!38 = !{!"float", !16, i64 0}
!39 = !DILocation(line: 13, column: 9, scope: !35)
!40 = !DILocation(line: 17, column: 1, scope: !35)
!41 = !DILocation(line: 20, column: 9, scope: !35)
!42 = !DILocation(line: 20, column: 13, scope: !35)
!43 = !DILocation(line: 20, column: 11, scope: !35)
!44 = !DILocation(line: 20, column: 7, scope: !35)
!45 = !DILocation(line: 22, column: 25, scope: !35)
!46 = !DILocation(line: 22, column: 5, scope: !35)
!47 = !DILocation(line: 21, column: 1, scope: !35)
!48 = !DILocation(line: 22, column: 10, scope: !35)
!49 = !DILocation(line: 22, column: 28, scope: !35)
!50 = !DILocation(line: 23, column: 9, scope: !35)
!51 = !DILocation(line: 23, column: 22, scope: !35)
!52 = !DILocation(line: 23, column: 32, scope: !35)
!53 = distinct !{!53, !47, !54}
!54 = !DILocation(line: 21, column: 67, scope: !35)
!55 = !DILocation(line: 26, column: 1, scope: !35)
!56 = distinct !DISubprogram(name: "test3", scope: !1, file: !1, line: 28, type: !13, scopeLine: 28, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!57 = !DILocation(line: 31, column: 23, scope: !56)
!58 = !DILocation(line: 31, column: 3, scope: !56)
!59 = !DILocation(line: 31, column: 8, scope: !56)
!60 = !DILocation(line: 30, column: 1, scope: !56)
!61 = !DILocation(line: 31, column: 26, scope: !56)
!62 = !DILocation(line: 33, column: 25, scope: !56)
!63 = !DILocation(line: 33, column: 5, scope: !56)
!64 = !DILocation(line: 32, column: 1, scope: !56)
!65 = !DILocation(line: 33, column: 10, scope: !56)
!66 = !DILocation(line: 33, column: 28, scope: !56)
!67 = distinct !{!67, !64, !68}
!68 = !DILocation(line: 32, column: 41, scope: !56)
!69 = !DILocation(line: 35, column: 25, scope: !56)
!70 = !DILocation(line: 35, column: 5, scope: !56)
!71 = !DILocation(line: 34, column: 1, scope: !56)
!72 = !DILocation(line: 35, column: 10, scope: !56)
!73 = !DILocation(line: 35, column: 28, scope: !56)
!74 = distinct !{!74, !71, !75}
!75 = !DILocation(line: 34, column: 41, scope: !56)
!76 = distinct !{!76, !60, !77}
!77 = !DILocation(line: 30, column: 36, scope: !56)
!78 = !DILocation(line: 38, column: 23, scope: !56)
!79 = !DILocation(line: 38, column: 3, scope: !56)
!80 = !DILocation(line: 38, column: 8, scope: !56)
!81 = !DILocation(line: 37, column: 1, scope: !56)
!82 = !DILocation(line: 38, column: 26, scope: !56)
!83 = !DILocation(line: 40, column: 25, scope: !56)
!84 = !DILocation(line: 40, column: 5, scope: !56)
!85 = !DILocation(line: 39, column: 1, scope: !56)
!86 = !DILocation(line: 40, column: 10, scope: !56)
!87 = !DILocation(line: 40, column: 28, scope: !56)
!88 = distinct !{!88, !85, !89}
!89 = !DILocation(line: 39, column: 35, scope: !56)
!90 = !DILocation(line: 42, column: 25, scope: !56)
!91 = !DILocation(line: 42, column: 5, scope: !56)
!92 = !DILocation(line: 41, column: 1, scope: !56)
!93 = !DILocation(line: 42, column: 10, scope: !56)
!94 = !DILocation(line: 42, column: 28, scope: !56)
!95 = distinct !{!95, !92, !96}
!96 = !DILocation(line: 41, column: 35, scope: !56)
!97 = distinct !{!97, !81, !98}
!98 = !DILocation(line: 37, column: 36, scope: !56)
!99 = !DILocation(line: 44, column: 1, scope: !56)
