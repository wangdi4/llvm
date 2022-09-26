; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -S --wholeprogramdevirt-downcasting-filter  -passes=wholeprogramdevirt -whole-program-assume %s | FileCheck %s

; This test case checks that the assume intrinsic was removed and the virtual
; call wasn't devirtualized since it presents a possible downcasting. This test
; case is the same as intel-downcasting-opq.ll, but it contains the pointers'
; types. The goal is to make sure that the DTrans metadata based filtering is
; called even if there are typed pointers. It was created from the following
; C++ example:

; int var = 0;
;
; class Base {
;   public:
;     virtual void foo() = 0;
; };
;
; class DerivedA : public Base {
;
;   public:
;     void foo() { var = 1}
;
; };
;
; class DerivedB : public Base {
;
;   public:
;     void foo() { var = 2}
;
; };
;
; void bar(Base *p){
;   DerivedB *ptr = (Derived *) p;
;   ptr->foo();
; }

; CHECK: define internal void @_Z3barP4Base(%class._ZTS4Base.Base* "intel_dtrans_func_index"="1" %p) local_unnamed_addr #0 !intel.dtrans.func.type !74 {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %tmp = bitcast %class._ZTS4Base.Base* %p to %class._ZTS8DerivedB.DerivedB*
; CHECK-NEXT:   %tmp1 = bitcast %class._ZTS4Base.Base* %p to void (%class._ZTS8DerivedB.DerivedB*)***
; CHECK-NEXT:   %tmp2 = load void (%class._ZTS8DerivedB.DerivedB*)**, void (%class._ZTS8DerivedB.DerivedB*)*** %tmp1, align 8, !tbaa !75
; CHECK-NEXT:   %tmp5 = load void (%class._ZTS8DerivedB.DerivedB*)*, void (%class._ZTS8DerivedB.DerivedB*)** %tmp2, align 8
; CHECK-NEXT:   tail call void %tmp5(%class._ZTS8DerivedB.DerivedB* nonnull align 8 dereferenceable(8) %tmp), !intel_dtrans_type !78
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class._ZTSNSt8ios_base4InitE.std::ios_base::Init" = type { i8 }
%class._ZTS4Base.Base = type { i32 (...)** }
%"class._ZTSSi.std::basic_istream" = type { i32 (...)**, i64, %"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" }
%"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" = type { %"class._ZTSSt8ios_base.std::ios_base", %"class._ZTSSo.std::basic_ostream"*, i8, i8, %"class._ZTSSt15basic_streambufIcSt11char_traitsIcEE.std::basic_streambuf"*, %"class._ZTSSt5ctypeIcE.std::ctype"*, %"class._ZTSSt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE.std::num_put"*, %"class._ZTSSt7num_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE.std::num_get"* }
%"class._ZTSSt8ios_base.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct._ZTSNSt8ios_base14_Callback_listE.std::ios_base::_Callback_list"*, %"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words", [8 x %"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words"], i32, %"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words"*, %"class._ZTSSt6locale.std::locale" }
%"struct._ZTSNSt8ios_base14_Callback_listE.std::ios_base::_Callback_list" = type { %"struct._ZTSNSt8ios_base14_Callback_listE.std::ios_base::_Callback_list"*, void (i32, %"class._ZTSSt8ios_base.std::ios_base"*, i32)*, i32, i32 }
%"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words" = type { i8*, i64 }
%"class._ZTSSt6locale.std::locale" = type { %"class._ZTSNSt6locale5_ImplE.std::locale::_Impl"* }
%"class._ZTSNSt6locale5_ImplE.std::locale::_Impl" = type { i32, %"class._ZTSNSt6locale5facetE.std::locale::facet"**, i64, %"class._ZTSNSt6locale5facetE.std::locale::facet"**, i8** }
%"class._ZTSNSt6locale5facetE.std::locale::facet" = type <{ i32 (...)**, i32, [4 x i8] }>
%"class._ZTSSo.std::basic_ostream" = type { i32 (...)**, %"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" }
%"class._ZTSSt15basic_streambufIcSt11char_traitsIcEE.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class._ZTSSt6locale.std::locale" }
%"class._ZTSSt5ctypeIcE.std::ctype" = type <{ %"class._ZTSNSt6locale5facetE.std::locale::facet.base", [4 x i8], %struct._ZTS15__locale_struct.__locale_struct*, i8, [7 x i8], i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class._ZTSNSt6locale5facetE.std::locale::facet.base" = type <{ i32 (...)**, i32 }>
%struct._ZTS15__locale_struct.__locale_struct = type { [13 x %struct._ZTS13__locale_data.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct._ZTS13__locale_data.__locale_data = type opaque
%"class._ZTSSt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE.std::num_put" = type { %"class._ZTSNSt6locale5facetE.std::locale::facet.base", [4 x i8] }
%"class._ZTSSt7num_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE.std::num_get" = type { %"class._ZTSNSt6locale5facetE.std::locale::facet.base", [4 x i8] }
%class._ZTS8DerivedB.DerivedB = type { %class._ZTS4Base.Base }
%class._ZTS8DerivedA.DerivedA = type { %class._ZTS4Base.Base }

$_ZTS4Base = comdat any

$_ZTI4Base = comdat any

@_ZStL8__ioinit = internal global %"class._ZTSNSt8ios_base4InitE.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@var = dso_local local_unnamed_addr global i32 0, align 4
@ptr = dso_local local_unnamed_addr global %class._ZTS4Base.Base* null, align 8, !intel_dtrans_type !0
@_ZSt3cin = external dso_local global %"class._ZTSSi.std::basic_istream", align 8
@_ZSt4cout = external dso_local global %"class._ZTSSo.std::basic_ostream", align 8
@.str = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@_ZTV8DerivedA = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI8DerivedA to i8*), i8* bitcast (void (%class._ZTS8DerivedA.DerivedA*)* @_ZN8DerivedA3fooEv to i8*)] }, align 8, !type !1, !type !2, !type !3, !type !4, !intel_dtrans_type !5
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTS8DerivedA = internal constant [10 x i8] c"8DerivedA\00", align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS4Base = internal constant [6 x i8] c"4Base\00", align 1
@_ZTI4Base = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @_ZTS4Base, i32 0, i32 0) }, align 8, !intel_dtrans_type !8
@_ZTI8DerivedA = internal constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @_ZTS8DerivedA, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, align 8, !intel_dtrans_type !9
@_ZTV8DerivedB = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI8DerivedB to i8*), i8* bitcast (void (%class._ZTS8DerivedB.DerivedB*)* @_ZN8DerivedB3fooEv to i8*)] }, align 8, !type !1, !type !2, !type !10, !type !11, !intel_dtrans_type !5
@_ZTS8DerivedB = internal constant [10 x i8] c"8DerivedB\00", align 1
@_ZTI8DerivedB = internal constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @_ZTS8DerivedB, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, align 8, !intel_dtrans_type !9

