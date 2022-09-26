; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -disable-output -whole-program-assume -dtransanalysis -dtrans-outofboundsok=false -dtrans-usecrulecompat=true -dtrans-print-types %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -whole-program-assume -passes='require<dtransanalysis>' -dtrans-outofboundsok=false -dtrans-usecrulecompat=true -dtrans-print-types %s 2>&1 | FileCheck %s

; Regression test for CMPLRLLVM-27935.
;
; This test is to check that DTrans analysis marks "Mismatched argument use" on
; the structures "class.xercesc_2_5::RefHashTableOf.21.6409" and
; "class.xercesc_2_5::RefHashTableOf.8.6456" due to the following usage
; scenario:
;
; 1) A pointer to %"class.xercesc_2_5::ValueStoreCache.4009" is passed to a
;    function that takes a pointer to "class.xercesc_2_5::ValueStoreCache.6461".
;
; 2) A pointer to %"class.xercesc_2_5::ValueStoreCache.4009" is passed to a
;    function takes a pointer to %"class.xercesc_2_5::ValueStoreCache.6412"
;
; 3) Within the class %"class.xercesc_2_5::ValueStoreCache.4009", the field
;      %"class.xercesc_2_5::RefHashTableOf.47"* is never used.
;    Within the class, the field %"class.xercesc_2_5::ValueStoreCache.6412" the
;      field %"class.xercesc_2_5::RefHashTableOf.21.6409" is used.
;    Within the class, %"class.xercesc_2_5::ValueStoreCache.6461", the field
;      %"class.xercesc_2_5::RefHashTableOf.8.6456" is used
;
; When analyzing these bitcasts for which fields are used, DTrans analysis
; needs to take into account all the types the class is used as when crossing
; function boundaries.
;
; Otherwise the safety flags will be incorrect, and some structures will be
; eligible for transformations (as occurred in CMPLRLLVM-27935).
; However, because there is another version of this structure that used via
; the casts at the function call boundaries, the safety flag needs to be
; cacscaded to the field members that are used. This is a trimmed down
; version of the IR before DTrans for the case spec2006/483 that exposed
; the problem.

; CHECK-LABEL:  LLVMType: %"class.xercesc_2_5::RefHashTableOf.21.6409" = type { %"class.xercesc_2_5::MemoryManager"*, i8, i8**, i32, i32, i32, %"class.xercesc_2_5::HashBase"* }
; CHECK:  Name: class.xercesc_2_5::RefHashTableOf.21.6409
; CHECK:   Safety data: Mismatched argument use

; Make sure the safety data matched before the next structure is printed.
; CHECK: DTRANS_StructInfo:

; CHECK: LLVMType: %"class.xercesc_2_5::RefHashTableOf.8.6456" = type { %"class.xercesc_2_5::MemoryManager"*, i8, i8**, i32, i32, i32, %"class.xercesc_2_5::HashBase"* }
; CHECK: Name: class.xercesc_2_5::RefHashTableOf.8.6456
; CHECK:   Safety data: Mismatched argument use

; Make sure the safety data matched before the next structure is printed.
; CHECK: DTRANS_StructInfo:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.xercesc_2_5::FieldActivator" = type { %"class.xercesc_2_5::ValueStoreCache"*, i8*, i8*, %"class.xercesc_2_5::MemoryManager"* }
%"class.xercesc_2_5::FieldActivator.4012" = type { %"class.xercesc_2_5::ValueStoreCache.4009"*, i8*, i8*, %"class.xercesc_2_5::MemoryManager"* }

