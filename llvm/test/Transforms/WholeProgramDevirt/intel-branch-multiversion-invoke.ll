; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; This test check that the multiversioning based on virtual function pointers
; was done correctly using an InvokeInst. The test case is pretty simple,
; this is how it looks like in C++:
; class Base {
;  virtual bool foo(int a) = 0;
; };
;
; class Derived : public Base {
;   bool foo(int a) {
;
;     if (a < 0)
;       throw "Invalid Num";
;
;     return true;
;   }
; };
;
; class Derived2 : public Base {
;   bool foo(int a) {
;
;     if (a < 0)
;       throw "Invalid Num";
;
;     return false;
;   }
; };
;
; Base *b;
; Derived d;
; Derived2 d2;
;
; int main(int argc, char* argv[]) {
;
;   if (argc < 2)
;     b = &d;
;   else
;     b = &d2;
;
;   try {
;     bool c = b->foo(argc);
;     std::cout << c <<"\n";
;   }
;
;   catch (cons char* msg) {
;     std::cerr << msg << std::endl;
;     exit(1);
;   }
;
;   return 0;
; }
; The devirtualization process should generate a multiversioning similar to
; the following (in pseudo-code):
; if (address of b->foo == address of Derived::foo)
;   call Derived::foo
; else if (address of b->foo == address of Derived2::foo)
;   call Derived2::foo
; else
;   call b->foo
;
; RUN: opt < %s -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=wholeprogramdevirt -whole-program-visibility -wholeprogramdevirt-multiversion -wholeprogramdevirt-multiversion-verify -S 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%class.Derived = type { %class.Base }
%class.Base = type { ptr }
%class.Derived2 = type { %class.Base }
%"class.std::basic_ostream" = type { ptr, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", ptr, i8, i8, ptr, ptr, ptr, ptr }
%"class.std::ios_base" = type { ptr, i64, i64, i32, i32, i32, ptr, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, ptr, %"class.std::locale" }
%"struct.std::ios_base::_Words" = type { ptr, i64 }
%"class.std::locale" = type { ptr }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], ptr, i8, [7 x i8], ptr, ptr, ptr, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ ptr, i32 }>

$_ZN7Derived3fooEi = comdat any

$_ZN8Derived23fooEi = comdat any

$_ZTV7Derived = comdat any

$_ZTS7Derived = comdat any

$_ZTS4Base = comdat any

$_ZTI4Base = comdat any

$_ZTI7Derived = comdat any

$_ZTV8Derived2 = comdat any

$_ZTS8Derived2 = comdat any

$_ZTI8Derived2 = comdat any

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@b = hidden local_unnamed_addr global ptr null, align 8
@d = hidden global %class.Derived zeroinitializer, align 8
@d2 = hidden global %class.Derived2 zeroinitializer, align 8
@_ZTIPKc = external dso_local constant ptr
@_ZSt4cout = external dso_local global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@_ZSt4cerr = external dso_local global %"class.std::basic_ostream", align 8
@_ZTV7Derived = linkonce_odr hidden unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @_ZTI7Derived, ptr @_ZN7Derived3fooEi] }, comdat, align 8, !type !0, !type !1, !type !2, !type !3
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global ptr
@_ZTS7Derived = linkonce_odr hidden constant [9 x i8] c"7Derived\00", comdat
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTS4Base = linkonce_odr hidden constant [6 x i8] c"4Base\00", comdat
@_ZTI4Base = linkonce_odr hidden constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS4Base }, comdat
@_ZTI7Derived = linkonce_odr hidden constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS7Derived, ptr @_ZTI4Base }, comdat
@.str.3 = private unnamed_addr constant [13 x i8] c"INVALID NUM\0A\00", align 1
@_ZTV8Derived2 = linkonce_odr hidden unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @_ZTI8Derived2, ptr @_ZN8Derived23fooEi] }, comdat, align 8, !type !0, !type !1, !type !4, !type !5
@_ZTS8Derived2 = linkonce_odr hidden constant [10 x i8] c"8Derived2\00", comdat
@_ZTI8Derived2 = linkonce_odr hidden constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS8Derived2, ptr @_ZTI4Base }, comdat
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_driver2.cpp, ptr null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(ptr) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(ptr) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr) local_unnamed_addr #2

