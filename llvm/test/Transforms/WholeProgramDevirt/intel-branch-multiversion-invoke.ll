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

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -wholeprogramdevirt -wholeprogramdevirt-multiversion -wholeprogramdevirt-multiversion-verify -S 2>&1 | FileCheck %s

%"class.std::ios_base::Init" = type { i8 }
%class.Base = type { i32 (...)** }
%class.Derived = type { %class.Base }
%class.Derived2 = type { %class.Base }
%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream"*, i8, i8, %"class.std::basic_streambuf"*, %"class.std::ctype"*, %"class.std::num_put"*, %"class.std::num_get"* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct.std::ios_base::_Callback_list"*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words"*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list"*, void (i32, %"class.std::ios_base"*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8*, i64 }
%"class.std::locale" = type { %"class.std::locale::_Impl"* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet"**, i64, %"class.std::locale::facet"**, i8** }
%"class.std::locale::facet" = type <{ i32 (...)**, i32, [4 x i8] }>
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], %struct.__locale_struct*, i8, [7 x i8], i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ i32 (...)**, i32 }>
%struct.__locale_struct = type { [13 x %struct.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::num_get" = type { %"class.std::locale::facet.base", [4 x i8] }

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
@b = hidden local_unnamed_addr global %class.Base* null, align 8
@d = hidden global %class.Derived zeroinitializer, align 8
@d2 = hidden global %class.Derived2 zeroinitializer, align 8
@_ZTIPKc = external dso_local constant i8*
@_ZSt4cout = external dso_local global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@_ZSt4cerr = external dso_local global %"class.std::basic_ostream", align 8
@_ZTV7Derived = linkonce_odr hidden unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI7Derived to i8*), i8* bitcast (i1 (%class.Derived*, i32)* @_ZN7Derived3fooEi to i8*)] }, comdat, align 8, !type !0, !type !1, !type !2, !type !3
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTS7Derived = linkonce_odr hidden constant [9 x i8] c"7Derived\00", comdat
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS4Base = linkonce_odr hidden constant [6 x i8] c"4Base\00", comdat
@_ZTI4Base = linkonce_odr hidden constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @_ZTS4Base, i32 0, i32 0) }, comdat
@_ZTI7Derived = linkonce_odr hidden constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @_ZTS7Derived, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, comdat
@.str.3 = private unnamed_addr constant [13 x i8] c"INVALID NUM\0A\00", align 1
@_ZTV8Derived2 = linkonce_odr hidden unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI8Derived2 to i8*), i8* bitcast (i1 (%class.Derived2*, i32)* @_ZN8Derived23fooEi to i8*)] }, comdat, align 8, !type !0, !type !1, !type !4, !type !5
@_ZTS8Derived2 = linkonce_odr hidden constant [10 x i8] c"8Derived2\00", comdat
@_ZTI8Derived2 = linkonce_odr hidden constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @_ZTS8Derived2, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, comdat
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_driver2.cpp, i8* null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #2

; Function Attrs: norecurse uwtable
define hidden i32 @main(i32 %argc, i8** nocapture readnone %argv) local_unnamed_addr #3 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %cmp = icmp slt i32 %argc, 2
  %. = select i1 %cmp, i1 (%class.Base*, i32)*** bitcast (%class.Derived* @d to i1 (%class.Base*, i32)***), i1 (%class.Base*, i32)*** bitcast (%class.Derived2* @d2 to i1 (%class.Base*, i32)***)
  %.16 = select i1 %cmp, %class.Base* getelementptr inbounds (%class.Derived, %class.Derived* @d, i64 0, i32 0), %class.Base* getelementptr inbounds (%class.Derived2, %class.Derived2* @d2, i64 0, i32 0)
  store i1 (%class.Base*, i32)*** %., i1 (%class.Base*, i32)**** bitcast (%class.Base** @b to i1 (%class.Base*, i32)****), align 8, !tbaa !8
  %vtable = load i1 (%class.Base*, i32)**, i1 (%class.Base*, i32)*** %., align 8, !tbaa !12
  %tmp = bitcast i1 (%class.Base*, i32)** %vtable to i8*
  %tmp1 = tail call i1 @llvm.type.test(i8* %tmp, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %tmp1)
  %tmp2 = load i1 (%class.Base*, i32)*, i1 (%class.Base*, i32)** %vtable, align 8
  %call = invoke zeroext i1 %tmp2(%class.Base* %.16, i32 %argc)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %call.i17 = invoke dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertIbEERSoT_(%"class.std::basic_ostream"* nonnull @_ZSt4cout, i1 zeroext %call)
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:                                     ; preds = %invoke.cont
  %call1.i18 = invoke dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %call.i17, i8* nonnull getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i64 0, i64 0), i64 1)
          to label %invoke.cont3 unwind label %lpad

