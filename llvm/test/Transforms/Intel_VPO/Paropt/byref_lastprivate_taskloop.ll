; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s
;
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
; ModuleID = 'byref_lastprivate_taskloop.cpp'
source_filename = "byref_lastprivate_taskloop.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [38 x i8] c"y_arr_ref_addr != (short*) &y_arr_ref\00", align 1
@.str.1 = private unnamed_addr constant [31 x i8] c"byref_lastprivate_taskloop.cpp\00", align 1
@__PRETTY_FUNCTION__._Z4workRsRA12_s = private unnamed_addr constant [34 x i8] c"void work(short &, short (&)[12])\00", align 1
@.str.2 = private unnamed_addr constant [19 x i8] c"yref_addr != &yref\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z4workRsRA12_s(i16* dereferenceable(2) %yref, [12 x i16]* dereferenceable(24) %y_arr_ref) #0 {
entry:
  %yref.addr = alloca i16*, align 8
  %y_arr_ref.addr = alloca [12 x i16]*, align 8
  %yref_addr = alloca i16*, align 8
  %y_arr_ref_addr = alloca i16*, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %.omp.stride = alloca i64, align 8
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store i16* %yref, i16** %yref.addr, align 8
  store [12 x i16]* %y_arr_ref, [12 x i16]** %y_arr_ref.addr, align 8
  %0 = load i16*, i16** %yref.addr, align 8
  store i16* %0, i16** %yref_addr, align 8
  %1 = load [12 x i16]*, [12 x i16]** %y_arr_ref.addr, align 8
  %2 = bitcast [12 x i16]* %1 to i16*
  store i16* %2, i16** %y_arr_ref_addr, align 8
  store i64 0, i64* %.omp.lb, align 8
  store i64 11, i64* %.omp.ub, align 8
  store i64 1, i64* %.omp.stride, align 8
  store i32 0, i32* %.omp.is_last, align 4
  %3 = load i16*, i16** %yref.addr, align 8
  %4 = load [12 x i16]*, [12 x i16]** %y_arr_ref.addr, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(), "QUAL.OMP.LASTPRIVATE:BYREF"(i16** %yref.addr), "QUAL.OMP.LASTPRIVATE:BYREF"([12 x i16]** %y_arr_ref.addr), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(i16** %y_arr_ref_addr), "QUAL.OMP.SHARED"(i16** %yref_addr) ]
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()

; Check that the task thunk has space for private copies of lastprivate variables.
; CHECK: [[PRIV_STRTY:%[a-zA-Z._0-9]+]] = type { i64, i16, [12 x i16], i32 }
; CHECK: [[SHARED_STRTY:%[a-zA-Z._0-9]+]] = type { i16**, [12 x i16]**, i16**, i16** }

