; RUN: opt < %s -vpo-paropt -S | FileCheck %s
;
; Test with both firstprivate and lastprivate.
; Checks for two things:
; barrier is not emitted (this should only be in parallel-for)
;
; %x.gep = getelementptr ... kmp_privates ... [0][0]
; ...
; %1 = load %x.gep
; store %1, %x.gep22
;
; CHECK: define{{.*}}OMP.TASKLOOP
; CHECK: [[LASTX:%x.gep[0-9]*]] = getelementptr{{.*}}struct.kmp_privates{{.*}}i32 0, i32 0
; CHECK-NOT: barrier
; CHECK: DIR.OMP.TASKLOOP{{.*}}:
; CHECK: [[XVAL:%[0-9]+]] = load i32{{.*}}%x.gep
; CHECK-NEXT: store i32 [[XVAL]]{{.*}}[[LASTX]]
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
define dso_local i32 @_Z7Computev() #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %x = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i = alloca i32, align 4
  %exn.slot = alloca i8*
  %ehselector.slot = alloca i32
  %0 = bitcast i32* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  store i32 -559038737, i32* %x, align 4, !tbaa !2
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i64* %.omp.lb), "QUAL.OMP.SHARED"(i32* %x), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  br label %DIR.OMP.SINGLE.3

DIR.OMP.SINGLE.3:                                 ; preds = %DIR.OMP.PARALLEL.2
  fence acquire
  %3 = load i32, i32* %x, align 4, !tbaa !2
  %call = invoke i32 (i8*, ...) @printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str, i64 0, i64 0), i32 %3)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %DIR.OMP.SINGLE.3
  %4 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #2
  %5 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %5) #2
  store i64 0, i64* %.omp.lb, align 8, !tbaa !6
  %6 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %6) #2
  store i64 3, i64* %.omp.ub, align 8, !tbaa !6
  br label %DIR.OMP.TASKLOOP.4

DIR.OMP.TASKLOOP.4:                               ; preds = %invoke.cont
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %x), "QUAL.OMP.LASTPRIVATE"(i32* %x), "QUAL.OMP.NUM_TASKS"(i32 4), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  br label %DIR.OMP.TASKLOOP.5

DIR.OMP.TASKLOOP.5:                               ; preds = %DIR.OMP.TASKLOOP.4
  %8 = load i64, i64* %.omp.lb, align 8, !tbaa !6
  %conv = trunc i64 %8 to i32
  store i32 %conv, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.TASKLOOP.5
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %conv1 = sext i32 %9 to i64
  %10 = load i64, i64* %.omp.ub, align 8, !tbaa !6
  %cmp = icmp ule i64 %conv1, %10
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %11) #2
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !2
  %13 = load i32, i32* %x, align 4, !tbaa !2
  %call3 = invoke i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.1, i64 0, i64 0), i32 %13)
          to label %invoke.cont2 unwind label %lpad

invoke.cont2:                                     ; preds = %omp.inner.for.body
  %14 = load i32, i32* %i, align 4, !tbaa !2
  %15 = load i32, i32* %x, align 4, !tbaa !2
  %add4 = add nsw i32 %15, %14
  store i32 %add4, i32* %x, align 4, !tbaa !2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %invoke.cont2
  %16 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %16) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %17 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %add5 = add nsw i32 %17, 1
  store i32 %add5, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

lpad:                                             ; preds = %omp.inner.for.body
  %18 = landingpad { i8*, i32 }
          catch i8* null
  %19 = extractvalue { i8*, i32 } %18, 0
  store i8* %19, i8** %exn.slot, align 8
  %20 = extractvalue { i8*, i32 } %18, 1
  store i32 %20, i32* %ehselector.slot, align 4
  %21 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %21) #2
  %22 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %22) #2
  %23 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %23) #2
  %24 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24) #2
  br label %terminate.handler

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TASKLOOP"() ]
  br label %DIR.OMP.END.TASKLOOP.6

DIR.OMP.END.TASKLOOP.6:                           ; preds = %omp.loop.exit
  %25 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %25) #2
  %26 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %26) #2
  %27 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %27) #2
  fence release
  br label %DIR.OMP.END.SINGLE.7

DIR.OMP.END.SINGLE.7:                             ; preds = %DIR.OMP.END.TASKLOOP.6
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SINGLE"() ]
  br label %DIR.OMP.END.SINGLE.8

DIR.OMP.END.SINGLE.8:                             ; preds = %DIR.OMP.END.SINGLE.7
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.END.SINGLE.8
  %call6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.2, i64 0, i64 0))
  %28 = load i32, i32* %x, align 4, !tbaa !2
  %29 = bitcast i32* %x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %29) #2
  ret i32 %28

terminate.lpad:                                   ; preds = %DIR.OMP.SINGLE.3
  %30 = landingpad { i8*, i32 }
          catch i8* null
  %31 = extractvalue { i8*, i32 } %30, 0
  call void @__clang_call_terminate(i8* %31) #6
  unreachable

terminate.handler:                                ; preds = %lpad
  %exn = load i8*, i8** %exn.slot, align 8
  call void @__clang_call_terminate(i8* %exn) #6
  unreachable
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local i32 @printf(i8*, ...) #3

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8*) #4 comdat {
  %2 = call i8* @__cxa_begin_catch(i8* %0) #2
  call void @_ZSt9terminatev() #6
  unreachable
}

declare dso_local i8* @__cxa_begin_catch(i8*)

declare dso_local void @_ZSt9terminatev()

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline noreturn nounwind }
attributes #5 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"long", !4, i64 0}