%"class.xercesc_2_5::ValueStoreCache" = type { %"class.xercesc_2_5::RefVectorOf"*, %"class.xercesc_2_5::RefHashTableOf.3373"*, %"class.xercesc_2_5::RefHash2KeysTableOf.3375"*, %"class.xercesc_2_5::RefStackOf.6460"*, %"class.xercesc_2_5::XMLScanner"*, %"class.xercesc_2_5::MemoryManager"* }
%"class.xercesc_2_5::ValueStoreCache.4009" = type { %"class.xercesc_2_5::RefVectorOf"*, %"class.xercesc_2_5::RefHashTableOf.47"*, %"class.xercesc_2_5::RefHash2KeysTableOf.49"*, %"class.xercesc_2_5::RefStackOf.6460"*, %"class.xercesc_2_5::XMLScanner"*, %"class.xercesc_2_5::MemoryManager"* }
%"class.xercesc_2_5::ValueStoreCache.6412" = type { %"class.xercesc_2_5::RefVectorOf.20"*, %"class.xercesc_2_5::RefHashTableOf.21.6409"*, %"class.xercesc_2_5::RefHash2KeysTableOf.55"*, %"class.xercesc_2_5::RefStackOf.6460"*, %"class.xercesc_2_5::XMLScanner"*, %"class.xercesc_2_5::MemoryManager"* }
%"class.xercesc_2_5::ValueStoreCache.6461" = type { %"class.xercesc_2_5::RefVectorOf.6453"*, %"class.xercesc_2_5::RefHashTableOf.8.6456"*, %"class.xercesc_2_5::RefHash2KeysTableOf.6458"*, %"class.xercesc_2_5::RefStackOf.6460"*, %"class.xercesc_2_5::XMLScanner"*, %"class.xercesc_2_5::MemoryManager"* }

%"class.xercesc_2_5::RefVectorOf" = type { i8 }
%"class.xercesc_2_5::RefVectorOf.20" = type { i8 }
%"class.xercesc_2_5::RefVectorOf.6453" = type { i8 }

%"class.xercesc_2_5::RefHashTableOf.21.6409" = type { %"class.xercesc_2_5::MemoryManager"*, i8, i8**, i32, i32, i32, %"class.xercesc_2_5::HashBase"* }
%"class.xercesc_2_5::RefHashTableOf.3373" = type { %"class.xercesc_2_5::MemoryManager"*, i8, i8**, i32, i32, i32, %"class.xercesc_2_5::HashBase"* }
%"class.xercesc_2_5::RefHashTableOf.47" = type { %"class.xercesc_2_5::MemoryManager"*, i8, i8**, i32, i32, i32, %"class.xercesc_2_5::HashBase"* }
%"class.xercesc_2_5::RefHashTableOf.8.6456" = type { %"class.xercesc_2_5::MemoryManager"*, i8, i8**, i32, i32, i32, %"class.xercesc_2_5::HashBase"* }

%"class.xercesc_2_5::RefHash2KeysTableOf.3375" = type { %"class.xercesc_2_5::MemoryManager"*, i8, i8**, i32, %"class.xercesc_2_5::HashBase"* }
%"class.xercesc_2_5::RefHash2KeysTableOf.49" = type { %"class.xercesc_2_5::MemoryManager"*, i8, i8**, i32, %"class.xercesc_2_5::HashBase"* }
%"class.xercesc_2_5::RefHash2KeysTableOf.55" = type { %"class.xercesc_2_5::MemoryManager"*, i8, i8**, i32, %"class.xercesc_2_5::HashBase"* }
%"class.xercesc_2_5::RefHash2KeysTableOf.6458" = type { %"class.xercesc_2_5::MemoryManager"*, i8, i8**, i32, %"class.xercesc_2_5::HashBase"* }

%"class.xercesc_2_5::RefStackOf.6460" = type { [8 x i8], i8 }

%"class.xercesc_2_5::HashBase" = type { i32 (...)** }

%"class.xercesc_2_5::IGXMLScanner" = type { %"class.xercesc_2_5::XMLScanner", i8, i32, i32, i32*, i8, i8*, i8*, i8*, i8*, i8*, %"class.xercesc_2_5::ValueStoreCache.4009"*, %"class.xercesc_2_5::FieldActivator.4012"*, i8*, i8*, i8*, i32, i8*, i8*, %"class.xercesc_2_5::RefHash2KeysTableOf.55"*, i8*, i8*, i8*, i8*, i8 }

%"class.xercesc_2_5::IdentityConstraint" = type <{ i8, i16*, i16*, i8*, i8*, %"class.xercesc_2_5::MemoryManager"*, i32, [4 x i8] }>
%"class.xercesc_2_5::IdentityConstraint.3994" = type <{ i8, i16*, i16*, i8*, i8*, %"class.xercesc_2_5::MemoryManager"*, i32, [4 x i8] }>

