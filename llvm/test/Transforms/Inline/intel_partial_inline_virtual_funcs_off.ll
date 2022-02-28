; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test case makes sure that the devirtualized functions weren't partially
; inlined since the proper AVX flags aren't set. The test case assumes that
; devirtualization with multiversioning already ran.

; RUN: opt < %s -enable-new-pm=0 -partial-inliner -skip-partial-inlining-cost-analysis -partial-inline-virtual-functions -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(partial-inliner)' -skip-partial-inlining-cost-analysis -partial-inline-virtual-functions -S 2>&1 | FileCheck %s

; ModuleID = 'intel_partial_inline_virtual_funcs.ll'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
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
%class.Base = type { %class.cObject }
%class.cObject = type { i32 (...)** }
%class.Manager = type { i32 (...)**, i32*, %class.Base*, i32, %class.ClassFactory }
%class.ClassFactory = type { %class.cObject** }
%class.Derived = type { %class.Base }
%class.Derived2 = type { %class.Base }

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@_ZTV7Manager = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTI7Manager to i8*), i8* bitcast (i1 (%class.Manager*, i32*, i32)* @_ZN7Manager6runnerEPii to i8*)] }, align 8, !type !0, !type !1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS7Manager = internal constant [9 x i8] c"7Manager\00", align 1
@_ZTI7Manager = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @_ZTS7Manager, i32 0, i32 0) }, align 8
@_ZTV7Derived = internal unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI7Derived to i8*), i8* bitcast (void (%class.cObject*)* @_ZN7cObjectD2Ev to i8*), i8* bitcast (void (%class.Derived*)* @_ZN7DerivedD0Ev to i8*), i8* bitcast (i1 (i8*, i32)* @_ZN7Derived3fooEPvi to i8*)] }, align 8, !type !2, !type !3, !type !4, !type !5, !type !6, !type !7
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTS7Derived = internal constant [9 x i8] c"7Derived\00", align 1
@_ZTS4Base = internal constant [6 x i8] c"4Base\00", align 1
@_ZTS7cObject = internal constant [9 x i8] c"7cObject\00", align 1
@_ZTI7cObject = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @_ZTS7cObject, i32 0, i32 0) }, align 8
@_ZTI4Base = internal constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @_ZTS4Base, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI7cObject to i8*) }, align 8
@_ZTI7Derived = internal constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @_ZTS7Derived, i32 0, i32 0), i8* bitcast ({ i8*, i8*, i8* }* @_ZTI4Base to i8*) }, align 8
@_ZTV8Derived2 = internal unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI8Derived2 to i8*), i8* bitcast (void (%class.cObject*)* @_ZN7cObjectD2Ev to i8*), i8* bitcast (void (%class.Derived2*)* @_ZN8Derived2D0Ev to i8*), i8* bitcast (i1 (i8*, i32)* @_ZN8Derived23fooEPvi to i8*)] }, align 8, !type !2, !type !3, !type !6, !type !7, !type !8, !type !9
@_ZTS8Derived2 = internal constant [10 x i8] c"8Derived2\00", align 1
@_ZTI8Derived2 = internal constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @_ZTS8Derived2, i32 0, i32 0), i8* bitcast ({ i8*, i8*, i8* }* @_ZTI4Base to i8*) }, align 8
@_ZSt4cout = external dso_local global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_test.cpp, i8* null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #2

; Function Attrs: norecurse uwtable
define internal i32 @main(i32 %argc, i8** nocapture readnone %argv) local_unnamed_addr #3 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = tail call i8* @_Znwm(i64 40) #12
  %i = bitcast i8* %call to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTV7Manager, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %i, align 8, !tbaa !12
  %call.i.i5 = invoke i8* @_Znam(i64 16) #12
          to label %call.i.i.noexc unwind label %lpad

call.i.i.noexc:                                   ; preds = %entry
  %Factory.i = getelementptr inbounds i8, i8* %call, i64 32
  %i1 = bitcast i8* %Factory.i to i8**
  store i8* %call.i.i5, i8** %i1, align 8, !tbaa !15
  %call2.i.i6 = invoke i8* @_Znwm(i64 8) #12
          to label %call2.i.i.noexc unwind label %lpad

