; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Test src:
;
; typedef char TYPE;
; typedef struct my_struct{TYPE y; TYPE x; void work(); my_struct() {};} MY_STRUCT_TYPE;
;
; void MY_STRUCT_TYPE::work() {
;
; #pragma omp simd private(y)
;   for (int i = 0; i < 10; i++) {
;     y = x;
;   }
; }
;
; ModuleID = 'cls_mbr_simd_private.cpp'
source_filename = "cls_mbr_simd_private.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { i8, i8 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_ZN9my_struct4workEv(%struct.my_struct* %this) #0 align 2 {
entry:
  %this.addr = alloca %struct.my_struct*, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store %struct.my_struct* %this, %struct.my_struct** %this.addr, align 8
  %this1 = load %struct.my_struct*, %struct.my_struct** %this.addr, align 8
  %y = getelementptr inbounds %struct.my_struct, %struct.my_struct* %this1, i32 0, i32 0
; Make sure that vpo-paropt-prepare stores %y to a temporary location
; PREPR: store i8* %y, i8** [[YADDR:%[a-zA-Z._0-9]+]]
;
; And the Value where %y is store is added to the directive in a
; "QUAL.OMP.OPERAND.ADDR" clause
; PREPR: call token @llvm.directive.region.entry()
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(i8* %y, i8** [[YADDR]])
;
; Make sure that the above renaming is removed after vpo-restore-operands pass.
; RESTR-NOT: store i8* %y, i8** {{%[a-zA-Z._0-9]+}}
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
  store i32 9, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"(i8* %y), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
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
  %x = getelementptr inbounds %struct.my_struct, %struct.my_struct* %this1, i32 0, i32 1
  %4 = load i8, i8* %x, align 1
  store i8 %4, i8* %y, align 1
; Check that after prepare, uses of %y are replaced with a load from YADDR.
; PREPR: [[YADDR_LOAD:%[a-zA-Z._0-9]+]] = load volatile i8*, i8** [[YADDR]]
; PREPR: [[LOAD_X:%[a-zA-Z._0-9]+]] = load i8, i8* %x
; PREPR: store i8 [[LOAD_X]], i8* [[YADDR_LOAD]]

; TFORM-DAG: [[LOAD_X:%[a-zA-Z._0-9]+]] = load i8, i8* %x
; TFORM-DAG: store i8 [[LOAD_X]], i8* [[Y_PRIV:%[a-zA-Z._0-9]+]]
; Local copy of y is where x is being stored. It should be an i8.
; TFORM-DAG: [[Y_PRIV]] = alloca i8

; Y_PRIV should still be in the SIMD directive after vpo-paropt pass.
; TFORM-DAG: call token @llvm.directive.region.entry()
; TFORM-DAG-SAME: "QUAL.OMP.PRIVATE"(i8* %y)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %5, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
