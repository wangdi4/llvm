; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="vpo-paropt" -S %s | FileCheck %s
;
; Test with both firstprivate and lastprivate.
; Checks for:
; barrier is not emitted (this should only be in parallel-for)
;
; CHECK: %__struct.kmp_privates.t = type { i32, i64, i64, i32 }
; CHECK: %__struct.shared.t = type { ptr }
; CHECK: define{{.*}}OMP.TASKLOOP
; CHECK: {{[^ ]+}} = getelementptr{{.*}}struct.kmp_privates{{.*}}i32 0, i32 0
; CHECK-NOT: barrier
; CHECK: DIR.OMP.TASKLOOP{{.*}}:
; CHECK-NOT: barrier

; #include <stdio.h>
;
; int __attribute__ ((noinline)) Compute(void)
; {
;   int x = 0xdeadbeef;
; #pragma omp parallel
;   {
; #pragma omp single
;   {
;     printf("x before loop = %d\n", x);
; #pragma omp taskloop firstprivate(x) lastprivate(x) num_tasks(4)
;     for (int i = 0; i < 4; i++) {
;       printf("x in loop = %d\n", x);
;       x += i;
;     }
;   }
;   }
;   printf("returning\n");
;   return x;
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$__clang_call_terminate = comdat any

@.str = private unnamed_addr constant [20 x i8] c"x before loop = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [16 x i8] c"x in loop = %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [11 x i8] c"returning\0A\00", align 1
@.str.3 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline uwtable
define dso_local i32 @_Z7Computev() #0 personality ptr @__gxx_personality_v0 {
entry:
  %x = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i = alloca i32, align 4
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  %0 = bitcast ptr %x to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %0) #1
  store i32 -559038737, ptr %x, align 4, !tbaa !2
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i64 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %x, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]

  br label %DIR.OMP.SINGLE.3

DIR.OMP.SINGLE.3:                                 ; preds = %DIR.OMP.PARALLEL.2
  fence acquire
  %3 = load i32, ptr %x, align 4, !tbaa !2
  %call = invoke i32 (ptr, ...) @printf(ptr @.str, i32 %3)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %DIR.OMP.SINGLE.3
  %4 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %4) #1
  %5 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %5) #1
  store i64 0, ptr %.omp.lb, align 8, !tbaa !6
  %6 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %6) #1
  store i64 3, ptr %.omp.ub, align 8, !tbaa !6
  br label %DIR.OMP.TASKLOOP.4

DIR.OMP.TASKLOOP.4:                               ; preds = %invoke.cont
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %x, i32 0, i32 1),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %x, i32 0, i32 1),
    "QUAL.OMP.NUM_TASKS"(i32 4),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  br label %DIR.OMP.TASKLOOP.5

DIR.OMP.TASKLOOP.5:                               ; preds = %DIR.OMP.TASKLOOP.4
  %8 = load i64, ptr %.omp.lb, align 8, !tbaa !6
  %conv = trunc i64 %8 to i32
  store i32 %conv, ptr %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.TASKLOOP.5
  %9 = load i32, ptr %.omp.iv, align 4, !tbaa !2
  %conv1 = sext i32 %9 to i64
  %10 = load i64, ptr %.omp.ub, align 8, !tbaa !6
  %cmp = icmp ule i64 %conv1, %10
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %11) #1
  %12 = load i32, ptr %.omp.iv, align 4, !tbaa !2
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4, !tbaa !2
  %13 = load i32, ptr %x, align 4, !tbaa !2
  %call3 = invoke i32 (ptr, ...) @printf(ptr @.str.1, i32 %13)
          to label %invoke.cont2 unwind label %lpad

invoke.cont2:                                     ; preds = %omp.inner.for.body
  %14 = load i32, ptr %i, align 4, !tbaa !2
  %15 = load i32, ptr %x, align 4, !tbaa !2
  %add4 = add nsw i32 %15, %14
  store i32 %add4, ptr %x, align 4, !tbaa !2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %invoke.cont2
  %16 = bitcast ptr %i to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %16) #1
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %17 = load i32, ptr %.omp.iv, align 4, !tbaa !2
  %add5 = add nsw i32 %17, 1
  store i32 %add5, ptr %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

lpad:                                             ; preds = %omp.inner.for.body
  %18 = landingpad { ptr, i32 }
          catch ptr null
  %19 = extractvalue { ptr, i32 } %18, 0
  store ptr %19, ptr %exn.slot, align 8
  %20 = extractvalue { ptr, i32 } %18, 1
  store i32 %20, ptr %ehselector.slot, align 4
  %21 = bitcast ptr %i to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %21) #1
  %22 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %22) #1
  %23 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %23) #1
  %24 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %24) #1
  br label %terminate.handler

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TASKLOOP"() ]

  br label %DIR.OMP.END.TASKLOOP.6

DIR.OMP.END.TASKLOOP.6:                           ; preds = %omp.loop.exit
  %25 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %25) #1
  %26 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %26) #1
  %27 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %27) #1
  fence release
  br label %DIR.OMP.END.SINGLE.7

DIR.OMP.END.SINGLE.7:                             ; preds = %DIR.OMP.END.TASKLOOP.6
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SINGLE"() ]

  br label %DIR.OMP.END.SINGLE.8

DIR.OMP.END.SINGLE.8:                             ; preds = %DIR.OMP.END.SINGLE.7
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.END.SINGLE.8
  %call6 = call i32 (ptr, ...) @printf(ptr @.str.2)
  %28 = load i32, ptr %x, align 4, !tbaa !2
  %29 = bitcast ptr %x to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %29) #1
  ret i32 %28

terminate.lpad:                                   ; preds = %DIR.OMP.SINGLE.3
  %30 = landingpad { ptr, i32 }
          catch ptr null
  %31 = extractvalue { ptr, i32 } %30, 0
  call void @__clang_call_terminate(ptr %31) #5
  unreachable

terminate.handler:                                ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  call void @__clang_call_terminate(ptr %exn) #5
  unreachable
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(ptr, ...) #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(ptr %0) #3 comdat {
  %2 = call ptr @__cxa_begin_catch(ptr %0) #1
  call void @_ZSt9terminatev() #5
  unreachable
}

declare dso_local ptr @__cxa_begin_catch(ptr)

declare dso_local void @_ZSt9terminatev()

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #4

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #4

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline noreturn nounwind }
attributes #4 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #5 = { noreturn nounwind }

!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"long", !4, i64 0}