call2.i.i.noexc:                                  ; preds = %call.i.i.noexc
  %i2 = bitcast i8* %call2.i.i6 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTV7Derived, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %i2, align 8, !tbaa !12
  %i3 = bitcast i8* %call.i.i5 to i8**
  store i8* %call2.i.i6, i8** %i3, align 8, !tbaa !19
  %call4.i.i7 = invoke i8* @_Znwm(i64 8) #12
          to label %call4.i.i.noexc unwind label %lpad

call4.i.i.noexc:                                  ; preds = %call2.i.i.noexc
  %i4 = bitcast i8* %call4.i.i7 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTV8Derived2, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %i4, align 8, !tbaa !12
  %arrayidx6.i.i = getelementptr inbounds i8, i8* %call.i.i5, i64 8
  %i5 = bitcast i8* %arrayidx6.i.i to i8**
  store i8* %call4.i.i7, i8** %i5, align 8, !tbaa !19
  %call.i8 = invoke i8* @_Znam(i64 40) #12
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %call4.i.i.noexc
  %a.i = getelementptr inbounds i8, i8* %call, i64 8
  %i6 = bitcast i8* %a.i to i32**
  %i7 = bitcast i8* %a.i to i8**
  store i8* %call.i8, i8** %i7, align 8, !tbaa !20
  %size.i = getelementptr inbounds i8, i8* %call, i64 24
  %i8 = bitcast i8* %size.i to i32*
  store i32 10, i32* %i8, align 8, !tbaa !24
  %cmp.i = icmp eq i32 %argc, 0
  br i1 %cmp.i, label %dynamic_cast.end.i, label %dynamic_cast.end6.i

dynamic_cast.end.i:                               ; preds = %invoke.cont
  %i9 = tail call i8* @__dynamic_cast(i8* nonnull %call2.i.i6, i8* bitcast ({ i8*, i8* }* @_ZTI7cObject to i8*), i8* bitcast ({ i8*, i8*, i8* }* @_ZTI7Derived to i8*), i64 0) #2
  %b.i = getelementptr inbounds i8, i8* %call, i64 16
  %i10 = bitcast i8* %b.i to i8**
  store i8* %i9, i8** %i10, align 8, !tbaa !25
  br label %whpr.wrap.i

dynamic_cast.end6.i:                              ; preds = %invoke.cont
  %i11 = tail call i8* @__dynamic_cast(i8* nonnull %call4.i.i7, i8* bitcast ({ i8*, i8* }* @_ZTI7cObject to i8*), i8* bitcast ({ i8*, i8*, i8* }* @_ZTI8Derived2 to i8*), i64 0) #2
  %b7.i = getelementptr inbounds i8, i8* %call, i64 16
  %i12 = bitcast i8* %b7.i to i8**
  store i8* %i11, i8** %i12, align 8, !tbaa !25
  br label %whpr.wrap.i

whpr.wrap.i:                                      ; preds = %dynamic_cast.end.i, %dynamic_cast.end6.i
  %i13 = tail call i1 @llvm.type.test(i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTV7Manager, i64 0, inrange i32 0, i64 2) to i8*), metadata !"_ZTS7Manager")
  tail call void @llvm.assume(i1 %i13)
  %b.i12 = getelementptr inbounds i8, i8* %call, i64 16
  %i14 = bitcast i8* %b.i12 to %class.Base**
  %i15 = load %class.Base*, %class.Base** %i14, align 8, !tbaa !25
  %i16 = bitcast %class.Base* %i15 to i1 (%class.Base*, i8*, i32)***
  %vtable.i = load i1 (%class.Base*, i8*, i32)**, i1 (%class.Base*, i8*, i32)*** %i16, align 8, !tbaa !12
  %i17 = bitcast i1 (%class.Base*, i8*, i32)** %vtable.i to i8*
  %i18 = tail call i1 @llvm.type.test(i8* %i17, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %i18)
  %vfn.i = getelementptr inbounds i1 (%class.Base*, i8*, i32)*, i1 (%class.Base*, i8*, i32)** %vtable.i, i64 2
  %i19 = load i1 (%class.Base*, i8*, i32)*, i1 (%class.Base*, i8*, i32)** %vfn.i, align 8
  %i20 = bitcast i1 (%class.Base*, i8*, i32)* %i19 to i8*
  %i21 = bitcast i1 (i8*, i32)* @_ZN7Derived3fooEPvi to i8*
  %i22 = icmp eq i8* %i20, %i21
  br i1 %i22, label %BBDevirt__ZN7Derived3fooEPvi, label %BBDevirt__ZN8Derived23fooEPvi

