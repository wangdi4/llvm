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

; CHECK:      Global optimization report for : __omp_offloading_10309_2fafac9__Z5test1_l3{{[[:space:]]}}
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

; CHECK:      Global optimization report for : __omp_offloading_10309_2fafac9__Z5test2_l17{{[[:space:]]}}
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

; CHECK:      Global optimization report for : __omp_offloading_10309_2fafac9__Z5test3_l30{{[[:space:]]}}
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

; CHECK:      Global optimization report for : __omp_offloading_10309_2fafac9__Z5test3_l37{{[[:space:]]}}
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

; CHECK: define internal void @__omp_offloading_10309_2fafac9__Z5test1_l3({{.+}} !intel.optreport.rootnode !{{[0-9]+}}
; CHECK: define internal void @__omp_offloading_10309_2fafac9__Z5test2_l17({{.+}} !intel.optreport.rootnode !{{[0-9]+}}
; CHECK: define internal void @__omp_offloading_10309_2fafac9__Z5test3_l30({{.+}} !intel.optreport.rootnode !{{[0-9]+}}
; CHECK: define internal void @__omp_offloading_10309_2fafac9__Z5test3_l37({{.+}} !intel.optreport.rootnode !{{[0-9]+}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@0 = private unnamed_addr constant [17 x i8] c";E;test.c;14;9;;\00", align 1
@1 = private unnamed_addr constant [17 x i8] c";C;test.c;12;9;;\00", align 1
@2 = private unnamed_addr constant [17 x i8] c";D;test.c;13;9;;\00", align 1
@3 = private unnamed_addr constant [17 x i8] c";F;test.c;15;7;;\00", align 1
@4 = private unnamed_addr constant [18 x i8] c";H;test.c;15;13;;\00", align 1
@5 = private unnamed_addr constant [18 x i8] c";G;test.c;15;10;;\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @test1(i32 noundef %X, i32 noundef %Y) #0 !dbg !13 {
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
  store i32 %X, ptr %X.addr, align 4
  call void @llvm.dbg.declare(metadata ptr %X.addr, metadata !18, metadata !DIExpression()), !dbg !19
  store i32 %Y, ptr %Y.addr, align 4
  call void @llvm.dbg.declare(metadata ptr %Y.addr, metadata !20, metadata !DIExpression()), !dbg !21
  call void @llvm.dbg.declare(metadata ptr %A, metadata !22, metadata !DIExpression()), !dbg !23
  call void @llvm.dbg.declare(metadata ptr %B, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.2, metadata !26, metadata !DIExpression()), !dbg !28
  %0 = load i32, ptr %X.addr, align 4, !dbg !29
  store i32 %0, ptr %.capture_expr.2, align 4, !dbg !29
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.3, metadata !30, metadata !DIExpression()), !dbg !28
  %1 = load i32, ptr %.capture_expr.2, align 4, !dbg !29
  %sub = sub nsw i32 %1, 0, !dbg !31
  %sub1 = sub nsw i32 %sub, 1, !dbg !31
  %add = add nsw i32 %sub1, 1, !dbg !31
  %div = sdiv i32 %add, 1, !dbg !31
  %sub2 = sub nsw i32 %div, 1, !dbg !31
  store i32 %sub2, ptr %.capture_expr.3, align 4, !dbg !31
  call void @llvm.dbg.declare(metadata ptr %.omp.lb, metadata !32, metadata !DIExpression()), !dbg !28
  store i32 0, ptr %.omp.lb, align 4, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %.omp.ub, metadata !34, metadata !DIExpression()), !dbg !28
  %2 = load i32, ptr %.capture_expr.3, align 4, !dbg !31
  store i32 %2, ptr %.omp.ub, align 4, !dbg !33
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %Y.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %A, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %B, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.capture_expr.2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.0, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1) ], !dbg !35

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %Y.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %A, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %B, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.0, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1) ], !dbg !36

  %5 = load i32, ptr %.capture_expr.2, align 4, !dbg !38
  %cmp = icmp slt i32 0, %5, !dbg !40
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end27, !dbg !41

omp.precond.then:                                 ; preds = %entry
  call void @llvm.dbg.declare(metadata ptr %.omp.iv, metadata !42, metadata !DIExpression()), !dbg !43
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %A, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.0, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1) ], !dbg !44

  %7 = load i32, ptr %.omp.lb, align 4, !dbg !45
  store i32 %7, ptr %.omp.iv, align 4, !dbg !45
  br label %omp.inner.for.cond, !dbg !41

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc23, %omp.precond.then
  %8 = load i32, ptr %.omp.iv, align 4, !dbg !45
  %9 = load i32, ptr %.omp.ub, align 4, !dbg !45
  %cmp3 = icmp sle i32 %8, %9, !dbg !40
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end25, !dbg !41

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.dbg.declare(metadata ptr %i, metadata !46, metadata !DIExpression()), !dbg !47
  %10 = load i32, ptr %.omp.iv, align 4, !dbg !45
  %mul = mul nsw i32 %10, 1, !dbg !48
  %add4 = add nsw i32 0, %mul, !dbg !48
  store i32 %add4, ptr %i, align 4, !dbg !48
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %Y.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %B, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.0, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1) ], !dbg !49

  call void @llvm.dbg.declare(metadata ptr %.capture_expr.0, metadata !51, metadata !DIExpression()), !dbg !53
  %12 = load i32, ptr %Y.addr, align 4, !dbg !54
  store i32 %12, ptr %.capture_expr.0, align 4, !dbg !54
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.1, metadata !55, metadata !DIExpression()), !dbg !53
  %13 = load i32, ptr %.capture_expr.0, align 4, !dbg !54
  %sub6 = sub nsw i32 %13, 0, !dbg !56
  %sub7 = sub nsw i32 %sub6, 1, !dbg !56
  %add8 = add nsw i32 %sub7, 1, !dbg !56
  %div9 = sdiv i32 %add8, 1, !dbg !56
  %sub10 = sub nsw i32 %div9, 1, !dbg !56
  store i32 %sub10, ptr %.capture_expr.1, align 4, !dbg !56
  %14 = load i32, ptr %.capture_expr.0, align 4, !dbg !54
  %cmp11 = icmp slt i32 0, %14, !dbg !56
  br i1 %cmp11, label %omp.precond.then12, label %omp.precond.end, !dbg !57

omp.precond.then12:                               ; preds = %omp.inner.for.body
  call void @llvm.dbg.declare(metadata ptr %.omp.iv13, metadata !58, metadata !DIExpression()), !dbg !53
  call void @llvm.dbg.declare(metadata ptr %.omp.lb14, metadata !59, metadata !DIExpression()), !dbg !53
  store i32 0, ptr %.omp.lb14, align 4, !dbg !60
  call void @llvm.dbg.declare(metadata ptr %.omp.ub15, metadata !61, metadata !DIExpression()), !dbg !53
  %15 = load i32, ptr %.capture_expr.1, align 4, !dbg !56
  store i32 %15, ptr %.omp.ub15, align 4, !dbg !60
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %B, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv13, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub15, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ], !dbg !62

  %17 = load i32, ptr %.omp.lb14, align 4, !dbg !60
  store i32 %17, ptr %.omp.iv13, align 4, !dbg !60
  br label %omp.inner.for.cond16, !dbg !57

omp.inner.for.cond16:                             ; preds = %omp.inner.for.inc, %omp.precond.then12
  %18 = load i32, ptr %.omp.iv13, align 4, !dbg !60
  %19 = load i32, ptr %.omp.ub15, align 4, !dbg !60
  %cmp17 = icmp sle i32 %18, %19, !dbg !56
  br i1 %cmp17, label %omp.inner.for.body18, label %omp.inner.for.end, !dbg !57

