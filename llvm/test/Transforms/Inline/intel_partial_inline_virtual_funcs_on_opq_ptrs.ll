; INTEL_FEATURE_SW_DTRANS
; XFAIL: *
; CMPLRLLVM-35553: Running the partial inling with opaque pointers produces a
; segmentation fault with instructions simplification. Most likely the issue
; comes from the community.
; REQUIRES: intel_feature_sw_dtrans

; This test case makes sure that the devirtualized functions were partially
; inlined since the proper AVX flags are set. The test case assumes that
; devirtualization with multiversioning already ran. This test case is
; the same as intel_partial_inline_virtual_funcs_on.ll, but it checks
; for opaque pointers.

; RUN: opt < %s -opaque-pointers -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -enable-new-pm=0 -partial-inliner -skip-partial-inlining-cost-analysis -partial-inline-virtual-functions -S 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes='module(partial-inliner)' -skip-partial-inlining-cost-analysis -partial-inline-virtual-functions -S 2>&1 | FileCheck %s

; ModuleID = 'intel_partial_inline_virtual_funcs.ll'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "i686-unknown-unknown"

%"class.std::ios_base::Init" = type { i8 }
%"class.std::basic_ostream" = type { ptr, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", ptr, i8, i8, ptr, ptr, ptr, ptr }
%"class.std::ios_base" = type { ptr, i64, i64, i32, i32, i32, ptr, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, ptr, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { ptr, ptr, i32, i32 }
%"struct.std::ios_base::_Words" = type { ptr, i64 }
%"class.std::locale" = type { ptr }
%"class.std::locale::_Impl" = type { i32, ptr, i64, ptr, ptr }
%"class.std::locale::facet" = type <{ ptr, i32, [4 x i8] }>
%"class.std::basic_streambuf" = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], ptr, i8, [7 x i8], ptr, ptr, ptr, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ ptr, i32 }>
%struct.__locale_struct = type { [13 x ptr], ptr, ptr, ptr, [13 x ptr] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::num_get" = type { %"class.std::locale::facet.base", [4 x i8] }
%class.Base = type { %class.cObject }
%class.cObject = type { ptr }
%class.Manager = type { ptr, ptr, ptr, i32, %class.ClassFactory }
%class.ClassFactory = type { ptr }
%class.Derived = type { %class.Base }
%class.Derived2 = type { %class.Base }

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@_ZTV7Manager = internal unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @_ZTI7Manager, ptr @_ZN7Manager6runnerEPii] }, align 8, !type !0, !type !1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTS7Manager = internal constant [9 x i8] c"7Manager\00", align 1
@_ZTI7Manager = internal constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr getelementptr inbounds ([9 x i8], ptr @_ZTS7Manager, i32 0, i32 0) }, align 8
@_ZTV7Derived = internal unnamed_addr constant { [5 x ptr] } { [5 x ptr] [ptr null, ptr @_ZTI7Derived, ptr @_ZN7cObjectD2Ev, ptr @_ZN7DerivedD0Ev, ptr @_ZN7Derived3fooEPvi] }, align 8, !type !2, !type !3, !type !4, !type !5, !type !6, !type !7
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global ptr
@_ZTS7Derived = internal constant [9 x i8] c"7Derived\00", align 1
@_ZTS4Base = internal constant [6 x i8] c"4Base\00", align 1
@_ZTS7cObject = internal constant [9 x i8] c"7cObject\00", align 1
@_ZTI7cObject = internal constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr getelementptr inbounds ([9 x i8], ptr @_ZTS7cObject, i32 0, i32 0) }, align 8
@_ZTI4Base = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr getelementptr inbounds ([6 x i8], ptr @_ZTS4Base, i32 0, i32 0), ptr @_ZTI7cObject }, align 8
@_ZTI7Derived = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr getelementptr inbounds ([9 x i8], ptr @_ZTS7Derived, i32 0, i32 0), ptr @_ZTI4Base }, align 8
@_ZTV8Derived2 = internal unnamed_addr constant { [5 x ptr] } { [5 x ptr] [ptr null, ptr @_ZTI8Derived2, ptr @_ZN7cObjectD2Ev, ptr @_ZN8Derived2D0Ev, ptr @_ZN8Derived23fooEPvi] }, align 8, !type !2, !type !3, !type !6, !type !7, !type !8, !type !9
@_ZTS8Derived2 = internal constant [10 x i8] c"8Derived2\00", align 1
@_ZTI8Derived2 = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr getelementptr inbounds ([10 x i8], ptr @_ZTS8Derived2, i32 0, i32 0), ptr @_ZTI4Base }, align 8
@_ZSt4cout = external dso_local global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_test.cpp, ptr null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(ptr) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(ptr) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr) local_unnamed_addr #2

