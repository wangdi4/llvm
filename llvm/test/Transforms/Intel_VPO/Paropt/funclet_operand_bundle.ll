;RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
;RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;UNSUPPORTED : linux
;
;SRC input:
;
;#include <omp.h>
;float running_calc = 0.0;
;int main () {
; #pragma omp parallel num_threads(1)
;  try {
;       throw 1;
;  } catch (int t) {
;    #pragma omp for simd
;      for (int i = 0; i < 32; i++) {
;        int intermediate = i * i;
;        #pragma omp ordered threads simd ;
;       running_calc = intermediate + running_calc;
;    };
;  };
;}
;

;check that when running on windows every generated kmpc call that lies within a catch block has the funclet operand bundle.
;CHECK: define dso_local i32 @main
;CHECK:  call void @__kmpc_end(%struct.ident_t* @{{.*}})

;CHECK: define internal void @main.DIR.OMP.PARALLEL{{[^ ]*}}
;CHECK:  call void @__kmpc_for_static_init_{{.*}}(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 34, i32* %{{.*}}, i32* %{{.*}}, i32* %{{.*}}, i32* %{{.*}}, i32 1, i32 1) [ "funclet"(token %{{.*}}) ]
;CHECK:  call void @__kmpc_ordered(%struct.ident_t* @{{.*}}, i32 %{{.*}}) [ "funclet"(token %{{.*}}) ]
;CHECK:  call void @__kmpc_end_ordered(%struct.ident_t* @{{.*}}, i32 %{{.*}}) [ "funclet"(token %{{.*}} ]
;CHECK:  call void @__kmpc_for_static_fini(%struct.ident_t* @{{.*}}, i32 %{{.*}}) [ "funclet"(token %{{.*}} ]
;CHECK:  call void @__kmpc_barrier(%struct.ident_t* @{{.*}}, i32 %{{.*}}) [ "funclet"(token %{{.*}}) ]

; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%rtti.TypeDescriptor2 = type { i8**, i8*, [3 x i8] }
%eh.CatchableType = type { i32, i32, i32, i32, i32, i32, i32 }
%eh.CatchableTypeArray.1 = type { i32, [1 x i32] }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

$"??_R0H@8" = comdat any

$"_CT??_R0H@84" = comdat any

$_CTA1H = comdat any

$_TI1H = comdat any

@"?running_calc@@3MA" = dso_local global float 0.000000e+00, align 4
@"??_7type_info@@6B@" = external constant i8*
@"??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { i8** @"??_7type_info@@6B@", i8* null, [3 x i8] c".H\00" }, comdat
@__ImageBase = external dso_local constant i8
@"_CT??_R0H@84" = linkonce_odr unnamed_addr constant %eh.CatchableType { i32 1, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor2* @"??_R0H@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 0, i32 -1, i32 0, i32 4, i32 0 }, section ".xdata", comdat
@_CTA1H = linkonce_odr unnamed_addr constant %eh.CatchableTypeArray.1 { i32 1, [1 x i32] [i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%eh.CatchableType* @"_CT??_R0H@84" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32)] }, section ".xdata", comdat
@_TI1H = linkonce_odr unnamed_addr constant %eh.ThrowInfo { i32 0, i32 0, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%eh.CatchableTypeArray.1* @_CTA1H to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, section ".xdata", comdat

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define dso_local i32 @main() #0 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %retval = alloca i32, align 4
  %tmp = alloca i32, align 4
  %t = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %intermediate = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 1), "QUAL.OMP.SHARED"(float* @"?running_calc@@3MA"), "QUAL.OMP.PRIVATE"(i32* %t), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %intermediate), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp1) ]
  store i32 1, i32* %tmp, align 4
  %1 = bitcast i32* %tmp to i8*
  invoke void @_CxxThrowException(i8* %1, %eh.ThrowInfo* @_TI1H) #2
          to label %unreachable unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %entry
  %2 = catchswitch within none [label %catch] unwind label %terminate

catch:                                            ; preds = %catch.dispatch
  %3 = catchpad within %2 [%rtti.TypeDescriptor2* @"??_R0H@8", i32 0, i32* %t]
  store i32 0, i32* %.omp.lb, align 4
  store i32 31, i32* %.omp.ub, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %intermediate) ]
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1) ]
  %6 = load i32, i32* %.omp.lb, align 4
  store i32 %6, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %catch
  %7 = load i32, i32* %.omp.iv, align 4
  %8 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %10 = load i32, i32* %i, align 4
  %11 = load i32, i32* %i, align 4
  %mul2 = mul nsw i32 %10, %11
  store i32 %mul2, i32* %intermediate, align 4
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(), "QUAL.OMP.ORDERED.THREADS"(), "QUAL.OMP.ORDERED.SIMD"() ]
  %13 = load i32, i32* %intermediate, align 4
  %conv = sitofp i32 %13 to float
  %14 = load float, float* @"?running_calc@@3MA", align 4
  %add3 = fadd float %conv, %14
  store float %add3, float* @"?running_calc@@3MA", align 4
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.ORDERED"() ]
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, i32* %.omp.iv, align 4
  %add4 = add nsw i32 %15, 1
  store i32 %add4, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.LOOP"() ]
  catchret from %3 to label %catchret.dest

catchret.dest:                                    ; preds = %omp.loop.exit
  br label %try.cont

try.cont:                                         ; preds = %catchret.dest
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %16 = load i32, i32* %retval, align 4
  ret i32 %16

unreachable:                                      ; preds = %entry
  unreachable

terminate:                                        ; preds = %catch.dispatch
  %17 = cleanuppad within none []
  call void @__std_terminate() #3 [ "funclet"(token %17) ]
  unreachable
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_CxxThrowException(i8*, %eh.ThrowInfo*)

declare dso_local i32 @__CxxFrameHandler3(...)

declare dso_local void @__std_terminate()

attributes #0 = { noinline norecurse nounwind optnone uwtable mustprogress "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { noreturn }
attributes #3 = { noreturn nounwind }
