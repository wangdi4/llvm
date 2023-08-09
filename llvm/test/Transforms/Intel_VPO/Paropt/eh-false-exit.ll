; RUN: opt -S -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
; volatile int cond = 1;
; __attribute__((always_inline)) void inner() {
;   try {
;     throw 4;
;   } catch (float i) {
;     printf("x");
;     // This handler does not fire; the integer type exception must be
;     // thrown up 1 level
;   }
; }
;
; __attribute__((always_inline)) void middle(int n) {
;   if (cond)
;     throw 5;
; #pragma omp parallel for
;   for (int i = 0; i < 100; i++) {
;     // After inlining, the OMP region contains the EH code from above
;     inner();
;   }
; }
;
; void outer() {
;   try {
;     middle(cond);
;   } catch (int i) {
;     // All the EH code in the middle and inner layers leads here
;     printf("x");
;   }
; }

; CHECK: define{{.*}}split
; CHECK: lpad{{.*}}loopexit:
; CHECK-NEXT: unreachable

; After inlining, the throws in the inner and middle regions have edges
; to the outer catch handler. This creates multiple-entry multiple-exit which
; crashes CodeExtractor.
; The EH edges can be broken before extraction.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@cond = dso_local global i32 1, align 4
@_ZTIf = external dso_local constant ptr
@_ZTIi = external dso_local constant ptr
@.str = private unnamed_addr constant [2 x i8] c"x\00", align 1

; Function Attrs: nofree
declare dso_local noalias ptr @__cxa_allocate_exception(i64) local_unnamed_addr #1

; Function Attrs: nofree noreturn
declare dso_local void @__cxa_throw(ptr, ptr, ptr) local_unnamed_addr #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nofree nosync nounwind readnone
declare i32 @llvm.eh.typeid.for(ptr) #3

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #4

; Function Attrs: nofree
declare dso_local ptr @__cxa_begin_catch(ptr) local_unnamed_addr #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #5

; Function Attrs: nofree
declare dso_local void @__cxa_end_catch() local_unnamed_addr #1

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #4

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #7

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #7

; Function Attrs: mustprogress uwtable
define dso_local void @_Z5outerv() local_unnamed_addr #8 personality ptr @__gxx_personality_v0 {
entry:
  %.omp.iv.i = alloca i32, align 4
  %.omp.lb.i = alloca i32, align 4
  %.omp.ub.i = alloca i32, align 4
  %i.i = alloca i32, align 4
  %0 = load volatile i32, ptr @cond, align 4, !tbaa !4
  %savedstack = call ptr @llvm.stacksave()
  call void @llvm.lifetime.start.p0(i64 4, ptr %i.i)
  %1 = load volatile i32, ptr @cond, align 4, !tbaa !4
  %tobool.not.i = icmp eq i32 %1, 0
  br i1 %tobool.not.i, label %DIR.OMP.PARALLEL.LOOP.37.i, label %if.then.i

if.then.i:                                        ; preds = %entry
  %exception.i = call ptr @__cxa_allocate_exception(i64 4) #7
  store i32 5, ptr %exception.i, align 16, !tbaa !4
  invoke void @__cxa_throw(ptr nonnull %exception.i, ptr nonnull @_ZTIi, ptr null) #11
          to label %.noexc unwind label %lpad

.noexc:                                           ; preds = %if.then.i
  unreachable

DIR.OMP.PARALLEL.LOOP.37.i:                       ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.iv.i) #7
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.lb.i) #7
  store i32 0, ptr %.omp.lb.i, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.ub.i) #7
  store volatile i32 99, ptr %.omp.ub.i, align 4, !tbaa !4
  %end.dir.temp.i = alloca i1, align 1
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %DIR.OMP.PARALLEL.LOOP.37.i
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:                          ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv.i, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb.i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub.i, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i.i, i32 0, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp.i) ]
  br label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.2
  %temp.load.i = load volatile i1, ptr %end.dir.temp.i, align 1
  br i1 %temp.load.i, label %_Z6middlei.exit, label %DIR.OMP.PARALLEL.LOOP.3.i

DIR.OMP.PARALLEL.LOOP.3.i:                        ; preds = %DIR.OMP.PARALLEL.LOOP.3
  %3 = load i32, ptr %.omp.lb.i, align 4, !tbaa !4
  store volatile i32 %3, ptr %.omp.iv.i, align 4, !tbaa !4
  br label %omp.inner.for.cond.i

