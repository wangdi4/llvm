; RUN: opt < %s -S -passes=optimize-dyn-casts | FileCheck %s

; Even when whole program is detected there could be library classes from
; standard header files with descedants in some other parts of library. In
; this case all information will be available for the linker, so we could
; rely on the linkage type of a type_info global (because there are no things
; like dlopen in the standard library which could break the ABI).  The
; linkage type will be identified by internalization pass  using information
; from the linker when the LTO is enabled. If the linkage type is not
; internal then we could not say that a class is final. That is why
; optimization should not be performed.
;
; Source code:
; #include <exception>
; #include <stdexcept>
;
; using namespace std;
;
;
; __attribute__((noinline)) extern "C" int test1(exception *p) {
;   runtime_error *res = dynamic_cast<runtime_error *>(p);
;   if (res != nullptr) {
;     return 1;
;   }
;   return 0;
; }
;
;
; int main() {
;   exception *ptr = new range_error("error");
;   test1(ptr);
;   delete ptr;
;   return 0;
; }
;
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::exception" = type { i32 (...)** }
%"class.std::allocator" = type { i8 }
%"class.std::basic_string" = type { %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" }
%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" = type { i8* }
%"class.std::range_error" = type { %"class.std::runtime_error" }
%"class.std::runtime_error" = type { %"class.std::exception", %"class.std::basic_string" }
%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep" = type { %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep_base" }
%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep_base" = type { i64, i64, i32 }

@_ZTISt9exception = external constant i8*
@_ZTISt13runtime_error = external constant i8*
@.str = private unnamed_addr constant [6 x i8] c"error\00", align 1
@_ZNSs4_Rep20_S_empty_rep_storageE = external global [0 x i64], align 8

; Function Attrs: noinline nounwind readonly uwtable
define internal dso_local i32 @test1(%"class.std::exception"* readonly %p) #0 {
; CHECK-LABEL: @test1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = icmp eq %"class.std::exception"* [[P:%.*]], null
; CHECK-NEXT:    br i1 [[TMP0]], label [[IF_END:%.*]], label [[DYNAMIC_CAST_NOTNULL:%.*]]
; CHECK:       dynamic_cast.notnull:
; CHECK-NEXT:    [[TMP1:%.*]] = bitcast %"class.std::exception"* [[P]] to i8*
; CHECK-NEXT:    [[TMP2:%.*]] = tail call i8* @__dynamic_cast(i8* [[TMP1]], i8* bitcast (i8** @_ZTISt9exception to i8*), i8* bitcast (i8** @_ZTISt13runtime_error to i8*), i64 0) #8
; CHECK-NEXT:    [[PHITMP:%.*]] = icmp eq i8* [[TMP2]], null
; CHECK-NEXT:    br i1 [[PHITMP]], label [[IF_END]], label [[CLEANUP:%.*]]
; CHECK:       if.end:
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ 0, [[IF_END]] ], [ 1, [[DYNAMIC_CAST_NOTNULL]] ]
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;
entry:
  %0 = icmp eq %"class.std::exception"* %p, null
  br i1 %0, label %if.end, label %dynamic_cast.notnull

dynamic_cast.notnull:                             ; preds = %entry
  %1 = bitcast %"class.std::exception"* %p to i8*
  %2 = tail call i8* @__dynamic_cast(i8* %1, i8* bitcast (i8** @_ZTISt9exception to i8*), i8* bitcast (i8** @_ZTISt13runtime_error to i8*), i64 0) #8
  %phitmp = icmp eq i8* %2, null
  br i1 %phitmp, label %if.end, label %cleanup

if.end:                                           ; preds = %dynamic_cast.notnull, %entry
  br label %cleanup

cleanup:                                          ; preds = %if.end, %dynamic_cast.notnull
  %retval.0 = phi i32 [ 0, %if.end ], [ 1, %dynamic_cast.notnull ]
  ret i32 %retval.0
}

; Function Attrs: nounwind readonly
declare i8* @__dynamic_cast(i8*, i8*, i8*, i64) local_unnamed_addr #1

