; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck %s

; Original code:
;
; int main() {
; #pragma omp parallel for collapse(2)
;     for (x = 0; x < 1024; ++x) { 
;       for (y = 0; y < 512; ++y) {
;       }
;     }
;   return 0;
; }

; Check that 'vpo-paropt-loop-collapse' pass could handle unrotated OpenMP loops.

; CHECK-LABEL: define dso_local i32 @main
; CHECK-LABEL: entry:
; CHECK: [[COLLAPSED_IV:%omp.collapsed.iv]] = alloca i64, align 8
; CHECK: [[COLLAPSED_LB:%omp.collapsed.lb]] = alloca i64, align 8
; CHECK: [[COLLAPSED_UB:%omp.collapsed.ub]] = alloca i64, align 8
; CHECK: [[UNCOLLAPSED_IV_1:%.omp.uncollapsed.iv]] = alloca i32, align 4
; CHECK: [[UNCOLLAPSED_IV_2:%.omp.uncollapsed.iv2]] = alloca i32, align 4
; CHECK: [[UNCOLLAPSED_LB_1:%.omp.uncollapsed.lb]] = alloca i32, align 4
; CHECK: [[UNCOLLAPSED_UB_1:%.omp.uncollapsed.ub]] = alloca i32, align 4
; CHECK: [[UNCOLLAPSED_LB_2:%.omp.uncollapsed.lb3]] = alloca i32, align 4
; CHECK: [[UNCOLLAPSED_UB_2:%.omp.uncollapsed.ub4]] = alloca i32, align 4
; CHECK: store i32 0, ptr [[UNCOLLAPSED_LB_1]], align 4
; CHECK: store i32 1023, ptr [[UNCOLLAPSED_UB_1]], align 4
; CHECK: store i32 0, ptr [[UNCOLLAPSED_LB_2]], align 4
; CHECK: store i32 511, ptr [[UNCOLLAPSED_UB_2]], align 4
; CHECK: br label %DIR.OMP.PARALLEL.LOOP{{.*}}

; CHECK-LABEL: DIR.OMP.PARALLEL.LOOP{{.*}}:
; CHECK-NEXT: br label %DIR.OMP.PARALLEL.LOOP{{.*}}

; CHECK-LABEL: DIR.OMP.PARALLEL.LOOP{{.*}}:
; CHECK: [[UB_1_VAL:%.omp.uncollapsed.ub.val]] = load i32, ptr [[UNCOLLAPSED_UB_1]], align 4
; CHECK: [[UB_1_VAL_SEXT:%.sext]] = sext i32 [[UB_1_VAL]] to i64
; CHECK: [[UB_1_VAL:%[0-9]+]] = add nuw nsw i64 [[UB_1_VAL_SEXT]], 1
; CHECK: [[CMP1:%[0-9]+]] = icmp slt i64 [[UB_1_VAL_SEXT]], 0
; CHECK: [[UB_2_VAL:%.omp.uncollapsed.ub4.val]] = load i32, ptr [[UNCOLLAPSED_UB_2]], align 4
; CHECK: [[UB_2_VAL_SEXT:%.sext1]] = sext i32 [[UB_2_VAL]] to i64
; CHECK: [[UB_2_VAL:%[0-9]+]] = add nuw nsw i64 [[UB_2_VAL_SEXT]], 1
; CHECK: [[CMP2:%[0-9]+]] = icmp slt i64 [[UB_2_VAL_SEXT]], 0
; CHECK: [[IS_UB_LESS_THAN_ZERO:%[0-9]+]] = or i1 [[CMP1]], [[CMP2]]
; CHECK: [[UB_MULT:%[0-9]+]] = mul nuw nsw i64 [[UB_1_VAL]], [[UB_2_VAL]]
; CHECK: [[TMP6:%[0-9]+]] = select i1 [[IS_UB_LESS_THAN_ZERO]], i64 0, i64 [[UB_MULT]]
; CHECK: [[COLLAPSED_UB_VAL:%omp.collapsed.ub.value]] = sub nuw nsw i64 [[TMP6]], 1
; CHECK: store i64 0, ptr [[COLLAPSED_LB]], align 8
; CHECK: store i64 [[COLLAPSED_UB_VAL]], ptr [[COLLAPSED_UB]], align 8

