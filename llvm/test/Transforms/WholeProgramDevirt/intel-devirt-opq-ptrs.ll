; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -opaque-pointers -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -S -wholeprogramdevirt-downcasting-filter -passes=wholeprogramdevirt -whole-program-assume %s | FileCheck %s

; This test case checks that multiversioning for virtual functions works with
; opaque pointers. The virtual call will be multiversioned into the direct
; call, and the default case will be added since the DTrans pointer analyzer
; couldn't collect the dominant type. This test case is for opaque pointers
; only and it was created from the following C++ example:

; #include <iostream>
;
; class Base {
; public:
;  virtual bool foo(int a) = 0;
; };
;
; class Derived : public Base {
; public:
;   bool foo(int a) {
;     return true;
;   }
; };
;
; class Derived2 : public Base {
; public:
;   bool foo(int a) {
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
;   bool c = b->foo(argc);
;   std::cout << c <<"\n";
;   return 0;
; }

; icpx -flto simple.cpp -qopt-mem-layout-trans=4

; Check that the pointer is compared with function Derived::foo
; CHECK:   %9 = bitcast ptr @_ZN7Derived3fooEi to ptr
; CHECK:   %10 = icmp eq ptr %8, %9
; CHECK:   br i1 %10, label %BBDevirt__ZN7Derived3fooEi, label %ElseDevirt__ZN7Derived3fooEi
; CHECK: BBDevirt__ZN7Derived3fooEi:
; CHECK:   %11 = tail call zeroext i1 @_ZN7Derived3fooEi(ptr nonnull align 8 dereferenceable(8) %4, i32 %0), !intel_dtrans_type !82
; CHECK:   br label %MergeBB

; Check that the pointer was compared with Derived2::foo
; CHECK: ElseDevirt__ZN7Derived3fooEi:
; CHECK:   %12 = bitcast ptr @_ZN8Derived23fooEi to ptr
; CHECK:   %13 = icmp eq ptr %8, %12
; CHECK:   br i1 %13, label %BBDevirt__ZN8Derived23fooEi, label %DefaultBB
; CHECK: BBDevirt__ZN8Derived23fooEi:
; CHECK:   %14 = tail call zeroext i1 @_ZN8Derived23fooEi(ptr nonnull align 8 dereferenceable(8) %4, i32 %0), !intel_dtrans_type !82
; CHECK:   br label %MergeBB

; Default case
; DefaultBB:
;   %15 = tail call zeroext i1 %7(ptr nonnull align 8 dereferenceable(8) %4, i32 %0), !intel_dtrans_type !82
;   br label %MergeBB

; Merge point
; CHECK: MergeBB:
; CHECK:   %16 = phi i1 [ %11, %BBDevirt__ZN7Derived3fooEi ], [ %14, %BBDevirt__ZN8Derived23fooEi ], [ %15, %DefaultBB ]
; CHECK:   br label %17

; ModuleID = 'simple2.ll'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class._ZTSNSt8ios_base4InitE.std::ios_base::Init" = type { i8 }
%class._ZTS4Base.Base = type { ptr }
%class._ZTS7Derived.Derived = type { %class._ZTS4Base.Base }
%class._ZTS8Derived2.Derived2 = type { %class._ZTS4Base.Base }
%"class._ZTSSo.std::basic_ostream" = type { ptr, %"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" }
%"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" = type { %"class._ZTSSt8ios_base.std::ios_base", ptr, i8, i8, ptr, ptr, ptr, ptr }
%"class._ZTSSt8ios_base.std::ios_base" = type { ptr, i64, i64, i32, i32, i32, ptr, %"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words", [8 x %"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words"], i32, ptr, %"class._ZTSSt6locale.std::locale" }
%"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words" = type { ptr, i64 }
%"class._ZTSSt6locale.std::locale" = type { ptr }
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

$_ZN8Derived23fooEi = comdat any

$_ZN7Derived3fooEi = comdat any

$_ZTS7Derived = comdat any

$_ZTS4Base = comdat any

$_ZTI4Base = comdat any

$_ZTI7Derived = comdat any

$_ZTS8Derived2 = comdat any

$_ZTI8Derived2 = comdat any

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_simple2.cpp, ptr null }]
@_ZStL8__ioinit = internal global %"class._ZTSNSt8ios_base4InitE.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@b = internal global ptr null, align 8, !intel_dtrans_type !0
@d = internal global { ptr } { ptr getelementptr inbounds ([3 x ptr], ptr @_ZTV7Derived.0, i32 0, i32 2) }, align 8
@d2 = internal global { ptr } { ptr getelementptr inbounds ([3 x ptr], ptr @_ZTV8Derived2.0, i32 0, i32 2) }, align 8
@_ZSt4cout = external dso_local global %"class._ZTSSo.std::basic_ostream", align 8
@.str = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global ptr
@_ZTS7Derived = internal constant [9 x i8] c"7Derived\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTS4Base = internal constant [6 x i8] c"4Base\00", comdat, align 1
@_ZTI4Base = internal constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr getelementptr inbounds ([6 x i8], ptr @_ZTS4Base, i32 0, i32 0) }, comdat, align 8, !intel_dtrans_type !1
@_ZTI7Derived = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr getelementptr inbounds ([9 x i8], ptr @_ZTS7Derived, i32 0, i32 0), ptr @_ZTI4Base }, comdat, align 8, !intel_dtrans_type !3
@_ZTS8Derived2 = internal constant [10 x i8] c"8Derived2\00", comdat, align 1
@_ZTI8Derived2 = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr getelementptr inbounds ([10 x i8], ptr @_ZTS8Derived2, i32 0, i32 0), ptr @_ZTI4Base }, comdat, align 8, !intel_dtrans_type !3
@_ZTV7Derived.0 = private constant [3 x ptr] [ptr null, ptr @_ZTI7Derived, ptr @_ZN7Derived3fooEi], !type !4, !type !5, !type !6, !type !7, !vcall_visibility !8
@_ZTV8Derived2.0 = private constant [3 x ptr] [ptr null, ptr @_ZTI8Derived2, ptr @_ZN8Derived23fooEi], !type !4, !type !5, !type !9, !type !10, !vcall_visibility !8

