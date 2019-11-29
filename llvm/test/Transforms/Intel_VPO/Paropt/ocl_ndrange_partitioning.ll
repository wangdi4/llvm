; RUN: opt < %s -prepare-switch-to-offload -switch-to-offload=true -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-gpu-execution-scheme=1 -S -pass-remarks-missed=openmp -pass-remarks-output=%t 2>&1 | FileCheck %s
; RUN: FileCheck --input-file=%t --check-prefix=YAML %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -prepare-switch-to-offload -switch-to-offload -vpo-paropt-gpu-execution-scheme=1 -S -pass-remarks-missed=openmp -pass-remarks-output=%t  2>&1 | FileCheck %s
; RUN: FileCheck --input-file=%t --check-prefix=YAML %s

; Original source code:
; #pragma omp declare target
; void f1() {
; #pragma omp parallel for
;   for (int i = 0; i < 100; ++i);
; }
; #pragma omp end declare target

; void f2() {
; #pragma omp target
; #pragma omp parallel for
;   for (int i = 0; i < 100; ++i);
; }

; void f3() {
; #pragma omp target teams distribute parallel for num_teams(1)
;   for (int i = 0; i < 100; ++i);
; }

; void f4() {
; #pragma omp target teams distribute parallel for
;   for (int i = 0; i < 100; ++i);
; }

; YAML: --- !Missed
; YAML-NEXT: Pass:            openmp
; YAML-NEXT: Name:            Target
; YAML-NEXT: DebugLoc:        { File: test.c, Line: 3, Column: 1 }
; YAML-NEXT: Function:        f1
; YAML-NEXT: Args:
; YAML-NEXT:  - String:          'Consider using OpenMP combined construct with "target" to get optimal performance'

; YAML: --- !Missed
; YAML-NEXT: Pass:            openmp
; YAML-NEXT: Name:            Target
; YAML-NEXT: DebugLoc:        { File: test.c, Line: 10, Column: 1 }
; YAML-NEXT: Function:        f2
; YAML-NEXT: Args:
; YAML-NEXT:  - String:          'Consider using OpenMP combined construct with "target" to get optimal performance'

; YAML: --- !Missed
; YAML-NEXT: Pass:            openmp
; YAML-NEXT: Name:            Target
; YAML-NEXT: DebugLoc:        { File: test.c, Line: 15, Column: 1 }
; YAML-NEXT: Function:        f3
; YAML-NEXT: Args:
; YAML-NEXT:   - String:          'Performance may be reduced due to the enclosing teams region '
; YAML-NEXT:   - String:          specifying num_teams

