; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR

; Test src:
;
; typedef char TYPE;
; typedef struct my_struct {
;   TYPE y;
;   TYPE x;
;   void work();
;   my_struct(){};
; } MY_STRUCT_TYPE;
;
; void MY_STRUCT_TYPE::work() {
;
; #pragma omp for firstprivate(y) lastprivate(y)
;   for (int i = 0; i < 10; i++) {
;     y = x;
;   }
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { i8, i8 }

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_ZN9my_struct4workEv(ptr noundef nonnull align 1 dereferenceable(2) %this) #0 align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %y = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %y2 = getelementptr inbounds %struct.my_struct, ptr %this1, i32 0, i32 0
  store ptr %y2, ptr %y, align 8
  %y3 = getelementptr inbounds %struct.my_struct, ptr %this1, i32 0, i32 0
  %y4 = getelementptr inbounds %struct.my_struct, ptr %this1, i32 0, i32 0
  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4

; Make sure that vpo-paropt-prepare stores %y3 to a temporary location
; PREPR: store ptr %y3, ptr [[YADDR:%[a-zA-Z._0-9]+]]
;
; And the Value where %y3 is store is added to the directive in a
; "QUAL.OMP.OPERAND.ADDR" clause
; PREPR: call token @llvm.directive.region.entry()
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %y3, ptr [[YADDR]])
;
; Make sure that the above renaming is removed after vpo-restore-operands pass.
; RESTR-NOT: store ptr %y3, ptr {{%[a-zA-Z._0-9]+}}
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"

; Firstprivate initialization code after transformation pass
; TFORM: [[Y_LOAD:%[a-zA-Z._0-9]+]] = load i8, ptr %y3
; TFORM: store i8 [[Y_LOAD]], ptr [[Y_FPLP:%[a-zA-Z._0-9]+]]
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y3, i8 0, i32 1),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %y3, i8 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %x = getelementptr inbounds %struct.my_struct, ptr %this1, i32 0, i32 1
  %5 = load i8, ptr %x, align 1
  store i8 %5, ptr %y3, align 1
; Check that after prepare, uses of %y3 are replaced with a load from YADDR.
; PREPR: [[YADDR_LOAD:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[YADDR]]
; PREPR: [[LOAD_X:%[a-zA-Z._0-9]+]] = load i8, ptr %x
; PREPR: store i8 [[LOAD_X]], ptr [[YADDR_LOAD]]

; After restore-operands, %y3 should be used again instead of YADDR_LOAD.
; RESTR: [[LOAD_X:%[a-zA-Z._0-9]+]] = load i8, ptr %x
; RESTR: store i8 [[LOAD_X]], ptr %y3

; Local copy of y is where x is stored after vpo-paropt
; TFORM: [[LOAD_X:%[a-zA-Z._0-9]+]] = load i8, ptr %x
; TFORM: store i8 [[LOAD_X]], ptr [[Y_FPLP]]

; Lastprivate copyout after vpo-paropt
; TFORM: [[Y_FPLP_LOAD:%[a-zA-Z._0-9]+]] = load i8, ptr [[Y_FPLP]]
; TFORM: store i8 [[Y_FPLP_LOAD]], ptr %y3
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %6, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
