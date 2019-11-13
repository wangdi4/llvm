; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s
;
; Test src:
;
; #include <assert.h>
;
; void byref_fp_lp(int &yref, float (&y_arr_ref)[10][10]) {
;   int *yref_addr = &yref;
;   float(*y_arr_ref_addr)[10][10] = &y_arr_ref;
;   yref = 2;
;   y_arr_ref[1][1] = 3;
;
; #pragma omp parallel for firstprivate(yref, y_arr_ref) lastprivate(yref, y_arr_ref)
;   for (int i = 0; i < 10; i++) {
;     assert(yref_addr != &yref);
;     assert(y_arr_ref_addr != &y_arr_ref);
;     assert(yref == 2);
;     assert(y_arr_ref[1][1] == 3);
;
;     yref = 5;
;     y_arr_ref[1][1] = 6;
;   }
; }
;
; int main() {
;   int y = 10;
;   float yarr[10][10];
;   byref_fp_lp(y, yarr);
;   assert(y == 5);
;   assert(yarr[1][1] == 6);
;   return 0;
; }
;
; ModuleID = 'byref_fp_lp.cpp'
source_filename = "byref_fp_lp.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [19 x i8] c"yref_addr != &yref\00", align 1
@.str.1 = private unnamed_addr constant [16 x i8] c"byref_fp_lp.cpp\00", align 1
@__PRETTY_FUNCTION__._Z11byref_fp_lpRiRA10_A10_f = private unnamed_addr constant [43 x i8] c"void byref_fp_lp(int &, float (&)[10][10])\00", align 1
@.str.2 = private unnamed_addr constant [29 x i8] c"y_arr_ref_addr != &y_arr_ref\00", align 1
@.str.3 = private unnamed_addr constant [10 x i8] c"yref == 2\00", align 1
@.str.4 = private unnamed_addr constant [21 x i8] c"y_arr_ref[1][1] == 3\00", align 1
@.str.5 = private unnamed_addr constant [7 x i8] c"y == 5\00", align 1
@__PRETTY_FUNCTION__.main = private unnamed_addr constant [11 x i8] c"int main()\00", align 1
@.str.6 = private unnamed_addr constant [16 x i8] c"yarr[1][1] == 6\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z11byref_fp_lpRiRA10_A10_f(i32* dereferenceable(4) %yref, [10 x [10 x float]]* dereferenceable(400) %y_arr_ref) #0 {
entry:
  %yref.addr = alloca i32*, align 8
  %y_arr_ref.addr = alloca [10 x [10 x float]]*, align 8
  %yref_addr = alloca i32*, align 8
  %y_arr_ref_addr = alloca [10 x [10 x float]]*, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store i32* %yref, i32** %yref.addr, align 8
  store [10 x [10 x float]]* %y_arr_ref, [10 x [10 x float]]** %y_arr_ref.addr, align 8
  %0 = load i32*, i32** %yref.addr, align 8
  store i32* %0, i32** %yref_addr, align 8
  %1 = load [10 x [10 x float]]*, [10 x [10 x float]]** %y_arr_ref.addr, align 8
  store [10 x [10 x float]]* %1, [10 x [10 x float]]** %y_arr_ref_addr, align 8
  %2 = load i32*, i32** %yref.addr, align 8
  store i32 2, i32* %2, align 4
  %3 = load [10 x [10 x float]]*, [10 x [10 x float]]** %y_arr_ref.addr, align 8
  %arrayidx = getelementptr inbounds [10 x [10 x float]], [10 x [10 x float]]* %3, i64 0, i64 1
  %arrayidx1 = getelementptr inbounds [10 x float], [10 x float]* %arrayidx, i64 0, i64 1
  store float 3.000000e+00, float* %arrayidx1, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 9, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  %4 = load i32*, i32** %yref.addr, align 8
  %5 = load [10 x [10 x float]]*, [10 x [10 x float]]** %y_arr_ref.addr, align 8
  %6 = load i32*, i32** %yref.addr, align 8
  %7 = load [10 x [10 x float]]*, [10 x [10 x float]]** %y_arr_ref.addr, align 8
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE:BYREF"(i32** %yref.addr), "QUAL.OMP.FIRSTPRIVATE:BYREF"([10 x [10 x float]]** %y_arr_ref.addr), "QUAL.OMP.LASTPRIVATE:BYREF"(i32** %yref.addr), "QUAL.OMP.LASTPRIVATE:BYREF"([10 x [10 x float]]** %y_arr_ref.addr), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(i32** %yref_addr), "QUAL.OMP.SHARED"([10 x [10 x float]]** %y_arr_ref_addr) ]
  %9 = load i32, i32* %.omp.lb, align 4
  store i32 %9, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()