; Function Attrs: norecurse uwtable
define hidden i32 @main(i32 %argc, ptr nocapture readnone %argv) local_unnamed_addr #3 personality ptr @__gxx_personality_v0 {
entry:
  %cmp = icmp slt i32 %argc, 2
  %. = select i1 %cmp, ptr @d, ptr @d2
  %.16 = select i1 %cmp, ptr @d, ptr @d2
  store ptr %., ptr @b, align 8, !tbaa !8
  %vtable = load ptr, ptr %., align 8, !tbaa !12
  %tmp = bitcast ptr %vtable to ptr
  %tmp1 = tail call i1 @llvm.type.test(ptr %tmp, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %tmp1)
  %tmp2 = load ptr, ptr %vtable, align 8
  %call = invoke zeroext i1 %tmp2(ptr %.16, i32 %argc)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %call.i17 = invoke dereferenceable(272) ptr @_ZNSo9_M_insertIbEERSoT_(ptr nonnull @_ZSt4cout, i1 zeroext %call)
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:                                     ; preds = %invoke.cont
  %call1.i18 = invoke dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull dereferenceable(272) %call.i17, ptr nonnull @.str, i64 1)
          to label %invoke.cont3 unwind label %lpad

invoke.cont3:                                     ; preds = %invoke.cont1
  ret i32 0

lpad:                                             ; preds = %invoke.cont1, %invoke.cont, %entry
  %tmp3 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIPKc
  %tmp4 = extractvalue { ptr, i32 } %tmp3, 0
  %tmp5 = extractvalue { ptr, i32 } %tmp3, 1
  %tmp6 = tail call i32 @llvm.eh.typeid.for(ptr @_ZTIPKc) #2
  %matches = icmp eq i32 %tmp5, %tmp6
  br i1 %matches, label %catch, label %eh.resume

catch:                                            ; preds = %lpad
  %tmp7 = tail call ptr @__cxa_begin_catch(ptr %tmp4) #2
  %tobool.i = icmp eq ptr %tmp7, null
  br i1 %tobool.i, label %if.then.i, label %if.else.i

if.then.i:                                        ; preds = %catch
  %vtable.i = load ptr, ptr @_ZSt4cerr, align 8, !tbaa !12
  %vbase.offset.ptr.i = getelementptr i8, ptr %vtable.i, i64 -24
  %tmp8 = bitcast ptr %vbase.offset.ptr.i to ptr
  %vbase.offset.i = load i64, ptr %tmp8, align 8
  %add.ptr.i = getelementptr inbounds i8, ptr @_ZSt4cerr, i64 %vbase.offset.i
  %tmp9 = bitcast ptr %add.ptr.i to ptr
  %_M_streambuf_state.i.i.i = getelementptr inbounds i8, ptr %add.ptr.i, i64 32
  %tmp10 = bitcast ptr %_M_streambuf_state.i.i.i to ptr
  %tmp11 = load i32, ptr %tmp10, align 8, !tbaa !14
  %or.i.i.i = or i32 %tmp11, 1
  invoke void @_ZNSt9basic_iosIcSt11char_traitsIcEE5clearESt12_Ios_Iostate(ptr %tmp9, i32 %or.i.i.i)
          to label %invoke.cont6 unwind label %lpad5

if.else.i:                                        ; preds = %catch
  %call.i.i19 = tail call i64 @strlen(ptr nonnull %tmp7) #2
  %call1.i20 = invoke dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull dereferenceable(272) @_ZSt4cerr, ptr nonnull %tmp7, i64 %call.i.i19)
          to label %invoke.cont6 unwind label %lpad5

invoke.cont6:                                     ; preds = %if.else.i, %if.then.i
  %vtable.i23 = load ptr, ptr @_ZSt4cerr, align 8, !tbaa !12
  %vbase.offset.ptr.i24 = getelementptr i8, ptr %vtable.i23, i64 -24
  %tmp12 = bitcast ptr %vbase.offset.ptr.i24 to ptr
  %vbase.offset.i25 = load i64, ptr %tmp12, align 8
  %add.ptr.i26 = getelementptr inbounds i8, ptr @_ZSt4cerr, i64 %vbase.offset.i25
  %_M_ctype.i = getelementptr inbounds i8, ptr %add.ptr.i26, i64 240
  %tmp13 = bitcast ptr %_M_ctype.i to ptr
  %tmp14 = load ptr, ptr %tmp13, align 8, !tbaa !23
  %tobool.i42 = icmp eq ptr %tmp14, null
  br i1 %tobool.i42, label %if.then.i43, label %call.i.noexc33

if.then.i43:                                      ; preds = %invoke.cont6
  invoke void @_ZSt16__throw_bad_castv() #9
          to label %.noexc45 unwind label %lpad5

.noexc45:                                         ; preds = %if.then.i43
  unreachable

