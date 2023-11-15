; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-transform  -disable-vpo-paropt-tile=false -print-blocks-preorder -pretty-print-directives -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-transform)' -disable-vpo-paropt-tile=false -print-blocks-preorder -pretty-print-directives -S < %s | FileCheck %s

; Verify that $omp tile pragma is handled correctly with offloading. Variables related to loop tiling (e.g. floor_{iv,lb,ub} etc.) are conveyed through clauses of
; outer WRNs if they are allocated and initialized before(outside) those WRFs. In this lit-test, allocations and initializations (i.e. stores) are done in the entry node.
; Note that this lit-test verifies only loop tiling.

; Test src:
;
; SUBROUTINE my_func()
;   integer :: y_min=1, y_max=10000, x_min=1, x_max=10000, k, j, arr(10000)
;   !$OMP TARGET TEAMS DISTRIBUTE PARALLEL DO COLLAPSE(2)
;   DO k=y_min,y_max+1
;   !$OMP TILE SIZES(4)
;     DO j=x_min,x_max+1
;       arr(k) = arr(j) + k * j - j
;     ENDDO
;   ENDDO
; END SUBROUTINE my_func

;
;; Function Attrs: noinline nounwind uwtable
;define void @my_module_mp_my_func_() #0 {
;alloca_1:
; ...
;  %floor_lb.tmp = alloca i32, align 4
; CHECK:  %floor_lb = addrspacecast ptr %floor_lb.tmp to ptr addrspace(4)
; CHECK:  store i32 0, ptr addrspace(4) %floor_lb, align 4
;         %floor_ub.tmp = alloca i32, align 4
; CHECK:  %floor_ub = addrspacecast ptr %floor_ub.tmp to ptr addrspace(4)
; CHECK:  %norm.orig.ub.val = load i32, ptr addrspace(4) %omp.pdo.norm.ub, align 4
; CHECK:  %norm.floor.ub.val = sdiv i32 %norm.orig.ub.val, 4
; CHECK:  store i32 %norm.floor.ub.val, ptr addrspace(4) %floor_ub, align 4
;         %floor_iv.tmp = alloca i32, align 4
; CHECK:  %floor_iv = addrspacecast ptr %floor_iv.tmp to ptr addrspace(4)
;         %cloned_orig_ub.tmp = alloca i32, align 4
; CHECK:  %cloned_orig_ub = addrspacecast ptr %cloned_orig_ub.tmp to ptr addrspace(4)
; CHECK:  store i32 %norm.orig.ub.val, ptr addrspace(4) %cloned_orig_ub, align 4
;  br label %bb_new2
;
;bb_new2:                                          ; preds = %alloca_1
;  br label %DIR.OMP.TARGET.1
;
;DIR.OMP.TARGET.1:                                 ; preds = %bb_new2
;  br label %DIR.OMP.TARGET.11
;
;DIR.OMP.TARGET.11:                                ; preds = %DIR.OMP.TARGET.1
; CHECK: %omp.target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
;    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
;    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv11, i32 0, i64 1),
;    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv, i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub15, i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb13, i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.start7, i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb, i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.start, i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MIN" to ptr addrspace(4)), i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MAX" to ptr addrspace(4)), i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$X_MIN" to ptr addrspace(4)), i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$X_MAX" to ptr addrspace(4)), i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$K", i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$ARR", i32 0, i64 10000),
; CHECK: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %floor_ub, i32 0, i64 1),
; CHECK: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %cloned_orig_ub, i32 0, i64 1),
; CHECK: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %floor_lb, i32 0, i64 1),
; CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %floor_iv, i32 0, i64 1) ]
;
;  br label %bb_new3
;
;bb_new3:                                          ; preds = %DIR.OMP.TARGET.11
;  br label %DIR.OMP.TEAMS.2
;
;DIR.OMP.TEAMS.2:                                  ; preds = %bb_new3
;  br label %DIR.OMP.TEAMS.22
;
;DIR.OMP.TEAMS.22:                                 ; preds = %DIR.OMP.TEAMS.2
; CHECK: %omp.teams = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub15, i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb13, i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.start7, i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb, i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.start, i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MIN" to ptr addrspace(4)), i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MAX" to ptr addrspace(4)), i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$X_MIN" to ptr addrspace(4)), i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$X_MAX" to ptr addrspace(4)), i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$ARR", i32 0, i64 10000),
; CHECK:   "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv11, i32 0, i64 1),
; CHECK:   "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv, i32 0, i64 1),
;    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$K", i32 0, i64 1),
; CHECK: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %floor_ub, i32 0, i64 1),
; CHECK: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %cloned_orig_ub, i32 0, i64 1),
; CHECK: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %floor_lb, i32 0, i64 1),
; CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %floor_iv, i32 0, i64 1) ]
;
;  br label %bb_new4
;
;bb_new4:                                          ; preds = %DIR.OMP.TEAMS.22
;  br label %DIR.OMP.DISTRIBUTE.PARLOOP.3
;
;DIR.OMP.DISTRIBUTE.PARLOOP.3:                     ; preds = %bb_new4
;  br label %DIR.OMP.DISTRIBUTE.PARLOOP.33
;
;DIR.OMP.DISTRIBUTE.PARLOOP.33:                    ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.3
; CHECK: %omp.distribute.parloop = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
; CHECK:   "QUAL.OMP.COLLAPSE"(i32 2),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MIN" to ptr addrspace(4)), i32 0, i64 1),
;    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MAX" to ptr addrspace(4)), i32 0, i64 1),
;    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$K", i32 0, i64 1),
;    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb, i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.start, i32 0, i64 1),
;    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb13, i32 0, i64 1),
; CHECK: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv11, i32 0, ptr addrspace(4) %floor_iv, i32 0),
; CHECK: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub15, i32 0, ptr addrspace(4) %floor_ub, i32 0),
; CHECK: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %floor_lb, i32 0, i32 1) ]
;
;
;
; CHECK: FLOOR.PREHEAD:
; CHECK: %floor.lb = load i32, ptr addrspace(4) %floor_lb, align 4
; CHECK:  store i32 %floor.lb, ptr addrspace(4) %floor_iv, align 4
;  br label %FLOOR.HEAD
;
; CHECK: FLOOR.HEAD:                                       ; preds = %FLOOR.LATCH, %FLOOR.PREHEAD
; CHECK:   %floor.iv = load i32, ptr addrspace(4) %floor_iv, align 4
; CHECK:   %floor.ub = load i32, ptr addrspace(4) %floor_ub, align 4
;  %tile.loop.cond = icmp sle i32 %floor.iv, %floor.ub
;  br i1 %tile.loop.cond, label %DIR.OMP.TILE.5, label %omp.pdo.epilog16
;
;CHECK: FLOOR.LATCH:                                      ; preds = %omp.pdo.cond13
;CHECK:   %floor.iv5 = load i32, ptr addrspace(4) %floor_iv, align 4
;CHECK:   %inc = add i32 %floor.iv5, 1
;CHECK:   store i32 %inc, ptr addrspace(4) %floor_iv, align 4
;CHECK:   br label %FLOOR.HEAD
;