%"class.xercesc_2_5::MemoryManager" = type { i32 (...)** }

%"class.xercesc_2_5::ValueStore" = type { i8, i32, %"class.xercesc_2_5::IdentityConstraint"*, i8, i8*, %"class.xercesc_2_5::ValueStore"*, %"class.xercesc_2_5::XMLScanner"*, %"class.xercesc_2_5::MemoryManager"* }
%"class.xercesc_2_5::ValueStore.4008" = type { i8, i32, %"class.xercesc_2_5::IdentityConstraint.3994"*, i8, %"class.xercesc_2_5::RefVectorOf"*, %"class.xercesc_2_5::ValueStore.4008"*, %"class.xercesc_2_5::XMLScanner"*, %"class.xercesc_2_5::MemoryManager"* }
%"class.xercesc_2_5::ValueStore.6405" = type { i8, i32, %"class.xercesc_2_5::IdentityConstraint"*, i8, i8*, %"class.xercesc_2_5::ValueStore.6405"*, %"class.xercesc_2_5::XMLScanner"*, %"class.xercesc_2_5::MemoryManager"* }

%"class.xercesc_2_5::XMLScanner" = type { i32 (...)**, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i32, i32, i32, i32, i32, i32, i32, i32, i32**, i32, i32, i32, i32, i32, %"class.xercesc_2_5::RefVectorOf"*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8, i8, i8*, i32, i8*, %"class.xercesc_2_5::MemoryManager"*, i8*, i8*, i8*, i16*, i16*, i16*, i8*, i32, %"class.xercesc_2_5::MemoryManager"*, i8, i8, i8, i8, i8, i8, i8, i8 }

@_ZTIN11xercesc_2_520OutOfMemoryExceptionE = external hidden constant { i8*, i8*, i8* }, align 8

; Function Attrs: nofree nosync nounwind readnone willreturn
declare i1 @llvm.type.test(i8*, metadata) #0

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #1

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local void @__clang_call_terminate(i8*)

declare dso_local i8* @__cxa_begin_catch(i8*)

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #2

declare dso_local void @__cxa_rethrow()

declare dso_local void @__cxa_end_catch()

declare dso_local noalias i8* @__cxa_allocate_exception(i64)

declare dso_local void @__cxa_throw(i8*, i8*, i8*)

declare dso_local void @__cxa_free_exception(i8*)

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #4

define hidden void @_ZN11xercesc_2_512IGXMLScanner10commonInitEv(%"class.xercesc_2_5::IGXMLScanner"* nonnull dereferenceable(904) %arg) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
bb:
  %i = getelementptr inbounds %"class.xercesc_2_5::IGXMLScanner", %"class.xercesc_2_5::IGXMLScanner"* %arg, i64 0, i32 0
  %i1 = getelementptr inbounds %"class.xercesc_2_5::XMLScanner", %"class.xercesc_2_5::XMLScanner"* %i, i64 0, i32 53
  %i44 = load %"class.xercesc_2_5::MemoryManager"*, %"class.xercesc_2_5::MemoryManager"** %i1, align 8
  %i45 = tail call i8* @_ZN11xercesc_2_57XMemorynwEmPNS_13MemoryManagerE(i64 48, %"class.xercesc_2_5::MemoryManager"* %i44)
  %i50 = load %"class.xercesc_2_5::MemoryManager"*, %"class.xercesc_2_5::MemoryManager"** %i1, align 8
  %i51 = tail call i8* @_ZN11xercesc_2_57XMemorynwEmPNS_13MemoryManagerE(i64 184, %"class.xercesc_2_5::MemoryManager"* %i50)
  %i57 = load %"class.xercesc_2_5::MemoryManager"*, %"class.xercesc_2_5::MemoryManager"** %i1, align 8
  %i58 = tail call i8* @_ZN11xercesc_2_57XMemorynwEmPNS_13MemoryManagerE(i64 24, %"class.xercesc_2_5::MemoryManager"* %i57)
  %i63 = load %"class.xercesc_2_5::MemoryManager"*, %"class.xercesc_2_5::MemoryManager"** %i1, align 8
  %i64 = tail call i8* @_ZN11xercesc_2_57XMemorynwEmPNS_13MemoryManagerE(i64 48, %"class.xercesc_2_5::MemoryManager"* %i63)
  %i65 = bitcast i8* %i64 to %"class.xercesc_2_5::ValueStoreCache.4009"*
  %i66 = load %"class.xercesc_2_5::MemoryManager"*, %"class.xercesc_2_5::MemoryManager"** %i1, align 8
  invoke void bitcast (void (%"class.xercesc_2_5::ValueStoreCache.6461"*, %"class.xercesc_2_5::MemoryManager"*)* @_ZN11xercesc_2_515ValueStoreCacheC2EPNS_13MemoryManagerE to void (%"class.xercesc_2_5::ValueStoreCache.4009"*, %"class.xercesc_2_5::MemoryManager"*)*)(%"class.xercesc_2_5::ValueStoreCache.4009"* nonnull dereferenceable(48) %i65, %"class.xercesc_2_5::MemoryManager"* %i66)
          to label %bb67 unwind label %bb308