call.i.noexc33:                                   ; preds = %invoke.cont6
  %_M_widen_ok.i = getelementptr inbounds %"class.std::ctype", ptr %tmp14, i64 0, i32 8
  %tmp15 = load i8, ptr %_M_widen_ok.i, align 8, !tbaa !26
  %tobool.i36 = icmp eq i8 %tmp15, 0
  br i1 %tobool.i36, label %if.end.i, label %if.then.i37

if.then.i37:                                      ; preds = %call.i.noexc33
  %arrayidx.i = getelementptr inbounds %"class.std::ctype", ptr %tmp14, i64 0, i32 9, i64 10
  %tmp16 = load i8, ptr %arrayidx.i, align 1, !tbaa !31
  br label %call.i.noexc

if.end.i:                                         ; preds = %call.i.noexc33
  invoke void @_ZNKSt5ctypeIcE13_M_widen_initEv(ptr nonnull %tmp14)
          to label %.noexc39 unwind label %lpad5

.noexc39:                                         ; preds = %if.end.i
  %tmp17 = bitcast ptr %tmp14 to ptr
  %vtable.i38 = load ptr, ptr %tmp17, align 8, !tbaa !12
  %vfn.i = getelementptr inbounds ptr, ptr %vtable.i38, i64 6
  %tmp18 = load ptr, ptr %vfn.i, align 8
  %call.i41 = invoke signext i8 %tmp18(ptr nonnull %tmp14, i8 signext 10)
          to label %call.i.noexc unwind label %lpad5

call.i.noexc:                                     ; preds = %.noexc39, %if.then.i37
  %retval.0.i = phi i8 [ %tmp16, %if.then.i37 ], [ %call.i41, %.noexc39 ]
  %call1.i29 = invoke dereferenceable(272) ptr @_ZNSo3putEc(ptr nonnull @_ZSt4cerr, i8 signext %retval.0.i)
          to label %call1.i.noexc28 unwind label %lpad5

call1.i.noexc28:                                  ; preds = %call.i.noexc
  %call.i32 = invoke dereferenceable(272) ptr @_ZNSo5flushEv(ptr nonnull %call1.i29)
          to label %invoke.cont8 unwind label %lpad5

invoke.cont8:                                     ; preds = %call1.i.noexc28
  tail call void @exit(i32 1) #13
  unreachable

lpad5:                                            ; preds = %call1.i.noexc28, %call.i.noexc, %.noexc39, %if.end.i, %if.then.i43, %if.else.i, %if.then.i
  %tmp19 = landingpad { ptr, i32 }
          cleanup
  %tmp20 = extractvalue { ptr, i32 } %tmp19, 0
  %tmp21 = extractvalue { ptr, i32 } %tmp19, 1
  tail call void @__cxa_end_catch() #2
  br label %eh.resume

eh.resume:                                        ; preds = %lpad5, %lpad
  %ehselector.slot.0 = phi i32 [ %tmp21, %lpad5 ], [ %tmp5, %lpad ]
  %exn.slot.0 = phi ptr [ %tmp20, %lpad5 ], [ %tmp4, %lpad ]
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn.slot.0, 0
  %lpad.val12 = insertvalue { ptr, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { ptr, i32 } %lpad.val12
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i1 @llvm.type.test(ptr, metadata) #4

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.assume(i1 noundef) #5

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind memory(none)
declare i32 @llvm.eh.typeid.for(ptr) #6

declare dso_local ptr @__cxa_begin_catch(ptr) local_unnamed_addr

; Function Attrs: noreturn nounwind
declare dso_local void @exit(i32) local_unnamed_addr #7

declare dso_local void @__cxa_end_catch() local_unnamed_addr

; Function Attrs: noinline uwtable
define linkonce_odr hidden zeroext i1 @_ZN7Derived3fooEi(ptr %this, i32 %a) unnamed_addr #8 comdat align 2 {
entry:
  %cmp = icmp slt i32 %a, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %exception = tail call ptr @__cxa_allocate_exception(i64 8) #2
  %tmp = bitcast ptr %exception to ptr
  store ptr @.str.3, ptr %tmp, align 16, !tbaa !32
  tail call void @__cxa_throw(ptr nonnull %exception, ptr @_ZTIPKc, ptr null) #9
  unreachable

if.end:                                           ; preds = %entry
  ret i1 true
}

declare dso_local noalias nonnull ptr @__cxa_allocate_exception(i64) local_unnamed_addr

; Function Attrs: noreturn
declare dso_local void @__cxa_throw(ptr, ptr, ptr) local_unnamed_addr #9

