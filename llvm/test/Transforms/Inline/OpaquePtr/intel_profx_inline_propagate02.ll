; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -inline -dtrans-inline-heuristics -intel-libirc-allowed -S 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers < %s -passes=inline -dtrans-inline-heuristics -intel-libirc-allowed -S 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that intel_profx counts on invokes are propagated correctly after
; inlining.

; CHECK: define dso_local i32 @main
; CHECK: call fastcc i32 @_ZL3fooi{{.*}}!intel-profx [[IPX1:![0-9]+]]
; CHECK: call fastcc i32 @_ZL3fooi{{.*}}!intel-profx [[IPX2:![0-9]+]]
; CHECK: call fastcc i32 @_ZL3fooi{{.*}}!intel-profx [[IPX1]]
; CHECK: define internal fastcc i32 @_ZL3fooi
; CHECK: call fastcc i32 @_ZL3bazi{{.*}}!intel-profx [[IPX3:![0-9]+]]
; CHECK: call fastcc i32 @_ZL3bazi{{.*}}!intel-profx [[IPX4:![0-9]+]]
; CHECK: call ptr @__cxa_begin_catch{{.*}}!intel-profx [[IPX3]]
; CHECK: call void @__cxa_end_catch{{.*}}!intel-profx [[IPX3]]
; CHECK: call ptr @__cxa_allocate_exception{{.*}}!intel-profx [[IPX3]]
; CHECK: to label %unreachable unwind label{{.*}}!intel-profx [[IPX3]]
; CHECK: define internal fastcc i32 @_ZL3bazi
; CHECK: [[IPX1]] = !{!"intel_profx", i64 1}
; CHECK: [[IPX2]] = !{!"intel_profx", i64 9999}
; CHECK: [[IPX3]] = !{!"intel_profx", i64 10001}
; CHECK: [[IPX4]] = !{!"intel_profx", i64 50005000}

@_ZTIi = external dso_local constant ptr

; Function Attrs: cold norecurse uwtable
define dso_local i32 @main() local_unnamed_addr #0 !prof !29 {
entry:
  %call = call fastcc i32 @_ZL3fooi(), !intel-profx !30
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 1, %entry ], [ %inc, %for.body ]
  %s.0 = phi i32 [ %call, %entry ], [ %add, %for.body ]
  %cmp = icmp ult i32 %i.0, 10000
  br i1 %cmp, label %for.body, label %for.end, !prof !31

for.body:                                         ; preds = %for.cond
  %call1 = call fastcc i32 @_ZL3fooi(), !intel-profx !32
  %add = add nsw i32 %s.0, %call1
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %call2 = call fastcc i32 @_ZL3fooi() #6, !intel-profx !30
  %add3 = add nsw i32 %s.0, %call2
  ret i32 %add3
}

; Function Attrs: uwtable
define internal fastcc i32 @_ZL3fooi() unnamed_addr #1 personality ptr @__gxx_personality_v0 !prof !33 !PGOFuncName !34 {
entry:
  %call = invoke fastcc i32 @_ZL3bari(i32 0)
          to label %for.cond unwind label %lpad, !intel-profx !35

for.cond:                                         ; preds = %invoke.cont1, %entry
  %t.0 = phi i32 [ %add3, %invoke.cont1 ], [ %call, %entry ]
  %j.0 = phi i32 [ %inc, %invoke.cont1 ], [ 0, %entry ]
  %cmp = icmp ult i32 %j.0, 5000
  br i1 %cmp, label %for.body, label %for.end, !prof !36

for.body:                                         ; preds = %for.cond
  %call2 = invoke fastcc i32 @_ZL3bari(i32 %j.0)
          to label %invoke.cont1 unwind label %lpad, !intel-profx !37

invoke.cont1:                                     ; preds = %for.body
  %mul = mul nsw i32 %j.0, %j.0
  %sub = sub nsw i32 %mul, %j.0
  %add = add nsw i32 %sub, %call2
  %add3 = add nsw i32 %t.0, %add
  %inc = add nuw nsw i32 %j.0, 1
  br label %for.cond

lpad:                                             ; preds = %if.then, %for.body, %entry
  %i = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIi
  %i1 = extractvalue { ptr, i32 } %i, 1
  %i2 = call i32 @llvm.eh.typeid.for(ptr @_ZTIi) #7
  %matches = icmp eq i32 %i1, %i2
  br i1 %matches, label %catch, label %ehcleanup, !prof !38

catch:                                            ; preds = %lpad
  %i3 = extractvalue { ptr, i32 } %i, 0
  %i4 = call ptr @__cxa_begin_catch(ptr %i3) #7, !intel-profx !35
  %i6 = load i32, ptr %i4, align 4, !tbaa !39
  %sub5 = sub nsw i32 0, %i6
  call void @__cxa_end_catch() #7, !intel-profx !35
  br label %cleanup

for.end:                                          ; preds = %for.cond
  %cmp4 = icmp slt i32 %t.0, 0
  br i1 %cmp4, label %if.then, label %cleanup, !prof !38

if.then:                                          ; preds = %for.end
  %exception = call ptr @__cxa_allocate_exception(i64 4) #7, !intel-profx !35
  store i32 100, ptr %exception, align 16, !tbaa !39
  invoke void @__cxa_throw(ptr nonnull %exception, ptr @_ZTIi, ptr null) #3
          to label %unreachable unwind label %lpad, !intel-profx !35

cleanup:                                          ; preds = %for.end, %catch
  %retval.0 = phi i32 [ %sub5, %catch ], [ %t.0, %for.end ]
  ret i32 %retval.0

ehcleanup:                                        ; preds = %lpad
  resume { ptr, i32 } %i

unreachable:                                      ; preds = %if.then
  unreachable
}