; Function Attrs: norecurse uwtable
define internal i32 @main(i32 %argc, ptr nocapture readnone %argv) local_unnamed_addr #3 personality ptr @__gxx_personality_v0 {
entry:
  %call = tail call ptr @_Znwm(i64 40) #13
  store ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV7Manager, i64 0, inrange i32 0, i64 2), ptr %call, align 8, !tbaa !12
  %call.i.i5 = invoke ptr @_Znam(i64 16) #13
          to label %call.i.i.noexc unwind label %lpad

call.i.i.noexc:                                   ; preds = %entry
  %Factory.i = getelementptr inbounds i8, ptr %call, i64 32
  store ptr %call.i.i5, ptr %Factory.i, align 8, !tbaa !15
  %call2.i.i6 = invoke ptr @_Znwm(i64 8) #13
          to label %call2.i.i.noexc unwind label %lpad

call2.i.i.noexc:                                  ; preds = %call.i.i.noexc
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTV7Derived, i64 0, inrange i32 0, i64 2), ptr %call2.i.i6, align 8, !tbaa !12
  store ptr %call2.i.i6, ptr %call.i.i5, align 8, !tbaa !19
  %call4.i.i7 = invoke ptr @_Znwm(i64 8) #13
          to label %call4.i.i.noexc unwind label %lpad

call4.i.i.noexc:                                  ; preds = %call2.i.i.noexc
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTV8Derived2, i64 0, inrange i32 0, i64 2), ptr %call4.i.i7, align 8, !tbaa !12
  %arrayidx6.i.i = getelementptr inbounds i8, ptr %call.i.i5, i64 8
  store ptr %call4.i.i7, ptr %arrayidx6.i.i, align 8, !tbaa !19
  %call.i8 = invoke ptr @_Znam(i64 40) #13
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %call4.i.i.noexc
  %a.i = getelementptr inbounds i8, ptr %call, i64 8
  store ptr %call.i8, ptr %a.i, align 8, !tbaa !20
  %size.i = getelementptr inbounds i8, ptr %call, i64 24
  store i32 10, ptr %size.i, align 8, !tbaa !24
  %cmp.i = icmp eq i32 %argc, 0
  br i1 %cmp.i, label %dynamic_cast.end.i, label %dynamic_cast.end6.i

dynamic_cast.end.i:                               ; preds = %invoke.cont
  %tmp1 = tail call ptr @__dynamic_cast(ptr nonnull %call2.i.i6, ptr @_ZTI7cObject, ptr @_ZTI7Derived, i64 0) #14
  %b.i = getelementptr inbounds i8, ptr %call, i64 16
  store ptr %tmp1, ptr %b.i, align 8, !tbaa !25
  br label %whpr.wrap.i

dynamic_cast.end6.i:                              ; preds = %invoke.cont
  %tmp2 = tail call ptr @__dynamic_cast(ptr nonnull %call4.i.i7, ptr @_ZTI7cObject, ptr @_ZTI8Derived2, i64 0) #14
  %b7.i = getelementptr inbounds i8, ptr %call, i64 16
  store ptr %tmp2, ptr %b7.i, align 8, !tbaa !25
  br label %whpr.wrap.i

whpr.wrap.i:                                      ; preds = %dynamic_cast.end.i, %dynamic_cast.end6.i
  %tmp3 = tail call i1 @llvm.type.test(ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV7Manager, i64 0, inrange i32 0, i64 2), metadata !"_ZTS7Manager")
  tail call void @llvm.assume(i1 %tmp3)
  %b.i12 = getelementptr inbounds i8, ptr %call, i64 16
  %tmp4 = load ptr, ptr %b.i12, align 8, !tbaa !25
  %vtable.i = load ptr, ptr %tmp4, align 8, !tbaa !12
  %tmp5 = tail call i1 @llvm.type.test(ptr %vtable.i, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %tmp5)
  %vfn.i = getelementptr inbounds ptr, ptr %vtable.i, i64 2
  %tmp6 = load ptr, ptr %vfn.i, align 8
  %tmp7 = icmp eq ptr %tmp6, @_ZN7Derived3fooEPvi
  br i1 %tmp7, label %BBDevirt__ZN7Derived3fooEPvi, label %BBDevirt__ZN8Derived23fooEPvi

