; This test verifies that "store i32 42, i32* %i11" is NOT removed by
; instcombine using Andersens's points-to analysis.

; RUN: opt < %s -passes='require<anders-aa>,instcombine' -S 2>&1 | FileCheck %s

; CHECK: store i32 42,

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.llvm::Any" = type { %"class.std::unique_ptr" }
%"class.std::unique_ptr" = type { %"struct.std::__uniq_ptr_data" }
%"struct.std::__uniq_ptr_data" = type { %"class.std::__uniq_ptr_impl" }
%"class.std::__uniq_ptr_impl" = type { %"class.std::tuple" }
%"class.std::tuple" = type { %"struct.std::_Tuple_impl" }
%"struct.std::_Tuple_impl" = type { %"struct.std::_Head_base.1" }
%"struct.std::_Head_base.1" = type { %"struct.llvm::Any::StorageBase"* }
%"struct.llvm::Any::StorageBase" = type { i32 (...)** }
%"struct.llvm::Any::StorageImpl" = type <{ %"struct.llvm::Any::StorageBase", i32, [4 x i8] }>

@.str = private unnamed_addr constant [17 x i8] c"ERROR: %d != %d\0A\00", align 1
@_ZTVN4llvm3Any11StorageImplIiEE = linkonce_odr dso_local unnamed_addr constant { [6 x i8*] } { [6 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN4llvm3Any11StorageImplIiEE to i8*), i8* bitcast (void (%"struct.llvm::Any::StorageBase"*)* @_ZN4llvm3Any11StorageBaseD2Ev to i8*), i8* bitcast (void (%"struct.llvm::Any::StorageImpl"*)* @_ZN4llvm3Any11StorageImplIiED0Ev to i8*), i8* bitcast (void (%"class.std::unique_ptr"*, %"struct.llvm::Any::StorageImpl"*)* @_ZNK4llvm3Any11StorageImplIiE5cloneEv to i8*), i8* bitcast (i8* (%"struct.llvm::Any::StorageImpl"*)* @_ZNK4llvm3Any11StorageImplIiE2idEv to i8*)] }, align 8
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTSN4llvm3Any11StorageImplIiEE = linkonce_odr dso_local constant [28 x i8] c"N4llvm3Any11StorageImplIiEE\00", align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTSN4llvm3Any11StorageBaseE = linkonce_odr dso_local constant [25 x i8] c"N4llvm3Any11StorageBaseE\00", align 1
@_ZTIN4llvm3Any11StorageBaseE = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([25 x i8], [25 x i8]* @_ZTSN4llvm3Any11StorageBaseE, i32 0, i32 0) }, align 8
@_ZTIN4llvm3Any11StorageImplIiEE = linkonce_odr dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([28 x i8], [28 x i8]* @_ZTSN4llvm3Any11StorageImplIiEE, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTIN4llvm3Any11StorageBaseE to i8*) }, align 8
@_ZN4llvm3Any6TypeIdIiE2IdE = linkonce_odr dso_local constant i8 0, align 1
@.str.2 = private unnamed_addr constant [47 x i8] c"Value && any_isa<U>(*Value) && \22Bad any cast!\22\00", align 1
@.str.3 = private unnamed_addr constant [67 x i8] c"/localdisk2/pvbattin/work/xmainvb/llvm/llvm/include/llvm/ADT/Any.h\00", align 1
@__PRETTY_FUNCTION__._ZN4llvm8any_castIiEEPT_PNS_3AnyE = private unnamed_addr constant [41 x i8] c"T *llvm::any_cast(llvm::Any *) [T = int]\00", align 1