; Function Attrs: nofree uwtable
define internal void @_GLOBAL__sub_I_simple2.cpp() #0 section ".text.startup" {
  tail call void @_ZNSt8ios_base4InitC1Ev(ptr nonnull align 1 dereferenceable(1) @_ZStL8__ioinit)
  %1 = tail call i32 @__cxa_atexit(ptr @_ZNSt8ios_base4InitD1Ev, ptr getelementptr inbounds (%"class._ZTSNSt8ios_base4InitE.std::ios_base::Init", ptr @_ZStL8__ioinit, i64 0, i32 0), ptr nonnull @__dso_handle) #9
  ret void
}

; Function Attrs: nofree
declare !intel.dtrans.func.type !73 dso_local void @_ZNSt8ios_base4InitC1Ev(ptr nonnull align 1 dereferenceable(1) "intel_dtrans_func_index"="1") unnamed_addr #1

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !75 dso_local void @_ZNSt8ios_base4InitD1Ev(ptr nonnull align 1 dereferenceable(1) "intel_dtrans_func_index"="1") unnamed_addr #2

; Function Attrs: nofree nounwind
declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr) local_unnamed_addr #3

; Function Attrs: mustprogress norecurse uwtable
define dso_local i32 @main(i32 %0, ptr nocapture readnone "intel_dtrans_func_index"="1" %1) #4 !intel.dtrans.func.type !76 {
  %3 = icmp slt i32 %0, 2
  %4 = select i1 %3, ptr @d, ptr @d2
  store ptr %4, ptr @b, align 8, !tbaa !77
  %5 = load ptr, ptr %4, align 8, !tbaa !81
  %6 = tail call i1 @llvm.type.test(ptr %5, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %6)
  %7 = load ptr, ptr %5, align 8
  %8 = tail call zeroext i1 %7(ptr nonnull align 8 dereferenceable(8) %4, i32 %0), !intel_dtrans_type !83
  %9 = tail call nonnull align 8 dereferenceable(8) ptr @_ZNSo9_M_insertIbEERSoT_(ptr nonnull align 8 dereferenceable(8) @_ZSt4cout, i1 zeroext %8)
  %10 = tail call nonnull align 8 dereferenceable(8) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull align 8 dereferenceable(8) %9, ptr nonnull getelementptr inbounds ([2 x i8], ptr @.str, i64 0, i64 0), i64 1)
  ret i32 0
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata) #5

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #6