BBDevirt__ZN7Derived3fooEPvi:                     ; preds = %whpr.wrap.i
  %i23 = tail call zeroext i1 bitcast (i1 (i8*, i32)* @_ZN7Derived3fooEPvi to i1 (%class.Base*, i8*, i32)*)(%class.Base* %i15, i8* nonnull %call.i8, i32 10), !_Intel.Devirt.Call !26
  br label %MergeBB

BBDevirt__ZN8Derived23fooEPvi:                    ; preds = %whpr.wrap.i
  %i24 = tail call zeroext i1 bitcast (i1 (i8*, i32)* @_ZN8Derived23fooEPvi to i1 (%class.Base*, i8*, i32)*)(%class.Base* %i15, i8* nonnull %call.i8, i32 10), !_Intel.Devirt.Call !26
  br label %MergeBB

MergeBB:                                          ; preds = %BBDevirt__ZN8Derived23fooEPvi, %BBDevirt__ZN7Derived3fooEPvi
  %i25 = phi i1 [ %i23, %BBDevirt__ZN7Derived3fooEPvi ], [ %i24, %BBDevirt__ZN8Derived23fooEPvi ]
  br label %bb

bb:                                               ; preds = %MergeBB
  br i1 %i25, label %for.body.i.i, label %_ZN7Manager3runEv.exit

for.body.i.i:                                     ; preds = %for.body.i.i, %bb
  %indvars.iv.i.i = phi i64 [ %indvars.iv.next.i.i, %for.body.i.i ], [ 0, %bb ]
  %i26 = load i32*, i32** %i6, align 8, !tbaa !20
  %arrayidx.i.i11 = getelementptr inbounds i32, i32* %i26, i64 %indvars.iv.i.i
  %i27 = load i32, i32* %arrayidx.i.i11, align 4, !tbaa !27
  %call.i.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"* nonnull @_ZSt4cout, i32 %i27)
  %call1.i.i.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %call.i.i, i8* nonnull getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i64 0, i64 0), i64 1)
  %indvars.iv.next.i.i = add nuw nsw i64 %indvars.iv.i.i, 1
  %exitcond.i.i = icmp eq i64 %indvars.iv.next.i.i, 10
  br i1 %exitcond.i.i, label %_ZN7Manager3runEv.exit, label %for.body.i.i

_ZN7Manager3runEv.exit:                           ; preds = %for.body.i.i, %bb
  ret i32 0

lpad:                                             ; preds = %call4.i.i.noexc, %call2.i.i.noexc, %call.i.i.noexc, %entry
  %i28 = landingpad { i8*, i32 }
          cleanup
  tail call void @_ZdlPv(i8* nonnull %call) #13
  resume { i8*, i32 } %i28
}

; Function Attrs: nobuiltin
declare dso_local noalias nonnull i8* @_Znwm(i64) local_unnamed_addr #4

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(i8*) local_unnamed_addr #5

; Function Attrs: nobuiltin
declare dso_local noalias nonnull i8* @_Znam(i64) local_unnamed_addr #4