omp.inner.for.body18:                             ; preds = %omp.inner.for.cond16
  call void @llvm.dbg.declare(metadata ptr %j, metadata !63, metadata !DIExpression()), !dbg !64
  %20 = load i32, ptr %.omp.iv13, align 4, !dbg !60
  %mul19 = mul nsw i32 %20, 1, !dbg !65
  %add20 = add nsw i32 0, %mul19, !dbg !65
  store i32 %add20, ptr %j, align 4, !dbg !65
  br label %omp.body.continue, !dbg !66

omp.body.continue:                                ; preds = %omp.inner.for.body18
  br label %omp.inner.for.inc, !dbg !66

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %21 = load i32, ptr %.omp.iv13, align 4, !dbg !60
  %add21 = add nsw i32 %21, 1, !dbg !56
  store i32 %add21, ptr %.omp.iv13, align 4, !dbg !56
  br label %omp.inner.for.cond16, !dbg !66, !llvm.loop !68

omp.inner.for.end:                                ; preds = %omp.inner.for.cond16
  br label %omp.loop.exit, !dbg !66

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.LOOP"() ], !dbg !62

  br label %omp.precond.end, !dbg !62

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.PARALLEL"() ], !dbg !49

  br label %omp.body.continue22, !dbg !70

omp.body.continue22:                              ; preds = %omp.precond.end
  br label %omp.inner.for.inc23, !dbg !70

omp.inner.for.inc23:                              ; preds = %omp.body.continue22
  %22 = load i32, ptr %.omp.iv, align 4, !dbg !45
  %add24 = add nsw i32 %22, 1, !dbg !40
  store i32 %add24, ptr %.omp.iv, align 4, !dbg !40
  br label %omp.inner.for.cond, !dbg !70, !llvm.loop !71

omp.inner.for.end25:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit26, !dbg !70

omp.loop.exit26:                                  ; preds = %omp.inner.for.end25
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.DISTRIBUTE"() ], !dbg !44

  br label %omp.precond.end27, !dbg !44

omp.precond.end27:                                ; preds = %omp.loop.exit26, %entry
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ], !dbg !36

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ], !dbg !35

  ret void, !dbg !73
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nounwind uwtable
define dso_local void @test2(i32 noundef %X) #0 !dbg !74 {
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
  store i32 %X, ptr %X.addr, align 4
  call void @llvm.dbg.declare(metadata ptr %X.addr, metadata !77, metadata !DIExpression()), !dbg !78
  call void @llvm.dbg.declare(metadata ptr %C, metadata !79, metadata !DIExpression()), !dbg !81
  store float 1.000000e+01, ptr %C, align 4, !dbg !81
  call void @llvm.dbg.declare(metadata ptr %D, metadata !82, metadata !DIExpression()), !dbg !83
  store float 1.100000e+01, ptr %D, align 4, !dbg !83
  call void @llvm.dbg.declare(metadata ptr %E, metadata !84, metadata !DIExpression()), !dbg !85
  call void @llvm.dbg.declare(metadata ptr %F, metadata !86, metadata !DIExpression()), !dbg !87
  call void @llvm.dbg.declare(metadata ptr %G, metadata !88, metadata !DIExpression()), !dbg !89
  call void @llvm.dbg.declare(metadata ptr %H, metadata !90, metadata !DIExpression()), !dbg !91
  call void @llvm.dbg.declare(metadata ptr %I, metadata !92, metadata !DIExpression()), !dbg !93
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.MAP.FROM"(ptr %E, ptr %E, i64 4, i64 34, ptr @0, ptr null), ; MAP type: 34 = 0x22 = TARGET_PARAM (0x20) | FROM (0x2)
    "QUAL.OMP.MAP.TO"(ptr %C, ptr %C, i64 4, i64 33, ptr @1, ptr null), ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %D, ptr %D, i64 4, i64 35, ptr @2, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.FROM"(ptr %F, ptr %F, i64 4, i64 34, ptr @3, ptr null), ; MAP type: 34 = 0x22 = TARGET_PARAM (0x20) | FROM (0x2)
    "QUAL.OMP.MAP.TOFROM"(ptr %H, ptr %H, i64 4, i64 35, ptr @4, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TO"(ptr %G, ptr %G, i64 4, i64 1, ptr @5, ptr null), ; MAP type: 1 = 0x1 = TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %X.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %I, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ], !dbg !94

  %1 = load float, ptr %C, align 4, !dbg !96
  %2 = load float, ptr %D, align 4, !dbg !98
  %add = fadd fast float %1, %2, !dbg !99
  store float %add, ptr %E, align 4, !dbg !100
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.4, metadata !101, metadata !DIExpression()), !dbg !103
  %3 = load i32, ptr %X.addr, align 4, !dbg !104
  store i32 %3, ptr %.capture_expr.4, align 4, !dbg !104
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.5, metadata !105, metadata !DIExpression()), !dbg !103
  %4 = load i32, ptr %.capture_expr.4, align 4, !dbg !104
  %sub = sub nsw i32 %4, 0, !dbg !106
  %sub1 = sub nsw i32 %sub, 1, !dbg !106
  %add2 = add nsw i32 %sub1, 1, !dbg !106
  %div = sdiv i32 %add2, 1, !dbg !106
  %sub3 = sub nsw i32 %div, 1, !dbg !106
  store i32 %sub3, ptr %.capture_expr.5, align 4, !dbg !106
  %5 = load i32, ptr %.capture_expr.4, align 4, !dbg !104
  %cmp = icmp slt i32 0, %5, !dbg !106
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end, !dbg !107

omp.precond.then:                                 ; preds = %entry
  call void @llvm.dbg.declare(metadata ptr %.omp.iv, metadata !108, metadata !DIExpression()), !dbg !103
  call void @llvm.dbg.declare(metadata ptr %.omp.lb, metadata !109, metadata !DIExpression()), !dbg !103
  store i32 0, ptr %.omp.lb, align 4, !dbg !110
  call void @llvm.dbg.declare(metadata ptr %.omp.ub, metadata !111, metadata !DIExpression()), !dbg !103
  %6 = load i32, ptr %.capture_expr.5, align 4, !dbg !106
  store i32 %6, ptr %.omp.ub, align 4, !dbg !110
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %F, i32 0, i32 1),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %F, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %G, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %H, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %I, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ], !dbg !112

  %8 = load i32, ptr %.omp.lb, align 4, !dbg !110
  store i32 %8, ptr %.omp.iv, align 4, !dbg !110
  br label %omp.inner.for.cond, !dbg !107

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %9 = load i32, ptr %.omp.iv, align 4, !dbg !110
  %10 = load i32, ptr %.omp.ub, align 4, !dbg !110
  %cmp4 = icmp sle i32 %9, %10, !dbg !106
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !107

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.dbg.declare(metadata ptr %i, metadata !113, metadata !DIExpression()), !dbg !114
  %11 = load i32, ptr %.omp.iv, align 4, !dbg !110
  %mul = mul nsw i32 %11, 1, !dbg !115
  %add5 = add nsw i32 0, %mul, !dbg !115
  store i32 %add5, ptr %i, align 4, !dbg !115
  store i32 10, ptr %G, align 4, !dbg !116
  %12 = load i32, ptr %H, align 4, !dbg !118
  %13 = load i32, ptr %I, align 4, !dbg !119
  br label %omp.body.continue, !dbg !120

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !120

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, ptr %.omp.iv, align 4, !dbg !110
  %add6 = add nsw i32 %14, 1, !dbg !106
  store i32 %add6, ptr %.omp.iv, align 4, !dbg !106
  br label %omp.inner.for.cond, !dbg !120, !llvm.loop !121

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !120

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !112

  br label %omp.precond.end, !dbg !112

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !94

  ret void, !dbg !123
}