; Look for allocation of local copies for byref firstprivates.
; CHECK: [[YREF_ARR_LOCAL:%y_arr_ref.addr.lpriv]] = alloca [10 x [10 x float]]
; CHECK: [[YREF_LOCAL:%yref.addr.lpriv]] = alloca i32
; CHECK: store [10 x [10 x float]]* [[YREF_ARR_LOCAL]], [10 x [10 x float]]** [[YREF_ARR_LOCAL_ADDR:%[a-zA-Z._0-9]+]]
; CHECK: store i32* [[YREF_LOCAL]], i32** [[YREF_LOCAL_ADDR:%[a-zA-Z._0-9]+]]

; Look for initialization of local copies of byref firstprivates.
; CHECK: [[L0:%[a-zA-Z._0-9]+]] = load [10 x [10 x float]]*, [10 x [10 x float]]** %y_arr_ref.addr
; CHECK: [[B1:%[a-zA-Z._0-9]+]]  = bitcast [10 x [10 x float]]* [[YREF_ARR_LOCAL]] to i8*
; CHECK: [[B2:%[a-zA-Z._0-9]+]] = bitcast [10 x [10 x float]]* [[L0]] to i8*
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8*{{.*}}[[B1]], i8*{{.*}}[[B2]], i64 400, i1 false)

; CHECK: [[L1:%[a-zA-Z._0-9]+]] = load i32*, i32** %yref.addr
; CHECK: [[L2:%[a-zA-Z._0-9]+]] = load i32, i32* [[L1]]
; CHECK: store i32 [[L2]], i32* [[YREF_LOCAL]]

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %10 = load i32, i32* %.omp.iv, align 4
  %11 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %10, %11
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %13 = load i32*, i32** %yref_addr, align 8
  %14 = load i32*, i32** %yref.addr, align 8
  %cmp2 = icmp ne i32* %13, %14
; Look for use of local 'yref' instead of the original inside the region.
; CHECK: [[L3:%[a-zA-Z._0-9]+]] = load i32*, i32** %yref_addr
; CHECK: [[L4:%[a-zA-Z._0-9]+]] = load i32*, i32** [[YREF_LOCAL_ADDR]]
; CHECK: icmp ne i32* [[L3]], [[L4]]
  br i1 %cmp2, label %cond.true, label %cond.false

cond.true:                                        ; preds = %omp.inner.for.body
  br label %cond.end

cond.false:                                       ; preds = %omp.inner.for.body
  call void @__assert_fail(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.1, i32 0, i32 0), i32 11, i8* getelementptr inbounds ([43 x i8], [43 x i8]* @__PRETTY_FUNCTION__._Z11byref_fp_lpRiRA10_A10_f, i32 0, i32 0)) #4
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %15 = load [10 x [10 x float]]*, [10 x [10 x float]]** %y_arr_ref_addr, align 8
  %16 = load [10 x [10 x float]]*, [10 x [10 x float]]** %y_arr_ref.addr, align 8
  %cmp3 = icmp ne [10 x [10 x float]]* %15, %16
; Look for use of local 'y_arr_ref' instead of the original inside the region.
; CHECK: [[L5:%[a-zA-Z._0-9]+]] = load [10 x [10 x float]]*, [10 x [10 x float]]** %y_arr_ref_addr
; CHECK: [[L6:%[a-zA-Z._0-9]+]] = load [10 x [10 x float]]*, [10 x [10 x float]]** [[YREF_ARR_LOCAL_ADDR]]
; CHECK: icmp ne [10 x [10 x float]]* [[L5]], [[L6]]
  br i1 %cmp3, label %cond.true4, label %cond.false5

cond.true4:                                       ; preds = %cond.end
  br label %cond.end6

cond.false5:                                      ; preds = %cond.end
  call void @__assert_fail(i8* getelementptr inbounds ([29 x i8], [29 x i8]* @.str.2, i32 0, i32 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.1, i32 0, i32 0), i32 12, i8* getelementptr inbounds ([43 x i8], [43 x i8]* @__PRETTY_FUNCTION__._Z11byref_fp_lpRiRA10_A10_f, i32 0, i32 0)) #4
  br label %cond.end6

cond.end6:                                        ; preds = %cond.false5, %cond.true4
  %17 = load i32*, i32** %yref.addr, align 8
  %18 = load i32, i32* %17, align 4
  %cmp7 = icmp eq i32 %18, 2
  br i1 %cmp7, label %cond.true8, label %cond.false9

cond.true8:                                       ; preds = %cond.end6
  br label %cond.end10

cond.false9:                                      ; preds = %cond.end6
  call void @__assert_fail(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.1, i32 0, i32 0), i32 13, i8* getelementptr inbounds ([43 x i8], [43 x i8]* @__PRETTY_FUNCTION__._Z11byref_fp_lpRiRA10_A10_f, i32 0, i32 0)) #4
  br label %cond.end10