; Input IR

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64_gen"
target device_triples = "spir64_gen"

@"my_module_mp_my_func_$X_MAX" = internal addrspace(1) global i32 10000, align 4
@"my_module_mp_my_func_$X_MIN" = internal addrspace(1) global i32 1, align 4
@"my_module_mp_my_func_$Y_MAX" = internal addrspace(1) global i32 10000, align 4
@"my_module_mp_my_func_$Y_MIN" = internal addrspace(1) global i32 1, align 4

define void @my_module_mp_my_func_() {
alloca_1:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"my_module_mp_my_func_$ARR" = alloca [10000 x i32], align 16
  %"my_module_mp_my_func_$J" = alloca i32, align 4
  %"my_module_mp_my_func_$K" = alloca i32, align 4
  %"ascast$my_module_mp_my_func_$K" = addrspacecast ptr %"my_module_mp_my_func_$K" to ptr addrspace(4)
  %"ascastB$my_module_mp_my_func_$J" = addrspacecast ptr %"my_module_mp_my_func_$J" to ptr addrspace(4)
  %"ascast$my_module_mp_my_func_$ARR" = addrspacecast ptr %"my_module_mp_my_func_$ARR" to ptr addrspace(4)
  %fetch.1 = load i32, ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MIN" to ptr addrspace(4)), align 4
  %temp6 = alloca i32, align 4
  %omp.pdo.start7 = addrspacecast ptr %temp6 to ptr addrspace(4)
  store i32 %fetch.1, ptr addrspace(4) %omp.pdo.start7, align 4
  %fetch.2 = load i32, ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MAX" to ptr addrspace(4)), align 4
  %add.1 = add nsw i32 %fetch.2, 1
  %temp8 = alloca i32, align 4
  %omp.pdo.end9 = addrspacecast ptr %temp8 to ptr addrspace(4)
  store i32 %add.1, ptr addrspace(4) %omp.pdo.end9, align 4
  %temp10 = alloca i32, align 4
  %omp.pdo.norm.iv11 = addrspacecast ptr %temp10 to ptr addrspace(4)
  %temp12 = alloca i32, align 4
  %omp.pdo.norm.lb13 = addrspacecast ptr %temp12 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %omp.pdo.norm.lb13, align 4
  %temp14 = alloca i32, align 4
  %omp.pdo.norm.ub15 = addrspacecast ptr %temp14 to ptr addrspace(4)
  %omp.pdo.start_fetch.3 = load i32, ptr addrspace(4) %omp.pdo.start7, align 4
  %omp.pdo.end_fetch.4 = load i32, ptr addrspace(4) %omp.pdo.end9, align 4
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.4, %omp.pdo.start_fetch.3
  %add.2 = add nsw i32 %sub.1, 1
  %sub.2 = sub nsw i32 %add.2, 1
  store i32 %sub.2, ptr addrspace(4) %omp.pdo.norm.ub15, align 4
  %"my_module_mp_my_func_$X_MIN_fetch.10" = load i32, ptr addrspace(1) @"my_module_mp_my_func_$X_MIN", align 4
  %temp = alloca i32, align 4
  %omp.pdo.start = addrspacecast ptr %temp to ptr addrspace(4)
  store i32 %"my_module_mp_my_func_$X_MIN_fetch.10", ptr addrspace(4) %omp.pdo.start, align 4
  %"my_module_mp_my_func_$X_MAX_fetch.11" = load i32, ptr addrspace(1) @"my_module_mp_my_func_$X_MAX", align 4
  %add.4 = add nsw i32 %"my_module_mp_my_func_$X_MAX_fetch.11", 1
  %temp2 = alloca i32, align 4
  %omp.pdo.end = addrspacecast ptr %temp2 to ptr addrspace(4)
  store i32 %add.4, ptr addrspace(4) %omp.pdo.end, align 4
  %temp3 = alloca i32, align 4
  %omp.pdo.norm.iv = addrspacecast ptr %temp3 to ptr addrspace(4)
  %temp4 = alloca i32, align 4
  %omp.pdo.norm.lb = addrspacecast ptr %temp4 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %omp.pdo.norm.lb, align 4
  %temp5 = alloca i32, align 4
  %omp.pdo.norm.ub = addrspacecast ptr %temp5 to ptr addrspace(4)
  %omp.pdo.start_fetch.12 = load i32, ptr addrspace(4) %omp.pdo.start, align 4
  %omp.pdo.end_fetch.13 = load i32, ptr addrspace(4) %omp.pdo.end, align 4
  %sub.3 = sub nsw i32 %omp.pdo.end_fetch.13, %omp.pdo.start_fetch.12
  %add.5 = add nsw i32 %sub.3, 1
  %sub.4 = sub nsw i32 %add.5, 1
  store i32 %sub.4, ptr addrspace(4) %omp.pdo.norm.ub, align 4
  br label %bb_new2

bb_new2:                                          ; preds = %alloca_1
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %bb_new2
  %omp.target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv11, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub15, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb13, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.start7, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.start, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MIN" to ptr addrspace(4)), i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MAX" to ptr addrspace(4)), i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$X_MIN" to ptr addrspace(4)), i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$X_MAX" to ptr addrspace(4)), i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$K", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$ARR", i32 0, i64 10000) ]

  br label %bb_new3