; Function Attrs: nounwind uwtable
define dso_local void @test3(i32 noundef %X, i32 noundef %Y) #0 !dbg !124 {
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
  store i32 %X, ptr %X.addr, align 4
  call void @llvm.dbg.declare(metadata ptr %X.addr, metadata !125, metadata !DIExpression()), !dbg !126
  store i32 %Y, ptr %Y.addr, align 4
  call void @llvm.dbg.declare(metadata ptr %Y.addr, metadata !127, metadata !DIExpression()), !dbg !128
  call void @llvm.dbg.declare(metadata ptr %J, metadata !129, metadata !DIExpression()), !dbg !130
  call void @llvm.dbg.declare(metadata ptr %K, metadata !131, metadata !DIExpression()), !dbg !132
  call void @llvm.dbg.declare(metadata ptr %L, metadata !133, metadata !DIExpression()), !dbg !134
  call void @llvm.dbg.declare(metadata ptr %M, metadata !135, metadata !DIExpression()), !dbg !136
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.10, metadata !137, metadata !DIExpression()), !dbg !139
  %0 = load i32, ptr %X.addr, align 4, !dbg !140
  store i32 %0, ptr %.capture_expr.10, align 4, !dbg !140
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.11, metadata !141, metadata !DIExpression()), !dbg !139
  %1 = load i32, ptr %.capture_expr.10, align 4, !dbg !140
  %sub = sub nsw i32 %1, 0, !dbg !142
  %sub1 = sub nsw i32 %sub, 1, !dbg !142
  %add = add nsw i32 %sub1, 1, !dbg !142
  %div = sdiv i32 %add, 1, !dbg !142
  %sub2 = sub nsw i32 %div, 1, !dbg !142
  store i32 %sub2, ptr %.capture_expr.11, align 4, !dbg !142
  call void @llvm.dbg.declare(metadata ptr %.omp.lb, metadata !143, metadata !DIExpression()), !dbg !139
  store i32 0, ptr %.omp.lb, align 4, !dbg !144
  call void @llvm.dbg.declare(metadata ptr %.omp.ub, metadata !145, metadata !DIExpression()), !dbg !139
  %2 = load i32, ptr %.capture_expr.11, align 4, !dbg !142
  store i32 %2, ptr %.omp.ub, align 4, !dbg !144
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %Y.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %J, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %K, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.capture_expr.10, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.7, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.6, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.9, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv30, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb31, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub32, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j36, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.8, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp22, i32 0, i32 1) ], !dbg !146

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %Y.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %J, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %K, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.10, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.7, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.6, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.9, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv30, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb31, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub32, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j36, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.8, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp22, i32 0, i32 1) ], !dbg !147

  %5 = load i32, ptr %.capture_expr.10, align 4, !dbg !149
  %cmp = icmp slt i32 0, %5, !dbg !151
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end50, !dbg !147

omp.precond.then:                                 ; preds = %entry
  call void @llvm.dbg.declare(metadata ptr %.omp.iv, metadata !152, metadata !DIExpression()), !dbg !153
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.7, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.6, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.9, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv30, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb31, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub32, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j36, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.8, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp22, i32 0, i32 1) ], !dbg !154

  %7 = load i32, ptr %.omp.lb, align 4, !dbg !155
  store i32 %7, ptr %.omp.iv, align 4, !dbg !155
  br label %omp.inner.for.cond, !dbg !147

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc46, %omp.precond.then
  %8 = load i32, ptr %.omp.iv, align 4, !dbg !155
  %9 = load i32, ptr %.omp.ub, align 4, !dbg !155
  %cmp3 = icmp sle i32 %8, %9, !dbg !151
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end48, !dbg !147

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.dbg.declare(metadata ptr %i, metadata !156, metadata !DIExpression()), !dbg !157
  %10 = load i32, ptr %.omp.iv, align 4, !dbg !155
  %mul = mul nsw i32 %10, 1, !dbg !158
  %add4 = add nsw i32 0, %mul, !dbg !158
  store i32 %add4, ptr %i, align 4, !dbg !158
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.6, metadata !159, metadata !DIExpression()), !dbg !162
  %11 = load i32, ptr %Y.addr, align 4, !dbg !163
  store i32 %11, ptr %.capture_expr.6, align 4, !dbg !163
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.7, metadata !164, metadata !DIExpression()), !dbg !162
  %12 = load i32, ptr %.capture_expr.6, align 4, !dbg !163
  %sub6 = sub nsw i32 %12, 0, !dbg !165
  %sub7 = sub nsw i32 %sub6, 1, !dbg !165
  %add8 = add nsw i32 %sub7, 1, !dbg !165
  %div9 = sdiv i32 %add8, 1, !dbg !165
  %sub10 = sub nsw i32 %div9, 1, !dbg !165
  store i32 %sub10, ptr %.capture_expr.7, align 4, !dbg !165
  %13 = load i32, ptr %.capture_expr.6, align 4, !dbg !163
  %cmp11 = icmp slt i32 0, %13, !dbg !165
  br i1 %cmp11, label %omp.precond.then12, label %omp.precond.end, !dbg !166

omp.precond.then12:                               ; preds = %omp.inner.for.body
  call void @llvm.dbg.declare(metadata ptr %.omp.iv13, metadata !167, metadata !DIExpression()), !dbg !162
  call void @llvm.dbg.declare(metadata ptr %.omp.lb14, metadata !168, metadata !DIExpression()), !dbg !162
  store i32 0, ptr %.omp.lb14, align 4, !dbg !169
  call void @llvm.dbg.declare(metadata ptr %.omp.ub15, metadata !170, metadata !DIExpression()), !dbg !162
  %14 = load i32, ptr %.capture_expr.7, align 4, !dbg !165
  store i32 %14, ptr %.omp.ub15, align 4, !dbg !169
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %J, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv13, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub15, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ], !dbg !171

  %16 = load i32, ptr %.omp.lb14, align 4, !dbg !169
  store i32 %16, ptr %.omp.iv13, align 4, !dbg !169
  br label %omp.inner.for.cond16, !dbg !166

omp.inner.for.cond16:                             ; preds = %omp.inner.for.inc, %omp.precond.then12
  %17 = load i32, ptr %.omp.iv13, align 4, !dbg !169
  %18 = load i32, ptr %.omp.ub15, align 4, !dbg !169
  %cmp17 = icmp sle i32 %17, %18, !dbg !165
  br i1 %cmp17, label %omp.inner.for.body18, label %omp.inner.for.end, !dbg !166

omp.inner.for.body18:                             ; preds = %omp.inner.for.cond16
  call void @llvm.dbg.declare(metadata ptr %j, metadata !172, metadata !DIExpression()), !dbg !173
  %19 = load i32, ptr %.omp.iv13, align 4, !dbg !169
  %mul19 = mul nsw i32 %19, 1, !dbg !174
  %add20 = add nsw i32 0, %mul19, !dbg !174
  store i32 %add20, ptr %j, align 4, !dbg !174
  br label %omp.body.continue, !dbg !175

omp.body.continue:                                ; preds = %omp.inner.for.body18
  br label %omp.inner.for.inc, !dbg !175

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %20 = load i32, ptr %.omp.iv13, align 4, !dbg !169
  %add21 = add nsw i32 %20, 1, !dbg !165
  store i32 %add21, ptr %.omp.iv13, align 4, !dbg !165
  br label %omp.inner.for.cond16, !dbg !175, !llvm.loop !177

omp.inner.for.end:                                ; preds = %omp.inner.for.cond16
  br label %omp.loop.exit, !dbg !175

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !171

  br label %omp.precond.end, !dbg !171

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.8, metadata !179, metadata !DIExpression()), !dbg !181
  %21 = load i32, ptr %Y.addr, align 4, !dbg !182
  store i32 %21, ptr %.capture_expr.8, align 4, !dbg !182
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.9, metadata !183, metadata !DIExpression()), !dbg !181
  %22 = load i32, ptr %.capture_expr.8, align 4, !dbg !182
  %sub23 = sub nsw i32 %22, 0, !dbg !184
  %sub24 = sub nsw i32 %sub23, 1, !dbg !184
  %add25 = add nsw i32 %sub24, 1, !dbg !184
  %div26 = sdiv i32 %add25, 1, !dbg !184
  %sub27 = sub nsw i32 %div26, 1, !dbg !184
  store i32 %sub27, ptr %.capture_expr.9, align 4, !dbg !184
  %23 = load i32, ptr %.capture_expr.8, align 4, !dbg !182
  %cmp28 = icmp slt i32 0, %23, !dbg !184
  br i1 %cmp28, label %omp.precond.then29, label %omp.precond.end44, !dbg !185