; Function Attrs: nofree
declare !intel.dtrans.func.type !85 dso_local nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZNSo9_M_insertIbEERSoT_(ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2", i1 zeroext) local_unnamed_addr #7

; Function Attrs: nofree
declare !intel.dtrans.func.type !86 dso_local nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3", i64) local_unnamed_addr #7

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn
define internal zeroext i1 @_ZN8Derived23fooEi(ptr nocapture nonnull readnone align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %0, i32 %1) unnamed_addr #8 comdat align 2 !intel.dtrans.func.type !87 {
  ret i1 false
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn
define internal zeroext i1 @_ZN7Derived3fooEi(ptr nocapture nonnull readnone align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %0, i32 %1) unnamed_addr #8 comdat align 2 !intel.dtrans.func.type !89 {
  ret i1 true
}

attributes #0 = { nofree uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nofree nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-destructor" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nofree nounwind }
attributes #4 = { mustprogress norecurse uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #5 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #6 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #7 = { nofree "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #8 = { mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #9 = { nounwind }

!intel.dtrans.types = !{!11, !13, !17, !19, !20, !22, !29, !36, !41, !42, !44, !47, !49, !50, !51, !59, !63, !64, !65}
!llvm.ident = !{!66}
!llvm.module.flags = !{!67, !68, !69, !70, !71, !72}