; Function Attrs: norecurse nounwind uwtable
define dso_local noundef i32 @main() local_unnamed_addr {
_ZN4llvm8any_castIiEET_RNS_3AnyE.exit33:
  %agg.tmp = alloca %"class.llvm::Any", align 8
  %call.i.i113 = tail call noalias noundef nonnull dereferenceable(16) i8* @_Znwm(i64 noundef 16)
  %i = bitcast i8* %call.i.i113 to %"struct.llvm::Any::StorageImpl"*
  %i1 = getelementptr inbounds %"struct.llvm::Any::StorageImpl", %"struct.llvm::Any::StorageImpl"* %i, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVN4llvm3Any11StorageImplIiEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %i1, align 8
  %Value2.i.i.i114 = getelementptr inbounds %"struct.llvm::Any::StorageImpl", %"struct.llvm::Any::StorageImpl"* %i, i64 0, i32 1
  store i32 7, i32* %Value2.i.i.i114, align 8
  %i2 = getelementptr %"struct.llvm::Any::StorageImpl", %"struct.llvm::Any::StorageImpl"* %i, i64 0, i32 0
  %Value4.i.i = getelementptr inbounds %"struct.llvm::Any::StorageBase", %"struct.llvm::Any::StorageBase"* %i2, i64 1
  %i3 = bitcast %"struct.llvm::Any::StorageBase"* %Value4.i.i to i32*
  %call3.i13.i.i31 = tail call noundef i8* @_ZNK4llvm3Any11StorageImplIiE2idEv(%"struct.llvm::Any::StorageImpl"* noundef nonnull align 8 dereferenceable(8) %i)
  %i4 = load i32, i32* %i3, align 4
  %cmp2.not = icmp eq i32 %i4, 7
  br i1 %cmp2.not, label %_ZN4llvm3AnyaSES0_.exit, label %_ZN4llvm3AnyD2Ev.exit85

_ZN4llvm3AnyaSES0_.exit:                          ; preds = %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit33
  %Storage.i36 = getelementptr inbounds %"class.llvm::Any", %"class.llvm::Any"* %agg.tmp, i64 0, i32 0
  %i5 = bitcast i8* %call.i.i113 to void (%"class.std::unique_ptr"*, %"struct.llvm::Any::StorageBase"*)***
  %vtable.i37 = load void (%"class.std::unique_ptr"*, %"struct.llvm::Any::StorageBase"*)**, void (%"class.std::unique_ptr"*, %"struct.llvm::Any::StorageBase"*)*** %i5, align 8
  %vfn.i38 = getelementptr inbounds void (%"class.std::unique_ptr"*, %"struct.llvm::Any::StorageBase"*)*, void (%"class.std::unique_ptr"*, %"struct.llvm::Any::StorageBase"*)** %vtable.i37, i64 2
  %i6 = load void (%"class.std::unique_ptr"*, %"struct.llvm::Any::StorageBase"*)*, void (%"class.std::unique_ptr"*, %"struct.llvm::Any::StorageBase"*)** %vfn.i38, align 8
  call void %i6(%"class.std::unique_ptr"* nonnull sret(%"class.std::unique_ptr") align 8 %Storage.i36, %"struct.llvm::Any::StorageBase"* noundef nonnull align 8 dereferenceable(8) %i2)
  %_M_head_impl.i.i.i.i.i.i.i.i.i.i43 = getelementptr inbounds %"class.llvm::Any", %"class.llvm::Any"* %agg.tmp, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %i7 = load %"struct.llvm::Any::StorageBase"*, %"struct.llvm::Any::StorageBase"** %_M_head_impl.i.i.i.i.i.i.i.i.i.i43, align 8
  store %"struct.llvm::Any::StorageBase"* null, %"struct.llvm::Any::StorageBase"** %_M_head_impl.i.i.i.i.i.i.i.i.i.i43, align 8
  %cmp.i.not.i.i = icmp eq %"struct.llvm::Any::StorageBase"* %i7, null
  br i1 %cmp.i.not.i.i, label %cond.false.i47, label %_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit.i

_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit.i:           ; preds = %_ZN4llvm3AnyaSES0_.exit
  %nonnull = bitcast %"struct.llvm::Any::StorageBase"* %i7 to i8* (%"struct.llvm::Any::StorageBase"*)***
  %vtable.i.i = load i8* (%"struct.llvm::Any::StorageBase"*)**, i8* (%"struct.llvm::Any::StorageBase"*)*** %nonnull, align 8
  %vfn.i.i = getelementptr inbounds i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vtable.i.i, i64 3
  %i9 = load i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vfn.i.i, align 8
  %call3.i.i = call noundef i8* %i9(%"struct.llvm::Any::StorageBase"* noundef nonnull align 8 dereferenceable(8) %i7)
  %cmp.i.i = icmp eq i8* %call3.i.i, @_ZN4llvm3Any6TypeIdIiE2IdE
  br i1 %cmp.i.i, label %_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit17.i, label %cond.false.i47

cond.false.i47:                                   ; preds = %_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit.i, %_ZN4llvm3AnyaSES0_.exit
  call void @__assert_fail(i8* noundef getelementptr inbounds ([47 x i8], [47 x i8]* @.str.2, i64 0, i64 0), i8* noundef getelementptr inbounds ([67 x i8], [67 x i8]* @.str.3, i64 0, i64 0), i32 noundef 149, i8* noundef getelementptr inbounds ([41 x i8], [41 x i8]* @__PRETTY_FUNCTION__._ZN4llvm8any_castIiEEPT_PNS_3AnyE, i64 0, i64 0))
  unreachable

_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit17.i:         ; preds = %_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit.i
  %vtable.i11.i = load i8* (%"struct.llvm::Any::StorageBase"*)**, i8* (%"struct.llvm::Any::StorageBase"*)*** %nonnull, align 8
  %vfn.i12.i = getelementptr inbounds i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vtable.i11.i, i64 3
  %i10 = load i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vfn.i12.i, align 8
  %call3.i13.i = call noundef i8* %i10(%"struct.llvm::Any::StorageBase"* noundef nonnull align 8 dereferenceable(8) %i7)
  %Value4.i = getelementptr inbounds %"struct.llvm::Any::StorageBase", %"struct.llvm::Any::StorageBase"* %i7, i64 1
  %i11 = bitcast %"struct.llvm::Any::StorageBase"* %Value4.i to i32*
  store i32 42, i32* %i11, align 4
  %vtable.i.i.i50 = load i8* (%"struct.llvm::Any::StorageBase"*)**, i8* (%"struct.llvm::Any::StorageBase"*)*** %nonnull, align 8
  %vfn.i.i.i51 = getelementptr inbounds i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vtable.i.i.i50, i64 3
  %i12 = load i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vfn.i.i.i51, align 8
  %call3.i.i.i52 = call noundef i8* %i12(%"struct.llvm::Any::StorageBase"* noundef nonnull align 8 dereferenceable(8) %i7)
  %cmp.i.i.i53 = icmp eq i8* %call3.i.i.i52, @_ZN4llvm3Any6TypeIdIiE2IdE
  br i1 %cmp.i.i.i53, label %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit60, label %cond.false.i.i55

cond.false.i.i55:                                 ; preds = %_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit17.i
  call void @__assert_fail(i8* noundef getelementptr inbounds ([47 x i8], [47 x i8]* @.str.2, i64 0, i64 0), i8* noundef getelementptr inbounds ([67 x i8], [67 x i8]* @.str.3, i64 0, i64 0), i32 noundef 149, i8* noundef getelementptr inbounds ([41 x i8], [41 x i8]* @__PRETTY_FUNCTION__._ZN4llvm8any_castIiEEPT_PNS_3AnyE, i64 0, i64 0))
  unreachable

_ZN4llvm8any_castIiEET_RNS_3AnyE.exit60:          ; preds = %_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit17.i
  %vtable.i11.i.i56 = load i8* (%"struct.llvm::Any::StorageBase"*)**, i8* (%"struct.llvm::Any::StorageBase"*)*** %nonnull, align 8
  %vfn.i12.i.i57 = getelementptr inbounds i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vtable.i11.i.i56, i64 3
  %i13 = load i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vfn.i12.i.i57, align 8
  %call3.i13.i.i58 = call noundef i8* %i13(%"struct.llvm::Any::StorageBase"* noundef nonnull align 8 dereferenceable(8) %i7)
  %i14 = load i32, i32* %i11, align 4
  %cmp8.not = icmp eq i32 %i14, 42
  br i1 %cmp8.not, label %_ZN4llvm3AnyD2Ev.exit85, label %_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit.i.i67

_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit.i.i67:       ; preds = %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit60
  %vtable.i.i.i63 = load i8* (%"struct.llvm::Any::StorageBase"*)**, i8* (%"struct.llvm::Any::StorageBase"*)*** %nonnull, align 8
  %vfn.i.i.i64 = getelementptr inbounds i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vtable.i.i.i63, i64 3
  %i15 = load i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vfn.i.i.i64, align 8
  %call3.i.i.i65 = call noundef i8* %i15(%"struct.llvm::Any::StorageBase"* noundef nonnull align 8 dereferenceable(8) %i7)
  %cmp.i.i.i66 = icmp eq i8* %call3.i.i.i65, @_ZN4llvm3Any6TypeIdIiE2IdE
  br i1 %cmp.i.i.i66, label %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit73, label %cond.false.i.i68

cond.false.i.i68:                                 ; preds = %_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit.i.i67
  call void @__assert_fail(i8* noundef getelementptr inbounds ([47 x i8], [47 x i8]* @.str.2, i64 0, i64 0), i8* noundef getelementptr inbounds ([67 x i8], [67 x i8]* @.str.3, i64 0, i64 0), i32 noundef 149, i8* noundef getelementptr inbounds ([41 x i8], [41 x i8]* @__PRETTY_FUNCTION__._ZN4llvm8any_castIiEEPT_PNS_3AnyE, i64 0, i64 0))
  unreachable

_ZN4llvm8any_castIiEET_RNS_3AnyE.exit73:          ; preds = %_ZN4llvm7any_isaIiEEbRKNS_3AnyE.exit.i.i67
  %vtable.i11.i.i69 = load i8* (%"struct.llvm::Any::StorageBase"*)**, i8* (%"struct.llvm::Any::StorageBase"*)*** %nonnull, align 8
  %vfn.i12.i.i70 = getelementptr inbounds i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vtable.i11.i.i69, i64 3
  %i16 = load i8* (%"struct.llvm::Any::StorageBase"*)*, i8* (%"struct.llvm::Any::StorageBase"*)** %vfn.i12.i.i70, align 8
  %call3.i13.i.i71 = call noundef i8* %i16(%"struct.llvm::Any::StorageBase"* noundef nonnull align 8 dereferenceable(8) %i7)
  %i17 = load i32, i32* %i11, align 4
  %call11 = call i32 (i8*, ...) @printf(i8* noundef nonnull dereferenceable(1) getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i64 0, i64 0), i32 noundef 42, i32 noundef %i17)
  br label %_ZN4llvm3AnyD2Ev.exit85