cond.end10:                                       ; preds = %cond.false9, %cond.true8
  %19 = load [10 x [10 x float]]*, [10 x [10 x float]]** %y_arr_ref.addr, align 8
  %arrayidx11 = getelementptr inbounds [10 x [10 x float]], [10 x [10 x float]]* %19, i64 0, i64 1
  %arrayidx12 = getelementptr inbounds [10 x float], [10 x float]* %arrayidx11, i64 0, i64 1
  %20 = load float, float* %arrayidx12, align 4
  %cmp13 = fcmp oeq float %20, 3.000000e+00
  br i1 %cmp13, label %cond.true14, label %cond.false15

cond.true14:                                      ; preds = %cond.end10
  br label %cond.end16

cond.false15:                                     ; preds = %cond.end10
  call void @__assert_fail(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @.str.4, i32 0, i32 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.1, i32 0, i32 0), i32 14, i8* getelementptr inbounds ([43 x i8], [43 x i8]* @__PRETTY_FUNCTION__._Z11byref_fp_lpRiRA10_A10_f, i32 0, i32 0)) #4
  br label %cond.end16

cond.end16:                                       ; preds = %cond.false15, %cond.true14
  %21 = load i32*, i32** %yref.addr, align 8
  store i32 5, i32* %21, align 4
  %22 = load [10 x [10 x float]]*, [10 x [10 x float]]** %y_arr_ref.addr, align 8
  %arrayidx17 = getelementptr inbounds [10 x [10 x float]], [10 x [10 x float]]* %22, i64 0, i64 1
  %arrayidx18 = getelementptr inbounds [10 x float], [10 x float]* %arrayidx17, i64 0, i64 1
  store float 6.000000e+00, float* %arrayidx18, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %cond.end16
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %23 = load i32, i32* %.omp.iv, align 4
  %add19 = add nsw i32 %23, 1
  store i32 %add19, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
; Look for lastprivate copyout instructions.
; CHECK: [[L7:%[a-zA-Z._0-9]+]] = load i32*, i32** %yref.addr
; CHECK: [[L8:%[a-zA-Z._0-9]+]] = load i32, i32* [[YREF_LOCAL]]
; CHECK: store i32 [[L8]], i32* [[L7]]

; CHECK: [[L9:%[a-zA-Z._0-9]+]] = load [10 x [10 x float]]*, [10 x [10 x float]]** %y_arr_ref.addr
; CHECK: [[B3:%[a-zA-Z._0-9]+]] = bitcast [10 x [10 x float]]* [[L9]] to i8*
; CHECK: [[B4:%[a-zA-Z._0-9]+]] = bitcast [10 x [10 x float]]* [[YREF_ARR_LOCAL]] to i8*
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8*{{.*}}[[B3]], i8*{{.*}}[[B4]], i64 400, i1 false)
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noreturn nounwind
declare dso_local void @__assert_fail(i8*, i8*, i32, i8*) #2

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #3 {
entry:
  %retval = alloca i32, align 4
  %y = alloca i32, align 4
  %yarr = alloca [10 x [10 x float]], align 16
  store i32 0, i32* %retval, align 4
  store i32 10, i32* %y, align 4
  call void @_Z11byref_fp_lpRiRA10_A10_f(i32* dereferenceable(4) %y, [10 x [10 x float]]* dereferenceable(400) %yarr)
  %0 = load i32, i32* %y, align 4
  %cmp = icmp eq i32 %0, 5
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  br label %cond.end

cond.false:                                       ; preds = %entry
  call void @__assert_fail(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.5, i32 0, i32 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.1, i32 0, i32 0), i32 25, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @__PRETTY_FUNCTION__.main, i32 0, i32 0)) #4
  unreachable
                                                  ; No predecessors!
  br label %cond.end

cond.end:                                         ; preds = %1, %cond.true
  %arrayidx = getelementptr inbounds [10 x [10 x float]], [10 x [10 x float]]* %yarr, i64 0, i64 1
  %arrayidx1 = getelementptr inbounds [10 x float], [10 x float]* %arrayidx, i64 0, i64 1
  %2 = load float, float* %arrayidx1, align 4
  %cmp2 = fcmp oeq float %2, 6.000000e+00
  br i1 %cmp2, label %cond.true3, label %cond.false4

cond.true3:                                       ; preds = %cond.end
  br label %cond.end5

cond.false4:                                      ; preds = %cond.end
  call void @__assert_fail(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.6, i32 0, i32 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.1, i32 0, i32 0), i32 26, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @__PRETTY_FUNCTION__.main, i32 0, i32 0)) #4
  unreachable
                                                  ; No predecessors!
  br label %cond.end5

cond.end5:                                        ; preds = %3, %cond.true3
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