; Function Attrs: norecurse uwtable
define dso_local i32 @main() #2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %ref.tmp.i13 = alloca %"class.std::allocator", align 1
  %ref.tmp.i = alloca %"class.std::allocator", align 1
  %ref.tmp = alloca %"class.std::basic_string", align 8
  %ref.tmp1 = alloca %"class.std::allocator", align 1
  %call = tail call i8* @_Znwm(i64 16) #9
  %0 = bitcast %"class.std::basic_string"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %0) #8
  %1 = getelementptr inbounds %"class.std::allocator", %"class.std::allocator"* %ref.tmp1, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %1) #8
  invoke void @_ZNSsC1EPKcRKSaIcE(%"class.std::basic_string"* nonnull %ref.tmp, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), %"class.std::allocator"* nonnull dereferenceable(1) %ref.tmp1)
  to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %2 = bitcast i8* %call to %"class.std::range_error"*
  invoke void @_ZNSt11range_errorC1ERKSs(%"class.std::range_error"* nonnull %2, %"class.std::basic_string"* nonnull dereferenceable(8) %ref.tmp)
  to label %invoke.cont3 unwind label %lpad2

invoke.cont3:                                     ; preds = %invoke.cont
  %3 = bitcast i8* %call to %"class.std::exception"*
  %_M_p.i.i.i14 = getelementptr inbounds %"class.std::basic_string", %"class.std::basic_string"* %ref.tmp, i64 0, i32 0, i32 0
  %4 = load i8*, i8** %_M_p.i.i.i14, align 8, !tbaa !2
  %arrayidx.i.i15 = getelementptr inbounds i8, i8* %4, i64 -24
  %5 = bitcast i8* %arrayidx.i.i15 to %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"*
  %6 = getelementptr inbounds %"class.std::allocator", %"class.std::allocator"* %ref.tmp.i13, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %6) #8
  %cmp.i.i16 = icmp eq i8* %arrayidx.i.i15, bitcast ([0 x i64]* @_ZNSs4_Rep20_S_empty_rep_storageE to i8*)
  br i1 %cmp.i.i16, label %_ZNSsD2Ev.exit26, label %if.then.i.i18, !prof !8

if.then.i.i18:                                    ; preds = %invoke.cont3
  %_M_refcount.i.i17 = getelementptr inbounds i8, i8* %4, i64 -8
  %7 = bitcast i8* %_M_refcount.i.i17 to i32*
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %if.then.i.i.i19, label %if.else.i.i.i21

if.then.i.i.i19:                                  ; preds = %if.then.i.i18
  %8 = atomicrmw volatile add i32* %7, i32 -1 acq_rel
  br label %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i24

if.else.i.i.i21:                                  ; preds = %if.then.i.i18
  %9 = load i32, i32* %7, align 4, !tbaa !9
  %add.i.i.i.i20 = add nsw i32 %9, -1
  store i32 %add.i.i.i.i20, i32* %7, align 4, !tbaa !9
  br label %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i24

_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i24: ; preds = %if.else.i.i.i21, %if.then.i.i.i19
  %retval.0.i.i.i22 = phi i32 [ %8, %if.then.i.i.i19 ], [ %9, %if.else.i.i.i21 ]
  %cmp3.i.i23 = icmp slt i32 %retval.0.i.i.i22, 1
  br i1 %cmp3.i.i23, label %if.then4.i.i25, label %_ZNSsD2Ev.exit26

if.then4.i.i25:                                   ; preds = %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i24
  call void @_ZNSs4_Rep10_M_destroyERKSaIcE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"* nonnull %5, %"class.std::allocator"* nonnull dereferenceable(1) %ref.tmp.i13) #8
  br label %_ZNSsD2Ev.exit26

_ZNSsD2Ev.exit26:                                 ; preds = %if.then4.i.i25, %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i24, %invoke.cont3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %6) #8
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %1) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #8
  %10 = bitcast i8* %call to void (%"class.std::exception"*)***
  %vtable = load void (%"class.std::exception"*)**, void (%"class.std::exception"*)*** %10, align 8, !tbaa !11
  %vfn = getelementptr inbounds void (%"class.std::exception"*)*, void (%"class.std::exception"*)** %vtable, i64 1
  %11 = load void (%"class.std::exception"*)*, void (%"class.std::exception"*)** %vfn, align 8
  call void %11(%"class.std::exception"* nonnull %3) #8
  ret i32 0

lpad:                                             ; preds = %entry
  %12 = landingpad { i8*, i32 }
  cleanup
  %13 = extractvalue { i8*, i32 } %12, 0
  %14 = extractvalue { i8*, i32 } %12, 1
  br label %ehcleanup