; Function Attrs: mustprogress noinline uwtable
define internal void @_Z3barP4Base(%class._ZTS4Base.Base* "intel_dtrans_func_index"="1" %p) local_unnamed_addr #3 !intel.dtrans.func.type !77 {
entry:
  %tmp = bitcast %class._ZTS4Base.Base* %p to %class._ZTS8DerivedB.DerivedB*

  %tmp1 = bitcast %class._ZTS4Base.Base* %p to void (%class._ZTS8DerivedB.DerivedB*)***

  %tmp2 = load void (%class._ZTS8DerivedB.DerivedB*)**, void (%class._ZTS8DerivedB.DerivedB*)*** %tmp1, align 8, !tbaa !78
  %tmp3 = bitcast void (%class._ZTS8DerivedB.DerivedB*)** %tmp2 to i8*
  %tmp4 = tail call i1 @llvm.type.test(i8* %tmp3, metadata !"_ZTS8DerivedB")
  tail call void @llvm.assume(i1 %tmp4)
  %tmp5 = load void (%class._ZTS8DerivedB.DerivedB*)*, void (%class._ZTS8DerivedB.DerivedB*)** %tmp2, align 8

  tail call void %tmp5(%class._ZTS8DerivedB.DerivedB* nonnull align 8 dereferenceable(8) %tmp), !intel_dtrans_type !81
  ret void
}

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(i8*, metadata) #6

; Function Attrs: inaccessiblememonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #7

; Function Attrs: mustprogress noinline nounwind uwtable
define internal void @_ZN8DerivedA3fooEv(%class._ZTS8DerivedA.DerivedA* nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this) unnamed_addr #11 align 2 !intel.dtrans.func.type !95 {
entry:
  store i32 1, i32* @var, align 4, !tbaa !86
  ret void
}

; Function Attrs: mustprogress noinline nounwind uwtable
define internal void @_ZN8DerivedB3fooEv(%class._ZTS8DerivedB.DerivedB* nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this) unnamed_addr #11 align 2 !intel.dtrans.func.type !96 {
entry:
  store i32 2, i32* @var, align 4, !tbaa !86
  ret void
}

