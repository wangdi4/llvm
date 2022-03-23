; CMPLRLLVM-8372, CMPLRLLVM-8373
; try/catch inside parallel region.
; The Code Extractor must tolerate the typeid call and complete the outline.

; RUN: opt -vpo-paropt -S %s | FileCheck %s
; CHECK: define{{.*}}main.DIR.OMP.PARALLEL
; CHECK: invoke void @_Z3foov
; CHECK: llvm.eh.typeid.for
; CHECK: @__cxa_begin_catch
; CHECK: resume

%struct.thing = type { i8 }

@g = dso_local global i32 1, align 4
@_ZTIi = external dso_local constant i8*
@.str = private unnamed_addr constant [8 x i8] c"caught\0A\00", align 1
@.str.1 = private unnamed_addr constant [13 x i8] c"CONSTRUCTED\0A\00", align 1
@.str.2 = private unnamed_addr constant [11 x i8] c"DESTROYED\0A\00", align 1
@str = private unnamed_addr constant [7 x i8] c"caught\00", align 1
@str.1 = private unnamed_addr constant [12 x i8] c"CONSTRUCTED\00", align 1
@str.2 = private unnamed_addr constant [10 x i8] c"DESTROYED\00", align 1

; Function Attrs: noinline uwtable
define dso_local void @_Z3foov() local_unnamed_addr #0 {
entry:
  %0 = load volatile i32, i32* @g, align 4, !tbaa !2
  %tobool = icmp eq i32 %0, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %exception = call i8* @__cxa_allocate_exception(i64 4) #3
  %1 = bitcast i8* %exception to i32*
  store i32 5, i32* %1, align 16, !tbaa !2
  call void @__cxa_throw(i8* nonnull %exception, i8* bitcast (i8** @_ZTIi to i8*), i8* null) #1
  unreachable

if.end:                                           ; preds = %entry
  ret void
}

declare dso_local noalias nonnull i8* @__cxa_allocate_exception(i64) local_unnamed_addr

; Function Attrs: noreturn
declare dso_local void @__cxa_throw(i8*, i8*, i8*) local_unnamed_addr #1

; Function Attrs: norecurse uwtable
define dso_local i32 @main() local_unnamed_addr #2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
DIR.OMP.PARALLEL.1:
  %t = alloca %struct.thing, align 1
  %i = alloca i32, align 4
  br label %DIR.OMP.PARALLEL.13

DIR.OMP.PARALLEL.13:                              ; preds = %DIR.OMP.PARALLEL.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(%struct.thing* %t), "QUAL.OMP.PRIVATE"(i32* %i) ]
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.13
  %1 = getelementptr inbounds %struct.thing, %struct.thing* %t, i64 0, i32 0
  %2 = call i8* @llvm.launder.invariant.group.p0i8(i8* nonnull %1)
  %3 = bitcast i8* %2 to %struct.thing*
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %2) #3
  %puts.i = call i32 @puts(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @str.1, i64 0, i64 0))
  invoke void @_Z3foov()
          to label %invoke.cont2 unwind label %lpad1

invoke.cont2:                                     ; preds = %DIR.OMP.PARALLEL.2
  %puts.i1 = call i32 @puts(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @str.2, i64 0, i64 0))
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %2) #3
  br label %DIR.OMP.END.PARALLEL.3

lpad1:                                            ; preds = %DIR.OMP.PARALLEL.2
  %4 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast (i8** @_ZTIi to i8*)
  %5 = extractvalue { i8*, i32 } %4, 0
  %6 = extractvalue { i8*, i32 } %4, 1
  %puts.i2 = call i32 @puts(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @str.2, i64 0, i64 0))
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %2) #3
  %7 = call i32 @llvm.eh.typeid.for(i8* bitcast (i8** @_ZTIi to i8*)) #3
  %matches = icmp eq i32 %6, %7
  br i1 %matches, label %invoke.cont6, label %eh.resume

invoke.cont6:                                     ; preds = %lpad1
  %8 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %8) #3
  %9 = call i8* @__cxa_begin_catch(i8* %5) #3
  %10 = bitcast i8* %9 to i32*
  %11 = load i32, i32* %10, align 4, !tbaa !2
  store i32 %11, i32* %i, align 4, !tbaa !2
  %puts = call i32 @puts(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @str, i64 0, i64 0))
  call void @__cxa_end_catch() #3
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %8) #3
  br label %DIR.OMP.END.PARALLEL.3

DIR.OMP.END.PARALLEL.3:                           ; preds = %invoke.cont6, %invoke.cont2
  br label %DIR.OMP.END.PARALLEL.34

DIR.OMP.END.PARALLEL.34:                          ; preds = %DIR.OMP.END.PARALLEL.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.END.PARALLEL.34
  ret i32 0

eh.resume:                                        ; preds = %lpad1
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %5, 0
  %lpad.val11 = insertvalue { i8*, i32 } %lpad.val, i32 %6, 1
  resume { i8*, i32 } %lpad.val11
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #4

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr

; Function Attrs: noreturn nounwind
declare dso_local void @_ZSt9terminatev() local_unnamed_addr #5

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #4

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #6

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #7

declare dso_local void @__cxa_end_catch() local_unnamed_addr

; Function Attrs: inaccessiblememonly nounwind speculatable
declare i8* @llvm.launder.invariant.group.p0i8(i8*) #8

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) #3

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noreturn }
attributes #2 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { argmemonly nounwind }
attributes #5 = { noreturn nounwind }
attributes #6 = { nounwind readnone }
attributes #7 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { inaccessiblememonly nounwind speculatable }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 59f0a4f3baa133224b131c19ed0a4a18f74d9a89) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 184fcb8b15072e3cac3e3fbb9aa8c46a28d273e3)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