; Check for lastprivate variables' addresses being stored to the shared area of task thunk.
; CHECK: [[LS1:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[SHARED_STRTY]], [[SHARED_STRTY]]* %taskt.shared.agg, i32 0, i32 0
; CHECK: store i16** %yref.addr, i16*** [[LS1]]
; CHECK: [[LS2:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[SHARED_STRTY]], [[SHARED_STRTY]]* %taskt.shared.agg, i32 0, i32 1
; CHECK: store [12 x i16]** %y_arr_ref.addr, [12 x i16]*** [[LS2]]

  %6 = load i64, i64* %.omp.lb, align 8
  %conv = trunc i64 %6 to i32
  store i32 %conv, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, i32* %.omp.iv, align 4
  %conv1 = sext i32 %7 to i64
  %8 = load i64, i64* %.omp.ub, align 8
  %cmp = icmp ule i64 %conv1, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %10 = load i16*, i16** %y_arr_ref_addr, align 8
  %11 = load [12 x i16]*, [12 x i16]** %y_arr_ref.addr, align 8
  %12 = bitcast [12 x i16]* %11 to i16*
  %cmp2 = icmp ne i16* %10, %12
  br i1 %cmp2, label %cond.true, label %cond.false

cond.true:                                        ; preds = %omp.inner.for.body
  br label %cond.end

cond.false:                                       ; preds = %omp.inner.for.body
  call void @__assert_fail(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([31 x i8], [31 x i8]* @.str.1, i32 0, i32 0), i32 12, i8* getelementptr inbounds ([34 x i8], [34 x i8]* @__PRETTY_FUNCTION__._Z4workRsRA12_s, i32 0, i32 0)) #3
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %13 = load i16*, i16** %yref_addr, align 8
  %14 = load i16*, i16** %yref.addr, align 8
  %cmp3 = icmp ne i16* %13, %14
  br i1 %cmp3, label %cond.true4, label %cond.false5

cond.true4:                                       ; preds = %cond.end
  br label %cond.end6

cond.false5:                                      ; preds = %cond.end
  call void @__assert_fail(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str.2, i32 0, i32 0), i8* getelementptr inbounds ([31 x i8], [31 x i8]* @.str.1, i32 0, i32 0), i32 13, i8* getelementptr inbounds ([34 x i8], [34 x i8]* @__PRETTY_FUNCTION__._Z4workRsRA12_s, i32 0, i32 0)) #3
  br label %cond.end6

cond.end6:                                        ; preds = %cond.false5, %cond.true4
  %15 = load i32, i32* %i, align 4
  %cmp7 = icmp eq i32 %15, 11
  br i1 %cmp7, label %if.then, label %if.end

if.then:                                          ; preds = %cond.end6
  %16 = load [12 x i16]*, [12 x i16]** %y_arr_ref.addr, align 8
  %17 = load i32, i32* %i, align 4
  %idxprom = sext i32 %17 to i64
  %arrayidx = getelementptr inbounds [12 x i16], [12 x i16]* %16, i64 0, i64 %idxprom
  store i16 3, i16* %arrayidx, align 2
  %18 = load i16*, i16** %yref.addr, align 8
  store i16 3, i16* %18, align 2
  br label %if.end

if.end:                                           ; preds = %if.then, %cond.end6
  br label %omp.body.continue

omp.body.continue:                                ; preds = %if.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %19 = load i32, i32* %.omp.iv, align 4
  %add8 = add nsw i32 %19, 1
  store i32 %add8, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TASKLOOP"() ]
  ret void

; Check for lastprivate copyout

; CHECK: [[LP4:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[PRIV_STRTY]], [[PRIV_STRTY]]* [[PRIVS:%[a-zA-Z._0-9]+]], i32 0, i32 1
; CHECK: [[LS4:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[SHARED_STRTY]], [[SHARED_STRTY]]* [[SHAREDS:%[a-zA-Z._0-9]+]], i32 0, i32 0
; CHECK: [[LS4_STAR:%[a-zA-Z._0-9]+]] = load i16**, i16*** [[LS4]]
; CHECK: [[LP3:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[PRIV_STRTY]], [[PRIV_STRTY]]* [[PRIVS]], i32 0, i32 2
; CHECK: [[LS3:%[a-zA-Z._0-9]+]] = getelementptr inbounds [[SHARED_STRTY]], [[SHARED_STRTY]]* [[SHAREDS]], i32 0, i32 1
; CHECK: [[LS3_STAR:%[a-zA-Z._0-9]+]] = load [12 x i16]**, [12 x i16]*** [[LS3]]

; CHECK: [[LS4_STAR_STAR:%[a-zA-Z._0-9]+]] = load i16*, i16** [[LS4_STAR]]
; CHECK: [[LP4_VAL:%[a-zA-Z._0-9]+]] = load i16, i16* [[LP4]]
; CHECK: store i16 [[LP4_VAL]], i16* [[LS4_STAR_STAR]]

; CHECK: [[LS3_STAR_STAR:%[a-zA-Z._0-9]+]] = load [12 x i16]*, [12 x i16]** [[LS3_STAR]]
; CHECK: [[LS3_STAR_STAR_BC:%[a-zA-Z._0-9]+]] = bitcast [12 x i16]* [[LS3_STAR_STAR]] to i8*
; CHECK: [[LP3_BC:%[a-zA-Z._0-9]+]] = bitcast [12 x i16]* [[LP3]] to i8*
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 2 [[LS3_STAR_STAR_BC]], i8* align 2 [[LP3_BC]], i64 24, i1 false)
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noreturn nounwind
declare dso_local void @__assert_fail(i8*, i8*, i32, i8*) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