BBDevirt__ZN7Derived3fooEPvi:                     ; preds = %whpr.wrap.i
  %tmp8 = tail call zeroext i1 @_ZN7Derived3fooEPvi(ptr %tmp4, ptr nonnull %call.i8, i32 10), !_Intel.Devirt.Call !26
  br label %MergeBB

BBDevirt__ZN8Derived23fooEPvi:                    ; preds = %whpr.wrap.i
  %tmp9 = tail call zeroext i1 @_ZN8Derived23fooEPvi(ptr %tmp4, ptr nonnull %call.i8, i32 10), !_Intel.Devirt.Call !26
  br label %MergeBB

MergeBB:                                          ; preds = %BBDevirt__ZN8Derived23fooEPvi, %BBDevirt__ZN7Derived3fooEPvi
  %tmp10 = phi i1 [ %tmp8, %BBDevirt__ZN7Derived3fooEPvi ], [ %tmp9, %BBDevirt__ZN8Derived23fooEPvi ]
  br label %tmp11

tmp11:                                               ; preds = %MergeBB
  br i1 %tmp10, label %for.body.i.i, label %_ZN7Manager3runEv.exit

for.body.i.i:                                     ; preds = %for.body.i.i, %tmp11
  %indvars.iv.i.i = phi i64 [ %indvars.iv.next.i.i, %for.body.i.i ], [ 0, %tmp11 ]
  %tmp12 = load ptr, ptr %a.i, align 8, !tbaa !20
  %arrayidx.i.i11 = getelementptr inbounds i32, ptr %tmp12, i64 %indvars.iv.i.i
  %tmp13 = load i32, ptr %arrayidx.i.i11, align 4, !tbaa !27
  %call.i.i = tail call dereferenceable(272) ptr @_ZNSolsEi(ptr nonnull @_ZSt4cout, i32 %tmp13)
  %call1.i.i.i = tail call dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull dereferenceable(272) %call.i.i, ptr nonnull getelementptr inbounds ([2 x i8], ptr @.str, i64 0, i64 0), i64 1)
  %indvars.iv.next.i.i = add nuw nsw i64 %indvars.iv.i.i, 1
  %exitcond.i.i = icmp eq i64 %indvars.iv.next.i.i, 10
  br i1 %exitcond.i.i, label %_ZN7Manager3runEv.exit, label %for.body.i.i

_ZN7Manager3runEv.exit:                           ; preds = %for.body.i.i, %tmp11
  ret i32 0

lpad:                                             ; preds = %call4.i.i.noexc, %call2.i.i.noexc, %call.i.i.noexc, %entry
  %tmp14 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZdlPv(ptr nonnull %call) #15
  resume { ptr, i32 } %tmp14
}

; Function Attrs: nobuiltin
declare dso_local noalias nonnull ptr @_Znwm(i64) local_unnamed_addr #4

declare dso_local i32 @__gxx_personality_v0(...) #5

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr) local_unnamed_addr #6

; Function Attrs: nobuiltin
declare dso_local noalias nonnull ptr @_Znam(i64) local_unnamed_addr #4

; Function Attrs: uwtable
define internal zeroext i1 @_ZN7Manager6runnerEPii(ptr %this, ptr %A, i32 %Size) unnamed_addr #7 align 2 {
entry:
  %b = getelementptr inbounds %class.Manager, ptr %this, i64 0, i32 2, !intel-tbaa !25
  %tmp0 = load ptr, ptr %b, align 8, !tbaa !25
  %vtable = load ptr, ptr %tmp0, align 8, !tbaa !12
  %tmp1 = tail call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %tmp1)
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %tmp2 = load ptr, ptr %vfn, align 8
  %tmp3 = icmp eq ptr %tmp2, @_ZN7Derived3fooEPvi
  br i1 %tmp3, label %BBDevirt__ZN7Derived3fooEPvi, label %BBDevirt__ZN8Derived23fooEPvi

BBDevirt__ZN7Derived3fooEPvi:                     ; preds = %entry
  %tmp4 = tail call zeroext i1 @_ZN7Derived3fooEPvi(ptr %A, i32 %Size)
  br label %MergeBB

BBDevirt__ZN8Derived23fooEPvi:                    ; preds = %entry
  %tmp5 = tail call zeroext i1 @_ZN8Derived23fooEPvi(ptr %A, i32 %Size)
  br label %MergeBB

MergeBB:                                          ; preds = %BBDevirt__ZN8Derived23fooEPvi, %BBDevirt__ZN7Derived3fooEPvi
  %tmp6 = phi i1 [ %tmp4, %BBDevirt__ZN7Derived3fooEPvi ], [ %tmp5, %BBDevirt__ZN8Derived23fooEPvi ]
  br label %tmp7