omp.inner.for.cond.i:                             ; preds = %_Z5innerv.exit.i, %DIR.OMP.PARALLEL.LOOP.3.i
  %4 = load volatile i32, ptr %.omp.iv.i, align 4, !tbaa !4
  %5 = load volatile i32, ptr %.omp.ub.i, align 4, !tbaa !4
  %cmp.not.i = icmp sgt i32 %4, %5
  br i1 %cmp.not.i, label %_Z6middlei.exit.loopexit, label %omp.inner.for.body.i

omp.inner.for.body.i:                             ; preds = %omp.inner.for.cond.i
  call void @llvm.lifetime.start.p0(i64 4, ptr %i.i) #7
  %6 = load volatile i32, ptr %.omp.iv.i, align 4, !tbaa !4
  store i32 %6, ptr %i.i, align 4, !tbaa !4
  %exception.i.i = call ptr @__cxa_allocate_exception(i64 4) #7
  store i32 4, ptr %exception.i.i, align 16, !tbaa !4
  invoke void @__cxa_throw(ptr nonnull %exception.i.i, ptr nonnull @_ZTIi, ptr null) #11
          to label %unreachable.i.i unwind label %lpad.i.i

lpad.i.i:                                         ; preds = %omp.inner.for.body.i
  %7 = landingpad { ptr, i32 }
          catch ptr @_ZTIf
          catch ptr @_ZTIi
  %8 = extractvalue { ptr, i32 } %7, 1
  %9 = call i32 @llvm.eh.typeid.for(ptr nonnull @_ZTIf) #7
  %matches.i.i = icmp eq i32 %8, %9
  br i1 %matches.i.i, label %_Z5innerv.exit.i, label %lpad.body.loopexit

unreachable.i.i:                                  ; preds = %omp.inner.for.body.i
  unreachable

_Z5innerv.exit.i:                                 ; preds = %lpad.i.i
  %10 = extractvalue { ptr, i32 } %7, 0
  %11 = call ptr @__cxa_begin_catch(ptr %10) #7
  %putchar.i.i = call i32 @putchar(i32 120) #7
  call void @__cxa_end_catch() #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.i) #7
  %12 = load volatile i32, ptr %.omp.iv.i, align 4, !tbaa !4
  %add1.i = add nsw i32 %12, 1
  store volatile i32 %add1.i, ptr %.omp.iv.i, align 4, !tbaa !4
  br label %omp.inner.for.cond.i

_Z6middlei.exit.loopexit:                         ; preds = %omp.inner.for.cond.i
  br label %_Z6middlei.exit

_Z6middlei.exit:                                  ; preds = %_Z6middlei.exit.loopexit, %DIR.OMP.PARALLEL.LOOP.3
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:                      ; preds = %_Z6middlei.exit
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.5

DIR.OMP.END.PARALLEL.LOOP.5:                      ; preds = %DIR.OMP.END.PARALLEL.LOOP.4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.ub.i) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.lb.i) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.iv.i) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr %i.i)
  call void @llvm.stackrestore(ptr %savedstack)
  br label %try.cont

lpad:                                             ; preds = %if.then.i
  %13 = landingpad { ptr, i32 }
          catch ptr @_ZTIi
  br label %lpad.body

lpad.body.loopexit:                               ; preds = %lpad.i.i
  br label %lpad.body

lpad.body:                                        ; preds = %lpad.body.loopexit, %lpad
  %eh.lpad-body = phi { ptr, i32 } [ %13, %lpad ], [ %7, %lpad.body.loopexit ]
  %14 = extractvalue { ptr, i32 } %eh.lpad-body, 1
  %15 = call i32 @llvm.eh.typeid.for(ptr nonnull @_ZTIi) #7
  %matches = icmp eq i32 %14, %15
  br i1 %matches, label %catch, label %eh.resume

catch:                                            ; preds = %lpad.body
  %16 = extractvalue { ptr, i32 } %eh.lpad-body, 0
  %17 = call ptr @__cxa_begin_catch(ptr %16) #7
  %putchar = call i32 @putchar(i32 120)
  call void @__cxa_end_catch() #7
  br label %try.cont

try.cont:                                         ; preds = %DIR.OMP.END.PARALLEL.LOOP.5, %catch
  ret void

eh.resume:                                        ; preds = %lpad.body
  resume { ptr, i32 } %eh.lpad-body
}

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) #9

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #10

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #10

attributes #0 = { alwaysinline mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree }
attributes #2 = { nofree noreturn }
attributes #3 = { nofree nosync nounwind readnone }
attributes #4 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #5 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #6 = { alwaysinline mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #7 = { nounwind }
attributes #8 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #9 = { nofree nounwind }
attributes #10 = { nocallback nofree nosync nounwind willreturn }
attributes #11 = { noreturn }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