omp.precond.then29:                               ; preds = %omp.precond.end
  call void @llvm.dbg.declare(metadata ptr %.omp.iv30, metadata !186, metadata !DIExpression()), !dbg !181
  call void @llvm.dbg.declare(metadata ptr %.omp.lb31, metadata !187, metadata !DIExpression()), !dbg !181
  store i32 0, ptr %.omp.lb31, align 4, !dbg !188
  call void @llvm.dbg.declare(metadata ptr %.omp.ub32, metadata !189, metadata !DIExpression()), !dbg !181
  %24 = load i32, ptr %.capture_expr.9, align 4, !dbg !184
  store i32 %24, ptr %.omp.ub32, align 4, !dbg !188
  %25 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %K, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv30, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb31, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub32, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j36, i32 0, i32 1) ], !dbg !190

  %26 = load i32, ptr %.omp.lb31, align 4, !dbg !188
  store i32 %26, ptr %.omp.iv30, align 4, !dbg !188
  br label %omp.inner.for.cond33, !dbg !185

omp.inner.for.cond33:                             ; preds = %omp.inner.for.inc40, %omp.precond.then29
  %27 = load i32, ptr %.omp.iv30, align 4, !dbg !188
  %28 = load i32, ptr %.omp.ub32, align 4, !dbg !188
  %cmp34 = icmp sle i32 %27, %28, !dbg !184
  br i1 %cmp34, label %omp.inner.for.body35, label %omp.inner.for.end42, !dbg !185

omp.inner.for.body35:                             ; preds = %omp.inner.for.cond33
  call void @llvm.dbg.declare(metadata ptr %j36, metadata !191, metadata !DIExpression()), !dbg !192
  %29 = load i32, ptr %.omp.iv30, align 4, !dbg !188
  %mul37 = mul nsw i32 %29, 1, !dbg !193
  %add38 = add nsw i32 0, %mul37, !dbg !193
  store i32 %add38, ptr %j36, align 4, !dbg !193
  br label %omp.body.continue39, !dbg !194

omp.body.continue39:                              ; preds = %omp.inner.for.body35
  br label %omp.inner.for.inc40, !dbg !194

omp.inner.for.inc40:                              ; preds = %omp.body.continue39
  %30 = load i32, ptr %.omp.iv30, align 4, !dbg !188
  %add41 = add nsw i32 %30, 1, !dbg !184
  store i32 %add41, ptr %.omp.iv30, align 4, !dbg !184
  br label %omp.inner.for.cond33, !dbg !194, !llvm.loop !196

omp.inner.for.end42:                              ; preds = %omp.inner.for.cond33
  br label %omp.loop.exit43, !dbg !194

omp.loop.exit43:                                  ; preds = %omp.inner.for.end42
  call void @llvm.directive.region.exit(token %25) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !190

  br label %omp.precond.end44, !dbg !190

omp.precond.end44:                                ; preds = %omp.loop.exit43, %omp.precond.end
  br label %omp.body.continue45, !dbg !198

omp.body.continue45:                              ; preds = %omp.precond.end44
  br label %omp.inner.for.inc46, !dbg !198

omp.inner.for.inc46:                              ; preds = %omp.body.continue45
  %31 = load i32, ptr %.omp.iv, align 4, !dbg !155
  %add47 = add nsw i32 %31, 1, !dbg !151
  store i32 %add47, ptr %.omp.iv, align 4, !dbg !151
  br label %omp.inner.for.cond, !dbg !198, !llvm.loop !199

omp.inner.for.end48:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit49, !dbg !198

omp.loop.exit49:                                  ; preds = %omp.inner.for.end48
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.DISTRIBUTE"() ], !dbg !154

  br label %omp.precond.end50, !dbg !154

omp.precond.end50:                                ; preds = %omp.loop.exit49, %entry
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ], !dbg !147

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ], !dbg !146

  call void @llvm.dbg.declare(metadata ptr %.capture_expr.16, metadata !201, metadata !DIExpression()), !dbg !203
  %32 = load i32, ptr %X.addr, align 4, !dbg !204
  store i32 %32, ptr %.capture_expr.16, align 4, !dbg !204
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.17, metadata !205, metadata !DIExpression()), !dbg !203
  %33 = load i32, ptr %.capture_expr.16, align 4, !dbg !204
  %sub51 = sub nsw i32 %33, 0, !dbg !206
  %sub52 = sub nsw i32 %sub51, 1, !dbg !206
  %add53 = add nsw i32 %sub52, 1, !dbg !206
  %div54 = sdiv i32 %add53, 1, !dbg !206
  %sub55 = sub nsw i32 %div54, 1, !dbg !206
  store i32 %sub55, ptr %.capture_expr.17, align 4, !dbg !206
  call void @llvm.dbg.declare(metadata ptr %.omp.lb56, metadata !207, metadata !DIExpression()), !dbg !203
  store i32 0, ptr %.omp.lb56, align 4, !dbg !208
  call void @llvm.dbg.declare(metadata ptr %.omp.ub57, metadata !209, metadata !DIExpression()), !dbg !203
  %34 = load i32, ptr %.capture_expr.17, align 4, !dbg !206
  store i32 %34, ptr %.omp.ub57, align 4, !dbg !208
  %35 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %Y.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %L, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %M, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv61, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb56, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.ub57, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i65, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.capture_expr.16, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv76, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb77, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub78, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j82, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.12, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv99, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb100, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub101, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j105, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp58, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp68, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp91, i32 0, i32 1) ], !dbg !210

  %36 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %Y.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %L, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %M, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv61, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.lb56, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.ub57, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i65, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.16, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv76, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb77, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub78, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j82, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.12, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv99, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb100, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub101, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j105, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp58, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp68, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp91, i32 0, i32 1) ], !dbg !211

  %37 = load i32, ptr %.capture_expr.16, align 4, !dbg !213
  %cmp59 = icmp slt i32 0, %37, !dbg !215
  br i1 %cmp59, label %omp.precond.then60, label %omp.precond.end119, !dbg !211

omp.precond.then60:                               ; preds = %omp.precond.end50
  call void @llvm.dbg.declare(metadata ptr %.omp.iv61, metadata !216, metadata !DIExpression()), !dbg !217
  %38 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv61, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb56, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub57, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i65, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv76, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb77, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub78, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j82, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.12, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv99, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb100, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub101, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j105, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp68, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp91, i32 0, i32 1) ], !dbg !218

  %39 = load i32, ptr %.omp.lb56, align 4, !dbg !219
  store i32 %39, ptr %.omp.iv61, align 4, !dbg !219
  br label %omp.inner.for.cond62, !dbg !211

omp.inner.for.cond62:                             ; preds = %omp.inner.for.inc115, %omp.precond.then60
  %40 = load i32, ptr %.omp.iv61, align 4, !dbg !219
  %41 = load i32, ptr %.omp.ub57, align 4, !dbg !219
  %cmp63 = icmp sle i32 %40, %41, !dbg !215
  br i1 %cmp63, label %omp.inner.for.body64, label %omp.inner.for.end117, !dbg !211