lpad2:                                            ; preds = %invoke.cont
  %15 = landingpad { i8*, i32 }
  cleanup
  %16 = extractvalue { i8*, i32 } %15, 0
  %17 = extractvalue { i8*, i32 } %15, 1
  %_M_p.i.i.i = getelementptr inbounds %"class.std::basic_string", %"class.std::basic_string"* %ref.tmp, i64 0, i32 0, i32 0
  %18 = load i8*, i8** %_M_p.i.i.i, align 8, !tbaa !2
  %arrayidx.i.i = getelementptr inbounds i8, i8* %18, i64 -24
  %19 = bitcast i8* %arrayidx.i.i to %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"*
  %20 = getelementptr inbounds %"class.std::allocator", %"class.std::allocator"* %ref.tmp.i, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %20) #8
  %cmp.i.i = icmp eq i8* %arrayidx.i.i, bitcast ([0 x i64]* @_ZNSs4_Rep20_S_empty_rep_storageE to i8*)
  br i1 %cmp.i.i, label %_ZNSsD2Ev.exit, label %if.then.i.i, !prof !8

if.then.i.i:                                      ; preds = %lpad2
  %_M_refcount.i.i = getelementptr inbounds i8, i8* %18, i64 -8
  %21 = bitcast i8* %_M_refcount.i.i to i32*
  br i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null), label %if.then.i.i.i, label %if.else.i.i.i

if.then.i.i.i:                                    ; preds = %if.then.i.i
  %22 = atomicrmw volatile add i32* %21, i32 -1 acq_rel
  br label %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i

if.else.i.i.i:                                    ; preds = %if.then.i.i
  %23 = load i32, i32* %21, align 4, !tbaa !9
  %add.i.i.i.i = add nsw i32 %23, -1
  store i32 %add.i.i.i.i, i32* %21, align 4, !tbaa !9
  br label %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i

_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i: ; preds = %if.else.i.i.i, %if.then.i.i.i
  %retval.0.i.i.i = phi i32 [ %22, %if.then.i.i.i ], [ %23, %if.else.i.i.i ]
  %cmp3.i.i = icmp slt i32 %retval.0.i.i.i, 1
  br i1 %cmp3.i.i, label %if.then4.i.i, label %_ZNSsD2Ev.exit

if.then4.i.i:                                     ; preds = %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i
  call void @_ZNSs4_Rep10_M_destroyERKSaIcE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"* nonnull %19, %"class.std::allocator"* nonnull dereferenceable(1) %ref.tmp.i) #8
  br label %_ZNSsD2Ev.exit

_ZNSsD2Ev.exit:                                   ; preds = %if.then4.i.i, %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i, %lpad2
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %20) #8
  br label %ehcleanup

ehcleanup:                                        ; preds = %_ZNSsD2Ev.exit, %lpad
  %exn.slot.0 = phi i8* [ %16, %_ZNSsD2Ev.exit ], [ %13, %lpad ]
  %ehselector.slot.0 = phi i32 [ %17, %_ZNSsD2Ev.exit ], [ %14, %lpad ]
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %1) #8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #8
  call void @_ZdlPv(i8* nonnull %call) #10
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0
  %lpad.val9 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { i8*, i32 } %lpad.val9
}

declare i32 @__gxx_personality_v0(...)

; Function Attrs: nobuiltin
declare noalias nonnull i8* @_Znwm(i64) local_unnamed_addr #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #4

declare void @_ZNSsC1EPKcRKSaIcE(%"class.std::basic_string"*, i8*, %"class.std::allocator"* dereferenceable(1)) unnamed_addr #5

declare void @_ZNSt11range_errorC1ERKSs(%"class.std::range_error"*, %"class.std::basic_string"* dereferenceable(8)) unnamed_addr #5

; Function Attrs: nounwind
declare extern_weak i32 @__pthread_key_create(i32*, void (i8*)*) #6

; Function Attrs: nounwind
declare void @_ZNSs4_Rep10_M_destroyERKSaIcE(%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Rep"*, %"class.std::allocator"* dereferenceable(1)) local_unnamed_addr #6

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #4

; Function Attrs: nobuiltin nounwind
declare void @_ZdlPv(i8*) local_unnamed_addr #7

attributes #0 = { noinline nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly }
attributes #2 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { argmemonly nounwind }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { nobuiltin nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { nounwind }
attributes #9 = { builtin }
attributes #10 = { builtin nounwind }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1}

!0 = !{!"clang version 6.0.0"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !5, i64 0}
!3 = !{!"struct@_ZTSSs", !4, i64 0}
!4 = !{!"struct@_ZTSNSs12_Alloc_hiderE", !5, i64 0}
!5 = !{!"pointer@_ZTSPc", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!"branch_weights", i32 2000, i32 1}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !6, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"vtable pointer", !7, i64 0}
