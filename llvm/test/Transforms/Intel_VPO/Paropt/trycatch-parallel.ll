; INTEL_CUSTOMIZATION
; CMPLRLLVM-8372, CMPLRLLVM-8373
; end INTEL_CUSTOMIZATION
; try/catch inside parallel region.
; The Code Extractor must tolerate the typeid call and complete the outline.

; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="vpo-paropt" -S %s | FileCheck %s

; CHECK: define{{.*}}main.DIR.OMP.PARALLEL
; CHECK: invoke void @_Z3foov
; CHECK: llvm.eh.typeid.for
; CHECK: @__cxa_begin_catch
; CHECK: resume

%struct.thing = type { i8 }

@g = dso_local global i32 1, align 4
@_ZTIi = external dso_local constant ptr
@.str = private unnamed_addr constant [8 x i8] c"caught\0A\00", align 1
@.str.1 = private unnamed_addr constant [13 x i8] c"CONSTRUCTED\0A\00", align 1
@.str.2 = private unnamed_addr constant [11 x i8] c"DESTROYED\0A\00", align 1
@str = private unnamed_addr constant [7 x i8] c"caught\00", align 1
@str.1 = private unnamed_addr constant [12 x i8] c"CONSTRUCTED\00", align 1
@str.2 = private unnamed_addr constant [10 x i8] c"DESTROYED\00", align 1

; Function Attrs: noinline uwtable
define dso_local void @_Z3foov() local_unnamed_addr #0 {
entry:
  %0 = load volatile i32, ptr @g, align 4, !tbaa !2
  %tobool = icmp eq i32 %0, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %exception = call ptr @__cxa_allocate_exception(i64 4) #3
  store i32 5, ptr %exception, align 16, !tbaa !2
  call void @__cxa_throw(ptr nonnull %exception, ptr @_ZTIi, ptr null) #1
  unreachable

if.end:                                           ; preds = %entry
  ret void
}

declare dso_local noalias nonnull ptr @__cxa_allocate_exception(i64) local_unnamed_addr

; Function Attrs: noreturn
declare dso_local void @__cxa_throw(ptr, ptr, ptr) local_unnamed_addr #1

; Function Attrs: norecurse uwtable
define dso_local i32 @main() local_unnamed_addr #2 personality ptr @__gxx_personality_v0 {
DIR.OMP.PARALLEL.1:
  %t = alloca %struct.thing, align 1
  %i = alloca i32, align 4
  br label %DIR.OMP.PARALLEL.13

DIR.OMP.PARALLEL.13:                              ; preds = %DIR.OMP.PARALLEL.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %t, %struct.thing zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.13
  %1 = call ptr @llvm.launder.invariant.group.p0(ptr nonnull %t)
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %1) #3
  %puts.i = call i32 @puts(ptr @str.1)
  invoke void @_Z3foov()
          to label %invoke.cont2 unwind label %lpad1

invoke.cont2:                                     ; preds = %DIR.OMP.PARALLEL.2
  %puts.i1 = call i32 @puts(ptr @str.2)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %1) #3
  br label %DIR.OMP.END.PARALLEL.3

lpad1:                                            ; preds = %DIR.OMP.PARALLEL.2
  %2 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIi
  %3 = extractvalue { ptr, i32 } %2, 0
  %4 = extractvalue { ptr, i32 } %2, 1
  %puts.i2 = call i32 @puts(ptr @str.2)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %1) #3
  %5 = call i32 @llvm.eh.typeid.for(ptr @_ZTIi) #3
  %matches = icmp eq i32 %4, %5
  br i1 %matches, label %invoke.cont6, label %eh.resume

invoke.cont6:                                     ; preds = %lpad1
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i) #3
  %6 = call ptr @__cxa_begin_catch(ptr %3) #3
  %7 = load i32, ptr %6, align 4, !tbaa !2
  store i32 %7, ptr %i, align 4, !tbaa !2
  %puts = call i32 @puts(ptr @str)
  call void @__cxa_end_catch() #3
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i) #3
  br label %DIR.OMP.END.PARALLEL.3

DIR.OMP.END.PARALLEL.3:                           ; preds = %invoke.cont6, %invoke.cont2
  br label %DIR.OMP.END.PARALLEL.34

DIR.OMP.END.PARALLEL.34:                          ; preds = %DIR.OMP.END.PARALLEL.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.END.PARALLEL.34
  ret i32 0

eh.resume:                                        ; preds = %lpad1
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %3, 0
  %lpad.val11 = insertvalue { ptr, i32 } %lpad.val, i32 %4, 1
  resume { ptr, i32 } %lpad.val11
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #4

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local ptr @__cxa_begin_catch(ptr) local_unnamed_addr

; Function Attrs: noreturn nounwind
declare dso_local void @_ZSt9terminatev() local_unnamed_addr #5

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #4

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(ptr) #6

; Function Attrs: nounwind
declare dso_local i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #7

declare dso_local void @__cxa_end_catch() local_unnamed_addr

; Function Attrs: inaccessiblememonly nounwind speculatable
declare ptr @llvm.launder.invariant.group.p0(ptr) #8

; Function Attrs: nounwind
declare i32 @puts(ptr nocapture readonly) #3

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
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