; Function Attrs: uwtable
define internal zeroext i1 @_ZN7Manager6runnerEPii(%class.Manager* %this, i32* %A, i32 %Size) unnamed_addr #6 align 2 {
entry:
  %b = getelementptr inbounds %class.Manager, %class.Manager* %this, i64 0, i32 2, !intel-tbaa !25
  %i = load %class.Base*, %class.Base** %b, align 8, !tbaa !25
  %i1 = bitcast i32* %A to i8*
  %i2 = bitcast %class.Base* %i to i1 (i8*, i32)***
  %vtable = load i1 (i8*, i32)**, i1 (i8*, i32)*** %i2, align 8, !tbaa !12
  %i3 = bitcast i1 (i8*, i32)** %vtable to i8*
  %i4 = tail call i1 @llvm.type.test(i8* %i3, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %i4)
  %vfn = getelementptr inbounds i1 (i8*, i32)*, i1 (i8*, i32)** %vtable, i64 2
  %i5 = load i1 (i8*, i32)*, i1 (i8*, i32)** %vfn, align 8
  %i6 = bitcast i1 (i8*, i32)* %i5 to i8*
  %i7 = bitcast i1 (i8*, i32)* @_ZN7Derived3fooEPvi to i8*
  %i8 = icmp eq i8* %i6, %i7
  br i1 %i8, label %BBDevirt__ZN7Derived3fooEPvi, label %BBDevirt__ZN8Derived23fooEPvi

BBDevirt__ZN7Derived3fooEPvi:                     ; preds = %entry
  %i9 = tail call zeroext i1 @_ZN7Derived3fooEPvi(i8* %i1, i32 %Size)
  br label %MergeBB

BBDevirt__ZN8Derived23fooEPvi:                    ; preds = %entry
  %i10 = tail call zeroext i1 @_ZN8Derived23fooEPvi(i8* %i1, i32 %Size)
  br label %MergeBB

MergeBB:                                          ; preds = %BBDevirt__ZN8Derived23fooEPvi, %BBDevirt__ZN7Derived3fooEPvi
  %i11 = phi i1 [ %i9, %BBDevirt__ZN7Derived3fooEPvi ], [ %i10, %BBDevirt__ZN8Derived23fooEPvi ]
  br label %bb

bb:                                               ; preds = %MergeBB
  ret i1 %i11
}

; Function Attrs: inlinehint uwtable
define internal void @_ZN7DerivedD0Ev(%class.Derived* %this) unnamed_addr #7 align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %i = bitcast %class.Derived* %this to i8*
  tail call void @_ZdlPv(i8* %i) #13
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define internal zeroext i1 @_ZN7Derived3fooEPvi(i8* %arr, i32 %size) unnamed_addr #8 align 2 !_Intel.Devirt.Target !28 {
entry:
  %i = bitcast i8* %arr to i32*
  %cmp = icmp eq i8* %arr, null
  br i1 %cmp, label %cleanup, label %for.cond.preheader

for.cond.preheader:                               ; preds = %entry
  %cmp29 = icmp sgt i32 %size, 0
  br i1 %cmp29, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %for.cond.preheader
  %wide.trip.count = sext i32 %size to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %i, i64 %indvars.iv
  %i1 = trunc i64 %indvars.iv to i32
  store i32 %i1, i32* %arrayidx, align 4, !tbaa !27
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %cleanup, label %for.body

cleanup:                                          ; preds = %for.body, %for.cond.preheader, %entry
  %retval.0 = phi i1 [ false, %entry ], [ true, %for.cond.preheader ], [ true, %for.body ]
  ret i1 %retval.0
}

; Function Attrs: norecurse nounwind uwtable
define internal void @_ZN7cObjectD2Ev(%class.cObject* %this) unnamed_addr #8 align 2 {
entry:
  ret void
}

; Function Attrs: inlinehint uwtable
define internal void @_ZN8Derived2D0Ev(%class.Derived2* %this) unnamed_addr #7 align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %i = bitcast %class.Derived2* %this to i8*
  tail call void @_ZdlPv(i8* %i) #13
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define internal zeroext i1 @_ZN8Derived23fooEPvi(i8* %arr, i32 %size) unnamed_addr #8 align 2 !_Intel.Devirt.Target !28 {
entry:
  %i = bitcast i8* %arr to i32*
  %cmp = icmp eq i8* %arr, null
  br i1 %cmp, label %cleanup, label %for.cond.preheader