omp.inner.for.body64:                             ; preds = %omp.inner.for.cond62
  call void @llvm.dbg.declare(metadata ptr %i65, metadata !220, metadata !DIExpression()), !dbg !221
  %42 = load i32, ptr %.omp.iv61, align 4, !dbg !219
  %mul66 = mul nsw i32 %42, 1, !dbg !222
  %add67 = add nsw i32 0, %mul66, !dbg !222
  store i32 %add67, ptr %i65, align 4, !dbg !222
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.12, metadata !223, metadata !DIExpression()), !dbg !226
  %43 = load i32, ptr %Y.addr, align 4, !dbg !227
  store i32 %43, ptr %.capture_expr.12, align 4, !dbg !227
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.13, metadata !228, metadata !DIExpression()), !dbg !226
  %44 = load i32, ptr %.capture_expr.12, align 4, !dbg !227
  %sub69 = sub nsw i32 %44, 0, !dbg !229
  %sub70 = sub nsw i32 %sub69, 1, !dbg !229
  %add71 = add nsw i32 %sub70, 1, !dbg !229
  %div72 = sdiv i32 %add71, 1, !dbg !229
  %sub73 = sub nsw i32 %div72, 1, !dbg !229
  store i32 %sub73, ptr %.capture_expr.13, align 4, !dbg !229
  %45 = load i32, ptr %.capture_expr.12, align 4, !dbg !227
  %cmp74 = icmp slt i32 0, %45, !dbg !229
  br i1 %cmp74, label %omp.precond.then75, label %omp.precond.end90, !dbg !230

omp.precond.then75:                               ; preds = %omp.inner.for.body64
  call void @llvm.dbg.declare(metadata ptr %.omp.iv76, metadata !231, metadata !DIExpression()), !dbg !226
  call void @llvm.dbg.declare(metadata ptr %.omp.lb77, metadata !232, metadata !DIExpression()), !dbg !226
  store i32 0, ptr %.omp.lb77, align 4, !dbg !233
  call void @llvm.dbg.declare(metadata ptr %.omp.ub78, metadata !234, metadata !DIExpression()), !dbg !226
  %46 = load i32, ptr %.capture_expr.13, align 4, !dbg !229
  store i32 %46, ptr %.omp.ub78, align 4, !dbg !233
  %47 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %L, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv76, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb77, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub78, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j82, i32 0, i32 1) ], !dbg !235

  %48 = load i32, ptr %.omp.lb77, align 4, !dbg !233
  store i32 %48, ptr %.omp.iv76, align 4, !dbg !233
  br label %omp.inner.for.cond79, !dbg !230

omp.inner.for.cond79:                             ; preds = %omp.inner.for.inc86, %omp.precond.then75
  %49 = load i32, ptr %.omp.iv76, align 4, !dbg !233
  %50 = load i32, ptr %.omp.ub78, align 4, !dbg !233
  %cmp80 = icmp sle i32 %49, %50, !dbg !229
  br i1 %cmp80, label %omp.inner.for.body81, label %omp.inner.for.end88, !dbg !230

omp.inner.for.body81:                             ; preds = %omp.inner.for.cond79
  call void @llvm.dbg.declare(metadata ptr %j82, metadata !236, metadata !DIExpression()), !dbg !237
  %51 = load i32, ptr %.omp.iv76, align 4, !dbg !233
  %mul83 = mul nsw i32 %51, 1, !dbg !238
  %add84 = add nsw i32 0, %mul83, !dbg !238
  store i32 %add84, ptr %j82, align 4, !dbg !238
  br label %omp.body.continue85, !dbg !239

omp.body.continue85:                              ; preds = %omp.inner.for.body81
  br label %omp.inner.for.inc86, !dbg !239

omp.inner.for.inc86:                              ; preds = %omp.body.continue85
  %52 = load i32, ptr %.omp.iv76, align 4, !dbg !233
  %add87 = add nsw i32 %52, 1, !dbg !229
  store i32 %add87, ptr %.omp.iv76, align 4, !dbg !229
  br label %omp.inner.for.cond79, !dbg !239, !llvm.loop !241

omp.inner.for.end88:                              ; preds = %omp.inner.for.cond79
  br label %omp.loop.exit89, !dbg !239

omp.loop.exit89:                                  ; preds = %omp.inner.for.end88
  call void @llvm.directive.region.exit(token %47) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !235

  br label %omp.precond.end90, !dbg !235

omp.precond.end90:                                ; preds = %omp.loop.exit89, %omp.inner.for.body64
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.14, metadata !243, metadata !DIExpression()), !dbg !245
  %53 = load i32, ptr %Y.addr, align 4, !dbg !246
  store i32 %53, ptr %.capture_expr.14, align 4, !dbg !246
  call void @llvm.dbg.declare(metadata ptr %.capture_expr.15, metadata !247, metadata !DIExpression()), !dbg !245
  %54 = load i32, ptr %.capture_expr.14, align 4, !dbg !246
  %sub92 = sub nsw i32 %54, 0, !dbg !248
  %sub93 = sub nsw i32 %sub92, 1, !dbg !248
  %add94 = add nsw i32 %sub93, 1, !dbg !248
  %div95 = sdiv i32 %add94, 1, !dbg !248
  %sub96 = sub nsw i32 %div95, 1, !dbg !248
  store i32 %sub96, ptr %.capture_expr.15, align 4, !dbg !248
  %55 = load i32, ptr %.capture_expr.14, align 4, !dbg !246
  %cmp97 = icmp slt i32 0, %55, !dbg !248
  br i1 %cmp97, label %omp.precond.then98, label %omp.precond.end113, !dbg !249

omp.precond.then98:                               ; preds = %omp.precond.end90
  call void @llvm.dbg.declare(metadata ptr %.omp.iv99, metadata !250, metadata !DIExpression()), !dbg !245
  call void @llvm.dbg.declare(metadata ptr %.omp.lb100, metadata !251, metadata !DIExpression()), !dbg !245
  store i32 0, ptr %.omp.lb100, align 4, !dbg !252
  call void @llvm.dbg.declare(metadata ptr %.omp.ub101, metadata !253, metadata !DIExpression()), !dbg !245
  %56 = load i32, ptr %.capture_expr.15, align 4, !dbg !248
  store i32 %56, ptr %.omp.ub101, align 4, !dbg !252
  %57 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %M, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv99, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb100, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub101, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j105, i32 0, i32 1) ], !dbg !254

  %58 = load i32, ptr %.omp.lb100, align 4, !dbg !252
  store i32 %58, ptr %.omp.iv99, align 4, !dbg !252
  br label %omp.inner.for.cond102, !dbg !249

omp.inner.for.cond102:                            ; preds = %omp.inner.for.inc109, %omp.precond.then98
  %59 = load i32, ptr %.omp.iv99, align 4, !dbg !252
  %60 = load i32, ptr %.omp.ub101, align 4, !dbg !252
  %cmp103 = icmp sle i32 %59, %60, !dbg !248
  br i1 %cmp103, label %omp.inner.for.body104, label %omp.inner.for.end111, !dbg !249

omp.inner.for.body104:                            ; preds = %omp.inner.for.cond102
  call void @llvm.dbg.declare(metadata ptr %j105, metadata !255, metadata !DIExpression()), !dbg !256
  %61 = load i32, ptr %.omp.iv99, align 4, !dbg !252
  %mul106 = mul nsw i32 %61, 1, !dbg !257
  %add107 = add nsw i32 0, %mul106, !dbg !257
  store i32 %add107, ptr %j105, align 4, !dbg !257
  br label %omp.body.continue108, !dbg !258

omp.body.continue108:                             ; preds = %omp.inner.for.body104
  br label %omp.inner.for.inc109, !dbg !258

omp.inner.for.inc109:                             ; preds = %omp.body.continue108
  %62 = load i32, ptr %.omp.iv99, align 4, !dbg !252
  %add110 = add nsw i32 %62, 1, !dbg !248
  store i32 %add110, ptr %.omp.iv99, align 4, !dbg !248
  br label %omp.inner.for.cond102, !dbg !258, !llvm.loop !260

omp.inner.for.end111:                             ; preds = %omp.inner.for.cond102
  br label %omp.loop.exit112, !dbg !258

