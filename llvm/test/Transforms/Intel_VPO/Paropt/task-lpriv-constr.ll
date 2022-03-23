; The given TASKLOOP directive has a clause lastprivate(f) where "f" is
; a constructible C++ object. Each task spawned by the taskloop must construct
; its own version of "f". The internal variable should be destructed at the
; end of the task, after any copy-out to the external scope.

; RUN: opt -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; CHECK: define{{.*}}TASKLOOP
; CHECK-NOT: define
; CHECK: call{{.*}}def_constr
; CHECK: last.done
; CHECK: call{{.*}}omp.destr

; #include <stdio.h>
;
; class foo {
; public:
;   static int kount;
;   __attribute__((noinline)) foo() throw() {  i = kount++; }
;   ~foo() { printf("Destructed %d\n", i); }
;   __attribute__((noinline))  foo (const foo &f) {
;     i = 1000 + f.i;
;   }
;   int i;
; };
;
; int foo::kount = 0;
; int __attribute__ ((noinline)) Compute(void)
; {
;   foo f;
; #pragma omp parallel
;   {
; #pragma omp single
;   {
; #pragma omp taskloop lastprivate(f) num_tasks(4)
;     for (int i = 0; i < 4; i++)
;       f.i = i;
;   }
;   }
;   return f.i;
;   }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.foo = type { i32 }

$_ZN3fooC2Ev = comdat any

$_ZN3fooD2Ev = comdat any

@_ZN3foo5kountE = dso_local global i32 0, align 4
@.str = private unnamed_addr constant [15 x i8] c"Destructed %d\0A\00", align 1

; Function Attrs: noinline uwtable
define dso_local i32 @_Z7Computev() #0 {
entry:
  %f = alloca %class.foo, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i = alloca i32, align 4
  %0 = bitcast %class.foo* %f to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  call void @_ZN3fooC2Ev(%class.foo* %f) #3
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i64* %.omp.lb), "QUAL.OMP.SHARED"(%class.foo* %f), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  fence acquire
  %3 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #3
  %4 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %4) #3
  store i64 0, i64* %.omp.lb, align 8, !tbaa !2
  %5 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %5) #3
  store i64 3, i64* %.omp.ub, align 8, !tbaa !2
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(), "QUAL.OMP.LASTPRIVATE:NONPOD"(%class.foo* %f, %class.foo* (%class.foo*)* @_ZTS3foo.omp.def_constr, void (%class.foo*, %class.foo*)* @_ZTS3foo.omp.copy_assign, void (%class.foo*)* @_ZTS3foo.omp.destr), "QUAL.OMP.NUM_TASKS"(i32 4), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %7 = load i64, i64* %.omp.lb, align 8, !tbaa !2
  %conv = trunc i64 %7 to i32
  store i32 %conv, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %8 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %conv1 = sext i32 %8 to i64
  %9 = load i64, i64* %.omp.ub, align 8, !tbaa !2
  %cmp = icmp ule i64 %conv1, %9
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10) #3
  %11 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %mul = mul nsw i32 %11, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !6
  %12 = load i32, i32* %i, align 4, !tbaa !6
  %i2 = getelementptr inbounds %class.foo, %class.foo* %f, i32 0, i32 0
  store i32 %12, i32* %i2, align 4, !tbaa !8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %13 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #3
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %add3 = add nsw i32 %14, 1
  store i32 %add3, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TASKLOOP"() ]
  %15 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %15) #3
  %16 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %16) #3
  %17 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #3
  fence release
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SINGLE"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  %i4 = getelementptr inbounds %class.foo, %class.foo* %f, i32 0, i32 0
  %18 = load i32, i32* %i4, align 4, !tbaa !8
  call void @_ZN3fooD2Ev(%class.foo* %f)
  %19 = bitcast %class.foo* %f to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #3
  ret i32 %18
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local void @_ZN3fooC2Ev(%class.foo* %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %class.foo*, align 8
  store %class.foo* %this, %class.foo** %this.addr, align 8, !tbaa !10
  %this1 = load %class.foo*, %class.foo** %this.addr, align 8
  %0 = load i32, i32* @_ZN3foo5kountE, align 4, !tbaa !6
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* @_ZN3foo5kountE, align 4, !tbaa !6
  %i = getelementptr inbounds %class.foo, %class.foo* %this1, i32 0, i32 0
  store i32 %0, i32* %i, align 4, !tbaa !8
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: uwtable
define internal %class.foo* @_ZTS3foo.omp.def_constr(%class.foo*) #4 section ".text.startup" {
entry:
  %.addr = alloca %class.foo*, align 8
  store %class.foo* %0, %class.foo** %.addr, align 8, !tbaa !10
  %1 = load %class.foo*, %class.foo** %.addr, align 8
  call void @_ZN3fooC2Ev(%class.foo* %1) #3
  ret %class.foo* %1
}

; Function Attrs: uwtable
define internal void @_ZTS3foo.omp.copy_assign(%class.foo*, %class.foo*) #4 {
entry:
  %.addr = alloca %class.foo*, align 8
  %.addr1 = alloca %class.foo*, align 8
  store %class.foo* %0, %class.foo** %.addr, align 8, !tbaa !10
  store %class.foo* %1, %class.foo** %.addr1, align 8, !tbaa !10
  %2 = load %class.foo*, %class.foo** %.addr, align 8
  %3 = load %class.foo*, %class.foo** %.addr1, align 8
  %4 = bitcast %class.foo* %2 to i8*
  %5 = bitcast %class.foo* %3 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %4, i8* align 4 %5, i64 4, i1 false), !tbaa.struct !12
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1 immarg) #1

; Function Attrs: uwtable
define internal void @_ZTS3foo.omp.destr(%class.foo*) #4 section ".text.startup" {
entry:
  %.addr = alloca %class.foo*, align 8
  store %class.foo* %0, %class.foo** %.addr, align 8, !tbaa !10
  %1 = load %class.foo*, %class.foo** %.addr, align 8
  call void @_ZN3fooD2Ev(%class.foo* %1)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN3fooD2Ev(%class.foo* %this) unnamed_addr #4 comdat align 2 {
entry:
  %this.addr = alloca %class.foo*, align 8
  store %class.foo* %this, %class.foo** %this.addr, align 8, !tbaa !10
  %this1 = load %class.foo*, %class.foo** %this.addr, align 8
  %i = getelementptr inbounds %class.foo, %class.foo* %this1, i32 0, i32 0
  %0 = load i32, i32* %i, align 4, !tbaa !8
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i64 0, i64 0), i32 %0)
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local i32 @printf(i8*, ...) #5

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!9, !7, i64 0}
!9 = !{!"struct@_ZTS3foo", !7, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"pointer@_ZTSP3foo", !4, i64 0}
!12 = !{i64 0, i64 4, !6}
