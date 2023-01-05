; This test verifies that type on original indirect call is used to infer
; types in PtrTypeAnalyzer instead of target functions when calls are
; marked with "_Intel.Devirt.Call" metadata.

; REQUIRES: asserts

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; CHECK:  %i = load ptr, ptr @b, align 8, !tbaa !42
; CHECK:    LocalPointerInfo: CompletelyAnalyzed
; CHECK:      Aliased types:
; CHECK:        %class._ZTS4Base.Base*
; CHECK:       i32 (...)***
; CHECK-NOT: %class._ZTS7Derived.Derived*
; CHECK-NOT: %class._ZTS8Derived2.Derived2*
; CHECK:      Element pointees:
; CHECK:  %i2 = load ptr, ptr %i, align 8, !tbaa !44


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class._ZTS4Base.Base = type <{ ptr, i32, [4 x i8] }>
%class._ZTS4Base.Base.base = type <{ ptr, i32 }>
%class._ZTS7Derived.Derived = type { %class._ZTS4Base.Base.base, [4 x i8] }
%class._ZTS8Derived2.Derived2 = type { %class._ZTS4Base.Base.base, [4 x i8] }

@b = internal global ptr null, align 8, !intel_dtrans_type !0
@d1 = internal global ptr null, align 8, !intel_dtrans_type !1
@d2 = internal global ptr null, align 8, !intel_dtrans_type !2
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global ptr, !intel_dtrans_type !3
@_ZTS7Derived = internal constant [9 x i8] c"7Derived\00", align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr, !intel_dtrans_type !3
@_ZTS4Base = internal constant [6 x i8] c"4Base\00", align 1
@_ZTI4Base = internal constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS4Base }, align 8, !intel_dtrans_type !4
@_ZTI7Derived = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS7Derived, ptr @_ZTI4Base }, align 8, !intel_dtrans_type !5
@_ZTS8Derived2 = internal constant [10 x i8] c"8Derived2\00", align 1
@_ZTI8Derived2 = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS8Derived2, ptr @_ZTI4Base }, align 8, !intel_dtrans_type !5
@_ZTV7Derived.0 = private constant [3 x ptr] [ptr null, ptr @_ZTI7Derived, ptr @_ZN7Derived3fooEi], !type !6, !type !7, !type !8, !type !9, !intel_dtrans_type !10
@_ZTV8Derived2.0 = private constant [3 x ptr] [ptr null, ptr @_ZTI8Derived2, ptr @_ZN8Derived23fooEi], !type !6, !type !7, !type !11, !type !12, !intel_dtrans_type !10

; Function Attrs: nobuiltin allocsize(0)
declare !intel.dtrans.func.type !30 dso_local noundef nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64 noundef) local_unnamed_addr

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable
define internal noundef i32 @_ZN8Derived23fooEi(ptr nocapture noundef nonnull readonly align 8 dereferenceable(12) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) unnamed_addr align 2 !intel.dtrans.func.type !31 !_Intel.Devirt.Target !32 {
bb:
  %i = getelementptr inbounds %class._ZTS4Base.Base, ptr %arg, i64 0, i32 1, !intel-tbaa !33
  %i2 = load i32, ptr %i, align 8, !tbaa !33
  %i3 = add nsw i32 %i2, 1
  ret i32 %i3
}

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable
define internal noundef i32 @_ZN7Derived3fooEi(ptr nocapture noundef nonnull readonly align 8 dereferenceable(12) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) unnamed_addr align 2 !intel.dtrans.func.type !38 !_Intel.Devirt.Target !32 {
bb:
  %i = getelementptr inbounds %class._ZTS4Base.Base, ptr %arg, i64 0, i32 1, !intel-tbaa !33
  %i2 = load i32, ptr %i, align 8, !tbaa !33
  %i3 = add nsw i32 %i2, -1
  ret i32 %i3
}