bb_new3:                                          ; preds = %DIR.OMP.TARGET.1
  br label %DIR.OMP.TEAMS.2

DIR.OMP.TEAMS.2:                                  ; preds = %bb_new3
  %omp.teams = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub15, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb13, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.start7, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.start, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MIN" to ptr addrspace(4)), i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MAX" to ptr addrspace(4)), i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$X_MIN" to ptr addrspace(4)), i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$X_MAX" to ptr addrspace(4)), i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$ARR", i32 0, i64 10000),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv11, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$K", i32 0, i64 1) ]

  br label %bb_new4

bb_new4:                                          ; preds = %DIR.OMP.TEAMS.2
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.3

DIR.OMP.DISTRIBUTE.PARLOOP.3:                     ; preds = %bb_new4
  %omp.distribute.parloop = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MIN" to ptr addrspace(4)), i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$Y_MAX" to ptr addrspace(4)), i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$K", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.start, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb13, i32 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv11, i32 0, ptr addrspace(4) %omp.pdo.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub15, i32 0, ptr addrspace(4) %omp.pdo.norm.ub, i32 0) ]

  br label %DIR.OMP.DISTRIBUTE.PARLOOP.4

DIR.OMP.DISTRIBUTE.PARLOOP.4:                     ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.3
  %omp.pdo.norm.lb_fetch.5 = load i32, ptr addrspace(4) %omp.pdo.norm.lb13, align 4
  store i32 %omp.pdo.norm.lb_fetch.5, ptr addrspace(4) %omp.pdo.norm.iv11, align 4
  br label %omp.pdo.cond6

