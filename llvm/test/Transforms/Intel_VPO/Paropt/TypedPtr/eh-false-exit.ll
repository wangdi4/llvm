; RUN: opt -S -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' %s | FileCheck %s
; CHECK: define{{.*}}split
; CHECK: lpad{{.*}}loopexit:
; CHECK-NEXT: unreachable

; After inlining, the throws in the inner and middle regions have edges
; to the outer catch handler. This creates multiple-entry multiple-exit which
; crashes CodeExtractor.
; The EH edges can be broken before extraction.

;#include <stdio.h>
;volatile int cond = 1;
;__attribute__((always_inline)) void inner() {
;  try {
;    throw 4;
;  }
;  catch (float i) {
;    printf("x");
;    // This handler does not fire; the integer type exception must be
;    // thrown up 1 level
;  }
;}
;
;__attribute__((always_inline)) void middle(int n) {
;  if (cond) throw 5;
;#pragma omp parallel for
;  for (int i = 0; i < 100; i++) {
;    // After inlining, the OMP region contains the EH code from above
;    inner();
;  }
;}
;
;void outer() {
;  try {
;    middle(cond);
;  }
;  catch (int i) {
;    // All the EH code in the middle and inner layers leads here
;    printf("x");
;  }
;}


; ModuleID = '18967.ll'
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@cond = external dso_local global i32, align 4
@_ZTIf = external dso_local constant i8*
@_ZTIi = external dso_local constant i8*

; Function Attrs: nofree
declare dso_local noalias nonnull i8* @__cxa_allocate_exception(i64) local_unnamed_addr #0

; Function Attrs: nofree noreturn
declare dso_local void @__cxa_throw(i8*, i8*, i8*) local_unnamed_addr #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #3

; Function Attrs: nofree
declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr #0

; Function Attrs: nofree
declare dso_local void @__cxa_end_catch() local_unnamed_addr #0

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #3

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: uwtable
define dso_local void @_Z5outerv() local_unnamed_addr #5 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %.omp.iv.i = alloca i32, align 4
  %.omp.lb.i = alloca i32, align 4
  %.omp.ub.i = alloca i32, align 4
  %i.i = alloca i32, align 4
  %0 = load volatile i32, i32* @cond, align 4, !tbaa !2
  %savedstack = call i8* @llvm.stacksave()
  %1 = load volatile i32, i32* @cond, align 4, !tbaa !2
  %tobool.i = icmp eq i32 %1, 0
  br i1 %tobool.i, label %if.end.i, label %if.then.i

if.then.i:                                        ; preds = %entry
  %exception.i = call i8* @__cxa_allocate_exception(i64 4) #4
  %2 = bitcast i8* %exception.i to i32*
  store i32 5, i32* %2, align 16, !tbaa !2
  invoke void @__cxa_throw(i8* nonnull %exception.i, i8* bitcast (i8** @_ZTIi to i8*), i8* null) #7
          to label %.noexc unwind label %lpad

.noexc:                                           ; preds = %if.then.i
  unreachable

if.end.i:                                         ; preds = %entry
  %3 = bitcast i32* %.omp.iv.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3) #4
  %4 = bitcast i32* %.omp.lb.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %4) #4
  store i32 0, i32* %.omp.lb.i, align 4, !tbaa !2
  %5 = bitcast i32* %.omp.ub.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %5) #4
  store volatile i32 99, i32* %.omp.ub.i, align 4, !tbaa !2
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %if.end.i
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:                          ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv.i), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb.i), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub.i), "QUAL.OMP.PRIVATE"(i32* %i.i) ]
  br label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.2
  br label %DIR.OMP.PARALLEL.LOOP.3.i

DIR.OMP.PARALLEL.LOOP.3.i:                        ; preds = %DIR.OMP.PARALLEL.LOOP.3
  %7 = load i32, i32* %.omp.lb.i, align 4, !tbaa !2
  store volatile i32 %7, i32* %.omp.iv.i, align 4, !tbaa !2
  br label %omp.inner.for.cond.i

omp.inner.for.cond.i:                             ; preds = %_Z5innerv.exit.i, %DIR.OMP.PARALLEL.LOOP.3.i
  %8 = load volatile i32, i32* %.omp.iv.i, align 4, !tbaa !2
  %9 = load volatile i32, i32* %.omp.ub.i, align 4, !tbaa !2
  %cmp.i = icmp sgt i32 %8, %9
  br i1 %cmp.i, label %_Z6middlei.exit.loopexit, label %omp.inner.for.body.i