attributes #0 = { nofree "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-destructor" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nofree nounwind }
attributes #3 = { mustprogress noinline uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #5 = { nounwind }
attributes #6 = { mustprogress nofree nosync nounwind readnone speculatable willreturn }
attributes #7 = { inaccessiblememonly mustprogress nofree nosync nounwind willreturn }
attributes #8 = { norecurse uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #9 = { nofree "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #10 = { nobuiltin allocsize(0) "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #11 = { mustprogress noinline nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #12 = { nofree uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #13 = { builtin allocsize(0) }

!llvm.module.flags = !{!12, !13, !14, !15, !16}
!intel.dtrans.types = !{!17, !19, !23, !25, !26, !29, !36, !42, !47, !48, !50, !53, !55, !56, !57, !58, !66, !70, !71, !72}
!llvm.ident = !{!73}

!0 = !{%class._ZTS4Base.Base zeroinitializer, i32 1}
!1 = !{i64 16, !"_ZTS4Base"}
!2 = !{i64 16, !"_ZTSM4BaseFvvE.virtual"}
!3 = !{i64 16, !"_ZTS8DerivedA"}
!4 = !{i64 16, !"_ZTSM8DerivedAFvvE.virtual"}
!5 = !{!"L", i32 1, !6}
!6 = !{!"A", i32 3, !7}
!7 = !{i8 0, i32 1}
!8 = !{!"L", i32 2, !7, !7}
!9 = !{!"L", i32 3, !7, !7, !7}
!10 = !{i64 16, !"_ZTS8DerivedB"}
!11 = !{i64 16, !"_ZTSM8DerivedBFvvE.virtual"}
!12 = !{i32 1, !"wchar_size", i32 4}
!13 = !{i32 1, !"Virtual Function Elim", i32 0}
!14 = !{i32 7, !"uwtable", i32 1}
!15 = !{i32 1, !"ThinLTO", i32 0}
!16 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!17 = !{!"S", %"class._ZTSNSt8ios_base4InitE.std::ios_base::Init" zeroinitializer, i32 1, !18}
!18 = !{i8 0, i32 0}
!19 = !{!"S", %class._ZTS4Base.Base zeroinitializer, i32 1, !20}
!20 = !{!21, i32 2}
!21 = !{!"F", i1 true, i32 0, !22}
!22 = !{i32 0, i32 0}
!23 = !{!"S", %class._ZTS8DerivedB.DerivedB zeroinitializer, i32 1, !24}
!24 = !{%class._ZTS4Base.Base zeroinitializer, i32 0}
!25 = !{!"S", %class._ZTS8DerivedA.DerivedA zeroinitializer, i32 1, !24}
!26 = !{!"S", %"class._ZTSSi.std::basic_istream" zeroinitializer, i32 3, !20, !27, !28}
!27 = !{i64 0, i32 0}
!28 = !{%"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" zeroinitializer, i32 0}
!29 = !{!"S", %"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" zeroinitializer, i32 8, !30, !31, !18, !18, !32, !33, !34, !35}
!30 = !{%"class._ZTSSt8ios_base.std::ios_base" zeroinitializer, i32 0}
!31 = !{%"class._ZTSSo.std::basic_ostream" zeroinitializer, i32 1}
!32 = !{%"class._ZTSSt15basic_streambufIcSt11char_traitsIcEE.std::basic_streambuf" zeroinitializer, i32 1}
!33 = !{%"class._ZTSSt5ctypeIcE.std::ctype" zeroinitializer, i32 1}
!34 = !{%"class._ZTSSt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE.std::num_put" zeroinitializer, i32 1}
!35 = !{%"class._ZTSSt7num_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE.std::num_get" zeroinitializer, i32 1}
!36 = !{!"S", %"class._ZTSSt8ios_base.std::ios_base" zeroinitializer, i32 12, !20, !27, !27, !22, !22, !22, !37, !38, !39, !22, !40, !41}
!37 = !{%"struct._ZTSNSt8ios_base14_Callback_listE.std::ios_base::_Callback_list" zeroinitializer, i32 1}
!38 = !{%"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words" zeroinitializer, i32 0}
!39 = !{!"A", i32 8, !38}
!40 = !{%"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words" zeroinitializer, i32 1}
!41 = !{%"class._ZTSSt6locale.std::locale" zeroinitializer, i32 0}
!42 = !{!"S", %"struct._ZTSNSt8ios_base14_Callback_listE.std::ios_base::_Callback_list" zeroinitializer, i32 4, !37, !43, !22, !22}
!43 = !{!44, i32 1}
!44 = !{!"F", i1 false, i32 3, !45, !22, !46, !22}
!45 = !{!"void", i32 0}
!46 = !{%"class._ZTSSt8ios_base.std::ios_base" zeroinitializer, i32 1}
!47 = !{!"S", %"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words" zeroinitializer, i32 2, !7, !27}
!48 = !{!"S", %"class._ZTSSt6locale.std::locale" zeroinitializer, i32 1, !49}
!49 = !{%"class._ZTSNSt6locale5_ImplE.std::locale::_Impl" zeroinitializer, i32 1}
!50 = !{!"S", %"class._ZTSNSt6locale5_ImplE.std::locale::_Impl" zeroinitializer, i32 5, !22, !51, !27, !51, !52}
!51 = !{%"class._ZTSNSt6locale5facetE.std::locale::facet" zeroinitializer, i32 2}
!52 = !{i8 0, i32 2}
!53 = !{!"S", %"class._ZTSNSt6locale5facetE.std::locale::facet" zeroinitializer, i32 3, !20, !22, !54}
!54 = !{!"A", i32 4, !18}
!55 = !{!"S", %"class._ZTSNSt6locale5facetE.std::locale::facet.base" zeroinitializer, i32 2, !20, !22}
!56 = !{!"S", %"class._ZTSSo.std::basic_ostream" zeroinitializer, i32 2, !20, !28}
!57 = !{!"S", %"class._ZTSSt15basic_streambufIcSt11char_traitsIcEE.std::basic_streambuf" zeroinitializer, i32 8, !20, !7, !7, !7, !7, !7, !7, !41}
!58 = !{!"S", %"class._ZTSSt5ctypeIcE.std::ctype" zeroinitializer, i32 13, !59, !54, !60, !18, !61, !62, !62, !63, !18, !64, !64, !18, !65}
!59 = !{%"class._ZTSNSt6locale5facetE.std::locale::facet.base" zeroinitializer, i32 0}
!60 = !{%struct._ZTS15__locale_struct.__locale_struct zeroinitializer, i32 1}
!61 = !{!"A", i32 7, !18}
!62 = !{i32 0, i32 1}
!63 = !{i16 0, i32 1}
!64 = !{!"A", i32 256, !18}
!65 = !{!"A", i32 6, !18}
!66 = !{!"S", %struct._ZTS15__locale_struct.__locale_struct zeroinitializer, i32 5, !67, !63, !62, !62, !69}
!67 = !{!"A", i32 13, !68}
!68 = !{%struct._ZTS13__locale_data.__locale_data zeroinitializer, i32 1}
!69 = !{!"A", i32 13, !7}
!70 = !{!"S", %struct._ZTS13__locale_data.__locale_data zeroinitializer, i32 -1}
!71 = !{!"S", %"class._ZTSSt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE.std::num_put" zeroinitializer, i32 2, !59, !54}
!72 = !{!"S", %"class._ZTSSt7num_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE.std::num_get" zeroinitializer, i32 2, !59, !54}
!73 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!74 = distinct !{!75}
!75 = !{%"class._ZTSNSt8ios_base4InitE.std::ios_base::Init" zeroinitializer, i32 1}
!76 = distinct !{!75}
!77 = distinct !{!0}
!78 = !{!79, !79, i64 0}
!79 = !{!"vtable pointer", !80, i64 0}
!80 = !{!"Simple C++ TBAA"}
!81 = !{!"F", i1 false, i32 1, !45, !82}
!82 = !{%class._ZTS8DerivedB.DerivedB zeroinitializer, i32 1}
!83 = distinct !{!0}
!84 = !{!"F", i1 false, i32 1, !45, !85}
!85 = !{%class._ZTS8DerivedA.DerivedA zeroinitializer, i32 1}
!86 = !{!87, !87, i64 0}
!87 = !{!"int", !88, i64 0}
!88 = !{!"omnipotent char", !80, i64 0}
!89 = !{!90, !90, i64 0}
!90 = !{!"pointer@_ZTSP4Base", !88, i64 0}
!91 = distinct !{!92, !92, !62}
!92 = !{%"class._ZTSSi.std::basic_istream" zeroinitializer, i32 1}
!93 = distinct !{!7}
!94 = distinct !{!31, !31}
!95 = distinct !{!85}
!96 = distinct !{!82}
!97 = distinct !{!31, !31, !7}

; end INTEL_FEATURE_SW_DTRANS