invoke.cont3:                                     ; preds = %invoke.cont1
  ret i32 0

lpad:                                             ; preds = %invoke.cont1, %invoke.cont, %entry
  %tmp3 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast (i8** @_ZTIPKc to i8*)
  %tmp4 = extractvalue { i8*, i32 } %tmp3, 0
  %tmp5 = extractvalue { i8*, i32 } %tmp3, 1
  %tmp6 = tail call i32 @llvm.eh.typeid.for(i8* bitcast (i8** @_ZTIPKc to i8*)) #2
  %matches = icmp eq i32 %tmp5, %tmp6
  br i1 %matches, label %catch, label %eh.resume

catch:                                            ; preds = %lpad
  %tmp7 = tail call i8* @__cxa_begin_catch(i8* %tmp4) #2
  %tobool.i = icmp eq i8* %tmp7, null
  br i1 %tobool.i, label %if.then.i, label %if.else.i

if.then.i:                                        ; preds = %catch
  %vtable.i = load i8*, i8** bitcast (%"class.std::basic_ostream"* @_ZSt4cerr to i8**), align 8, !tbaa !12
  %vbase.offset.ptr.i = getelementptr i8, i8* %vtable.i, i64 -24
  %tmp8 = bitcast i8* %vbase.offset.ptr.i to i64*
  %vbase.offset.i = load i64, i64* %tmp8, align 8
  %add.ptr.i = getelementptr inbounds i8, i8* bitcast (%"class.std::basic_ostream"* @_ZSt4cerr to i8*), i64 %vbase.offset.i
  %tmp9 = bitcast i8* %add.ptr.i to %"class.std::basic_ios"*
  %_M_streambuf_state.i.i.i = getelementptr inbounds i8, i8* %add.ptr.i, i64 32
  %tmp10 = bitcast i8* %_M_streambuf_state.i.i.i to i32*
  %tmp11 = load i32, i32* %tmp10, align 8, !tbaa !14
  %or.i.i.i = or i32 %tmp11, 1
  invoke void @_ZNSt9basic_iosIcSt11char_traitsIcEE5clearESt12_Ios_Iostate(%"class.std::basic_ios"* %tmp9, i32 %or.i.i.i)
          to label %invoke.cont6 unwind label %lpad5

if.else.i:                                        ; preds = %catch
  %call.i.i19 = tail call i64 @strlen(i8* nonnull %tmp7) #2
  %call1.i20 = invoke dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) @_ZSt4cerr, i8* nonnull %tmp7, i64 %call.i.i19)
          to label %invoke.cont6 unwind label %lpad5

invoke.cont6:                                     ; preds = %if.else.i, %if.then.i
  %vtable.i23 = load i8*, i8** bitcast (%"class.std::basic_ostream"* @_ZSt4cerr to i8**), align 8, !tbaa !12
  %vbase.offset.ptr.i24 = getelementptr i8, i8* %vtable.i23, i64 -24
  %tmp12 = bitcast i8* %vbase.offset.ptr.i24 to i64*
  %vbase.offset.i25 = load i64, i64* %tmp12, align 8
  %add.ptr.i26 = getelementptr inbounds i8, i8* bitcast (%"class.std::basic_ostream"* @_ZSt4cerr to i8*), i64 %vbase.offset.i25
  %_M_ctype.i = getelementptr inbounds i8, i8* %add.ptr.i26, i64 240
  %tmp13 = bitcast i8* %_M_ctype.i to %"class.std::ctype"**
  %tmp14 = load %"class.std::ctype"*, %"class.std::ctype"** %tmp13, align 8, !tbaa !23
  %tobool.i42 = icmp eq %"class.std::ctype"* %tmp14, null
  br i1 %tobool.i42, label %if.then.i43, label %call.i.noexc33

