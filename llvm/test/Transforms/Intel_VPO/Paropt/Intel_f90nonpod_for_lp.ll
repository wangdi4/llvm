; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; The IR was modified from this source to use F90_NONPOD
; instead of NONPOD, and copy-constructor instead of a constructor,
; for the lastprivate operand c.

; Test src:

; #include <stdio.h>
; #include <stdlib.h>
; #include <cstring>
;
; struct C {
;   int a;
;   int *p;
;   // Constructor
;   C() {
;     p = (int *)calloc(5, sizeof(int));
;     printf("allocating %p\n", this);
;   }
;   // Copy constructor
;   C(const C& c1) {
;       a = c1.a;
;       p = (int*) calloc(5, sizeof(int));
;       printf("copy constructing %p\n", this);
;       std::memcpy(p, c1.p, 5 * sizeof(int));
;   }
;   // Copy assignment operator
;   C& operator=(const C& c1) {
;      a = c1.a;
;      std::memcpy(p, c1.p, 5 * sizeof(int));
;      printf("copy assigning to %p\n", this);
;      return *this;
;   }
;   // Destructor
;   ~C() {
;     free(p);
;     printf("freeing %p\n", this);
;   }
; };
;
;   C c;
; int main() {
;   c.p[1] = 100;
; //#pragma omp parallel num_threads(1)
; #pragma omp for lastprivate(c)
;   for (int i = 0; i < 1; i++) {
; //    printf("inside parallel: %d, %p, %p.\n", c.p[1], &c, &c.p[1]);
;     c.p[1] = 20;
; //    printf("inside parallel: %d, %p, %p.\n", c.p[1], &c, &c.p[1]);
;   }
;
; //  printf("outside parallel: %d, %p, %p.\n", c.p[1], &c, &c.p[1]);
; }

; CHECK: %[[CPRIV:c[^ ]+]] = alloca %struct.C, align 8
; CHECK: call void @_ZTS1C.omp.copy_constr(%struct.C* %[[CPRIV]], %struct.C* @c)
; CHECK: call void @__kmpc_for_static_init_4({{.*}})
; CHECK: call void @__kmpc_for_static_fini({{.*}})
; CHECK: call void @_ZTS1C.omp.copy_assign(%struct.C* @c, %struct.C* %[[CPRIV]])
; CHECK: call void @_ZTS1C.omp.destr(%struct.C* %[[CPRIV]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.C = type { i32, i32* }

@c = dso_local global %struct.C zeroinitializer, align 8
@__dso_handle = external hidden global i8
@.str = private unnamed_addr constant [15 x i8] c"allocating %p\0A\00", align 1
@.str.1 = private unnamed_addr constant [12 x i8] c"freeing %p\0A\00", align 1
@.str.2 = private unnamed_addr constant [22 x i8] c"copy constructing %p\0A\00", align 1
@.str.3 = private unnamed_addr constant [22 x i8] c"copy assigning to %p\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_ctor_dtor_nonpod.cpp, i8* null }]

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define dso_local i32 @main() #5 {
entry:
  %retval = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load i32*, i32** getelementptr inbounds (%struct.C, %struct.C* @c, i32 0, i32 1), align 8
  %ptridx = getelementptr inbounds i32, i32* %0, i64 1
  store i32 100, i32* %ptridx, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 0, i32* %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.LASTPRIVATE:F90_NONPOD"(%struct.C* @c, void (%struct.C*, %struct.C*)* @_ZTS1C.omp.copy_constr, void (%struct.C*, %struct.C*)* @_ZTS1C.omp.copy_assign, void (%struct.C*)* @_ZTS1C.omp.destr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %2 = load i32, i32* %.omp.lb, align 4
  store i32 %2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32* %.omp.iv, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %6 = load i32*, i32** getelementptr inbounds (%struct.C, %struct.C* @c, i32 0, i32 1), align 8
  %ptridx1 = getelementptr inbounds i32, i32* %6, i64 1
  store i32 20, i32* %ptridx1, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %7, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.LOOP"() ]
  %8 = load i32, i32* %retval, align 4
  ret i32 %8
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64) #3
declare dso_local i32 @printf(i8*, ...) #4
; Function Attrs: nounwind
declare dso_local void @free(i8*) #3
; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2
; Function Attrs: noinline nounwind uwtable
declare void @_ZTS1C.omp.copy_constr(%struct.C* %0, %struct.C* %1) #0
; Function Attrs: noinline nounwind uwtable
declare void @_ZTS1C.omp.destr(%struct.C* %0) #0 section ".text.startup"
; Function Attrs: noinline nounwind uwtable
declare void @_ZTS1C.omp.copy_assign(%struct.C* %0, %struct.C* %1) #0
; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #7
; Function Attrs: noinline nounwind uwtable
declare void @_GLOBAL__sub_I_ctor_dtor_nonpod.cpp() #0 section ".text.startup"

attributes #0 = { noinline nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }
attributes #3 = { nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #5 = { noinline norecurse nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #6 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #7 = { argmemonly nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
; end INTEL_CUSTOMIZATION