for.cond.preheader:                               ; preds = %entry
  %cmp29 = icmp sgt i32 %size, 0
  br i1 %cmp29, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %for.cond.preheader
  %wide.trip.count = sext i32 %size to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds i32, i32* %i, i64 %indvars.iv
  %i1 = trunc i64 %indvars.iv.next to i32
  store i32 %i1, i32* %arrayidx, align 4, !tbaa !27
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %cleanup, label %for.body

cleanup:                                          ; preds = %for.body, %for.cond.preheader, %entry
  %retval.0 = phi i1 [ false, %entry ], [ true, %for.cond.preheader ], [ true, %for.body ]
  ret i1 %retval.0
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(i8*, metadata) #9

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #10

; Function Attrs: nounwind readonly
declare dso_local i8* @__dynamic_cast(i8*, i8*, i8*, i64) local_unnamed_addr #11

declare dso_local dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"*, i32) local_unnamed_addr #0

declare dso_local dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* dereferenceable(272), i8*, i64) local_unnamed_addr #0

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_test.cpp() #6 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZSt8__ioinit)
  %i = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZSt8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #2
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nobuiltin nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { inlinehint uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #9 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #10 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #11 = { nounwind readonly }
attributes #12 = { builtin }
attributes #13 = { builtin nounwind }

!llvm.module.flags = !{!10}
!llvm.ident = !{!11}

!0 = !{i64 16, !"_ZTS7Manager"}
!1 = !{i64 16, !"_ZTSM7ManagerFbPiiE.virtual"}
!2 = !{i64 16, !"_ZTS4Base"}
!3 = !{i64 32, !"_ZTSM4BaseFbPviE.virtual"}
!4 = !{i64 16, !"_ZTS7Derived"}
!5 = !{i64 32, !"_ZTSM7DerivedFbPviE.virtual"}
!6 = !{i64 16, !"_ZTS7cObject"}
!7 = !{i64 32, !"_ZTSM7cObjectFbPviE.virtual"}
!8 = !{i64 16, !"_ZTS8Derived2"}
!9 = !{i64 32, !"_ZTSM8Derived2FbPviE.virtual"}
!10 = !{i32 1, !"wchar_size", i32 4}
!11 = !{!"clang version 8.0.0"}
!12 = !{!13, !13, i64 0}
!13 = !{!"vtable pointer", !14, i64 0}
!14 = !{!"Simple C++ TBAA"}
!15 = !{!16, !17, i64 0}
!16 = !{!"struct@_ZTS12ClassFactory", !17, i64 0}
!17 = !{!"unspecified pointer", !18, i64 0}
!18 = !{!"omnipotent char", !14, i64 0}
!19 = !{!17, !17, i64 0}
!20 = !{!21, !22, i64 8}
!21 = !{!"struct@_ZTS7Manager", !22, i64 8, !17, i64 16, !23, i64 24, !16, i64 32}
!22 = !{!"pointer@_ZTSPi", !18, i64 0}
!23 = !{!"int", !18, i64 0}
!24 = !{!21, !23, i64 24}
!25 = !{!21, !17, i64 16}
!26 = !{!"_Intel.Devirt.Call"}
!27 = !{!23, !23, i64 0}
!28 = !{!"_Intel.Devirt.Target"}

; Since partial inlining didn't run, then we must have direct calls to
; Derived::foo and Derived2::foo.

; Call to Derived::foo
; CHECK: BBDevirt__ZN7Derived3fooEPvi:                     ; preds = %whpr.wrap.i
; CHECK:   %i23 = tail call zeroext i1 bitcast (i1 (i8*, i32)* @_ZN7Derived3fooEPvi to i1 (%class.Base*, i8*, i32)*)(%class.Base* %i15, i8* nonnull %call.i8, i32 10), !_Intel.Devirt.Call

; Call to Derived2::foo
; CHECK: BBDevirt__ZN8Derived23fooEPvi:
; CHECK:   %i24 = tail call zeroext i1 bitcast (i1 (i8*, i32)* @_ZN8Derived23fooEPvi to i1 (%class.Base*, i8*, i32)*)(%class.Base* %i15, i8* nonnull %call.i8, i32 10), !_Intel.Devirt.Call

; end INTEL_FEATURE_SW_DTRANS