; Function Attrs: inlinehint uwtable
define internal fastcc i32 @_ZL3bari(i32 %x) unnamed_addr #2 personality ptr @__gxx_personality_v0 !prof !43 !PGOFuncName !44 {
entry:
  %cmp = icmp slt i32 %x, 0
  br i1 %cmp, label %if.then, label %try.cont, !prof !45

if.then:                                          ; preds = %entry
  %exception = call ptr @__cxa_allocate_exception(i64 4) #7, !intel-profx !46
  store i32 200, ptr %exception, align 16, !tbaa !39
  invoke void @__cxa_throw(ptr nonnull %exception, ptr @_ZTIi, ptr null) #3
          to label %unreachable unwind label %lpad, !intel-profx !46

lpad:                                             ; preds = %if.then
  %i1 = landingpad { ptr, i32 }
          catch ptr @_ZTIi
  %i2 = extractvalue { ptr, i32 } %i1, 1
  %i3 = call i32 @llvm.eh.typeid.for(ptr @_ZTIi) #7
  %matches = icmp eq i32 %i2, %i3
  br i1 %matches, label %catch, label %eh.resume

catch:                                            ; preds = %lpad
  %i4 = extractvalue { ptr, i32 } %i1, 0
  %i5 = call ptr @__cxa_begin_catch(ptr %i4) #7, !intel-profx !46
  %i7 = load i32, ptr %i5, align 4, !tbaa !39
  %mul = mul nsw i32 %i7, -2
  call void @__cxa_end_catch() #7, !intel-profx !46
  br label %return

try.cont:                                         ; preds = %entry
  %call = call fastcc i32 @_ZL3bazi(i32 %x), !intel-profx !47
  br label %return

return:                                           ; preds = %try.cont, %catch
  %retval.0 = phi i32 [ %mul, %catch ], [ %call, %try.cont ]
  ret i32 %retval.0

eh.resume:                                        ; preds = %lpad
  resume { ptr, i32 } %i1

unreachable:                                      ; preds = %if.then
  unreachable
}

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local noalias nonnull ptr @__cxa_allocate_exception(i64) local_unnamed_addr

; Function Attrs: noreturn
declare dso_local void @__cxa_throw(ptr, ptr, ptr) local_unnamed_addr #3

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(ptr) #4

declare dso_local ptr @__cxa_begin_catch(ptr) local_unnamed_addr

declare dso_local void @__cxa_end_catch() local_unnamed_addr

; Function Attrs: inlinehint noinline nounwind uwtable
define internal fastcc i32 @_ZL3bazi(i32 %x) unnamed_addr #5 !prof !43 !PGOFuncName !48 {
entry:
  ret i32 %x
}

attributes #0 = { cold norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { inlinehint uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn }
attributes #4 = { nounwind readnone }
attributes #5 = { inlinehint noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { noinline }
attributes #7 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!28}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ProfileSummary", !2}
!2 = !{!3, !4, !5, !6, !7, !8, !9, !10}
!3 = !{!"ProfileFormat", !"InstrProf"}
!4 = !{!"TotalCount", i64 200080005}
!5 = !{!"MaxCount", i64 50015001}
!6 = !{!"MaxInternalCount", i64 50005000}
!7 = !{!"MaxFunctionCount", i64 50015001}
!8 = !{!"NumCounts", i64 14}
!9 = !{!"NumFunctions", i64 4}
!10 = !{!"DetailedSummary", !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27}
!12 = !{i32 10000, i64 50015001, i32 2}
!13 = !{i32 100000, i64 50015001, i32 2}
!14 = !{i32 200000, i64 50015001, i32 2}
!15 = !{i32 300000, i64 50015001, i32 2}
!16 = !{i32 400000, i64 50015001, i32 2}
!17 = !{i32 500000, i64 50005000, i32 4}
!18 = !{i32 600000, i64 50005000, i32 4}
!19 = !{i32 700000, i64 50005000, i32 4}
!20 = !{i32 800000, i64 50005000, i32 4}
!21 = !{i32 900000, i64 50005000, i32 4}
!22 = !{i32 950000, i64 50005000, i32 4}
!23 = !{i32 990000, i64 50005000, i32 4}
!24 = !{i32 999000, i64 50005000, i32 4}
!25 = !{i32 999900, i64 10001, i32 7}
!26 = !{i32 999990, i64 9999, i32 8}
!27 = !{i32 999999, i64 9999, i32 8}
!28 = !{!"icx (ICX) dev.8.x.0"}
!29 = !{!"function_entry_count", i64 1}
!30 = !{!"intel_profx", i64 1}
!31 = !{!"branch_weights", i32 9999, i32 1}
!32 = !{!"intel_profx", i64 9999}
!33 = !{!"function_entry_count", i64 10001}
!34 = !{!"sm.cpp:_ZL3fooi"}
!35 = !{!"intel_profx", i64 10001}
!36 = !{!"branch_weights", i32 50005000, i32 10001}
!37 = !{!"intel_profx", i64 50005000}
!38 = !{!"branch_weights", i32 10001, i32 0}
!39 = !{!40, !40, i64 0}
!40 = !{!"int", !41, i64 0}
!41 = !{!"omnipotent char", !42, i64 0}
!42 = !{!"Simple C++ TBAA"}
!43 = !{!"function_entry_count", i64 50015001}
!44 = !{!"sm.cpp:_ZL3bari"}
!45 = !{!"branch_weights", i32 0, i32 50015001}
!46 = !{!"intel_profx", i64 0}
!47 = !{!"intel_profx", i64 50015001}
!48 = !{!"sm.cpp:_ZL3bazi"}
; end INTEL_FEATURE_SW_ADVANCED
