; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine)' -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Test src:
;
;
; #include <string.h>
; #include <stdio.h>
;
; void print_int (void *c);
;
; int main() {
;
;   int y = 1;
;   print_int((void*) &y);
;
; #pragma omp parallel num_threads(1) firstprivate(y)
;   {
;     print_int((void*) &y);
;   }
;
;   return 0;
; }

; ModuleID = 'rename_and_restore_int_addrcast.c'
source_filename = "rename_and_restore_int_addrcast.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %y = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = bitcast i32* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 1, i32* %y, align 4, !tbaa !2
  %1 = bitcast i32* %y to i8*
  call void @print_int(i8* %1)


; Check for the renaming of %y after prepare vpo-paropt-prepare.
; PREPR: [[YADDR:%[a-zA-Z._0-9]+]] = alloca i32*
; PREPR: store i32* %y, i32** [[YADDR]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE"(i32* %y)
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(i32* %y, i32** [[YADDR]])
; PREPR: [[YRENAMED:%[a-zA-Z._0-9]+]] = load volatile i32*, i32** [[YADDR]]
; PREPR: [[YRENAMED_BC:%[a-zA-Z._0-9]+]] = bitcast i32* [[YRENAMED]] to i8*
; PREPR: call void @print_int(i8* [[YRENAMED_BC]])

; Check that the IR was not modified by CSE + Instcombine

; INSTCMB: [[YADDR:%[a-zA-Z._0-9]+]] = alloca i32*
; INSTCMB: "QUAL.OMP.OPERAND.ADDR"(i32* %y, i32** [[YADDR]])
; INSTCMB: [[YRENAMED:%[a-zA-Z._0-9]+]] = load volatile i32*, i32** [[YADDR]]
; INSTCMB: [[YRENAMED_BC:%[a-zA-Z._0-9]+]] = bitcast i32* [[YRENAMED]] to i8*
; INSTCMB: call void @print_int(i8* [[YRENAMED_BC]])

; Check for restore-operands was able to undo the renaming:
; RESTR: "QUAL.OMP.FIRSTPRIVATE"(i32* %y)
; RESTR-NOT: alloca i32*
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: [[YRENAMED:%[a-zA-Z._0-9]+]] = bitcast i32* %y to i8*
; RESTR: call void @print_int(i8* [[YRENAMED]])
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 1), "QUAL.OMP.FIRSTPRIVATE"(i32* %y) ]
  %3 = bitcast i32* %y to i8*
  call void @print_int(i8* %3)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  %4 = bitcast i32* %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #3
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local void @print_int(i8*) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