omp.loop.exit112:                                 ; preds = %omp.inner.for.end111
  call void @llvm.directive.region.exit(token %57) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !254

  br label %omp.precond.end113, !dbg !254

omp.precond.end113:                               ; preds = %omp.loop.exit112, %omp.precond.end90
  br label %omp.body.continue114, !dbg !262

omp.body.continue114:                             ; preds = %omp.precond.end113
  br label %omp.inner.for.inc115, !dbg !262

omp.inner.for.inc115:                             ; preds = %omp.body.continue114
  %63 = load i32, ptr %.omp.iv61, align 4, !dbg !219
  %add116 = add nsw i32 %63, 1, !dbg !215
  store i32 %add116, ptr %.omp.iv61, align 4, !dbg !215
  br label %omp.inner.for.cond62, !dbg !262, !llvm.loop !263

omp.inner.for.end117:                             ; preds = %omp.inner.for.cond62
  br label %omp.loop.exit118, !dbg !262

omp.loop.exit118:                                 ; preds = %omp.inner.for.end117
  call void @llvm.directive.region.exit(token %38) [ "DIR.OMP.END.DISTRIBUTE"() ], !dbg !218

  br label %omp.precond.end119, !dbg !218

omp.precond.end119:                               ; preds = %omp.loop.exit118, %omp.precond.end50
  call void @llvm.directive.region.exit(token %36) [ "DIR.OMP.END.TEAMS"() ], !dbg !211

  call void @llvm.directive.region.exit(token %35) [ "DIR.OMP.END.TARGET"() ], !dbg !210

  ret void, !dbg !265
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!2, !3, !4, !5}
!llvm.module.flags = !{!6, !7, !8, !9, !10, !11}
!llvm.ident = !{!12}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)", isOptimized: false, flags: "-g -fopenmp-version=51 -fopenmp-targets=spir64 -Xclang -fopenmp-typed-clauses -Xclang -opaque-pointers -O0 -fiopenmp -Xclang -disable-llvm-passes -mllvm -pretty-print-directives -emit-llvm -S test.c -o cfe.ll", runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "/tmp")
!2 = !{i32 0, i32 66313, i32 50002633, !"_Z5test2", i32 17, i32 1, i32 0}
!3 = !{i32 0, i32 66313, i32 50002633, !"_Z5test3", i32 30, i32 2, i32 0}
!4 = !{i32 0, i32 66313, i32 50002633, !"_Z5test3", i32 37, i32 3, i32 0}
!5 = !{i32 0, i32 66313, i32 50002633, !"_Z5test1", i32 3, i32 0, i32 0}
!6 = !{i32 7, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{i32 7, !"openmp", i32 51}
!10 = !{i32 7, !"uwtable", i32 2}
!11 = !{i32 7, !"frame-pointer", i32 2}
!12 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!13 = distinct !DISubprogram(name: "test1", scope: !1, file: !1, line: 1, type: !14, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !17)
!14 = !DISubroutineType(types: !15)
!15 = !{null, !16, !16}
!16 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!17 = !{}
!18 = !DILocalVariable(name: "X", arg: 1, scope: !13, file: !1, line: 1, type: !16)
!19 = !DILocation(line: 1, column: 16, scope: !13)
!20 = !DILocalVariable(name: "Y", arg: 2, scope: !13, file: !1, line: 1, type: !16)
!21 = !DILocation(line: 1, column: 23, scope: !13)
!22 = !DILocalVariable(name: "A", scope: !13, file: !1, line: 2, type: !16)
!23 = !DILocation(line: 2, column: 7, scope: !13)
!24 = !DILocalVariable(name: "B", scope: !13, file: !1, line: 2, type: !16)
!25 = !DILocation(line: 2, column: 10, scope: !13)
!26 = !DILocalVariable(name: ".capture_expr.2", scope: !27, type: !16, flags: DIFlagArtificial)
!27 = distinct !DILexicalBlock(scope: !13, file: !1, line: 3, column: 1)
!28 = !DILocation(line: 0, scope: !27)
!29 = !DILocation(line: 5, column: 23, scope: !27)
!30 = !DILocalVariable(name: ".capture_expr.3", scope: !27, type: !16, flags: DIFlagArtificial)
!31 = !DILocation(line: 5, column: 3, scope: !27)
!32 = !DILocalVariable(name: ".omp.lb", scope: !27, type: !16, flags: DIFlagArtificial)
!33 = !DILocation(line: 5, column: 8, scope: !27)
!34 = !DILocalVariable(name: ".omp.ub", scope: !27, type: !16, flags: DIFlagArtificial)
!35 = !DILocation(line: 3, column: 1, scope: !27)
!36 = !DILocation(line: 3, column: 1, scope: !37)
!37 = distinct !DILexicalBlock(scope: !27, file: !1, line: 3, column: 1)
!38 = !DILocation(line: 5, column: 23, scope: !39)
!39 = distinct !DILexicalBlock(scope: !37, file: !1, line: 4, column: 1)
!40 = !DILocation(line: 5, column: 3, scope: !39)
!41 = !DILocation(line: 4, column: 1, scope: !37)
!42 = !DILocalVariable(name: ".omp.iv", scope: !39, type: !16, flags: DIFlagArtificial)
!43 = !DILocation(line: 0, scope: !39)
!44 = !DILocation(line: 4, column: 1, scope: !39)
!45 = !DILocation(line: 5, column: 8, scope: !39)
!46 = !DILocalVariable(name: "i", scope: !39, file: !1, line: 5, type: !16)
!47 = !DILocation(line: 5, column: 12, scope: !39)
!48 = !DILocation(line: 5, column: 26, scope: !39)
!49 = !DILocation(line: 6, column: 1, scope: !50)
!50 = distinct !DILexicalBlock(scope: !39, file: !1, line: 6, column: 1)
!51 = !DILocalVariable(name: ".capture_expr.0", scope: !52, type: !16, flags: DIFlagArtificial)
!52 = distinct !DILexicalBlock(scope: !50, file: !1, line: 7, column: 1)
!53 = !DILocation(line: 0, scope: !52)
!54 = !DILocation(line: 8, column: 25, scope: !52)
!55 = !DILocalVariable(name: ".capture_expr.1", scope: !52, type: !16, flags: DIFlagArtificial)
!56 = !DILocation(line: 8, column: 5, scope: !52)
!57 = !DILocation(line: 7, column: 1, scope: !50)
!58 = !DILocalVariable(name: ".omp.iv", scope: !52, type: !16, flags: DIFlagArtificial)
!59 = !DILocalVariable(name: ".omp.lb", scope: !52, type: !16, flags: DIFlagArtificial)
!60 = !DILocation(line: 8, column: 10, scope: !52)
!61 = !DILocalVariable(name: ".omp.ub", scope: !52, type: !16, flags: DIFlagArtificial)
!62 = !DILocation(line: 7, column: 1, scope: !52)
!63 = !DILocalVariable(name: "j", scope: !52, file: !1, line: 8, type: !16)
!64 = !DILocation(line: 8, column: 14, scope: !52)
!65 = !DILocation(line: 8, column: 28, scope: !52)
!66 = !DILocation(line: 8, column: 34, scope: !67)
!67 = distinct !DILexicalBlock(scope: !52, file: !1, line: 8, column: 33)
!68 = distinct !{!68, !62, !69}
!69 = !DILocation(line: 7, column: 32, scope: !52)
!70 = !DILocation(line: 6, column: 21, scope: !50)
!71 = distinct !{!71, !44, !72}
!72 = !DILocation(line: 4, column: 39, scope: !39)
!73 = !DILocation(line: 9, column: 1, scope: !13)
!74 = distinct !DISubprogram(name: "test2", scope: !1, file: !1, line: 11, type: !75, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !17)
!75 = !DISubroutineType(types: !76)
!76 = !{null, !16}
!77 = !DILocalVariable(name: "X", arg: 1, scope: !74, file: !1, line: 11, type: !16)
!78 = !DILocation(line: 11, column: 16, scope: !74)
!79 = !DILocalVariable(name: "C", scope: !74, file: !1, line: 12, type: !80)
!80 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!81 = !DILocation(line: 12, column: 9, scope: !74)
!82 = !DILocalVariable(name: "D", scope: !74, file: !1, line: 13, type: !80)
!83 = !DILocation(line: 13, column: 9, scope: !74)
!84 = !DILocalVariable(name: "E", scope: !74, file: !1, line: 14, type: !80)
!85 = !DILocation(line: 14, column: 9, scope: !74)
!86 = !DILocalVariable(name: "F", scope: !74, file: !1, line: 15, type: !16)
!87 = !DILocation(line: 15, column: 7, scope: !74)
!88 = !DILocalVariable(name: "G", scope: !74, file: !1, line: 15, type: !16)
!89 = !DILocation(line: 15, column: 10, scope: !74)
!90 = !DILocalVariable(name: "H", scope: !74, file: !1, line: 15, type: !16)
!91 = !DILocation(line: 15, column: 13, scope: !74)
!92 = !DILocalVariable(name: "I", scope: !74, file: !1, line: 15, type: !16)
!93 = !DILocation(line: 15, column: 16, scope: !74)
!94 = !DILocation(line: 17, column: 1, scope: !95)
!95 = distinct !DILexicalBlock(scope: !74, file: !1, line: 17, column: 1)
!96 = !DILocation(line: 20, column: 9, scope: !97)
!97 = distinct !DILexicalBlock(scope: !95, file: !1, line: 19, column: 3)
!98 = !DILocation(line: 20, column: 13, scope: !97)
!99 = !DILocation(line: 20, column: 11, scope: !97)
!100 = !DILocation(line: 20, column: 7, scope: !97)
!101 = !DILocalVariable(name: ".capture_expr.4", scope: !102, type: !16, flags: DIFlagArtificial)
!102 = distinct !DILexicalBlock(scope: !97, file: !1, line: 21, column: 1)
!103 = !DILocation(line: 0, scope: !102)
!104 = !DILocation(line: 22, column: 25, scope: !102)
!105 = !DILocalVariable(name: ".capture_expr.5", scope: !102, type: !16, flags: DIFlagArtificial)
!106 = !DILocation(line: 22, column: 5, scope: !102)
!107 = !DILocation(line: 21, column: 1, scope: !97)
!108 = !DILocalVariable(name: ".omp.iv", scope: !102, type: !16, flags: DIFlagArtificial)
!109 = !DILocalVariable(name: ".omp.lb", scope: !102, type: !16, flags: DIFlagArtificial)
!110 = !DILocation(line: 22, column: 10, scope: !102)
!111 = !DILocalVariable(name: ".omp.ub", scope: !102, type: !16, flags: DIFlagArtificial)
!112 = !DILocation(line: 21, column: 1, scope: !102)
!113 = !DILocalVariable(name: "i", scope: !102, file: !1, line: 22, type: !16)
!114 = !DILocation(line: 22, column: 14, scope: !102)
!115 = !DILocation(line: 22, column: 28, scope: !102)
!116 = !DILocation(line: 23, column: 9, scope: !117)
!117 = distinct !DILexicalBlock(scope: !102, file: !1, line: 22, column: 33)
!118 = !DILocation(line: 23, column: 22, scope: !117)
!119 = !DILocation(line: 23, column: 32, scope: !117)
!120 = !DILocation(line: 24, column: 5, scope: !117)
!121 = distinct !{!121, !112, !122}
!122 = !DILocation(line: 21, column: 67, scope: !102)
!123 = !DILocation(line: 26, column: 1, scope: !74)
!124 = distinct !DISubprogram(name: "test3", scope: !1, file: !1, line: 28, type: !14, scopeLine: 28, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !17)
!125 = !DILocalVariable(name: "X", arg: 1, scope: !124, file: !1, line: 28, type: !16)
!126 = !DILocation(line: 28, column: 16, scope: !124)
!127 = !DILocalVariable(name: "Y", arg: 2, scope: !124, file: !1, line: 28, type: !16)
!128 = !DILocation(line: 28, column: 23, scope: !124)
!129 = !DILocalVariable(name: "J", scope: !124, file: !1, line: 29, type: !16)
!130 = !DILocation(line: 29, column: 7, scope: !124)
!131 = !DILocalVariable(name: "K", scope: !124, file: !1, line: 29, type: !16)
!132 = !DILocation(line: 29, column: 10, scope: !124)
!133 = !DILocalVariable(name: "L", scope: !124, file: !1, line: 29, type: !16)
!134 = !DILocation(line: 29, column: 13, scope: !124)
!135 = !DILocalVariable(name: "M", scope: !124, file: !1, line: 29, type: !16)
!136 = !DILocation(line: 29, column: 16, scope: !124)
!137 = !DILocalVariable(name: ".capture_expr.10", scope: !138, type: !16, flags: DIFlagArtificial)
!138 = distinct !DILexicalBlock(scope: !124, file: !1, line: 30, column: 1)
!139 = !DILocation(line: 0, scope: !138)
!140 = !DILocation(line: 31, column: 23, scope: !138)
!141 = !DILocalVariable(name: ".capture_expr.11", scope: !138, type: !16, flags: DIFlagArtificial)
!142 = !DILocation(line: 31, column: 3, scope: !138)
!143 = !DILocalVariable(name: ".omp.lb", scope: !138, type: !16, flags: DIFlagArtificial)
!144 = !DILocation(line: 31, column: 8, scope: !138)
!145 = !DILocalVariable(name: ".omp.ub", scope: !138, type: !16, flags: DIFlagArtificial)
!146 = !DILocation(line: 30, column: 1, scope: !138)
!147 = !DILocation(line: 30, column: 1, scope: !148)
!148 = distinct !DILexicalBlock(scope: !138, file: !1, line: 30, column: 1)
!149 = !DILocation(line: 31, column: 23, scope: !150)
!150 = distinct !DILexicalBlock(scope: !148, file: !1, line: 30, column: 1)
!151 = !DILocation(line: 31, column: 3, scope: !150)
!152 = !DILocalVariable(name: ".omp.iv", scope: !150, type: !16, flags: DIFlagArtificial)
!153 = !DILocation(line: 0, scope: !150)
!154 = !DILocation(line: 30, column: 1, scope: !150)
!155 = !DILocation(line: 31, column: 8, scope: !150)
!156 = !DILocalVariable(name: "i", scope: !150, file: !1, line: 31, type: !16)
!157 = !DILocation(line: 31, column: 12, scope: !150)
!158 = !DILocation(line: 31, column: 26, scope: !150)
!159 = !DILocalVariable(name: ".capture_expr.6", scope: !160, type: !16, flags: DIFlagArtificial)
!160 = distinct !DILexicalBlock(scope: !161, file: !1, line: 32, column: 1)
!161 = distinct !DILexicalBlock(scope: !150, file: !1, line: 31, column: 31)
!162 = !DILocation(line: 0, scope: !160)
!163 = !DILocation(line: 33, column: 25, scope: !160)
!164 = !DILocalVariable(name: ".capture_expr.7", scope: !160, type: !16, flags: DIFlagArtificial)
!165 = !DILocation(line: 33, column: 5, scope: !160)
!166 = !DILocation(line: 32, column: 1, scope: !161)
!167 = !DILocalVariable(name: ".omp.iv", scope: !160, type: !16, flags: DIFlagArtificial)
!168 = !DILocalVariable(name: ".omp.lb", scope: !160, type: !16, flags: DIFlagArtificial)
!169 = !DILocation(line: 33, column: 10, scope: !160)
!170 = !DILocalVariable(name: ".omp.ub", scope: !160, type: !16, flags: DIFlagArtificial)
!171 = !DILocation(line: 32, column: 1, scope: !160)
!172 = !DILocalVariable(name: "j", scope: !160, file: !1, line: 33, type: !16)
!173 = !DILocation(line: 33, column: 14, scope: !160)
!174 = !DILocation(line: 33, column: 28, scope: !160)
!175 = !DILocation(line: 33, column: 34, scope: !176)
!176 = distinct !DILexicalBlock(scope: !160, file: !1, line: 33, column: 33)
!177 = distinct !{!177, !171, !178}
!178 = !DILocation(line: 32, column: 41, scope: !160)
!179 = !DILocalVariable(name: ".capture_expr.8", scope: !180, type: !16, flags: DIFlagArtificial)
!180 = distinct !DILexicalBlock(scope: !161, file: !1, line: 34, column: 1)
!181 = !DILocation(line: 0, scope: !180)
!182 = !DILocation(line: 35, column: 25, scope: !180)
!183 = !DILocalVariable(name: ".capture_expr.9", scope: !180, type: !16, flags: DIFlagArtificial)
!184 = !DILocation(line: 35, column: 5, scope: !180)
!185 = !DILocation(line: 34, column: 1, scope: !161)
!186 = !DILocalVariable(name: ".omp.iv", scope: !180, type: !16, flags: DIFlagArtificial)
!187 = !DILocalVariable(name: ".omp.lb", scope: !180, type: !16, flags: DIFlagArtificial)
!188 = !DILocation(line: 35, column: 10, scope: !180)
!189 = !DILocalVariable(name: ".omp.ub", scope: !180, type: !16, flags: DIFlagArtificial)
!190 = !DILocation(line: 34, column: 1, scope: !180)
!191 = !DILocalVariable(name: "j", scope: !180, file: !1, line: 35, type: !16)
!192 = !DILocation(line: 35, column: 14, scope: !180)
!193 = !DILocation(line: 35, column: 28, scope: !180)
!194 = !DILocation(line: 35, column: 34, scope: !195)
!195 = distinct !DILexicalBlock(scope: !180, file: !1, line: 35, column: 33)
!196 = distinct !{!196, !190, !197}
!197 = !DILocation(line: 34, column: 41, scope: !180)
!198 = !DILocation(line: 36, column: 3, scope: !161)
!199 = distinct !{!199, !154, !200}
!200 = !DILocation(line: 30, column: 36, scope: !150)
!201 = !DILocalVariable(name: ".capture_expr.16", scope: !202, type: !16, flags: DIFlagArtificial)
!202 = distinct !DILexicalBlock(scope: !124, file: !1, line: 37, column: 1)
!203 = !DILocation(line: 0, scope: !202)
!204 = !DILocation(line: 38, column: 23, scope: !202)
!205 = !DILocalVariable(name: ".capture_expr.17", scope: !202, type: !16, flags: DIFlagArtificial)
!206 = !DILocation(line: 38, column: 3, scope: !202)
!207 = !DILocalVariable(name: ".omp.lb", scope: !202, type: !16, flags: DIFlagArtificial)
!208 = !DILocation(line: 38, column: 8, scope: !202)
!209 = !DILocalVariable(name: ".omp.ub", scope: !202, type: !16, flags: DIFlagArtificial)
!210 = !DILocation(line: 37, column: 1, scope: !202)
!211 = !DILocation(line: 37, column: 1, scope: !212)
!212 = distinct !DILexicalBlock(scope: !202, file: !1, line: 37, column: 1)
!213 = !DILocation(line: 38, column: 23, scope: !214)
!214 = distinct !DILexicalBlock(scope: !212, file: !1, line: 37, column: 1)
!215 = !DILocation(line: 38, column: 3, scope: !214)
!216 = !DILocalVariable(name: ".omp.iv", scope: !214, type: !16, flags: DIFlagArtificial)
!217 = !DILocation(line: 0, scope: !214)
!218 = !DILocation(line: 37, column: 1, scope: !214)
!219 = !DILocation(line: 38, column: 8, scope: !214)
!220 = !DILocalVariable(name: "i", scope: !214, file: !1, line: 38, type: !16)
!221 = !DILocation(line: 38, column: 12, scope: !214)
!222 = !DILocation(line: 38, column: 26, scope: !214)
!223 = !DILocalVariable(name: ".capture_expr.12", scope: !224, type: !16, flags: DIFlagArtificial)
!224 = distinct !DILexicalBlock(scope: !225, file: !1, line: 39, column: 1)
!225 = distinct !DILexicalBlock(scope: !214, file: !1, line: 38, column: 31)
!226 = !DILocation(line: 0, scope: !224)
!227 = !DILocation(line: 40, column: 25, scope: !224)
!228 = !DILocalVariable(name: ".capture_expr.13", scope: !224, type: !16, flags: DIFlagArtificial)
!229 = !DILocation(line: 40, column: 5, scope: !224)
!230 = !DILocation(line: 39, column: 1, scope: !225)
!231 = !DILocalVariable(name: ".omp.iv", scope: !224, type: !16, flags: DIFlagArtificial)
!232 = !DILocalVariable(name: ".omp.lb", scope: !224, type: !16, flags: DIFlagArtificial)
!233 = !DILocation(line: 40, column: 10, scope: !224)
!234 = !DILocalVariable(name: ".omp.ub", scope: !224, type: !16, flags: DIFlagArtificial)
!235 = !DILocation(line: 39, column: 1, scope: !224)
!236 = !DILocalVariable(name: "j", scope: !224, file: !1, line: 40, type: !16)
!237 = !DILocation(line: 40, column: 14, scope: !224)
!238 = !DILocation(line: 40, column: 28, scope: !224)
!239 = !DILocation(line: 40, column: 34, scope: !240)
!240 = distinct !DILexicalBlock(scope: !224, file: !1, line: 40, column: 33)
!241 = distinct !{!241, !235, !242}
!242 = !DILocation(line: 39, column: 35, scope: !224)
!243 = !DILocalVariable(name: ".capture_expr.14", scope: !244, type: !16, flags: DIFlagArtificial)
!244 = distinct !DILexicalBlock(scope: !225, file: !1, line: 41, column: 1)
!245 = !DILocation(line: 0, scope: !244)
!246 = !DILocation(line: 42, column: 25, scope: !244)
!247 = !DILocalVariable(name: ".capture_expr.15", scope: !244, type: !16, flags: DIFlagArtificial)
!248 = !DILocation(line: 42, column: 5, scope: !244)
!249 = !DILocation(line: 41, column: 1, scope: !225)
!250 = !DILocalVariable(name: ".omp.iv", scope: !244, type: !16, flags: DIFlagArtificial)
!251 = !DILocalVariable(name: ".omp.lb", scope: !244, type: !16, flags: DIFlagArtificial)
!252 = !DILocation(line: 42, column: 10, scope: !244)
!253 = !DILocalVariable(name: ".omp.ub", scope: !244, type: !16, flags: DIFlagArtificial)
!254 = !DILocation(line: 41, column: 1, scope: !244)
!255 = !DILocalVariable(name: "j", scope: !244, file: !1, line: 42, type: !16)
!256 = !DILocation(line: 42, column: 14, scope: !244)
!257 = !DILocation(line: 42, column: 28, scope: !244)
!258 = !DILocation(line: 42, column: 34, scope: !259)
!259 = distinct !DILexicalBlock(scope: !244, file: !1, line: 42, column: 33)
!260 = distinct !{!260, !254, !261}
!261 = !DILocation(line: 41, column: 35, scope: !244)
!262 = !DILocation(line: 43, column: 3, scope: !225)
!263 = distinct !{!263, !218, !264}
!264 = !DILocation(line: 37, column: 36, scope: !214)
!265 = !DILocation(line: 44, column: 1, scope: !124)
!266 = distinct !DISubprogram(linkageName: ".omp_offloading.requires_reg", scope: !1, file: !1, type: !267, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !0, retainedNodes: !17)
!267 = !DISubroutineType(types: !17)