if.then.i43:                                      ; preds = %invoke.cont6
  invoke void @_ZSt16__throw_bad_castv() #7
          to label %.noexc45 unwind label %lpad5

.noexc45:                                         ; preds = %if.then.i43
  unreachable

call.i.noexc33:                                   ; preds = %invoke.cont6
  %_M_widen_ok.i = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %tmp14, i64 0, i32 8
  %tmp15 = load i8, i8* %_M_widen_ok.i, align 8, !tbaa !26
  %tobool.i36 = icmp eq i8 %tmp15, 0
  br i1 %tobool.i36, label %if.end.i, label %if.then.i37

if.then.i37:                                      ; preds = %call.i.noexc33
  %arrayidx.i = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %tmp14, i64 0, i32 9, i64 10
  %tmp16 = load i8, i8* %arrayidx.i, align 1, !tbaa !31
  br label %call.i.noexc

if.end.i:                                         ; preds = %call.i.noexc33
  invoke void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"* nonnull %tmp14)
          to label %.noexc39 unwind label %lpad5

.noexc39:                                         ; preds = %if.end.i
  %tmp17 = bitcast %"class.std::ctype"* %tmp14 to i8 (%"class.std::ctype"*, i8)***
  %vtable.i38 = load i8 (%"class.std::ctype"*, i8)**, i8 (%"class.std::ctype"*, i8)*** %tmp17, align 8, !tbaa !12
  %vfn.i = getelementptr inbounds i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %vtable.i38, i64 6
  %tmp18 = load i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %vfn.i, align 8
  %call.i41 = invoke signext i8 %tmp18(%"class.std::ctype"* nonnull %tmp14, i8 signext 10)
          to label %call.i.noexc unwind label %lpad5

call.i.noexc:                                     ; preds = %.noexc39, %if.then.i37
  %retval.0.i = phi i8 [ %tmp16, %if.then.i37 ], [ %call.i41, %.noexc39 ]
  %call1.i29 = invoke dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"* nonnull @_ZSt4cerr, i8 signext %retval.0.i)
          to label %call1.i.noexc28 unwind label %lpad5

call1.i.noexc28:                                  ; preds = %call.i.noexc
  %call.i32 = invoke dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"* nonnull %call1.i29)
          to label %invoke.cont8 unwind label %lpad5

invoke.cont8:                                     ; preds = %call1.i.noexc28
  tail call void @exit(i32 1) #11
  unreachable

lpad5:                                            ; preds = %call1.i.noexc28, %call.i.noexc, %.noexc39, %if.end.i, %if.then.i43, %if.else.i, %if.then.i
  %tmp19 = landingpad { i8*, i32 }
          cleanup
  %tmp20 = extractvalue { i8*, i32 } %tmp19, 0
  %tmp21 = extractvalue { i8*, i32 } %tmp19, 1
  tail call void @__cxa_end_catch() #2
  br label %eh.resume

eh.resume:                                        ; preds = %lpad5, %lpad
  %ehselector.slot.0 = phi i32 [ %tmp21, %lpad5 ], [ %tmp5, %lpad ]
  %exn.slot.0 = phi i8* [ %tmp20, %lpad5 ], [ %tmp4, %lpad ]
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0
  %lpad.val12 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { i8*, i32 } %lpad.val12
}

; Function Attrs: nounwind readnone
declare i1 @llvm.type.test(i8*, metadata) #4

; Function Attrs: nounwind
declare void @llvm.assume(i1) #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #4

declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr

; Function Attrs: noreturn nounwind
declare dso_local void @exit(i32) local_unnamed_addr #5

declare dso_local void @__cxa_end_catch() local_unnamed_addr

