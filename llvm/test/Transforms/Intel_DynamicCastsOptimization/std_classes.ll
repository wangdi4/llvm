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

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::allocator" = type { i8 }
%"class.std::basic_string" = type { %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" }
%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" = type { ptr }

@_ZTISt9exception = external constant ptr
@_ZTISt13runtime_error = external constant ptr
@.str = private unnamed_addr constant [6 x i8] c"error\00", align 1
@_ZNSs4_Rep20_S_empty_rep_storageE = external global [0 x i64], align 8

; Function Attrs: noinline nounwind readonly uwtable
define internal i32 @test1(ptr readonly %p) #0 {
; CHECK-LABEL: @test1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = icmp eq ptr [[P:%.*]], null
; CHECK-NEXT:    br i1 [[TMP0]], label [[IF_END:%.*]], label [[DYNAMIC_CAST_NOTNULL:%.*]]
; CHECK:       dynamic_cast.notnull:
; CHECK-NEXT:    [[TMP2:%.*]] = tail call ptr @__dynamic_cast(ptr [[P]], ptr @_ZTISt9exception, ptr @_ZTISt13runtime_error, i64 0) #8
; CHECK-NEXT:    [[PHITMP:%.*]] = icmp eq ptr [[TMP2]], null
; CHECK-NEXT:    br i1 [[PHITMP]], label [[IF_END]], label [[CLEANUP:%.*]]
; CHECK:       if.end:
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ 0, [[IF_END]] ], [ 1, [[DYNAMIC_CAST_NOTNULL]] ]
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;
entry:
  %i = icmp eq ptr %p, null
  br i1 %i, label %if.end, label %dynamic_cast.notnull

dynamic_cast.notnull:                             ; preds = %entry
  %i2 = tail call ptr @__dynamic_cast(ptr %p, ptr @_ZTISt9exception, ptr @_ZTISt13runtime_error, i64 0) #8
  %phitmp = icmp eq ptr %i2, null
  br i1 %phitmp, label %if.end, label %cleanup

if.end:                                           ; preds = %dynamic_cast.notnull, %entry
  br label %cleanup

cleanup:                                          ; preds = %if.end, %dynamic_cast.notnull
  %retval.0 = phi i32 [ 0, %if.end ], [ 1, %dynamic_cast.notnull ]
  ret i32 %retval.0
}

; Function Attrs: nounwind readonly
declare ptr @__dynamic_cast(ptr, ptr, ptr, i64) local_unnamed_addr #1

; Function Attrs: norecurse uwtable
define dso_local i32 @main() #2 personality ptr @__gxx_personality_v0 {
entry:
  %ref.tmp.i13 = alloca %"class.std::allocator", align 1
  %ref.tmp.i = alloca %"class.std::allocator", align 1
  %ref.tmp = alloca %"class.std::basic_string", align 8
  %ref.tmp1 = alloca %"class.std::allocator", align 1
  %call = tail call ptr @_Znwm(i64 16) #9
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %ref.tmp) #8
  %i1 = getelementptr inbounds %"class.std::allocator", ptr %ref.tmp1, i64 0, i32 0
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i1) #8
  invoke void @_ZNSsC1EPKcRKSaIcE(ptr nonnull %ref.tmp, ptr @.str, ptr nonnull dereferenceable(1) %ref.tmp1)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  invoke void @_ZNSt11range_errorC1ERKSs(ptr nonnull %call, ptr nonnull dereferenceable(8) %ref.tmp)
          to label %invoke.cont3 unwind label %lpad2

invoke.cont3:                                     ; preds = %invoke.cont
  %_M_p.i.i.i14 = getelementptr inbounds %"class.std::basic_string", ptr %ref.tmp, i64 0, i32 0, i32 0
  %i4 = load ptr, ptr %_M_p.i.i.i14, align 8, !tbaa !2
  %arrayidx.i.i15 = getelementptr inbounds i8, ptr %i4, i64 -24
  %i6 = getelementptr inbounds %"class.std::allocator", ptr %ref.tmp.i13, i64 0, i32 0
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i6) #8
  %cmp.i.i16 = icmp eq ptr %arrayidx.i.i15, @_ZNSs4_Rep20_S_empty_rep_storageE
  br i1 %cmp.i.i16, label %_ZNSsD2Ev.exit26, label %if.then.i.i18, !prof !8

if.then.i.i18:                                    ; preds = %invoke.cont3
  %_M_refcount.i.i17 = getelementptr inbounds i8, ptr %i4, i64 -8
  br i1 icmp ne (ptr @__pthread_key_create, ptr null), label %if.then.i.i.i19, label %if.else.i.i.i21

if.then.i.i.i19:                                  ; preds = %if.then.i.i18
  %i8 = atomicrmw volatile add ptr %_M_refcount.i.i17, i32 -1 acq_rel, align 4
  br label %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i24

if.else.i.i.i21:                                  ; preds = %if.then.i.i18
  %i9 = load i32, ptr %_M_refcount.i.i17, align 4, !tbaa !9
  %add.i.i.i.i20 = add nsw i32 %i9, -1
  store i32 %add.i.i.i.i20, ptr %_M_refcount.i.i17, align 4, !tbaa !9
  br label %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i24

_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i24: ; preds = %if.else.i.i.i21, %if.then.i.i.i19
  %retval.0.i.i.i22 = phi i32 [ %i8, %if.then.i.i.i19 ], [ %i9, %if.else.i.i.i21 ]
  %cmp3.i.i23 = icmp slt i32 %retval.0.i.i.i22, 1
  br i1 %cmp3.i.i23, label %if.then4.i.i25, label %_ZNSsD2Ev.exit26