omp.pdo.cond6:                                    ; preds = %DIR.OMP.END.TILE.6, %DIR.OMP.DISTRIBUTE.PARLOOP.4
  %omp.pdo.norm.iv_fetch.6 = load i32, ptr addrspace(4) %omp.pdo.norm.iv11, align 4
  %omp.pdo.norm.ub_fetch.7 = load i32, ptr addrspace(4) %omp.pdo.norm.ub15, align 4
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
  br i1 %rel.1, label %omp.pdo.body7.lr.ph8, label %omp.pdo.epilog9

omp.pdo.body7.lr.ph8:                             ; preds = %omp.pdo.cond6
  %omp.pdo.start_fetch.8 = load i32, ptr addrspace(4) %omp.pdo.start7, align 4
  %omp.pdo.norm.iv_fetch.9 = load i32, ptr addrspace(4) %omp.pdo.norm.iv11, align 4
  %add.3 = add nsw i32 %omp.pdo.norm.iv_fetch.9, %omp.pdo.start_fetch.8
  store i32 %add.3, ptr addrspace(4) %"ascast$my_module_mp_my_func_$K", align 4
  br label %bb_new11

bb_new11:                                         ; preds = %omp.pdo.body7.lr.ph8
  %omp.tile = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
    "QUAL.OMP.SIZES"(i32 4),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i32 0),
    "QUAL.OMP.LIVEIN"(ptr addrspace(4) %omp.pdo.norm.lb),
    "QUAL.OMP.LIVEIN"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$X_MIN" to ptr addrspace(4))),
    "QUAL.OMP.LIVEIN"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @"my_module_mp_my_func_$X_MAX" to ptr addrspace(4))),
    "QUAL.OMP.LIVEIN"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$K"),
    "QUAL.OMP.LIVEIN"(ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J"),
    "QUAL.OMP.LIVEIN"(ptr addrspace(4) %"ascast$my_module_mp_my_func_$ARR") ]

  br label %DIR.OMP.TILE.5

DIR.OMP.TILE.5:                                   ; preds = %bb_new11
  %omp.pdo.norm.lb_fetch.14 = load i32, ptr addrspace(4) %omp.pdo.norm.lb, align 4
  store i32 %omp.pdo.norm.lb_fetch.14, ptr addrspace(4) %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond13

omp.pdo.cond13:                                   ; preds = %omp.pdo.body14.lr.ph15, %DIR.OMP.TILE.5
  %omp.pdo.norm.iv_fetch.15 = load i32, ptr addrspace(4) %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.16 = load i32, ptr addrspace(4) %omp.pdo.norm.ub, align 4
  %rel.2 = icmp sle i32 %omp.pdo.norm.iv_fetch.15, %omp.pdo.norm.ub_fetch.16
  br i1 %rel.2, label %omp.pdo.body14.lr.ph15, label %omp.pdo.epilog16