tmp7:                                               ; preds = %MergeBB
  ret i1 %tmp6
}

; Function Attrs: inlinehint uwtable
define internal void @_ZN7DerivedD0Ev(ptr %this) unnamed_addr #8 align 2 personality ptr @__gxx_personality_v0 {
entry:
  tail call void @_ZdlPv(ptr %this) #15
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define internal zeroext i1 @_ZN7Derived3fooEPvi(ptr %arr, i32 %size) unnamed_addr #9 align 2 !_Intel.Devirt.Target !28 {
entry:
  %cmp = icmp eq ptr %arr, null
  br i1 %cmp, label %cleanup, label %for.cond.preheader

for.cond.preheader:                               ; preds = %entry
  %cmp29 = icmp sgt i32 %size, 0
  br i1 %cmp29, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %for.cond.preheader
  %wide.trip.count = sext i32 %size to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv
  %tmp0 = trunc i64 %indvars.iv to i32
  store i32 %tmp0, ptr %arrayidx, align 4, !tbaa !27
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %cleanup, label %for.body

cleanup:                                          ; preds = %for.body, %for.cond.preheader, %entry
  %retval.0 = phi i1 [ false, %entry ], [ true, %for.cond.preheader ], [ true, %for.body ]
  ret i1 %retval.0
}

; Function Attrs: norecurse nounwind uwtable
define internal void @_ZN7cObjectD2Ev(ptr %this) unnamed_addr #9 align 2 {
entry:
  ret void
}

; Function Attrs: inlinehint uwtable
define internal void @_ZN8Derived2D0Ev(ptr %this) unnamed_addr #8 align 2 personality ptr @__gxx_personality_v0 {
entry:
  tail call void @_ZdlPv(ptr %this) #15
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define internal zeroext i1 @_ZN8Derived23fooEPvi(ptr %arr, i32 %size) unnamed_addr #9 align 2 !_Intel.Devirt.Target !28 {
entry:
  %cmp = icmp eq ptr %arr, null
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
  %arrayidx = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv
  %tmp0 = trunc i64 %indvars.iv.next to i32
  store i32 %tmp0, ptr %arrayidx, align 4, !tbaa !27
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %cleanup, label %for.body

cleanup:                                          ; preds = %for.body, %for.cond.preheader, %entry
  %retval.0 = phi i1 [ false, %entry ], [ true, %for.cond.preheader ], [ true, %for.body ]
  ret i1 %retval.0
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata) #10

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #11

; Function Attrs: nounwind readonly
declare dso_local ptr @__dynamic_cast(ptr, ptr, ptr, i64) local_unnamed_addr #12

declare dso_local dereferenceable(272) ptr @_ZNSolsEi(ptr, i32) local_unnamed_addr #0

declare dso_local dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr dereferenceable(272), ptr, i64) local_unnamed_addr #0

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_test.cpp() #7 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(ptr nonnull @_ZSt8__ioinit)
  %tmp0 = tail call i32 @__cxa_atexit(ptr @_ZNSt8ios_base4InitD1Ev, ptr getelementptr inbounds (%"class.std::ios_base::Init", ptr @_ZSt8__ioinit, i64 0, i32 0), ptr nonnull @__dso_handle) #14
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "target-features"="+avx2" }
attributes #3 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "target-features"="+avx2" }
attributes #6 = { nobuiltin nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { inlinehint uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #9 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #10 = { nofree nosync nounwind readnone speculatable willreturn "target-features"="+avx2" }
attributes #11 = { inaccessiblememonly nofree nosync nounwind willreturn "target-features"="+avx2" }
attributes #12 = { nounwind readonly "target-features"="+avx2" }
attributes #13 = { builtin }
attributes #14 = { nounwind }
attributes #15 = { builtin nounwind }

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

; The following checks are a place holder for what we need to look for when the
; issue with the inliner is fixed.

; Make sure Derived::foo was partially inlined by checking the call to the
; body preheader
; CHECK:    call void @_ZN7Derived3fooEPvi.2.for.body.preheader(i32 %Size, ptr %A)
; CHECK:    br label %_ZN7Derived3fooEPvi.2.exit

; Check that Derived2::foo was partially inlined by looking for the call
; to the body preheader
; CHECK:    call void @_ZN8Derived23fooEPvi.1.for.body.preheader(i32 %Size, ptr %A)
; CHECK:    br label %_ZN8Derived23fooEPvi.1.exit

; end INTEL_FEATURE_SW_DTRANS