if.then4.i.i25:                                   ; preds = %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i24
  call void @_ZNSs4_Rep10_M_destroyERKSaIcE(ptr nonnull %arrayidx.i.i15, ptr nonnull dereferenceable(1) %ref.tmp.i13) #8
  br label %_ZNSsD2Ev.exit26

_ZNSsD2Ev.exit26:                                 ; preds = %if.then4.i.i25, %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i24, %invoke.cont3
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i6) #8
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i1) #8
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %ref.tmp) #8
  %vtable = load ptr, ptr %call, align 8, !tbaa !11
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 1
  %i11 = load ptr, ptr %vfn, align 8
  call void %i11(ptr nonnull %call) #8
  ret i32 0

lpad:                                             ; preds = %entry
  %i12 = landingpad { ptr, i32 }
          cleanup
  %i13 = extractvalue { ptr, i32 } %i12, 0
  %i14 = extractvalue { ptr, i32 } %i12, 1
  br label %ehcleanup

lpad2:                                            ; preds = %invoke.cont
  %i15 = landingpad { ptr, i32 }
          cleanup
  %i16 = extractvalue { ptr, i32 } %i15, 0
  %i17 = extractvalue { ptr, i32 } %i15, 1
  %_M_p.i.i.i = getelementptr inbounds %"class.std::basic_string", ptr %ref.tmp, i64 0, i32 0, i32 0
  %i18 = load ptr, ptr %_M_p.i.i.i, align 8, !tbaa !2
  %arrayidx.i.i = getelementptr inbounds i8, ptr %i18, i64 -24
  %i20 = getelementptr inbounds %"class.std::allocator", ptr %ref.tmp.i, i64 0, i32 0
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i20) #8
  %cmp.i.i = icmp eq ptr %arrayidx.i.i, @_ZNSs4_Rep20_S_empty_rep_storageE
  br i1 %cmp.i.i, label %_ZNSsD2Ev.exit, label %if.then.i.i, !prof !8

if.then.i.i:                                      ; preds = %lpad2
  %_M_refcount.i.i = getelementptr inbounds i8, ptr %i18, i64 -8
  br i1 icmp ne (ptr @__pthread_key_create, ptr null), label %if.then.i.i.i, label %if.else.i.i.i

if.then.i.i.i:                                    ; preds = %if.then.i.i
  %i22 = atomicrmw volatile add ptr %_M_refcount.i.i, i32 -1 acq_rel, align 4
  br label %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i

if.else.i.i.i:                                    ; preds = %if.then.i.i
  %i23 = load i32, ptr %_M_refcount.i.i, align 4, !tbaa !9
  %add.i.i.i.i = add nsw i32 %i23, -1
  store i32 %add.i.i.i.i, ptr %_M_refcount.i.i, align 4, !tbaa !9
  br label %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i

_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i: ; preds = %if.else.i.i.i, %if.then.i.i.i
  %retval.0.i.i.i = phi i32 [ %i22, %if.then.i.i.i ], [ %i23, %if.else.i.i.i ]
  %cmp3.i.i = icmp slt i32 %retval.0.i.i.i, 1
  br i1 %cmp3.i.i, label %if.then4.i.i, label %_ZNSsD2Ev.exit

if.then4.i.i:                                     ; preds = %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i
  call void @_ZNSs4_Rep10_M_destroyERKSaIcE(ptr nonnull %arrayidx.i.i, ptr nonnull dereferenceable(1) %ref.tmp.i) #8
  br label %_ZNSsD2Ev.exit

_ZNSsD2Ev.exit:                                   ; preds = %if.then4.i.i, %_ZN9__gnu_cxx27__exchange_and_add_dispatchEPii.exit.i.i, %lpad2
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i20) #8
  br label %ehcleanup

ehcleanup:                                        ; preds = %_ZNSsD2Ev.exit, %lpad
  %exn.slot.0 = phi ptr [ %i16, %_ZNSsD2Ev.exit ], [ %i13, %lpad ]
  %ehselector.slot.0 = phi i32 [ %i17, %_ZNSsD2Ev.exit ], [ %i14, %lpad ]
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i1) #8
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %ref.tmp) #8
  call void @_ZdlPv(ptr nonnull %call) #10
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn.slot.0, 0
  %lpad.val9 = insertvalue { ptr, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { ptr, i32 } %lpad.val9
}

declare i32 @__gxx_personality_v0(...)

; Function Attrs: nobuiltin
declare noalias nonnull ptr @_Znwm(i64) local_unnamed_addr #3

declare void @_ZNSsC1EPKcRKSaIcE(ptr, ptr, ptr dereferenceable(1)) unnamed_addr #4

declare void @_ZNSt11range_errorC1ERKSs(ptr, ptr dereferenceable(8)) unnamed_addr #4

; Function Attrs: nounwind
declare extern_weak i32 @__pthread_key_create(ptr, ptr) #5

; Function Attrs: nounwind
declare void @_ZNSs4_Rep10_M_destroyERKSaIcE(ptr, ptr dereferenceable(1)) local_unnamed_addr #5

; Function Attrs: nobuiltin nounwind
declare void @_ZdlPv(ptr) local_unnamed_addr #6

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #7

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #7

attributes #0 = { noinline nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly }
attributes #2 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { nobuiltin nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { argmemonly nocallback nofree nosync nounwind willreturn }
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
