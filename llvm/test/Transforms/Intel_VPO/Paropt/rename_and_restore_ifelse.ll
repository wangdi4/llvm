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
; extern volatile bool flag;
;
; void work() {
;   TYPE *y;
;
;   if (flag)
; #pragma omp parallel firstprivate(y)
;     printf ("y = %p\n", y);
;   else
; #pragma omp parallel firstprivate(y)
;     printf ("y = %p\n", y);
; }
;
; ModuleID = 'rename_and_restore_ifelse.cpp'
source_filename = "rename_and_restore_ifelse.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$__clang_call_terminate = comdat any

@flag = external dso_local global i8, align 1
@.str = private unnamed_addr constant [8 x i8] c"y = %p\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @_Z4workv() #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %y = alloca i8*, align 8
  %0 = bitcast i8** %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #2
  %1 = load volatile i8, i8* @flag, align 1, !tbaa !2, !range !6
  %tobool = trunc i8 %1 to i1
  br i1 %tobool, label %if.then, label %if.else

; Renaming of %y for first region:
; PREPR: store i8** %y, i8*** [[YADDR1:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE"(i8** %y)
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(i8** %y, i8*** [[YADDR1]])
; PREPR: [[YRENAMED1:%[a-zA-Z._0-9]+]] = load volatile i8**, i8*** [[YADDR1]]
; PREPR: load i8*, i8** [[YRENAMED1]]
; PREPR: "DIR.OMP.END.PARALLEL"

; Renaming for inner region:
; PREPR: store i8** %y, i8*** [[YADDR2:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE"(i8** %y)
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(i8** %y, i8*** [[YADDR2]])
; PREPR: [[YRENAMED2:%[a-zA-Z._0-9]+]] = load volatile i8**, i8*** [[YADDR2]]
; PREPR: load i8*, i8** [[YRENAMED2]]
; PREPR: "DIR.OMP.END.PARALLEL"

; Check for the undone renaming after restore:
; RESTR-NOT alloca i8**
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: "QUAL.OMP.FIRSTPRIVATE"(i8** %y)
; RESTR: load i8*, i8** %y
; RESTR: "DIR.OMP.END.PARALLEL"
; RESTR: "QUAL.OMP.FIRSTPRIVATE"(i8** %y)
; RESTR: load i8*, i8** %y
; RESTR: "DIR.OMP.END.PARALLEL"

if.then:                                          ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"(i8** %y) ]
  %3 = load i8*, i8** %y, align 8, !tbaa !7
  %call = invoke i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i8* %3)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %if.then
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  br label %if.end

terminate.lpad:                                   ; preds = %if.then
  %4 = landingpad { i8*, i32 }
          catch i8* null
  %5 = extractvalue { i8*, i32 } %4, 0
  call void @__clang_call_terminate(i8* %5) #5
  unreachable

if.else:                                          ; preds = %entry
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"(i8** %y) ]
  %7 = load i8*, i8** %y, align 8, !tbaa !7
  %call2 = invoke i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i8* %7)
          to label %invoke.cont1 unwind label %terminate.lpad3

invoke.cont1:                                     ; preds = %if.else
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.PARALLEL"() ]
  br label %if.end

terminate.lpad3:                                  ; preds = %if.else
  %8 = landingpad { i8*, i32 }
          catch i8* null
  %9 = extractvalue { i8*, i32 } %8, 0
  call void @__clang_call_terminate(i8* %9) #5
  unreachable

if.end:                                           ; preds = %invoke.cont1, %invoke.cont
  %10 = bitcast i8** %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %10) #2
  ret void
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
!2 = !{!3, !3, i64 0}
!3 = !{!"bool", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{i8 0, i8 2}
!7 = !{!8, !8, i64 0}
!8 = !{!"pointer@_ZTSPc", !4, i64 0}