; Function Attrs: noinline uwtable
define linkonce_odr hidden zeroext i1 @_ZN7Derived3fooEi(%class.Derived* %this, i32 %a) unnamed_addr #6 comdat align 2 {
entry:
  %cmp = icmp slt i32 %a, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %exception = tail call i8* @__cxa_allocate_exception(i64 8) #2
  %tmp = bitcast i8* %exception to i8**
  store i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.3, i64 0, i64 0), i8** %tmp, align 16, !tbaa !32
  tail call void @__cxa_throw(i8* nonnull %exception, i8* bitcast (i8** @_ZTIPKc to i8*), i8* null) #7
  unreachable

if.end:                                           ; preds = %entry
  ret i1 true
}

declare dso_local noalias nonnull i8* @__cxa_allocate_exception(i64) local_unnamed_addr

; Function Attrs: noreturn
declare dso_local void @__cxa_throw(i8*, i8*, i8*) local_unnamed_addr #7

; Function Attrs: noinline uwtable
define linkonce_odr hidden zeroext i1 @_ZN8Derived23fooEi(%class.Derived2* %this, i32 %a) unnamed_addr #6 comdat align 2 {
entry:
  %cmp = icmp slt i32 %a, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %exception = tail call i8* @__cxa_allocate_exception(i64 8) #2
  %tmp = bitcast i8* %exception to i8**
  store i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.3, i64 0, i64 0), i8** %tmp, align 16, !tbaa !32
  tail call void @__cxa_throw(i8* nonnull %exception, i8* bitcast (i8** @_ZTIPKc to i8*), i8* null) #7
  unreachable

if.end:                                           ; preds = %entry
  ret i1 false
}

declare dso_local dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertIbEERSoT_(%"class.std::basic_ostream"*, i1 zeroext) local_unnamed_addr #0

declare dso_local dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* dereferenceable(272), i8*, i64) local_unnamed_addr #0

declare dso_local void @_ZNSt9basic_iosIcSt11char_traitsIcEE5clearESt12_Ios_Iostate(%"class.std::basic_ios"*, i32) local_unnamed_addr #0

; Function Attrs: argmemonly nounwind readonly
declare dso_local i64 @strlen(i8* nocapture) local_unnamed_addr #8

declare dso_local dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"*, i8 signext) local_unnamed_addr #0

declare dso_local dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"*) local_unnamed_addr #0

; Function Attrs: noreturn
declare dso_local void @_ZSt16__throw_bad_castv() local_unnamed_addr #9

declare dso_local void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"*) local_unnamed_addr #0

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_driver2.cpp() #10 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZSt8__ioinit)
  %tmp = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZSt8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #2
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTV7Derived, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** getelementptr inbounds (%class.Derived, %class.Derived* @d, i64 0, i32 0, i32 0), align 8, !tbaa !12
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTV8Derived2, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** getelementptr inbounds (%class.Derived2, %class.Derived2* @d2, i64 0, i32 0, i32 0), align 8, !tbaa !12
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readnone }
attributes #5 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { noreturn }
attributes #8 = { argmemonly nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #9 = { noreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #10 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #11 = { noreturn nounwind }

!llvm.module.flags = !{!6}
!llvm.ident = !{!7}

!0 = !{i64 16, !"_ZTS4Base"}
!1 = !{i64 16, !"_ZTSM4BaseFbiE.virtual"}
!2 = !{i64 16, !"_ZTS7Derived"}
!3 = !{i64 16, !"_ZTSM7DerivedFbiE.virtual"}
!4 = !{i64 16, !"_ZTS8Derived2"}
!5 = !{i64 16, !"_ZTSM8Derived2FbiE.virtual"}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 0bc2dcdd432fcebd3a6fb90d6ddade4c117d08da) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 689500bd9a058e2ca986622012ad222e45d77bda)"}
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
; CHECK:       define hidden i32 @main(i32 %argc, i8** nocapture readnone %argv) local_unnamed_addr #3 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {

; Compare the address of the virtual function with Derived::foo
; CHECK:       %0 = bitcast i1 (%class.Base*, i32)* %tmp2 to i8*
; CHECK-NEXT:  %1 = bitcast i1 (%class.Derived*, i32)* @_ZN7Derived3fooEi to i8*
; CHECK-NEXT:  %2 = icmp eq i8* %0, %1
; CHECK-NEXT:  br i1 %2, label %BBDevirt__ZN7Derived3fooEi_0_0, label %ElseDevirt__ZN7Derived3fooEi_0_0