; Function Attrs: mustprogress norecurse uwtable
define dso_local noundef i32 @main(i32 noundef %arg, ptr nocapture noundef readnone "intel_dtrans_func_index"="1" %arg1) !intel.dtrans.func.type !39 {
bb:
  %i = load ptr, ptr @b, align 8, !tbaa !41
  %i2 = load ptr, ptr %i, align 8, !tbaa !43
  %i3 = call i1 @llvm.type.test(ptr %i2, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %i3)
  %i4 = load ptr, ptr %i2, align 8
  %i5 = bitcast ptr %i4 to ptr
  %i6 = bitcast ptr @_ZN7Derived3fooEi to ptr
  %i7 = icmp eq ptr %i5, %i6
  br i1 %i7, label %bb8, label %bb10

bb8:                                              ; preds = %bb
  %i9 = tail call noundef i32 @_ZN7Derived3fooEi(ptr noundef nonnull align 8 dereferenceable(12) %i, i32 noundef %arg), !intel_dtrans_type !45, !_Intel.Devirt.Call !46
  br label %bb12

bb10:                                             ; preds = %bb
  %i11 = tail call noundef i32 @_ZN8Derived23fooEi(ptr noundef nonnull align 8 dereferenceable(12) %i, i32 noundef %arg), !intel_dtrans_type !45, !_Intel.Devirt.Call !46
  br label %bb12

bb12:                                             ; preds = %bb10, %bb8
  %i13 = phi i32 [ %i9, %bb8 ], [ %i11, %bb10 ]
  br label %bb14

bb14:                                             ; preds = %bb12
  ret i32 %i13
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.public.type.test(ptr, metadata)

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef)

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata)

!intel.dtrans.types = !{!13, !19, !20, !22}
!llvm.ident = !{!23}
!llvm.module.flags = !{!24, !25, !26, !27, !28, !29}

!0 = !{%class._ZTS4Base.Base zeroinitializer, i32 1}
!1 = !{%class._ZTS7Derived.Derived zeroinitializer, i32 1}
!2 = !{%class._ZTS8Derived2.Derived2 zeroinitializer, i32 1}
!3 = !{i8 0, i32 1}
!4 = !{!"L", i32 2, !3, !3}
!5 = !{!"L", i32 3, !3, !3, !3}
!6 = !{i32 16, !"_ZTS4Base"}
!7 = !{i32 16, !"_ZTSM4BaseFiiE.virtual"}
!8 = !{i32 16, !"_ZTS7Derived"}
!9 = !{i32 16, !"_ZTSM7DerivedFiiE.virtual"}
!10 = !{!"A", i32 3, !3}
!11 = !{i32 16, !"_ZTS8Derived2"}
!12 = !{i32 16, !"_ZTSM8Derived2FiiE.virtual"}
!13 = !{!"S", %class._ZTS4Base.Base zeroinitializer, i32 3, !14, !16, !17}
!14 = !{!15, i32 2}
!15 = !{!"F", i1 true, i32 0, !16}
!16 = !{i32 0, i32 0}
!17 = !{!"A", i32 4, !18}
!18 = !{i8 0, i32 0}
!19 = !{!"S", %class._ZTS4Base.Base.base zeroinitializer, i32 2, !14, !16}
!20 = !{!"S", %class._ZTS7Derived.Derived zeroinitializer, i32 2, !21, !17}
!21 = !{%class._ZTS4Base.Base.base zeroinitializer, i32 0}
!22 = !{!"S", %class._ZTS8Derived2.Derived2 zeroinitializer, i32 2, !21, !17}
!23 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!24 = !{i32 1, !"wchar_size", i32 4}
!25 = !{i32 1, !"Virtual Function Elim", i32 0}
!26 = !{i32 7, !"uwtable", i32 2}
!27 = !{i32 1, !"ThinLTO", i32 0}
!28 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!29 = !{i32 1, !"LTOPostLink", i32 1}
!30 = distinct !{!3}
!31 = distinct !{!2}
!32 = !{!"_Intel.Devirt.Target"}
!33 = !{!34, !35, i64 8}
!34 = !{!"struct@_ZTS4Base", !35, i64 8}
!35 = !{!"int", !36, i64 0}
!36 = !{!"omnipotent char", !37, i64 0}
!37 = !{!"Simple C++ TBAA"}
!38 = distinct !{!1}
!39 = distinct !{!40}
!40 = !{i8 0, i32 2}
!41 = !{!42, !42, i64 0}
!42 = !{!"pointer@_ZTSP4Base", !36, i64 0}
!43 = !{!44, !44, i64 0}
!44 = !{!"vtable pointer", !37, i64 0}
!45 = !{!"F", i1 false, i32 2, !16, !0, !16}
!46 = !{!"_Intel.Devirt.Call"}