; CHECK: [[BEGIN_DIR:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
; CHECK-SAME:   "QUAL.OMP.COLLAPSE"(i32 2),
; CHECK-SAME:   "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[UNCOLLAPSED_LB_1]], i32 0, i32 1),
; CHECK-SAME:   "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[UNCOLLAPSED_LB_2]], i32 0, i32 1),
; CHECK-SAME:   "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[COLLAPSED_IV]], i64 0),
; CHECK-SAME:   "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[COLLAPSED_UB]], i64 0),
; CHECK-SAME:   "QUAL.OMP.PRIVATE:TYPED"(ptr [[UNCOLLAPSED_IV_1]], i32 0, i32 1),
; CHECK-SAME:   "QUAL.OMP.PRIVATE:TYPED"(ptr [[UNCOLLAPSED_IV_2]], i32 0, i32 1),
; CHECK-SAME:   "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[COLLAPSED_LB]], i64 0, i32 1),
; CHECK-SAME:   "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[UNCOLLAPSED_UB_1]], i32 0, i32 1),
; CHECK-SAME:   "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[UNCOLLAPSED_UB_2]], i32 0, i32 1) ]
; CHECK-NEXT: br label %DIR.OMP.PARALLEL.LOOP.{{[0-9]+}}.split

; CHECK-LABEL: DIR.OMP.PARALLEL.LOOP{{.*}}split:
; CHECK: [[UNCOLLAPSED_UB_2_VAL2:%.omp.uncollapsed.ub4.val2]] = load i32, ptr [[UNCOLLAPSED_UB_2]], align 4
; CHECK: [[UB_2_ZEXT:%.zext]] = zext i32 [[UNCOLLAPSED_UB_2_VAL2]] to i64
; CHECK: [[TMP8:%[0-9]+]] = add nuw nsw i64 [[UB_2_ZEXT]], 1
; CHECK: [[TMP9:%[0-9]+]] = mul nuw nsw i64 [[TMP8]], 1
; CHECK: br label %[[DIR_OMP_LOOP_3:[^,]+]]

; CHECK-LABEL: DIR.OMP.PARALLEL.LOOP.{{.*}}:
; CHECK: [[LB1:%[0-9]+]] = load i32, ptr [[UNCOLLAPSED_LB_1]], align 4
; CHECK: store i32 [[LB1]], ptr [[UNCOLLAPSED_IV_1]], align 4
; CHECK: br label %DIR.OMP.PARALLEL.LOOP.{{[0-9]+}}.split

; CHECK-LABEL: DIR.OMP.PARALLEL.LOOP{{.*}}split:
; CHECK: [[COLLAPSE_LB_VAL1:%[0-9]+]] = load i64, ptr [[COLLAPSED_LB]], align 8
; CHECK: store i64 [[COLLAPSE_LB_VAL1]], ptr [[COLLAPSED_IV]], align 8
; CHECK: br label %omp.collapsed.loop.cond

; CHECK-LABEL: omp.collapsed.loop.cond:                         
; CHECK: [[COLLAPSE_IV_VAL1:%[0-9]+]] = load i64, ptr [[COLLAPSED_IV]], align 8
; CHECK: [[COLLAPSE_UB_VAL1:%[0-9]+]] = load i64, ptr [[COLLAPSED_UB]], align 8
; CHECK: [[CMP5:%[0-9]+]] = icmp sle i64 [[COLLAPSE_IV_VAL1]], [[COLLAPSE_UB_VAL1]]
; CHECK: br i1 [[CMP5]], label %omp.collapsed.loop.body, label %omp.collapsed.loop.exit

; CHECK-LABEL: omp.collapsed.loop.body:
; CHECK: br label %omp.uncollapsed.loop.cond
; 
; CHECK-LABEL: omp.collapsed.loop.exit:
; CHECK: br label %omp.collapsed.loop.postexit
; 
; CHECK-LABEL: omp.uncollapsed.loop.cond:
; CHECK: br i1 %{{.*}}, label %omp.uncollapsed.loop.body, label %omp.collapsed.loop.inc
; 
; CHECK-LABEL: omp.uncollapsed.loop.body:
; CHECK: br label %omp.uncollapsed.loop.cond5
; 
; CHECK-LABEL: omp.uncollapsed.loop.cond5:
; CHECK: br i1 %{{.*}}, label %omp.uncollapsed.loop.body{{.*}}, label %omp.uncollapsed.loop.end
; 
; CHECK-LABEL: omp.uncollapsed.loop.body{{.*}}:
; CHECK: br label %omp.body.continue
; 
; CHECK-LABEL: omp.body.continue:
; CHECK: br label %omp.uncollapsed.loop.inc
; 
; CHECK-LABEL: omp.uncollapsed.loop.inc:
; CHECK: br label %omp.uncollapsed.loop.cond{{.*}}
; 
; CHECK-LABEL: omp.uncollapsed.loop.end:
; CHECK: br label %omp.uncollapsed.loop.inc{{.*}}
; 
; CHECK-LABEL: omp.uncollapsed.loop.inc{{.*}}:
; CHECK: br label %omp.uncollapsed.loop.cond
; 
; CHECK-LABEL: omp.collapsed.loop.inc:
; CHECK: [[LOAD_COLLAPSED_IV:%[0-9]+]] = load i64, ptr [[COLLAPSED_IV]], align 8
; CHECK: [[INC_COLLAPSED_IV:%[0-9]+]] = add nuw nsw i64 [[LOAD_COLLAPSED_IV]], 1
; CHECK: store i64 [[INC_COLLAPSED_IV]], ptr [[COLLAPSED_IV]], align 8
; CHECK: br label %omp.collapsed.loop.cond
; 
; CHECK-LABEL: omp.collapsed.loop.postexit:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %tmp = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv2 = alloca i32, align 4
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.lb3 = alloca i32, align 4
  %.omp.uncollapsed.ub4 = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 0, ptr %x, align 4
  store i32 0, ptr %y, align 4
  store i32 0, ptr %.omp.uncollapsed.lb, align 4
  store i32 1023, ptr %.omp.uncollapsed.ub, align 4
  store i32 0, ptr %.omp.uncollapsed.lb3, align 4
  store i32 511, ptr %.omp.uncollapsed.ub4, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %x, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %y, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.uncollapsed.iv, i32 0, ptr %.omp.uncollapsed.iv2, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.uncollapsed.ub, i32 0, ptr %.omp.uncollapsed.ub4, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb3, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.uncollapsed.lb, align 4
  store i32 %1, ptr %.omp.uncollapsed.iv, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc11, %entry
  %2 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %3 = load i32, ptr %.omp.uncollapsed.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end13

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %4 = load i32, ptr %.omp.uncollapsed.lb3, align 4
  store i32 %4, ptr %.omp.uncollapsed.iv2, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %5 = load i32, ptr %.omp.uncollapsed.iv2, align 4
  %6 = load i32, ptr %.omp.uncollapsed.ub4, align 4
  %cmp6 = icmp sle i32 %5, %6
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  %7 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %x, align 4
  %8 = load i32, ptr %.omp.uncollapsed.iv2, align 4
  %mul8 = mul nsw i32 %8, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr %y, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body7
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %9 = load i32, ptr %.omp.uncollapsed.iv2, align 4
  %add10 = add nsw i32 %9, 1
  store i32 %add10, ptr %.omp.uncollapsed.iv2, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc11

omp.uncollapsed.loop.inc11:                       ; preds = %omp.uncollapsed.loop.end
  %10 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %add12 = add nsw i32 %10, 1
  store i32 %add12, ptr %.omp.uncollapsed.iv, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end13:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