bb67:                                             ; preds = %bb
  ret void

bb308:                                            ; preds = %bb
  %i309 = landingpad { i8*, i32 }
          cleanup
  br label %bb340

bb324:                                            ; No predecessors!
  br label %bb340

bb340:                                            ; preds = %bb324, %bb308
  resume { i8*, i32 } zeroinitializer
}

define hidden void @_ZN11xercesc_2_512IGXMLScanner10scanEndTagERb(%"class.xercesc_2_5::IGXMLScanner"* nonnull dereferenceable(904) %arg, i8* nocapture nonnull align 1 dereferenceable(1) %arg1) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %i427 = getelementptr inbounds %"class.xercesc_2_5::IGXMLScanner", %"class.xercesc_2_5::IGXMLScanner"* %arg, i64 0, i32 11
  %i460 = load %"class.xercesc_2_5::ValueStoreCache.4009"*, %"class.xercesc_2_5::ValueStoreCache.4009"** %i427, align 8
  call void bitcast (void (%"class.xercesc_2_5::ValueStoreCache.6461"*)* @_ZN11xercesc_2_515ValueStoreCache10transplantEPNS_18IdentityConstraintEi to void (%"class.xercesc_2_5::ValueStoreCache.4009"*)*)(%"class.xercesc_2_5::ValueStoreCache.4009"* nonnull dereferenceable(48) %i460)
  %i481 = load %"class.xercesc_2_5::ValueStoreCache.4009"*, %"class.xercesc_2_5::ValueStoreCache.4009"** %i427, align 8
  call void bitcast (void (%"class.xercesc_2_5::ValueStoreCache.6461"*)* @_ZN11xercesc_2_515ValueStoreCache10endElementEv to void (%"class.xercesc_2_5::ValueStoreCache.4009"*)*)(%"class.xercesc_2_5::ValueStoreCache.4009"* nonnull dereferenceable(48) %i481)
  %i513 = load %"class.xercesc_2_5::ValueStoreCache.4009"*, %"class.xercesc_2_5::ValueStoreCache.4009"** %i427, align 8
  %i530 = call %"class.xercesc_2_5::ValueStore.4008"* @_ZN11xercesc_2_515ValueStoreCache16getValueStoreForEPKNS_18IdentityConstraintEi(%"class.xercesc_2_5::ValueStoreCache.4009"* nonnull dereferenceable(48) %i513, %"class.xercesc_2_5::IdentityConstraint.3994"* null, i32 0)
  %i533 = load %"class.xercesc_2_5::ValueStoreCache.4009"*, %"class.xercesc_2_5::ValueStoreCache.4009"** %i427, align 8
  call void bitcast (void (%"class.xercesc_2_5::ValueStore.6405"*, %"class.xercesc_2_5::ValueStoreCache.6412"*)* @_ZN11xercesc_2_510ValueStore20endDcocumentFragmentEPNS_15ValueStoreCacheE to void (%"class.xercesc_2_5::ValueStore.4008"*, %"class.xercesc_2_5::ValueStoreCache.4009"*)*)(%"class.xercesc_2_5::ValueStore.4008"* nonnull dereferenceable(80) %i530, %"class.xercesc_2_5::ValueStoreCache.4009"* %i533)
  ret void
}