; If the address matches with Derived:foo, then call it
; CHECK-LABEL: BBDevirt__ZN7Derived3fooEi_0_0:
; CHECK:        %3 = invoke zeroext i1 bitcast (i1 (%class.Derived*, i32)* @_ZN7Derived3fooEi to i1 (%class.Base*, i32)*)(%class.Base* %.16, i32 %argc)
; CHECK-NEXT:          to label %MergeBB_0_0 unwind label %lpad

; Else, compare the address with Derived2:foo
; CHECK-LABEL: ElseDevirt__ZN7Derived3fooEi_0_0:
; CHECK:        %4 = bitcast i1 (%class.Derived2*, i32)* @_ZN8Derived23fooEi to i8*
; CHECK-NEXT:   %5 = icmp eq i8* %0, %4
; CHECK-NEXT:   br i1 %5, label %BBDevirt__ZN8Derived23fooEi_0_0, label %DefaultBB_0_0

; The address matches Derived2::foo
; CHECK-LABEL: BBDevirt__ZN8Derived23fooEi_0_0:
; CHECK:        %6 = invoke zeroext i1 bitcast (i1 (%class.Derived2*, i32)* @_ZN8Derived23fooEi to i1 (%class.Base*, i32)*)(%class.Base* %.16, i32 %argc)
; CHECK-NEXT:          to label %MergeBB_0_0 unwind label %lpad

; Default case
; CHECK-LABEL: DefaultBB_0_0:
; CHECK:        %7 = invoke zeroext i1 %tmp2(%class.Base* %.16, i32 %argc)
; CHECK-NEXT:          to label %MergeBB_0_0 unwind label %lpad

; Check that the PhiNode was generated correctly
; CHECK-LABEL: MergeBB_0_0:
; CHECK:        %8 = phi i1 [ %3, %BBDevirt__ZN7Derived3fooEi_0_0 ], [ %6, %BBDevirt__ZN8Derived23fooEi_0_0 ], [ %7, %DefaultBB_0_0 ]
; CHECK-NEXT: br label %invoke.cont


; Make sure that the users were replaced correctly
; CHECK-LABEL: invoke.cont:
; CHECK:        %call.i17 = invoke dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertIbEERSoT_(%"class.std::basic_ostream"* nonnull @_ZSt4cout, i1 zeroext %8)
; CHECK-NEXT:          to label %invoke.cont1 unwind label %lpad


; Check that the unwind destination have all the basic blocks connected correctly
; CHECK:       lpad:                                             ; preds = %BBDevirt__ZN8Derived23fooEi_0_0, %BBDevirt__ZN7Derived3fooEi_0_0, %invoke.cont1, %invoke.cont, %DefaultBB_0_0
; CHECK-NEXT:   %tmp3 = landingpad { i8*, i32 }
; CHECK-NEXT:            cleanup
; CHECK-NEXT:            catch i8* bitcast (i8** @_ZTIPKc to i8*)
; CHECK-NEXT:   %tmp4 = extractvalue { i8*, i32 } %tmp3, 0
; CHECK-NEXT:   %tmp5 = extractvalue { i8*, i32 } %tmp3, 1
; CHECK-NEXT:   %tmp6 = tail call i32 @llvm.eh.typeid.for(i8* bitcast (i8** @_ZTIPKc to i8*)) #2
; CHECK-NEXT:   %matches = icmp eq i32 %tmp5, %tmp6
; CHECK-NEXT:   br i1 %matches, label %catch, label %eh.resume

; Check that the metadata was added correctly
; CHECK: define linkonce_odr hidden zeroext i1 @_ZN7Derived3fooEi(%class.Derived* %this, i32 %a) unnamed_addr #{{.*}} comdat align 2 !_Intel.Devirt.Target
; CHECK: define linkonce_odr hidden zeroext i1 @_ZN8Derived23fooEi(%class.Derived2* %this, i32 %a) unnamed_addr #{{.*}} comdat align 2 !_Intel.Devirt.Target
