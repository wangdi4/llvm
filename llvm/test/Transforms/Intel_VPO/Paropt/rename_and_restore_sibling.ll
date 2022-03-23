; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Test src:
;
; #include <stdio.h>
; typedef char TYPE;
;
; void work() {
; TYPE y[10];
;
; #pragma omp parallel firstprivate(y)
;   printf ("y[1] = %d\n", y[1]);
;
; #pragma omp parallel firstprivate(y)
;   printf ("y[1] = %d\n", y[1]);
; }
;
; ModuleID = 'rename_and_restore_sibling.cpp'
source_filename = "rename_and_restore_sibling.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$__clang_call_terminate = comdat any

@.str = private unnamed_addr constant [11 x i8] c"y[1] = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @_Z4workv() #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %y = alloca [10 x i8], align 1
  %0 = bitcast [10 x i8]* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 10, i8* %0) #2


; Renaming of %y for first region:
; PREPR: store [10 x i8]* %y, [10 x i8]** [[YADDR1:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE"([10 x i8]* %y)
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"([10 x i8]* %y, [10 x i8]** [[YADDR1]])
; PREPR: [[YRENAMED1:%[a-zA-Z._0-9]+]] = load volatile [10 x i8]*, [10 x i8]** [[YADDR1]]
; PREPR: getelementptr inbounds [10 x i8], [10 x i8]* [[YRENAMED1]]
; PREPR: "DIR.OMP.END.PARALLEL"

; Renaming for second region:
; PREPR: store [10 x i8]* %y, [10 x i8]** [[YADDR2:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE"([10 x i8]* %y)
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"([10 x i8]* %y, [10 x i8]** [[YADDR2]])
; PREPR: [[YRENAMED2:%[a-zA-Z._0-9]+]] = load volatile [10 x i8]*, [10 x i8]** [[YADDR2]]
; PREPR: getelementptr inbounds [10 x i8], [10 x i8]* [[YRENAMED2]]
; PREPR: "DIR.OMP.END.PARALLEL"

; Check for the undone renaming after restore:
; RESTR-NOT alloca [10 x i8]*
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: "QUAL.OMP.FIRSTPRIVATE"([10 x i8]* %y)
; RESTR: getelementptr inbounds [10 x i8], [10 x i8]* %y
; RESTR: "DIR.OMP.END.PARALLEL"
; RESTR: "QUAL.OMP.FIRSTPRIVATE"([10 x i8]* %y)
; RESTR: getelementptr inbounds [10 x i8], [10 x i8]* %y
; RESTR: "DIR.OMP.END.PARALLEL"


  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"([10 x i8]* %y) ]
  %arrayidx = getelementptr inbounds [10 x i8], [10 x i8]* %y, i64 0, i64 1
  %2 = load i8, i8* %arrayidx, align 1, !tbaa !2
  %conv = sext i8 %2 to i32
  %call = invoke i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 %conv)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %entry
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"([10 x i8]* %y) ]
  %arrayidx1 = getelementptr inbounds [10 x i8], [10 x i8]* %y, i64 0, i64 1
  %4 = load i8, i8* %arrayidx1, align 1, !tbaa !2
  %conv2 = sext i8 %4 to i32
  %call4 = invoke i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 %conv2)
          to label %invoke.cont3 unwind label %terminate.lpad5

invoke.cont3:                                     ; preds = %invoke.cont
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]
  %5 = bitcast [10 x i8]* %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 10, i8* %5) #2
  ret void

terminate.lpad:                                   ; preds = %entry
  %6 = landingpad { i8*, i32 }
          catch i8* null
  %7 = extractvalue { i8*, i32 } %6, 0
  call void @__clang_call_terminate(i8* %7) #5
  unreachable

terminate.lpad5:                                  ; preds = %invoke.cont
  %8 = landingpad { i8*, i32 }
          catch i8* null
  %9 = extractvalue { i8*, i32 } %8, 0
  call void @__clang_call_terminate(i8* %9) #5
  unreachable
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local i32 @printf(i8*, ...) #3

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8*) #4 comdat {
  %2 = call i8* @__cxa_begin_catch(i8* %0) #2
  call void @_ZSt9terminatev() #5
  unreachable
}

declare dso_local i8* @__cxa_begin_catch(i8*)

declare dso_local void @_ZSt9terminatev()

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline noreturn nounwind }
attributes #5 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10_c", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
