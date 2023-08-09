; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <cassert>
;
; #define N 12
;
; void work(short &yref, short (&y_arr_ref)[N]) {
;
;   short *yref_addr = &yref;
;   short *y_arr_ref_addr = (short *)&y_arr_ref;
;
; #pragma omp taskloop lastprivate(yref, y_arr_ref)
;   for (int i = 0; i < N; i++) {
;     assert(y_arr_ref_addr != (short *)&y_arr_ref);
;     assert(yref_addr != &yref);
;     if (i == N - 1) {
;       y_arr_ref[i] = 3;
;       yref = 3;
;     }
;   }
; };
;

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [38 x i8] c"y_arr_ref_addr != (short*) &y_arr_ref\00", align 1
@.str.1 = private unnamed_addr constant [31 x i8] c"byref_lastprivate_taskloop.cpp\00", align 1
@__PRETTY_FUNCTION__._Z4workRsRA12_s = private unnamed_addr constant [34 x i8] c"void work(short &, short (&)[12])\00", align 1
@.str.2 = private unnamed_addr constant [19 x i8] c"yref_addr != &yref\00", align 1

define dso_local void @_Z4workRsRA12_s(ptr dereferenceable(2) %yref, ptr dereferenceable(24) %y_arr_ref) {
entry:
  %yref.addr = alloca ptr, align 8
  %y_arr_ref.addr = alloca ptr, align 8
  %yref_addr = alloca ptr, align 8
  %y_arr_ref_addr = alloca ptr, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %.omp.stride = alloca i64, align 8
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %yref, ptr %yref.addr, align 8
  store ptr %y_arr_ref, ptr %y_arr_ref.addr, align 8
  %0 = load ptr, ptr %yref.addr, align 8
  store ptr %0, ptr %yref_addr, align 8
  %1 = load ptr, ptr %y_arr_ref.addr, align 8
  store ptr %1, ptr %y_arr_ref_addr, align 8
  store i64 0, ptr %.omp.lb, align 8
  store i64 11, ptr %.omp.ub, align 8
  store i64 1, ptr %.omp.stride, align 8
  store i32 0, ptr %.omp.is_last, align 4
  %2 = load ptr, ptr %yref.addr, align 8
  %3 = load ptr, ptr %y_arr_ref.addr, align 8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.LASTPRIVATE:BYREF.TYPED"(ptr %yref.addr, i16 0, i32 1),
    "QUAL.OMP.LASTPRIVATE:BYREF.TYPED"(ptr %y_arr_ref.addr, [12 x i16] zeroinitializer, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %y_arr_ref_addr, ptr null, i32 1 ),
    "QUAL.OMP.SHARED:TYPED"(ptr %yref_addr, ptr null, i32 1) ]
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()

; Check that the task thunk has space for private copies of lastprivate variables.
; CHECK: [[PRIV_STRTY:%[a-zA-Z._0-9]+]] = type { i64, i64, i16, [12 x i16], i32 }
; CHECK: [[SHARED_STRTY:%[a-zA-Z._0-9]+]] = type { ptr, ptr, ptr, ptr }

; Check for lastprivate variables' addresses being stored to the shared area of task thunk.
; CHECK: [[LS1:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[SHARED_STRTY]], ptr %taskt.shared.agg, i32 0, i32 0
; CHECK: store ptr %yref.addr, ptr [[LS1]]
; CHECK: [[LS2:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[SHARED_STRTY]], ptr %taskt.shared.agg, i32 0, i32 1
; CHECK: store ptr %y_arr_ref.addr, ptr [[LS2]]

  %5 = load i64, ptr %.omp.lb, align 8
  %conv = trunc i64 %5 to i32
  store i32 %conv, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, ptr %.omp.iv, align 4
  %conv1 = sext i32 %6 to i64
  %7 = load i64, ptr %.omp.ub, align 8
  %cmp = icmp ule i64 %conv1, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %9 = load ptr, ptr %y_arr_ref_addr, align 8
  %10 = load ptr, ptr %y_arr_ref.addr, align 8
  %cmp2 = icmp ne ptr %9, %10
  br i1 %cmp2, label %cond.true, label %cond.false

cond.true:                                        ; preds = %omp.inner.for.body
  br label %cond.end

cond.false:                                       ; preds = %omp.inner.for.body
  call void @__assert_fail(ptr @.str, ptr @.str.1, i32 12, ptr @__PRETTY_FUNCTION__._Z4workRsRA12_s)
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %11 = load ptr, ptr %yref_addr, align 8
  %12 = load ptr, ptr %yref.addr, align 8
  %cmp3 = icmp ne ptr %11, %12
  br i1 %cmp3, label %cond.true4, label %cond.false5

cond.true4:                                       ; preds = %cond.end
  br label %cond.end6

cond.false5:                                      ; preds = %cond.end
  call void @__assert_fail(ptr @.str.2, ptr @.str.1, i32 13, ptr @__PRETTY_FUNCTION__._Z4workRsRA12_s)
  br label %cond.end6

cond.end6:                                        ; preds = %cond.false5, %cond.true4
  %13 = load i32, ptr %i, align 4
  %cmp7 = icmp eq i32 %13, 11
  br i1 %cmp7, label %if.then, label %if.end

if.then:                                          ; preds = %cond.end6
  %14 = load ptr, ptr %y_arr_ref.addr, align 8
  %15 = load i32, ptr %i, align 4
  %idxprom = sext i32 %15 to i64
  %arrayidx = getelementptr inbounds [12 x i16], ptr %14, i64 0, i64 %idxprom
  store i16 3, ptr %arrayidx, align 2
  %16 = load ptr, ptr %yref.addr, align 8
  store i16 3, ptr %16, align 2
  br label %if.end

if.end:                                           ; preds = %if.then, %cond.end6
  br label %omp.body.continue

omp.body.continue:                                ; preds = %if.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %17 = load i32, ptr %.omp.iv, align 4
  %add8 = add nsw i32 %17, 1
  store i32 %add8, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TASKLOOP"() ]
  ret void

; Check for lastprivate copyout
; CHECK: [[LP4:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[PRIV_STRTY]], ptr [[PRIVS:%[a-zA-Z._0-9]+]], i32 0, i32 2
; CHECK: [[LS4:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[SHARED_STRTY]], ptr [[SHAREDS:%[a-zA-Z._0-9]+]], i32 0, i32 0
; CHECK: [[LS4_STAR:%[a-zA-Z._0-9]+]] = load ptr, ptr [[LS4]]
; CHECK: [[LP3:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[PRIV_STRTY]], ptr [[PRIVS]], i32 0, i32 3
; CHECK: [[LS3:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[SHARED_STRTY]], ptr [[SHAREDS]], i32 0, i32 1
; CHECK: [[LS3_STAR:%[a-zA-Z._0-9]+]] = load ptr, ptr [[LS3]]

; CHECK: [[LS4_STAR_STAR:%[a-zA-Z._0-9]+]] = load ptr, ptr [[LS4_STAR]]
; CHECK: [[LP4_VAL:%[a-zA-Z._0-9]+]] = load i16, ptr [[LP4]]
; CHECK: store i16 [[LP4_VAL]], ptr [[LS4_STAR_STAR]]

; CHECK: [[LS3_STAR_STAR:%[a-zA-Z._0-9]+]] = load ptr, ptr [[LS3_STAR]]
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 2 [[LS3_STAR_STAR]], ptr align 2 [[LP3]], i64 24, i1 false)
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @__assert_fail(ptr, ptr, i32, ptr)
