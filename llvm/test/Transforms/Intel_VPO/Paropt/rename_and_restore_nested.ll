; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Test SRC:
;
;
; #include <stdio.h>
; typedef char TYPE;
;
; void work() {
; TYPE y[10];
;
; #pragma omp parallel private(y)
;   {
;     printf ("y = %d\n", y[0]);
; #pragma omp parallel private(y)
;     {
;       printf ("y[0] = %d\n", y[0]);
;     }
;   }
; }
; ModuleID = 'rename_and_restore_nested.c'
source_filename = "rename_and_restore_nested.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"y = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [11 x i8] c"y[0] = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @work() #0 {
entry:
  %y = alloca [10 x i8], align 1
  %0 = bitcast [10 x i8]* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 10, i8* %0) #2

; Renaming of %y for outer region:
; PREPR: store [10 x i8]* %y, [10 x i8]** [[YADDR1:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.PRIVATE"([10 x i8]* %y)
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"([10 x i8]* %y, [10 x i8]** [[YADDR1]])
; PREPR: [[YRENAMED1:%[a-zA-Z._0-9]+]] = load volatile [10 x i8]*, [10 x i8]** [[YADDR1]]
; PREPR: getelementptr inbounds [10 x i8], [10 x i8]* [[YRENAMED1]]

; Renaming for inner region:
; PREPR: store [10 x i8]* [[YRENAMED1]], [10 x i8]** [[YADDR2:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.PRIVATE"([10 x i8]* [[YRENAMED1]])
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"([10 x i8]* [[YRENAMED1]], [10 x i8]** [[YADDR2]])
; PREPR: [[YRENAMED2:%[a-zA-Z._0-9]+]] = load volatile [10 x i8]*, [10 x i8]** [[YADDR2]]
; PREPR: getelementptr inbounds [10 x i8], [10 x i8]* [[YRENAMED2]]


; Check for the undone renaming after restore:
; RESTR-NOT alloca [10 x i8]*
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: "QUAL.OMP.PRIVATE"([10 x i8]* %y)
; RESTR: getelementptr inbounds [10 x i8], [10 x i8]* %y
; RESTR: "QUAL.OMP.PRIVATE"([10 x i8]* %y)
; RESTR: getelementptr inbounds [10 x i8], [10 x i8]* %y

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"([10 x i8]* %y) ]
  %arrayidx = getelementptr inbounds [10 x i8], [10 x i8]* %y, i64 0, i64 0
  %2 = load i8, i8* %arrayidx, align 1, !tbaa !2
  %conv = sext i8 %2 to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i32 %conv)

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"([10 x i8]* %y) ]
  %arrayidx1 = getelementptr inbounds [10 x i8], [10 x i8]* %y, i64 0, i64 0
  %4 = load i8, i8* %arrayidx1, align 1, !tbaa !2
  %conv2 = sext i8 %4 to i32
  %call3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.1, i64 0, i64 0), i32 %conv2)
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]



  %5 = bitcast [10 x i8]* %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 10, i8* %5) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local i32 @printf(i8*, ...) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10_c", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
