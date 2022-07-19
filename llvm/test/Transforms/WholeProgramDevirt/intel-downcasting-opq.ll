; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -opaque-pointers -S -passes=wholeprogramdevirt -whole-program-assume %s | FileCheck %s

; This test case checks that the assume intrinsic was removed and the virtual
; call wasn't devirtualized since it presents a possible downcasting. This test
; is for opaque pointers. It is similar to intel-downcasting.ll but with the
; DTrans metadata included and was created from the following C++ example:

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


; Check that the call to %temp3 wasn't converted to a direct call

; CHECK: define internal void @_Z3barP4Base(ptr "intel_dtrans_func_index"="1" %p) local_unnamed_addr #0 !intel.dtrans.func.type !74 {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %tmp = load ptr, ptr %p, align 8, !tbaa !75
; CHECK-NEXT:   %tmp3 = load ptr, ptr %tmp, align 8
; CHECK-NEXT:   tail call void %tmp3(ptr nonnull align 8 dereferenceable(8) %p), !intel_dtrans_type !78
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; ModuleID = 'intel-downcasting-opq-bk.ll'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class._ZTSNSt8ios_base4InitE.std::ios_base::Init" = type { i8 }
%class._ZTS8DerivedB.DerivedB = type { %class._ZTS4Base.Base }
%class._ZTS4Base.Base = type { ptr }
%class._ZTS8DerivedA.DerivedA = type { %class._ZTS4Base.Base }
%"class._ZTSSi.std::basic_istream" = type { ptr, i64, %"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" }
%"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" = type { %"class._ZTSSt8ios_base.std::ios_base", ptr, i8, i8, ptr, ptr, ptr, ptr }
%"class._ZTSSt8ios_base.std::ios_base" = type { ptr, i64, i64, i32, i32, i32, ptr, %"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words", [8 x %"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words"], i32, ptr, %"class._ZTSSt6locale.std::locale" }
%"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words" = type { ptr, i64 }
%"class._ZTSSt6locale.std::locale" = type { ptr }
%"class._ZTSSo.std::basic_ostream" = type { ptr, %"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" }
%"class._ZTSSt15basic_streambufIcSt11char_traitsIcEE.std::basic_streambuf" = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, %"class._ZTSSt6locale.std::locale" }
%"class._ZTSSt5ctypeIcE.std::ctype" = type <{ %"class._ZTSNSt6locale5facetE.std::locale::facet.base", [4 x i8], ptr, i8, [7 x i8], ptr, ptr, ptr, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class._ZTSNSt6locale5facetE.std::locale::facet.base" = type <{ ptr, i32 }>
%"class._ZTSSt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE.std::num_put" = type { %"class._ZTSNSt6locale5facetE.std::locale::facet.base", [4 x i8] }
%"class._ZTSSt7num_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE.std::num_get" = type { %"class._ZTSNSt6locale5facetE.std::locale::facet.base", [4 x i8] }
%"struct._ZTSNSt8ios_base14_Callback_listE.std::ios_base::_Callback_list" = type { ptr, ptr, i32, i32 }
%"class._ZTSNSt6locale5_ImplE.std::locale::_Impl" = type { i32, ptr, i64, ptr, ptr }
%"class._ZTSNSt6locale5facetE.std::locale::facet" = type <{ ptr, i32, [4 x i8] }>
%struct._ZTS15__locale_struct.__locale_struct = type { [13 x ptr], ptr, ptr, ptr, [13 x ptr] }
%struct._ZTS13__locale_data.__locale_data = type opaque

@_ZStL8__ioinit = internal global %"class._ZTSNSt8ios_base4InitE.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@var = dso_local local_unnamed_addr global i32 0, align 4
@ptr = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !0
@_ZSt3cin = external dso_local global %"class._ZTSSi.std::basic_istream", align 8
@_ZSt4cout = external dso_local global %"class._ZTSSo.std::basic_ostream", align 8
@.str = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@_ZTV8DerivedA = internal unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @_ZTI8DerivedA, ptr @_ZN8DerivedA3fooEv] }, align 8, !type !1, !type !2, !type !3, !type !4, !intel_dtrans_type !5
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global ptr
@_ZTS8DerivedA = internal constant [10 x i8] c"8DerivedA\00", align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTS4Base = internal constant [6 x i8] c"4Base\00", align 1
@_ZTI4Base = internal constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr getelementptr inbounds ([6 x i8], ptr @_ZTS4Base, i32 0, i32 0) }, align 8, !intel_dtrans_type !8
@_ZTI8DerivedA = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr getelementptr inbounds ([10 x i8], ptr @_ZTS8DerivedA, i32 0, i32 0), ptr @_ZTI4Base }, align 8, !intel_dtrans_type !9
@_ZTV8DerivedB = internal unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @_ZTI8DerivedB, ptr @_ZN8DerivedB3fooEv] }, align 8, !type !1, !type !2, !type !10, !type !11, !intel_dtrans_type !5
@_ZTS8DerivedB = internal constant [10 x i8] c"8DerivedB\00", align 1
@_ZTI8DerivedB = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr getelementptr inbounds ([10 x i8], ptr @_ZTS8DerivedB, i32 0, i32 0), ptr @_ZTI4Base }, align 8, !intel_dtrans_type !9

; Function Attrs: mustprogress noinline uwtable
define internal void @_Z3barP4Base(ptr "intel_dtrans_func_index"="1" %p) local_unnamed_addr #0 !intel.dtrans.func.type !74 {
entry:
  %tmp = load ptr, ptr %p, align 8, !tbaa !75
  %tmp2 = tail call i1 @llvm.type.test(ptr %tmp, metadata !"_ZTS8DerivedB")
  tail call void @llvm.assume(i1 %tmp2)
  %tmp3 = load ptr, ptr %tmp, align 8
  tail call void %tmp3(ptr nonnull align 8 dereferenceable(8) %p), !intel_dtrans_type !78
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata) #1

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #2

; Function Attrs: mustprogress noinline nounwind uwtable
define internal void @_ZN8DerivedA3fooEv(ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this) unnamed_addr #3 align 2 !intel.dtrans.func.type !80 {
entry:
  store i32 1, ptr @var, align 4, !tbaa !82
  ret void
}

; Function Attrs: mustprogress noinline nounwind uwtable
define internal void @_ZN8DerivedB3fooEv(ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this) unnamed_addr #3 align 2 !intel.dtrans.func.type !85 {
entry:
  store i32 2, ptr @var, align 4, !tbaa !82
  ret void
}

attributes #0 = { mustprogress noinline uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #3 = { mustprogress noinline nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

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
!74 = distinct !{!0}
!75 = !{!76, !76, i64 0}
!76 = !{!"vtable pointer", !77, i64 0}
!77 = !{!"Simple C++ TBAA"}
!78 = !{!"F", i1 false, i32 1, !45, !79}
!79 = !{%class._ZTS8DerivedB.DerivedB zeroinitializer, i32 1}
!80 = distinct !{!81}
!81 = !{%class._ZTS8DerivedA.DerivedA zeroinitializer, i32 1}
!82 = !{!83, !83, i64 0}
!83 = !{!"int", !84, i64 0}
!84 = !{!"omnipotent char", !77, i64 0}
!85 = distinct !{!79}

; end INTEL_FEATURE_SW_DTRANS