define %"class.xercesc_2_5::ValueStore.4008"* @_ZN11xercesc_2_515ValueStoreCache16getValueStoreForEPKNS_18IdentityConstraintEi(%"class.xercesc_2_5::ValueStoreCache.4009"* nocapture nonnull readonly dereferenceable(48) %0, %"class.xercesc_2_5::IdentityConstraint.3994"* %1, i32 %2) {
  ret %"class.xercesc_2_5::ValueStore.4008"* null
}

define hidden void @_ZN11xercesc_2_510ValueStore20endDcocumentFragmentEPNS_15ValueStoreCacheE(%"class.xercesc_2_5::ValueStore.6405"* nocapture nonnull dereferenceable(80) %arg, %"class.xercesc_2_5::ValueStoreCache.6412"* nocapture readonly %arg1) personality i32 (...)* @__gxx_personality_v0 {
  %i31 = getelementptr inbounds %"class.xercesc_2_5::ValueStoreCache.6412", %"class.xercesc_2_5::ValueStoreCache.6412"* %arg1, i64 0, i32 1
  %i32 = load %"class.xercesc_2_5::RefHashTableOf.21.6409"*, %"class.xercesc_2_5::RefHashTableOf.21.6409"** %i31, align 8
  %i33 = getelementptr inbounds %"class.xercesc_2_5::RefHashTableOf.21.6409", %"class.xercesc_2_5::RefHashTableOf.21.6409"* %i32, i64 0, i32 6
  %i34 = load %"class.xercesc_2_5::HashBase"*, %"class.xercesc_2_5::HashBase"** %i33, align 8
  %i35 = icmp eq %"class.xercesc_2_5::HashBase"* null, %i34
  br i1 %i35, label %bb47, label %bb49

bb47:                                             ; preds = %0
  ret void

bb49:                                             ; preds = %0
  store %"class.xercesc_2_5::HashBase"* null, %"class.xercesc_2_5::HashBase"** %i33, align 8
  ret void
}

define void @_ZN11xercesc_2_515ValueStoreCache7cleanUpEv(%"class.xercesc_2_5::ValueStoreCache.6461"* nocapture nonnull readonly dereferenceable(48) %0) {
  ret void
}

define hidden void @_ZN11xercesc_2_515ValueStoreCache4initEv(%"class.xercesc_2_5::ValueStoreCache.6461"* nocapture nonnull dereferenceable(48) %0) {
  ret void
}

define hidden void @_ZN11xercesc_2_515ValueStoreCacheC2EPNS_13MemoryManagerE(%"class.xercesc_2_5::ValueStoreCache.6461"* nocapture nonnull dereferenceable(48) %arg, %"class.xercesc_2_5::MemoryManager"* %arg1) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
bb:
  %i = getelementptr inbounds %"class.xercesc_2_5::ValueStoreCache.6461", %"class.xercesc_2_5::ValueStoreCache.6461"* %arg, i64 0, i32 0
  store %"class.xercesc_2_5::RefVectorOf.6453"* null, %"class.xercesc_2_5::RefVectorOf.6453"** %i, align 8
  %i2 = getelementptr inbounds %"class.xercesc_2_5::ValueStoreCache.6461", %"class.xercesc_2_5::ValueStoreCache.6461"* %arg, i64 0, i32 1
  store %"class.xercesc_2_5::RefHashTableOf.8.6456"* null, %"class.xercesc_2_5::RefHashTableOf.8.6456"** %i2, align 8
  %i3 = getelementptr inbounds %"class.xercesc_2_5::ValueStoreCache.6461", %"class.xercesc_2_5::ValueStoreCache.6461"* %arg, i64 0, i32 2
  store %"class.xercesc_2_5::RefHash2KeysTableOf.6458"* null, %"class.xercesc_2_5::RefHash2KeysTableOf.6458"** %i3, align 8
  %i4 = getelementptr inbounds %"class.xercesc_2_5::ValueStoreCache.6461", %"class.xercesc_2_5::ValueStoreCache.6461"* %arg, i64 0, i32 3
  store %"class.xercesc_2_5::RefStackOf.6460"* null, %"class.xercesc_2_5::RefStackOf.6460"** %i4, align 8
  %i5 = getelementptr inbounds %"class.xercesc_2_5::ValueStoreCache.6461", %"class.xercesc_2_5::ValueStoreCache.6461"* %arg, i64 0, i32 4
  store %"class.xercesc_2_5::XMLScanner"* null, %"class.xercesc_2_5::XMLScanner"** %i5, align 8
  %i6 = getelementptr inbounds %"class.xercesc_2_5::ValueStoreCache.6461", %"class.xercesc_2_5::ValueStoreCache.6461"* %arg, i64 0, i32 5
  store %"class.xercesc_2_5::MemoryManager"* %arg1, %"class.xercesc_2_5::MemoryManager"** %i6, align 8
  invoke void @_ZN11xercesc_2_515ValueStoreCache4initEv(%"class.xercesc_2_5::ValueStoreCache.6461"* nonnull dereferenceable(48) %arg)
          to label %bb21 unwind label %bb7