; Function Attrs: noinline uwtable
define linkonce_odr hidden zeroext i1 @_ZN8Derived23fooEi(ptr %this, i32 %a) unnamed_addr #8 comdat align 2 {
entry:
  %cmp = icmp slt i32 %a, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %exception = tail call ptr @__cxa_allocate_exception(i64 8) #2
  %tmp = bitcast ptr %exception to ptr
  store ptr @.str.3, ptr %tmp, align 16, !tbaa !32
  tail call void @__cxa_throw(ptr nonnull %exception, ptr @_ZTIPKc, ptr null) #9
  unreachable

if.end:                                           ; preds = %entry
  ret i1 false
}

declare dso_local dereferenceable(272) ptr @_ZNSo9_M_insertIbEERSoT_(ptr, i1 zeroext) local_unnamed_addr #0

declare dso_local dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr dereferenceable(272), ptr, i64) local_unnamed_addr #0

declare dso_local void @_ZNSt9basic_iosIcSt11char_traitsIcEE5clearESt12_Ios_Iostate(ptr, i32) local_unnamed_addr #0

; Function Attrs: nounwind memory(argmem: read)
declare dso_local i64 @strlen(ptr nocapture) local_unnamed_addr #10

declare dso_local dereferenceable(272) ptr @_ZNSo3putEc(ptr, i8 signext) local_unnamed_addr #0

declare dso_local dereferenceable(272) ptr @_ZNSo5flushEv(ptr) local_unnamed_addr #0

; Function Attrs: noreturn
declare dso_local void @_ZSt16__throw_bad_castv() local_unnamed_addr #11

declare dso_local void @_ZNKSt5ctypeIcE13_M_widen_initEv(ptr) local_unnamed_addr #0

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_driver2.cpp() #12 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(ptr nonnull @_ZSt8__ioinit)
  %tmp = tail call i32 @__cxa_atexit(ptr @_ZNSt8ios_base4InitD1Ev, ptr @_ZSt8__ioinit, ptr nonnull @__dso_handle) #2
  store ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV7Derived, i64 0, inrange i32 0, i64 2), ptr @d, align 8, !tbaa !12
  store ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV8Derived2, i64 0, inrange i32 0, i64 2), ptr @d2, align 8, !tbaa !12
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #5 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #6 = { nounwind memory(none) }
attributes #7 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #9 = { noreturn }
attributes #10 = { nounwind memory(argmem: read) "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #11 = { noreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #12 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #13 = { noreturn nounwind }

!llvm.module.flags = !{!6}
!llvm.ident = !{!7}

!0 = !{i64 16, !"_ZTS4Base"}
!1 = !{i64 16, !"_ZTSM4BaseFbiE.virtual"}
!2 = !{i64 16, !"_ZTS7Derived"}
!3 = !{i64 16, !"_ZTSM7DerivedFbiE.virtual"}
!4 = !{i64 16, !"_ZTS8Derived2"}
!5 = !{i64 16, !"_ZTSM8Derived2FbiE.virtual"}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{!"clang version 8.0.0"}
!8 = !{!9, !9, i64 0}
!9 = !{!"unspecified pointer", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"vtable pointer", !11, i64 0}
!14 = !{!15, !18, i64 32}
!15 = !{!"struct@_ZTSSt8ios_base", !16, i64 8, !16, i64 16, !17, i64 24, !18, i64 28, !18, i64 32, !9, i64 40, !19, i64 48, !10, i64 64, !21, i64 192, !9, i64 200, !22, i64 208}
!16 = !{!"long", !10, i64 0}
!17 = !{!"_ZTSSt13_Ios_Fmtflags", !10, i64 0}
!18 = !{!"_ZTSSt12_Ios_Iostate", !10, i64 0}
!19 = !{!"struct@_ZTSNSt8ios_base6_WordsE", !20, i64 0, !16, i64 8}
!20 = !{!"pointer@_ZTSPv", !10, i64 0}
!21 = !{!"int", !10, i64 0}
!22 = !{!"struct@_ZTSSt6locale", !9, i64 0}
!23 = !{!24, !9, i64 240}
!24 = !{!"struct@_ZTSSt9basic_iosIcSt11char_traitsIcEE", !9, i64 216, !10, i64 224, !25, i64 225, !9, i64 232, !9, i64 240, !9, i64 248, !9, i64 256}
!25 = !{!"bool", !10, i64 0}
!26 = !{!27, !10, i64 56}
!27 = !{!"struct@_ZTSSt5ctypeIcE", !9, i64 16, !25, i64 24, !28, i64 32, !28, i64 40, !29, i64 48, !10, i64 56, !30, i64 57, !30, i64 313, !10, i64 569}
!28 = !{!"pointer@_ZTSPKi", !10, i64 0}
!29 = !{!"pointer@_ZTSPKt", !10, i64 0}
!30 = !{!"array@_ZTSA256_c", !10, i64 0}
!31 = !{!27, !10, i64 57}
!32 = !{!33, !33, i64 0}
!33 = !{!"pointer@_ZTSPKc", !10, i64 0}