; CHECK: call i64 @_Z13get_global_idj
; CHECK-NOT: call{{.*}}get_global_id

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @f1() #0 !dbg !10 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.iv.ascast, metadata !13, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.lb.ascast, metadata !17, metadata !DIExpression()), !dbg !16
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.ub.ascast, metadata !19, metadata !DIExpression()), !dbg !16
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4, !dbg !18
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ], !dbg !20
  %1 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !dbg !18
  store i32 %1, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !18
  br label %omp.inner.for.cond, !dbg !20

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !18
  %3 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !dbg !18
  %cmp = icmp sle i32 %2, %3, !dbg !21
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !20

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %i.ascast, metadata !22, metadata !DIExpression()), !dbg !23
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !18
  %mul = mul nsw i32 %4, 1, !dbg !24
  %add = add nsw i32 0, %mul, !dbg !24
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !dbg !24
  br label %omp.body.continue, !dbg !20

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !25

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !18
  %add1 = add nsw i32 %5, 1, !dbg !21
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !21
  br label %omp.inner.for.cond, !dbg !25, !llvm.loop !26

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !25

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !20
  ret void, !dbg !28
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @f2() #3 !dbg !29 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ], !dbg !30
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.iv.ascast, metadata !31, metadata !DIExpression()), !dbg !34
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.lb.ascast, metadata !35, metadata !DIExpression()), !dbg !34
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.ub.ascast, metadata !37, metadata !DIExpression()), !dbg !34
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4, !dbg !36
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ], !dbg !38
  %2 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !dbg !36
  store i32 %2, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !36
  br label %omp.inner.for.cond, !dbg !38

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !36
  %4 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !dbg !36
  %cmp = icmp sle i32 %3, %4, !dbg !39
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !38

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %i.ascast, metadata !40, metadata !DIExpression()), !dbg !41
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !36
  %mul = mul nsw i32 %5, 1, !dbg !42
  %add = add nsw i32 0, %mul, !dbg !42
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !dbg !42
  br label %omp.body.continue, !dbg !38

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !43

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !36
  %add1 = add nsw i32 %6, 1, !dbg !39
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !39
  br label %omp.inner.for.cond, !dbg !43, !llvm.loop !44

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !43

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !38
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !30
  ret void, !dbg !46
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @f3() #3 !dbg !47 {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.lb.ascast, metadata !48, metadata !DIExpression()), !dbg !50
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !dbg !51
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.ub.ascast, metadata !52, metadata !DIExpression()), !dbg !50
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4, !dbg !51
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ], !dbg !53
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ], !dbg !54
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.iv.ascast, metadata !55, metadata !DIExpression()), !dbg !58
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ], !dbg !59
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !dbg !60
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !60
  br label %omp.inner.for.cond, !dbg !59

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !60
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !dbg !60
  %cmp = icmp sle i32 %4, %5, !dbg !61
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !59

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %i.ascast, metadata !62, metadata !DIExpression()), !dbg !63
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !60
  %mul = mul nsw i32 %6, 1, !dbg !64
  %add = add nsw i32 0, %mul, !dbg !64
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !dbg !64
  br label %omp.body.continue, !dbg !59

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !65

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !60
  %add1 = add nsw i32 %7, 1, !dbg !61
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !61
  br label %omp.inner.for.cond, !dbg !65, !llvm.loop !66

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !65

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ], !dbg !59
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ], !dbg !54
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !53
  ret void, !dbg !68
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @f4() #3 !dbg !69 {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.lb.ascast, metadata !70, metadata !DIExpression()), !dbg !72
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !dbg !73
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.ub.ascast, metadata !74, metadata !DIExpression()), !dbg !72
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4, !dbg !73
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ], !dbg !75
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ], !dbg !76
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.iv.ascast, metadata !77, metadata !DIExpression()), !dbg !80
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ], !dbg !81
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !dbg !82
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !82
  br label %omp.inner.for.cond, !dbg !81

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !82
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !dbg !82
  %cmp = icmp sle i32 %4, %5, !dbg !83
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !81

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %i.ascast, metadata !84, metadata !DIExpression()), !dbg !85
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !82
  %mul = mul nsw i32 %6, 1, !dbg !86
  %add = add nsw i32 0, %mul, !dbg !86
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !dbg !86
  br label %omp.body.continue, !dbg !81

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !87

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !82
  %add1 = add nsw i32 %7, 1, !dbg !83
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !83
  br label %omp.inner.for.cond, !dbg !87, !llvm.loop !88

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !87

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ], !dbg !81
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ], !dbg !76
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !75
  ret void, !dbg !90
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }
attributes #3 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!3, !4, !5}
!llvm.module.flags = !{!6, !7, !8}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based DPC++ Compiler 2021.1 (YYYY.8.x.0.MMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "/export/users/vzakhari/workspaces/xmain01/test/tc2")
!2 = !{}
!3 = !{i32 0, i32 2054, i32 108221255, !"f2", i32 9, i32 0, i32 0}
!4 = !{i32 0, i32 2054, i32 108221255, !"f3", i32 15, i32 1, i32 0}
!5 = !{i32 0, i32 2054, i32 108221255, !"f4", i32 20, i32 2, i32 0}
!6 = !{i32 2, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{!"clang version 8.0.0"}
!10 = distinct !DISubprogram(name: "f1", scope: !1, file: !1, line: 2, type: !11, scopeLine: 2, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!11 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !12)
!12 = !{null}
!13 = !DILocalVariable(name: ".omp.iv", scope: !14, type: !15, flags: DIFlagArtificial)
!14 = distinct !DILexicalBlock(scope: !10, file: !1, line: 3, column: 1)
!15 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!16 = !DILocation(line: 0, scope: !14)
!17 = !DILocalVariable(name: ".omp.lb", scope: !14, type: !15, flags: DIFlagArtificial)
!18 = !DILocation(line: 4, column: 8, scope: !14)
!19 = !DILocalVariable(name: ".omp.ub", scope: !14, type: !15, flags: DIFlagArtificial)
!20 = !DILocation(line: 3, column: 1, scope: !10)
!21 = !DILocation(line: 4, column: 3, scope: !14)
!22 = !DILocalVariable(name: "i", scope: !14, file: !1, line: 4, type: !15)
!23 = !DILocation(line: 4, column: 12, scope: !14)
!24 = !DILocation(line: 4, column: 28, scope: !14)
!25 = !DILocation(line: 3, column: 1, scope: !14)
!26 = distinct !{!26, !25, !27}
!27 = !DILocation(line: 3, column: 25, scope: !14)
!28 = !DILocation(line: 5, column: 1, scope: !10)
!29 = distinct !DISubprogram(name: "f2", scope: !1, file: !1, line: 8, type: !11, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!30 = !DILocation(line: 9, column: 1, scope: !29)
!31 = !DILocalVariable(name: ".omp.iv", scope: !32, type: !15, flags: DIFlagArtificial)
!32 = distinct !DILexicalBlock(scope: !33, file: !1, line: 10, column: 1)
!33 = distinct !DILexicalBlock(scope: !29, file: !1, line: 9, column: 1)
!34 = !DILocation(line: 0, scope: !32)
!35 = !DILocalVariable(name: ".omp.lb", scope: !32, type: !15, flags: DIFlagArtificial)
!36 = !DILocation(line: 11, column: 8, scope: !32)
!37 = !DILocalVariable(name: ".omp.ub", scope: !32, type: !15, flags: DIFlagArtificial)
!38 = !DILocation(line: 10, column: 1, scope: !33)
!39 = !DILocation(line: 11, column: 3, scope: !32)
!40 = !DILocalVariable(name: "i", scope: !32, file: !1, line: 11, type: !15)
!41 = !DILocation(line: 11, column: 12, scope: !32)
!42 = !DILocation(line: 11, column: 28, scope: !32)
!43 = !DILocation(line: 10, column: 1, scope: !32)
!44 = distinct !{!44, !43, !45}
!45 = !DILocation(line: 10, column: 25, scope: !32)
!46 = !DILocation(line: 12, column: 1, scope: !29)
!47 = distinct !DISubprogram(name: "f3", scope: !1, file: !1, line: 14, type: !11, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!48 = !DILocalVariable(name: ".omp.lb", scope: !49, type: !15, flags: DIFlagArtificial)
!49 = distinct !DILexicalBlock(scope: !47, file: !1, line: 15, column: 1)
!50 = !DILocation(line: 0, scope: !49)
!51 = !DILocation(line: 16, column: 8, scope: !49)
!52 = !DILocalVariable(name: ".omp.ub", scope: !49, type: !15, flags: DIFlagArtificial)
!53 = !DILocation(line: 15, column: 1, scope: !47)
!54 = !DILocation(line: 15, column: 1, scope: !49)
!55 = !DILocalVariable(name: ".omp.iv", scope: !56, type: !15, flags: DIFlagArtificial)
!56 = distinct !DILexicalBlock(scope: !57, file: !1, line: 15, column: 1)
!57 = distinct !DILexicalBlock(scope: !49, file: !1, line: 15, column: 1)
!58 = !DILocation(line: 0, scope: !56)
!59 = !DILocation(line: 15, column: 1, scope: !57)
!60 = !DILocation(line: 16, column: 8, scope: !56)
!61 = !DILocation(line: 16, column: 3, scope: !56)
!62 = !DILocalVariable(name: "i", scope: !56, file: !1, line: 16, type: !15)
!63 = !DILocation(line: 16, column: 12, scope: !56)
!64 = !DILocation(line: 16, column: 28, scope: !56)
!65 = !DILocation(line: 15, column: 1, scope: !56)
!66 = distinct !{!66, !65, !67}
!67 = !DILocation(line: 15, column: 62, scope: !56)
!68 = !DILocation(line: 17, column: 1, scope: !47)
!69 = distinct !DISubprogram(name: "f4", scope: !1, file: !1, line: 19, type: !11, scopeLine: 19, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!70 = !DILocalVariable(name: ".omp.lb", scope: !71, type: !15, flags: DIFlagArtificial)
!71 = distinct !DILexicalBlock(scope: !69, file: !1, line: 20, column: 1)
!72 = !DILocation(line: 0, scope: !71)
!73 = !DILocation(line: 21, column: 8, scope: !71)
!74 = !DILocalVariable(name: ".omp.ub", scope: !71, type: !15, flags: DIFlagArtificial)
!75 = !DILocation(line: 20, column: 1, scope: !69)
!76 = !DILocation(line: 20, column: 1, scope: !71)
!77 = !DILocalVariable(name: ".omp.iv", scope: !78, type: !15, flags: DIFlagArtificial)
!78 = distinct !DILexicalBlock(scope: !79, file: !1, line: 20, column: 1)
!79 = distinct !DILexicalBlock(scope: !71, file: !1, line: 20, column: 1)
!80 = !DILocation(line: 0, scope: !78)
!81 = !DILocation(line: 20, column: 1, scope: !79)
!82 = !DILocation(line: 21, column: 8, scope: !78)
!83 = !DILocation(line: 21, column: 3, scope: !78)
!84 = !DILocalVariable(name: "i", scope: !78, file: !1, line: 21, type: !15)
!85 = !DILocation(line: 21, column: 12, scope: !78)
!86 = !DILocation(line: 21, column: 28, scope: !78)
!87 = !DILocation(line: 20, column: 1, scope: !78)
!88 = distinct !{!88, !87, !89}
!89 = !DILocation(line: 20, column: 49, scope: !78)
!90 = !DILocation(line: 22, column: 1, scope: !69)