omp.inner.for.body.i:                             ; preds = %omp.inner.for.cond.i
  %10 = bitcast i32* %i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10) #4
  %11 = load volatile i32, i32* %.omp.iv.i, align 4, !tbaa !2
  store i32 %11, i32* %i.i, align 4, !tbaa !2
  %exception.i.i = call i8* @__cxa_allocate_exception(i64 4) #4
  %12 = bitcast i8* %exception.i.i to i32*
  store i32 4, i32* %12, align 16, !tbaa !2
  invoke void @__cxa_throw(i8* nonnull %exception.i.i, i8* bitcast (i8** @_ZTIi to i8*), i8* null) #7
          to label %unreachable.i.i unwind label %lpad.i.i

lpad.i.i:                                         ; preds = %omp.inner.for.body.i
  %13 = landingpad { i8*, i32 }
          catch i8* bitcast (i8** @_ZTIf to i8*)
          catch i8* bitcast (i8** @_ZTIi to i8*)
  %14 = extractvalue { i8*, i32 } %13, 1
  %15 = call i32 @llvm.eh.typeid.for(i8* bitcast (i8** @_ZTIf to i8*)) #4
  %matches.i.i = icmp eq i32 %14, %15
  br i1 %matches.i.i, label %_Z5innerv.exit.i, label %lpad.body.loopexit

unreachable.i.i:                                  ; preds = %omp.inner.for.body.i
  unreachable

_Z5innerv.exit.i:                                 ; preds = %lpad.i.i
  %16 = extractvalue { i8*, i32 } %13, 0
  %17 = call i8* @__cxa_begin_catch(i8* %16) #4
  %putchar.i.i = call i32 @putchar(i32 120) #4
  call void @__cxa_end_catch() #4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %10) #4
  %18 = load volatile i32, i32* %.omp.iv.i, align 4, !tbaa !2
  %add1.i = add nsw i32 %18, 1
  store volatile i32 %add1.i, i32* %.omp.iv.i, align 4, !tbaa !2
  br label %omp.inner.for.cond.i

_Z6middlei.exit.loopexit:                         ; preds = %omp.inner.for.cond.i
  br label %_Z6middlei.exit

_Z6middlei.exit:                                  ; preds = %_Z6middlei.exit.loopexit
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:                      ; preds = %_Z6middlei.exit
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.5

DIR.OMP.END.PARALLEL.LOOP.5:                      ; preds = %DIR.OMP.END.PARALLEL.LOOP.4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %5) #4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %4) #4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3) #4
  call void @llvm.stackrestore(i8* %savedstack)
  br label %try.cont

lpad:                                             ; preds = %if.then.i
  %19 = landingpad { i8*, i32 }
          catch i8* bitcast (i8** @_ZTIi to i8*)
  br label %lpad.body

lpad.body.loopexit:                               ; preds = %lpad.i.i
  br label %lpad.body

lpad.body:                                        ; preds = %lpad.body.loopexit, %lpad
  %eh.lpad-body = phi { i8*, i32 } [ %19, %lpad ], [ %13, %lpad.body.loopexit ]
  %20 = extractvalue { i8*, i32 } %eh.lpad-body, 1
  %21 = call i32 @llvm.eh.typeid.for(i8* bitcast (i8** @_ZTIi to i8*)) #4
  %matches = icmp eq i32 %20, %21
  br i1 %matches, label %catch, label %eh.resume

catch:                                            ; preds = %lpad.body
  %22 = extractvalue { i8*, i32 } %eh.lpad-body, 0
  %23 = call i8* @__cxa_begin_catch(i8* %22) #4
  %putchar = call i32 @putchar(i32 120)
  call void @__cxa_end_catch() #4
  br label %try.cont

try.cont:                                         ; preds = %catch, %DIR.OMP.END.PARALLEL.LOOP.5
  ret void

eh.resume:                                        ; preds = %lpad.body
  resume { i8*, i32 } %eh.lpad-body
}

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) #6

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #4

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #4

attributes #0 = { nofree }
attributes #1 = { nofree noreturn }
attributes #2 = { nounwind readnone }
attributes #3 = { argmemonly nounwind willreturn }
attributes #4 = { nounwind }
attributes #5 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { nofree nounwind }
attributes #7 = { noreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler Pro 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