bb7:                                              ; preds = %bb
  %i8 = landingpad { i8*, i32 }
          catch i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN11xercesc_2_520OutOfMemoryExceptionE to i8*)
          catch i8* null
  %i9 = extractvalue { i8*, i32 } %i8, 0
  %i10 = extractvalue { i8*, i32 } %i8, 1
  %i11 = tail call i32 @llvm.eh.typeid.for(i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN11xercesc_2_520OutOfMemoryExceptionE to i8*))
  %i12 = icmp eq i32 %i10, %i11
  br i1 %i12, label %bb13, label %bb15

bb13:                                             ; preds = %bb7
  %i14 = tail call i8* @__cxa_begin_catch(i8* %i9)
  invoke void @__cxa_rethrow()
          to label %bb27 unwind label %bb19

bb15:                                             ; preds = %bb7
  %i16 = tail call i8* @__cxa_begin_catch(i8* %i9)
  tail call void @_ZN11xercesc_2_515ValueStoreCache7cleanUpEv(%"class.xercesc_2_5::ValueStoreCache.6461"* nonnull dereferenceable(48) %arg)
  invoke void @__cxa_rethrow()
          to label %bb27 unwind label %bb17

bb17:                                             ; preds = %bb15
  %i18 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb22 unwind label %bb24

bb19:                                             ; preds = %bb13
  %i20 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb22 unwind label %bb24

bb21:                                             ; preds = %bb
  ret void

bb22:                                             ; preds = %bb19, %bb17
  resume { i8*, i32 } zeroinitializer

bb24:                                             ; preds = %bb19, %bb17
  %i25 = landingpad { i8*, i32 }
          catch i8* null
  %i26 = extractvalue { i8*, i32 } %i25, 0
  tail call void @__clang_call_terminate(i8* %i26)
  unreachable

bb27:                                             ; preds = %bb15, %bb13
  unreachable
}

define void @_ZN11xercesc_2_515ValueStoreCache10endElementEv(%"class.xercesc_2_5::ValueStoreCache.6461"* nocapture nonnull readonly dereferenceable(48) %0) {
  ret void
}

define void @_ZN11xercesc_2_515ValueStoreCache10transplantEPNS_18IdentityConstraintEi(%"class.xercesc_2_5::ValueStoreCache.6461"* %0) {
  ret void
}

declare dso_local i8* @_ZN11xercesc_2_57XMemorynwEmPNS_13MemoryManagerE(i64, %"class.xercesc_2_5::MemoryManager"*)

attributes #0 = { nofree nosync nounwind readnone willreturn }
attributes #1 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind readnone }
attributes #3 = { argmemonly nofree nosync nounwind willreturn }
attributes #4 = { argmemonly nofree nosync nounwind willreturn writeonly }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"Virtual Function Elim", i32 0}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 1, !"LTOPostLink", i32 1}