omp.pdo.body14.lr.ph15:                           ; preds = %omp.pdo.cond13
  %omp.pdo.start_fetch.17 = load i32, ptr addrspace(4) %omp.pdo.start, align 4
  %omp.pdo.norm.iv_fetch.18 = load i32, ptr addrspace(4) %omp.pdo.norm.iv, align 4
  %add.6 = add nsw i32 %omp.pdo.norm.iv_fetch.18, %omp.pdo.start_fetch.17
  store i32 %add.6, ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", align 4
  %"ascastB$my_module_mp_my_func_$J_fetch.19" = load i32, ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", align 4
  %int_sext = sext i32 %"ascastB$my_module_mp_my_func_$J_fetch.19" to i64
  %"my_module_mp_my_func_$ARR[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"my_module_mp_my_func_$ARR", i64 %int_sext)
  %"my_module_mp_my_func_$ARR[]_fetch.20" = load i32, ptr %"my_module_mp_my_func_$ARR[]", align 4
  %"ascast$my_module_mp_my_func_$K_fetch.21" = load i32, ptr addrspace(4) %"ascast$my_module_mp_my_func_$K", align 4
  %"ascastB$my_module_mp_my_func_$J_fetch.22" = load i32, ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", align 4
  %mul.1 = mul nsw i32 %"ascast$my_module_mp_my_func_$K_fetch.21", %"ascastB$my_module_mp_my_func_$J_fetch.22"
  %add.7 = add nsw i32 %"my_module_mp_my_func_$ARR[]_fetch.20", %mul.1
  %"ascastB$my_module_mp_my_func_$J_fetch.23" = load i32, ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", align 4
  %sub.5 = sub nsw i32 %add.7, %"ascastB$my_module_mp_my_func_$J_fetch.23"
  %"ascast$my_module_mp_my_func_$K_fetch.24" = load i32, ptr addrspace(4) %"ascast$my_module_mp_my_func_$K", align 4
  %int_sext1 = sext i32 %"ascast$my_module_mp_my_func_$K_fetch.24" to i64
  %"ascast$my_module_mp_my_func_$ARR[]" = call ptr addrspace(4) @llvm.intel.subscript.p4.i64.i64.p4.i64(i8 0, i64 1, i64 4, ptr addrspace(4) elementtype(i32) %"ascast$my_module_mp_my_func_$ARR", i64 %int_sext1)
  store i32 %sub.5, ptr addrspace(4) %"ascast$my_module_mp_my_func_$ARR[]", align 4
  %"ascastB$my_module_mp_my_func_$J_fetch.25" = load i32, ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", align 4
  %add.8 = add nsw i32 %"ascastB$my_module_mp_my_func_$J_fetch.25", 1
  store i32 %add.8, ptr addrspace(4) %"ascastB$my_module_mp_my_func_$J", align 4
  %omp.pdo.norm.iv_fetch.26 = load i32, ptr addrspace(4) %omp.pdo.norm.iv, align 4
  %add.9 = add nsw i32 %omp.pdo.norm.iv_fetch.26, 1
  store i32 %add.9, ptr addrspace(4) %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond13

omp.pdo.epilog16:                                 ; preds = %omp.pdo.cond13
  call void @llvm.directive.region.exit(token %omp.tile) [ "DIR.OMP.END.TILE"() ]

  br label %DIR.OMP.END.TILE.6

DIR.OMP.END.TILE.6:                               ; preds = %omp.pdo.epilog16
  %omp.pdo.norm.iv_fetch.27 = load i32, ptr addrspace(4) %omp.pdo.norm.iv11, align 4
  %add.10 = add nsw i32 %omp.pdo.norm.iv_fetch.27, 1
  store i32 %add.10, ptr addrspace(4) %omp.pdo.norm.iv11, align 4
  br label %omp.pdo.cond6

omp.pdo.epilog9:                                  ; preds = %omp.pdo.cond6
  call void @llvm.directive.region.exit(token %omp.distribute.parloop) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.7

DIR.OMP.END.DISTRIBUTE.PARLOOP.7:                 ; preds = %omp.pdo.epilog9
  call void @llvm.directive.region.exit(token %omp.teams) [ "DIR.OMP.END.TEAMS"() ]

  br label %DIR.OMP.END.TEAMS.8

DIR.OMP.END.TEAMS.8:                              ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.7
  call void @llvm.directive.region.exit(token %omp.target) [ "DIR.OMP.END.TARGET"() ]

  br label %DIR.OMP.END.TARGET.9

DIR.OMP.END.TARGET.9:                             ; preds = %DIR.OMP.END.TEAMS.8
  ret void
}

declare token @llvm.directive.region.entry()
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)
declare ptr addrspace(4) @llvm.intel.subscript.p4.i64.i64.p4.i64(i8, i64, i64, ptr addrspace(4), i64)
declare void @llvm.directive.region.exit(token)
!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2054, i32 91374091, !"my_module_mp_my_func_", i32 5, i32 0, i32 0, i32 0}



; end INTEL_CUSTOMIZATION
