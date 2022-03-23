; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine)' -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Test src:
;
; #include <stdio.h>
;
; typedef struct _FourChars { char a, b, c, d; } FourChars;
;
; __attribute__((noinline)) void foo(const FourChars f) {
;   #pragma omp parallel shared(f) num_threads(1)
;   {
;     printf("a = %c\n", f.a);
;   }
; }
; ModuleID = 'rename_and_restore_struct_castoutside.c'
source_filename = "rename_and_restore_struct_castoutside.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._FourChars = type { i8, i8, i8, i8 }

@.str = private unnamed_addr constant [8 x i8] c"a = %c\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(i32 %f.coerce) #0 {
entry:
  %f = alloca %struct._FourChars, align 1
  %0 = bitcast %struct._FourChars* %f to i32*
  store i32 %f.coerce, i32* %0, align 1


; Check for the renaming of %y after prepare vpo-paropt-prepare.
; PREPR: [[FADDR:%[a-zA-Z._0-9]+]] = alloca %struct._FourChars*
; PREPR: store %struct._FourChars* %f, %struct._FourChars** [[FADDR]]
; PREPR: "QUAL.OMP.SHARED"(%struct._FourChars* %f)
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(%struct._FourChars* %f, %struct._FourChars** [[FADDR]])
; PREPR: [[FRENAMED:%[a-zA-Z._0-9]+]] = load volatile %struct._FourChars*, %struct._FourChars** [[FADDR]]
; PREPR: getelementptr inbounds %struct._FourChars, %struct._FourChars* [[FRENAMED]]

; Check that the IR was not modified by CSE + Instcombine

; INSTCMB: [[F:%[a-zA-Z._0-9]+]] = alloca i32
; INSTCMB: [[FCAST:%[a-zA-Z._0-9]+]] = bitcast i32* [[F]] to %struct._FourChars*
; INSTCMB: [[FADDR:%[a-zA-Z._0-9]+]] = alloca %struct._FourChars*
; INSTCMB: [[FADDR_CAST:%[a-zA-Z._0-9]+]] = bitcast %struct._FourChars** [[FADDR]] to i32**
; INSTCMB: store i32* [[F]], i32** [[FADDR_CAST]]
; INSTCMB: "QUAL.OMP.OPERAND.ADDR"(%struct._FourChars* [[FCAST]], %struct._FourChars** [[FADDR]])
; INSTCMB: [[FRENAMED:%[a-zA-Z._0-9]+]] = load volatile %struct._FourChars*, %struct._FourChars** [[FADDR]]
; INSTCMB: getelementptr inbounds %struct._FourChars, %struct._FourChars* [[FRENAMED]]

; Check for restore-operands was able to undo the renaming:
; RESTR: [[F:%[a-zA-Z._0-9]+]] = alloca i32
; RESTR: [[FCAST:%[a-zA-Z._0-9]+]] = bitcast i32* [[F]] to %struct._FourChars*
; RESTR: "QUAL.OMP.SHARED"(%struct._FourChars* [[FCAST]])
; RESTR-NOT: alloca %struct._FourChars*
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: getelementptr inbounds %struct._FourChars, %struct._FourChars* [[FCAST]]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(%struct._FourChars* %f), "QUAL.OMP.NUM_THREADS"(i32 1) ]
  %a = getelementptr inbounds %struct._FourChars, %struct._FourChars* %f, i32 0, i32 0
  %2 = load i8, i8* %a, align 1, !tbaa !2
  %conv = sext i8 %2 to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i32 %conv)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]


  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@_FourChars", !4, i64 0, !4, i64 1, !4, i64 2, !4, i64 3}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