_ZN4llvm3AnyD2Ev.exit85:                          ; preds = %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit73, %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit60, %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit33
  %retval.0111 = phi i32 [ -1, %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit33 ], [ 0, %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit60 ], [ 0, %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit73 ]
  %A.sroa.0.0109 = phi %"struct.llvm::Any::StorageBase"* [ null, %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit33 ], [ %i7, %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit60 ], [ %i7, %_ZN4llvm8any_castIiEET_RNS_3AnyE.exit73 ]
  %i18 = bitcast i8* %call.i.i113 to void (%"struct.llvm::Any::StorageBase"*)***
  %vtable.i.i.i76 = load void (%"struct.llvm::Any::StorageBase"*)**, void (%"struct.llvm::Any::StorageBase"*)*** %i18, align 8
  %vfn.i.i.i77 = getelementptr inbounds void (%"struct.llvm::Any::StorageBase"*)*, void (%"struct.llvm::Any::StorageBase"*)** %vtable.i.i.i76, i64 1
  %i19 = load void (%"struct.llvm::Any::StorageBase"*)*, void (%"struct.llvm::Any::StorageBase"*)** %vfn.i.i.i77, align 8
  call void %i19(%"struct.llvm::Any::StorageBase"* noundef nonnull align 8 dereferenceable(8) %i2)
  %cmp.not.i.i87 = icmp eq %"struct.llvm::Any::StorageBase"* %A.sroa.0.0109, null
  br i1 %cmp.not.i.i87, label %_ZN4llvm3AnyD2Ev.exit91, label %_ZNKSt14default_deleteIN4llvm3Any11StorageBaseEEclEPS2_.exit.i.i90

