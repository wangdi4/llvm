; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=NOFASTRED --check-prefix=ALL
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
; FASTRED: %[[FXPRIV:x.fast_red]] = getelementptr inbounds %struct.fast_red_t, ptr %[[FRSTR]], i32 0, i32 0

; ALL: call void @.omp_initializer.(ptr %[[XPRIV]], ptr @x)

; ALL: [[ZTT:%.+]] = icmp sle i32 0, %{{.+}}
; ALL: br i1 [[ZTT]], label %[[PHB:[^,]+]], label %[[REXIT:[^,]+]]
; ALL: [[PHB]]:
; ALL: [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),{{.*}}"QUAL.OMP.REDUCTION.UDR:TYPED"(ptr %[[XPRIV]],{{.*}} ]
; ALL: br label %[[LOOPBODY:[^,]+]]
; ALL: [[LOOPBODY]]:
; ALL: br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LEXIT:[^,]+]]
; ALL: [[LEXIT]]:
; ALL: call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]

; FASTRED: %[[XPRIV_VAL:[^ ]+]] = load %class.C, ptr %[[XPRIV]], align 4
; FASTRED: store %class.C %[[XPRIV_VAL]], ptr %[[FXPRIV]], align 4

; ALL: br label %[[LEXIT_SPLIT:[^,]+]]
; ALL: [[LEXIT_SPLIT]]: 
; ALL: br label %[[LEXIT_SPLIT_SPLIT:[^,]+]]
; ALL: [[LEXIT_SPLIT_SPLIT]]:

; FASTRED: call void @.omp_combiner.(ptr @x, ptr %[[FXPRIV]])
; NOFASTRED: call void @.omp_combiner.(ptr @x, ptr %[[XPRIV]])

; ALL: br label %[[REXIT]]
; ALL: [[REXIT]]:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.C = type { i32 }

@x = dso_local global %class.C zeroinitializer, align 4
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define dso_local void @_Z3foov() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 9, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr @x, %class.C zeroinitializer, i32 1, ptr null, ptr null, ptr @.omp_combiner., ptr @.omp_initializer.),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %1 = load i32, ptr %.omp.iv, align 4
  %2 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %3 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %3, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %4 = load i32, ptr @x, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %4) #2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  ret void
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token) 

declare void @.omp_combiner.(ptr noalias noundef %0, ptr noalias noundef %1)

declare void @.omp_initializer.(ptr noalias noundef %0, ptr noalias noundef %1)

declare dso_local i32 @printf(ptr noundef, ...)