!0 = !{%class._ZTS4Base.Base zeroinitializer, i32 1}
!1 = !{!"L", i32 2, !2, !2}
!2 = !{i8 0, i32 1}
!3 = !{!"L", i32 3, !2, !2, !2}
!4 = !{i32 16, !"_ZTS4Base"}
!5 = !{i32 16, !"_ZTSM4BaseFbiE.virtual"}
!6 = !{i32 16, !"_ZTS7Derived"}
!7 = !{i32 16, !"_ZTSM7DerivedFbiE.virtual"}
!8 = !{i64 1}
!9 = !{i32 16, !"_ZTS8Derived2"}
!10 = !{i32 16, !"_ZTSM8Derived2FbiE.virtual"}
!11 = !{!"S", %"class._ZTSNSt8ios_base4InitE.std::ios_base::Init" zeroinitializer, i32 1, !12}
!12 = !{i8 0, i32 0}
!13 = !{!"S", %class._ZTS4Base.Base zeroinitializer, i32 1, !14}
!14 = !{!15, i32 2}
!15 = !{!"F", i1 true, i32 0, !16}
!16 = !{i32 0, i32 0}
!17 = !{!"S", %class._ZTS7Derived.Derived zeroinitializer, i32 1, !18}
!18 = !{%class._ZTS4Base.Base zeroinitializer, i32 0}
!19 = !{!"S", %class._ZTS8Derived2.Derived2 zeroinitializer, i32 1, !18}
!20 = !{!"S", %"class._ZTSSo.std::basic_ostream" zeroinitializer, i32 2, !14, !21}
!21 = !{%"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" zeroinitializer, i32 0}
!22 = !{!"S", %"class._ZTSSt9basic_iosIcSt11char_traitsIcEE.std::basic_ios" zeroinitializer, i32 8, !23, !24, !12, !12, !25, !26, !27, !28}
!23 = !{%"class._ZTSSt8ios_base.std::ios_base" zeroinitializer, i32 0}
!24 = !{%"class._ZTSSo.std::basic_ostream" zeroinitializer, i32 1}
!25 = !{%"class._ZTSSt15basic_streambufIcSt11char_traitsIcEE.std::basic_streambuf" zeroinitializer, i32 1}
!26 = !{%"class._ZTSSt5ctypeIcE.std::ctype" zeroinitializer, i32 1}
!27 = !{%"class._ZTSSt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE.std::num_put" zeroinitializer, i32 1}
!28 = !{%"class._ZTSSt7num_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE.std::num_get" zeroinitializer, i32 1}
!29 = !{!"S", %"class._ZTSSt8ios_base.std::ios_base" zeroinitializer, i32 12, !14, !30, !30, !16, !16, !16, !31, !32, !33, !16, !34, !35}
!30 = !{i64 0, i32 0}
!31 = !{%"struct._ZTSNSt8ios_base14_Callback_listE.std::ios_base::_Callback_list" zeroinitializer, i32 1}
!32 = !{%"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words" zeroinitializer, i32 0}
!33 = !{!"A", i32 8, !32}
!34 = !{%"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words" zeroinitializer, i32 1}
!35 = !{%"class._ZTSSt6locale.std::locale" zeroinitializer, i32 0}
!36 = !{!"S", %"struct._ZTSNSt8ios_base14_Callback_listE.std::ios_base::_Callback_list" zeroinitializer, i32 4, !31, !37, !16, !16}
!37 = !{!38, i32 1}
!38 = !{!"F", i1 false, i32 3, !39, !16, !40, !16}
!39 = !{!"void", i32 0}
!40 = !{%"class._ZTSSt8ios_base.std::ios_base" zeroinitializer, i32 1}
!41 = !{!"S", %"struct._ZTSNSt8ios_base6_WordsE.std::ios_base::_Words" zeroinitializer, i32 2, !2, !30}
!42 = !{!"S", %"class._ZTSSt6locale.std::locale" zeroinitializer, i32 1, !43}
!43 = !{%"class._ZTSNSt6locale5_ImplE.std::locale::_Impl" zeroinitializer, i32 1}
!44 = !{!"S", %"class._ZTSNSt6locale5_ImplE.std::locale::_Impl" zeroinitializer, i32 5, !16, !45, !30, !45, !46}
!45 = !{%"class._ZTSNSt6locale5facetE.std::locale::facet" zeroinitializer, i32 2}
!46 = !{i8 0, i32 2}
!47 = !{!"S", %"class._ZTSNSt6locale5facetE.std::locale::facet" zeroinitializer, i32 3, !14, !16, !48}
!48 = !{!"A", i32 4, !12}
!49 = !{!"S", %"class._ZTSNSt6locale5facetE.std::locale::facet.base" zeroinitializer, i32 2, !14, !16}
!50 = !{!"S", %"class._ZTSSt15basic_streambufIcSt11char_traitsIcEE.std::basic_streambuf" zeroinitializer, i32 8, !14, !2, !2, !2, !2, !2, !2, !35}
!51 = !{!"S", %"class._ZTSSt5ctypeIcE.std::ctype" zeroinitializer, i32 13, !52, !48, !53, !12, !54, !55, !55, !56, !12, !57, !57, !12, !58}
!52 = !{%"class._ZTSNSt6locale5facetE.std::locale::facet.base" zeroinitializer, i32 0}
!53 = !{%struct._ZTS15__locale_struct.__locale_struct zeroinitializer, i32 1}
!54 = !{!"A", i32 7, !12}
!55 = !{i32 0, i32 1}
!56 = !{i16 0, i32 1}
!57 = !{!"A", i32 256, !12}
!58 = !{!"A", i32 6, !12}
!59 = !{!"S", %struct._ZTS15__locale_struct.__locale_struct zeroinitializer, i32 5, !60, !56, !55, !55, !62}
!60 = !{!"A", i32 13, !61}
!61 = !{%struct._ZTS13__locale_data.__locale_data zeroinitializer, i32 1}
!62 = !{!"A", i32 13, !2}
!63 = !{!"S", %struct._ZTS13__locale_data.__locale_data zeroinitializer, i32 -1}
!64 = !{!"S", %"class._ZTSSt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE.std::num_put" zeroinitializer, i32 2, !52, !48}
!65 = !{!"S", %"class._ZTSSt7num_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE.std::num_get" zeroinitializer, i32 2, !52, !48}
!66 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!67 = !{i32 1, !"wchar_size", i32 4}
!68 = !{i32 1, !"Virtual Function Elim", i32 0}
!69 = !{i32 7, !"uwtable", i32 1}
!70 = !{i32 1, !"ThinLTO", i32 0}
!71 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!72 = !{i32 1, !"LTOPostLink", i32 1}
!73 = distinct !{!74}
!74 = !{%"class._ZTSNSt8ios_base4InitE.std::ios_base::Init" zeroinitializer, i32 1}
!75 = distinct !{!74}
!76 = distinct !{!46}
!77 = !{!78, !78, i64 0}
!78 = !{!"pointer@_ZTSP4Base", !79, i64 0}
!79 = !{!"omnipotent char", !80, i64 0}
!80 = !{!"Simple C++ TBAA"}
!81 = !{!82, !82, i64 0}
!82 = !{!"vtable pointer", !80, i64 0}
!83 = !{!"F", i1 false, i32 2, !84, !0, !16}
!84 = !{i1 false, i32 0}
!85 = distinct !{!24, !24}
!86 = distinct !{!24, !24, !2}
!87 = distinct !{!88}
!88 = !{%class._ZTS8Derived2.Derived2 zeroinitializer, i32 1}
!89 = distinct !{!90}
!90 = !{%class._ZTS7Derived.Derived zeroinitializer, i32 1}

; end INTEL_FEATURE_SW_DTRANS