_ZNKSt14default_deleteIN4llvm3Any11StorageBaseEEclEPS2_.exit.i.i90: ; preds = %_ZN4llvm3AnyD2Ev.exit85
  %i20 = bitcast %"struct.llvm::Any::StorageBase"* %A.sroa.0.0109 to void (%"struct.llvm::Any::StorageBase"*)***
  %vtable.i.i.i88 = load void (%"struct.llvm::Any::StorageBase"*)**, void (%"struct.llvm::Any::StorageBase"*)*** %i20, align 8
  %vfn.i.i.i89 = getelementptr inbounds void (%"struct.llvm::Any::StorageBase"*)*, void (%"struct.llvm::Any::StorageBase"*)** %vtable.i.i.i88, i64 1
  %i21 = load void (%"struct.llvm::Any::StorageBase"*)*, void (%"struct.llvm::Any::StorageBase"*)** %vfn.i.i.i89, align 8
  call void %i21(%"struct.llvm::Any::StorageBase"* noundef nonnull align 8 dereferenceable(8) %A.sroa.0.0109)
  br label %_ZN4llvm3AnyD2Ev.exit91

_ZN4llvm3AnyD2Ev.exit91:                          ; preds = %_ZNKSt14default_deleteIN4llvm3Any11StorageBaseEEclEPS2_.exit.i.i90, %_ZN4llvm3AnyD2Ev.exit85
  ret i32 %retval.0111
}

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(i8* nocapture noundef readonly, ...) local_unnamed_addr

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noundef nonnull i8* @_Znwm(i64 noundef) local_unnamed_addr

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr dso_local void @_ZN4llvm3Any11StorageImplIiED0Ev(%"struct.llvm::Any::StorageImpl"* noundef nonnull align 8 dereferenceable(12) %this) unnamed_addr align 2 {
entry:
  %i = bitcast %"struct.llvm::Any::StorageImpl"* %this to i8*
  tail call void @_ZdlPv(i8* noundef nonnull %i)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZNK4llvm3Any11StorageImplIiE5cloneEv(%"class.std::unique_ptr"* noalias sret(%"class.std::unique_ptr") align 8 %agg.result, %"struct.llvm::Any::StorageImpl"* noundef nonnull align 8 dereferenceable(12) %this) unnamed_addr align 2 {
_ZNSt10unique_ptrIN4llvm3Any11StorageImplIiEESt14default_deleteIS3_EED2Ev.exit:
  %Value = getelementptr inbounds %"struct.llvm::Any::StorageImpl", %"struct.llvm::Any::StorageImpl"* %this, i64 0, i32 1
  %call.i = tail call noalias noundef nonnull dereferenceable(16) i8* @_Znwm(i64 noundef 16)
  %i = bitcast i8* %call.i to %"struct.llvm::Any::StorageImpl"*
  %i1 = getelementptr inbounds %"struct.llvm::Any::StorageImpl", %"struct.llvm::Any::StorageImpl"* %i, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVN4llvm3Any11StorageImplIiEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %i1, align 8
  %Value2.i.i = getelementptr inbounds %"struct.llvm::Any::StorageImpl", %"struct.llvm::Any::StorageImpl"* %i, i64 0, i32 1
  %i2 = load i32, i32* %Value, align 8
  store i32 %i2, i32* %Value2.i.i, align 8
  %i3 = getelementptr %"struct.llvm::Any::StorageImpl", %"struct.llvm::Any::StorageImpl"* %i, i64 0, i32 0
  %_M_head_impl.i.i.i.i.i.i = getelementptr inbounds %"class.std::unique_ptr", %"class.std::unique_ptr"* %agg.result, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0
  store %"struct.llvm::Any::StorageBase"* %i3, %"struct.llvm::Any::StorageBase"** %_M_head_impl.i.i.i.i.i.i, align 8
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef i8* @_ZNK4llvm3Any11StorageImplIiE2idEv(%"struct.llvm::Any::StorageImpl"* noundef nonnull align 8 dereferenceable(12) %this) unnamed_addr align 2 {
entry:
  ret i8* @_ZN4llvm3Any6TypeIdIiE2IdE
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN4llvm3Any11StorageBaseD2Ev(%"struct.llvm::Any::StorageBase"* noundef nonnull align 8 dereferenceable(8) %this) unnamed_addr align 2 {
entry:
  ret void
}

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(i8* noundef) local_unnamed_addr

; Function Attrs: nofree noreturn nounwind
declare dso_local void @__assert_fail(i8* noundef, i8* noundef, i32 noundef, i8* noundef) local_unnamed_addr
