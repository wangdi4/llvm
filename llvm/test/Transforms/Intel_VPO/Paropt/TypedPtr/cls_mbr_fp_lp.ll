; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Test src:
;
; typedef short TYPE;
; typedef struct my_struct{TYPE y; TYPE x; void work(); my_struct() {};} MY_STRUCT_TYPE;
;
; void MY_STRUCT_TYPE::work() {
;
; #pragma omp for firstprivate(y) lastprivate(y)
;   for (int i = 0; i < 10; i++) {
;     y = x;
;   }
; }
;
; ModuleID = 'cls_mbr_fp_lp.cpp'
source_filename = "cls_mbr_fp_lp.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { i16, i16 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_ZN9my_struct4workEv(%struct.my_struct* %this) #0 align 2 {
entry:
  %this.addr = alloca %struct.my_struct*, align 8
  %y = alloca i16*, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store %struct.my_struct* %this, %struct.my_struct** %this.addr, align 8
  %this1 = load %struct.my_struct*, %struct.my_struct** %this.addr, align 8
  %y2 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %this1, i32 0, i32 0
  store i16* %y2, i16** %y, align 8
  %y3 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %this1, i32 0, i32 0
  %y4 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %this1, i32 0, i32 0
  store i32 0, i32* %.omp.lb, align 4
  store i32 9, i32* %.omp.ub, align 4

; Make sure that vpo-paropt-prepare stores %y3 to a temporary location
; PREPR: store i16* %y3, i16** [[YADDR:%[a-zA-Z._0-9]+]]
;
; And the Value where %y3 is store is added to the directive in a
; "QUAL.OMP.OPERAND.ADDR" clause
; PREPR: call token @llvm.directive.region.entry()
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(i16* %y3, i16** [[YADDR]])
;
; Make sure that the above renaming is removed after vpo-restore-operands pass.
; RESTR-NOT: store i16* %y3, i16** {{%[a-zA-Z._0-9]+}}
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"

; Firstprivate initialization code after transformation pass
; TFORM: [[Y_LOAD:%[a-zA-Z._0-9]+]] = load i16, i16* %y3
; TFORM: store i16 [[Y_LOAD]], i16* [[Y_FPLP:%[a-zA-Z._0-9]+]]
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i16* %y3), "QUAL.OMP.LASTPRIVATE"(i16* %y3), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %x = getelementptr inbounds %struct.my_struct, %struct.my_struct* %this1, i32 0, i32 1
  %5 = load i16, i16* %x, align 2
  store i16 %5, i16* %y3, align 2

; Check that after prepare, uses of %y3 are replaced with a load from YADDR.
; PREPR: [[YADDR_LOAD:%[a-zA-Z._0-9]+]] = load volatile i16*, i16** [[YADDR]]
; PREPR: [[LOAD_X:%[a-zA-Z._0-9]+]] = load i16, i16* %x
; PREPR: store i16 [[LOAD_X]], i16* [[YADDR_LOAD]]

; After restore-operands, %y3 should be used again instead of YADDR_LOAD.
; RESTR: [[LOAD_X:%[a-zA-Z._0-9]+]] = load i16, i16* %x
; RESTR: store i16 [[LOAD_X]], i16* %y3

; Local copy of y is where x is stored after vpo-paropt
; TFORM: [[LOAD_X:%[a-zA-Z._0-9]+]] = load i16, i16* %x
; TFORM: store i16 [[LOAD_X]], i16* [[Y_FPLP]]

; Lastprivate copyout after vpo-paropt
; TFORM: [[Y_FPLP_LOAD:%[a-zA-Z._0-9]+]] = load i16, i16* [[Y_FPLP]]
; TFORM: store i16 [[Y_FPLP_LOAD]], i16* %y3
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, i32* %.omp.iv, align 4
  %add5 = add nsw i32 %6, 1
  store i32 %add5, i32* %.omp.iv, align 4
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

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
