; Checks that objects in the private clauses of tasks and taskloops,
; have their constructors and destructors called.

; RUN: opt < %s -vpo-paropt  -S | FileCheck %s
; CHECK: define{{.*}}TASKLOOP{{.*}}split
; CHECK: call{{.*}}def_con{{.*}}class.c
; CHECK: for.body:
; CHECK: for.end
; CHECK: call{{.*}}omp.destr{{.*}}class.c

; CHECK: define{{.*}}DIR.OMP.TASK.
; CHECK: call{{.*}}def_con{{.*}}class.c
; CHECK: OMP.END.TASK.
; CHECK: call{{.*}}omp.destr{{.*}}class.c

; class c {
; public:
;   __attribute__((noinline)) c() { i = 5; }
;   __attribute__((noinline)) ~c() { i = -1; }
;   int i;
; };
;
; int foo() {
;   c c1;
; #pragma omp parallel
; #pragma omp single
;   {
; #pragma omp task private(c1)
;     c1.i = 4;
;   }
;
; #pragma omp taskloop private(c1)
;   for (int i = 0; i < 10; i++)
;     c1.i = 4;
;   return c1.i;
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.c = type { i32 }

$_ZN1cC2Ev = comdat any

$_ZN1cD2Ev = comdat any

@.str = private unnamed_addr constant [2 x i8] c"c\00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"d\00", align 1

; Function Attrs: uwtable
define dso_local i32 @_Z3foov() #0 {
entry:
  %c1 = alloca %class.c, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i2 = alloca i32, align 4
  %0 = bitcast %class.c* %c1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  call void @_ZN1cC2Ev(%class.c* %c1)
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(%class.c* %c1) ]
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  br label %DIR.OMP.SINGLE.3

DIR.OMP.SINGLE.3:                                 ; preds = %DIR.OMP.PARALLEL.2
  fence acquire
  br label %DIR.OMP.TASK.4

DIR.OMP.TASK.4:                                   ; preds = %DIR.OMP.SINGLE.3
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.PRIVATE:NONPOD"(%class.c* %c1, %class.c* (%class.c*)* @_ZTS1c.omp.def_constr, void (%class.c*)* @_ZTS1c.omp.destr) ]
  br label %DIR.OMP.TASK.5

DIR.OMP.TASK.5:                                   ; preds = %DIR.OMP.TASK.4
  %i = getelementptr inbounds %class.c, %class.c* %c1, i32 0, i32 0
  store i32 4, i32* %i, align 4, !tbaa !2
  br label %DIR.OMP.END.TASK.6

DIR.OMP.END.TASK.6:                               ; preds = %DIR.OMP.TASK.5
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.7

DIR.OMP.END.TASK.7:                               ; preds = %DIR.OMP.END.TASK.6
  fence release
  br label %DIR.OMP.END.SINGLE.8

DIR.OMP.END.SINGLE.8:                             ; preds = %DIR.OMP.END.TASK.7
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SINGLE"() ]
  br label %DIR.OMP.END.SINGLE.9

DIR.OMP.END.SINGLE.9:                             ; preds = %DIR.OMP.END.SINGLE.8
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.10

DIR.OMP.END.PARALLEL.10:                          ; preds = %DIR.OMP.END.SINGLE.9
  %4 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #3
  %5 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %5) #3
  store i64 0, i64* %.omp.lb, align 8, !tbaa !7
  %6 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %6) #3
  store i64 9, i64* %.omp.ub, align 8, !tbaa !7
  br label %DIR.OMP.TASKLOOP.11

DIR.OMP.TASKLOOP.11:                              ; preds = %DIR.OMP.END.PARALLEL.10
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(), "QUAL.OMP.PRIVATE:NONPOD"(%class.c* %c1, %class.c* (%class.c*)* @_ZTS1c.omp.def_constr, void (%class.c*)* @_ZTS1c.omp.destr), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i2) ]
  br label %DIR.OMP.TASKLOOP.12

DIR.OMP.TASKLOOP.12:                              ; preds = %DIR.OMP.TASKLOOP.11
  %8 = load i64, i64* %.omp.lb, align 8, !tbaa !7
  %conv = trunc i64 %8 to i32
  store i32 %conv, i32* %.omp.iv, align 4, !tbaa !9
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.TASKLOOP.12
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !9
  %conv1 = sext i32 %9 to i64
  %10 = load i64, i64* %.omp.ub, align 8, !tbaa !7
  %cmp = icmp ule i64 %conv1, %10
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = bitcast i32* %i2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %11) #3
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !9
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i2, align 4, !tbaa !9
  %i3 = getelementptr inbounds %class.c, %class.c* %c1, i32 0, i32 0
  store i32 4, i32* %i3, align 4, !tbaa !2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %13 = bitcast i32* %i2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #3
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, i32* %.omp.iv, align 4, !tbaa !9
  %add4 = add nsw i32 %14, 1
  store i32 %add4, i32* %.omp.iv, align 4, !tbaa !9
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TASKLOOP"() ]
  br label %DIR.OMP.END.TASKLOOP.13

DIR.OMP.END.TASKLOOP.13:                          ; preds = %omp.loop.exit
  %15 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %15) #3
  %16 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %16) #3
  %17 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #3
  %i5 = getelementptr inbounds %class.c, %class.c* %c1, i32 0, i32 0
  %18 = load i32, i32* %i5, align 4, !tbaa !2
  call void @_ZN1cD2Ev(%class.c* %c1)
  %19 = bitcast %class.c* %c1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #3
  ret i32 %18
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: noinline uwtable
define linkonce_odr dso_local void @_ZN1cC2Ev(%class.c* %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %class.c*, align 8
  store %class.c* %this, %class.c** %this.addr, align 8, !tbaa !10
  %this1 = load %class.c*, %class.c** %this.addr, align 8
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i64 0, i64 0))
  %i = getelementptr inbounds %class.c, %class.c* %this1, i32 0, i32 0
  store i32 5, i32* %i, align 4, !tbaa !2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: uwtable
define internal %class.c* @_ZTS1c.omp.def_constr(%class.c*) #4 section ".text.startup" {
entry:
  %.addr = alloca %class.c*, align 8
  store %class.c* %0, %class.c** %.addr, align 8, !tbaa !10
  %1 = load %class.c*, %class.c** %.addr, align 8
  call void @_ZN1cC2Ev(%class.c* %1)
  ret %class.c* %1
}

; Function Attrs: uwtable
define internal void @_ZTS1c.omp.destr(%class.c*) #4 section ".text.startup" {
entry:
  %.addr = alloca %class.c*, align 8
  store %class.c* %0, %class.c** %.addr, align 8, !tbaa !10
  %1 = load %class.c*, %class.c** %.addr, align 8
  call void @_ZN1cD2Ev(%class.c* %1)
  ret void
}

; Function Attrs: noinline uwtable
define linkonce_odr dso_local void @_ZN1cD2Ev(%class.c* %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %class.c*, align 8
  store %class.c* %this, %class.c** %this.addr, align 8, !tbaa !10
  %this1 = load %class.c*, %class.c** %this.addr, align 8
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.1, i64 0, i64 0))
  %i = getelementptr inbounds %class.c, %class.c* %this1, i32 0, i32 0
  store i32 -1, i32* %i, align 4, !tbaa !2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local i32 @printf(i8*, ...) #5

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@_ZTS1c", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"long", !5, i64 0}
!9 = !{!4, !4, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"pointer@_ZTSP1c", !5, i64 0}
