; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=NOFASTRED --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=NOFASTRED --check-prefix=ALL

; Ensure that Paropt doesn't try to emit kmpc_reduce/critical around
; UDR finalization code (which would cause a comp-fail).

; #include <cstdio>
;
; class C {
; public:
;   int a = 0;
; };
;
; void my_comb(C &out, C &in) { out.a += in.a; };
; void my_init(C &priv, C &orig) { priv.a = orig.a; }
;
; #pragma omp declare reduction(my_add:C                                         \
;                               : my_comb(omp_out, omp_in))                      \
;     initializer(my_init(omp_priv, omp_orig))
;
; C x;
;
; void foo() {
; #pragma omp simd reduction(my_add : x)
;   for (int i = 0; i < 10; i++) {
;     //  C temp; temp.a = i;
;     //  my_comb(x, temp);
;     printf("%d\n", x.a);
;   }
; }

; ALL: %[[XPRIV:x.red]] = alloca %class.C, align 4
; FASTRED: %[[FRSTR:fast_red_struct]] = alloca %struct.fast_red_t, align 8
; FASTRED: %[[FXPRIV:x.fast_red]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %[[FRSTR]], i32 0, i32 0

; ALL: call void @.omp_initializer.(%class.C* %[[XPRIV]], %class.C* @x)

; ALL: [[ZTT:%.+]] = icmp sle i32 0, %{{.+}}
; ALL: br i1 [[ZTT]], label %[[PHB:[^,]+]], label %[[REXIT:[^,]+]]
; ALL: [[PHB]]:
; ALL: [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),{{.*}}"QUAL.OMP.REDUCTION.UDR"(%class.C* %[[XPRIV]],{{.*}} ]
; ALL: br label %[[LOOPBODY:[^,]+]]
; ALL: [[LOOPBODY]]:
; ALL: getelementptr inbounds %class.C, %class.C* %[[XPRIV]], i32 0, i32 0
; ALL: br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LEXIT:[^,]+]]
; ALL: [[LEXIT]]:
; ALL: call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]

; FASTRED: %[[XPRIV_VAL:[^ ]+]] = load %class.C, %class.C* %[[XPRIV]], align 4
; FASTRED: store %class.C %[[XPRIV_VAL]], %class.C* %[[FXPRIV]], align 4

; ALL: br label %[[LEXIT_SPLIT:[^,]+]]
; ALL: [[LEXIT_SPLIT]]:
; ALL: br label %[[LEXIT_SPLIT_SPLIT:[^,]+]]
; ALL: [[LEXIT_SPLIT_SPLIT]]:

; FASTRED: call void @.omp_combiner.(%class.C* @x, %class.C* %[[FXPRIV]])
; NOFASTRED: call void @.omp_combiner.(%class.C* @x, %class.C* %[[XPRIV]])

; ALL: br label %[[REXIT]]
; ALL: [[REXIT]]:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.C = type { i32 }

@x = dso_local global %class.C zeroinitializer, align 4
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z3foov() #1 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 9, i32* %.omp.ub, align 4

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.UDR"(%class.C* @x, i8* null, void (%class.C*)* @_ZTS1C.omp.destr, void (%class.C*, %class.C*)* @.omp_combiner., void (%class.C*, %class.C*)* @.omp_initializer.), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1) ]
  store i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %1 = load i32, i32* %.omp.iv, align 4
  %2 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %3 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %3, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %4 = load i32, i32* getelementptr inbounds (%class.C, %class.C* @x, i32 0, i32 0), align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %4) #2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
declare void @_Z7my_combR1CS0_(%class.C* nonnull align 4 dereferenceable(4) %out, %class.C* nonnull align 4 dereferenceable(4) %in) #0

; Function Attrs: noinline nounwind optnone uwtable mustprogress
declare void @_Z7my_initR1CS0_(%class.C* nonnull align 4 dereferenceable(4) %priv, %class.C* nonnull align 4 dereferenceable(4) %orig) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline uwtable
declare void @.omp_combiner.(%class.C* noalias %0, %class.C* noalias %1) #3

; Function Attrs: noinline uwtable
declare void @.omp_initializer.(%class.C* noalias %0, %class.C* noalias %1) #3

; Function Attrs: noinline uwtable
declare void @_ZTS1C.omp.destr(%class.C* %0) #3 section ".text.startup"

declare dso_local i32 @printf(i8*, ...) #4

attributes #0 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #4 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