; Check that the transformation was applied correctly in function main
; CHECK:       efine hidden i32 @main(i32 %argc, ptr nocapture readnone %argv) local_unnamed_addr #3 personality ptr @__gxx_personality_v0 {

; Compare the address of the virtual function with Derived::foo
; CHECK:       %0 = bitcast ptr %tmp2 to ptr
; CHECK-NEXT:  %1 = bitcast ptr @_ZN7Derived3fooEi to ptr
; CHECK-NEXT:  %2 = icmp eq ptr %0, %1
; CHECK-NEXT:  br i1 %2, label %BBDevirt__ZN7Derived3fooEi, label %ElseDevirt__ZN7Derived3fooEi

; If the address matches with Derived:foo, then call it
; CHECK-LABEL: BBDevirt__ZN7Derived3fooEi:
; CHECK:        %3 = invoke zeroext i1 @_ZN7Derived3fooEi(ptr %.16, i32 %argc)
; CHECK-NEXT:          to label %MergeBB unwind label %lpad

; Else, compare the address with Derived2:foo
; CHECK-LABEL: ElseDevirt__ZN7Derived3fooEi:
; CHECK:        %4 = bitcast ptr @_ZN8Derived23fooEi to ptr
; CHECK-NEXT:   %5 = icmp eq ptr %0, %4
; CHECK-NEXT:   br i1 %5, label %BBDevirt__ZN8Derived23fooEi, label %DefaultBB

; The address matches Derived2::foo
; CHECK-LABEL: BBDevirt__ZN8Derived23fooEi:
; CHECK:        %6 = invoke zeroext i1 @_ZN8Derived23fooEi(ptr %.16, i32 %argc)
; CHECK-NEXT:          to label %MergeBB unwind label %lpad

; Default case
; CHECK-LABEL: DefaultBB:
; CHECK:        %7 = invoke zeroext i1 %tmp2(ptr %.16, i32 %argc)
; CHECK-NEXT:          to label %MergeBB unwind label %lpad

; Check that the PhiNode was generated correctly
; CHECK-LABEL: MergeBB:
; CHECK:        %8 = phi i1 [ %3, %BBDevirt__ZN7Derived3fooEi ], [ %6, %BBDevirt__ZN8Derived23fooEi ], [ %7, %DefaultBB ]
; CHECK-NEXT: br label %invoke.cont


; Make sure that the users were replaced correctly
; CHECK-LABEL: invoke.cont:
; CHECK:        %call.i17 = invoke dereferenceable(272) ptr @_ZNSo9_M_insertIbEERSoT_(ptr nonnull @_ZSt4cout, i1 zeroext %8)
; CHECK-NEXT:          to label %invoke.cont1 unwind label %lpad


; Check that the unwind destination have all the basic blocks connected correctly
; CHECK:       lpad:                                             ; preds = %BBDevirt__ZN8Derived23fooEi, %BBDevirt__ZN7Derived3fooEi, %invoke.cont1, %invoke.cont, %DefaultBB
; CHECK-NEXT:   %tmp3 = landingpad { ptr, i32 }
; CHECK-NEXT:            cleanup
; CHECK-NEXT:            catch ptr @_ZTIPKc
; CHECK-NEXT:   %tmp4 = extractvalue { ptr, i32 } %tmp3, 0
; CHECK-NEXT:   %tmp5 = extractvalue { ptr, i32 } %tmp3, 1
; CHECK-NEXT:   %tmp6 = tail call i32 @llvm.eh.typeid.for(ptr @_ZTIPKc)
; CHECK-NEXT:   %matches = icmp eq i32 %tmp5, %tmp6
; CHECK-NEXT:   br i1 %matches, label %catch, label %eh.resume

; Check that the metadata was added correctly
; CHECK: define linkonce_odr hidden zeroext i1 @_ZN7Derived3fooEi(ptr %this, i32 %a) unnamed_addr #{{.*}} comdat align 2 !_Intel.Devirt.Target
; CHECK: define linkonce_odr hidden zeroext i1 @_ZN8Derived23fooEi(ptr %this, i32 %a) unnamed_addr #{{.*}} comdat align 2 !_Intel.Devirt.Target

; end INTEL_FEATURE_SW_DTRANS
