; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-opt-predicate -disable-output -print-after=hir-opt-predicate < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -xmain-opt-level=3 -disable-output < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
; + UNKNOWN LOOP i1
; |   <i1 = 0>
; |   for.body:
; |   %7 = (@_ZN11cSimulation5evPtrE)[0];
; |   if ((%7)[0].1 == 0)
; |   {
; |   ... code ...
; |     %7 = ...
; |   ... code ...
; |   }
; |   %call.i = @_ZSt18_Rb_tree_incrementPSt18_Rb_tree_node_base(&((%iter.sroa.0.090)[0]));
; |   %iter.sroa.0.090 = &((%call.i)[0]);
; |   if (&((%call.i)[0]) != &((%this)[0].4.0.0.1))
; |   {
; |      <i1 = i1 + 1>
; |      goto for.body;
; |   }
; + END LOOP
; END REGION

; CHECK: Function
; CHECK: BEGIN REGION { modified }
; CHECK: %7 = (@_ZN11cSimulation5evPtrE)[0];
; CHECK: if ((%7)[0].1 == 0)
; CHECK: {
; CHECK:   + UNKNOWN LOOP i1
; CHECK:   |   <i1 = 0>
; CHECK:   |   %7 = (@_ZN11cSimulation5evPtrE)[0];
; CHECK:   |   if ((%7)[0].1 == 0)
; CHECK:   |   {
;          |   ... code ...
; CHECK:   |   }
; CHECK:   |   %call.i = @_ZSt18_Rb_tree_incrementPSt18_Rb_tree_node_base(&((%iter.sroa.0.090)[0]));
; CHECK:   |   %iter.sroa.0.090 = &((%call.i)[0]);
; CHECK:   |   if (&((%call.i)[0]) != &((%this)[0].4.0.0.1))
; CHECK:   |   {
; CHECK:   |      <i1 = i1 + 1>
; CHECK:   |   }
; CHECK:   + END LOOP
; CHECK: }
; CHECK-NOT:   + UNKNOWN LOOP i1
; CHECK: END REGION

;Module Before HIR; ModuleID = '/export/iusers/pgprokof/tests/tc_icx/benchspec/CPU/520.omnetpp_r/build/build_base_empty.0000/_ZN16MACRelayUnitBase17printAddressTableEv.ll'
source_filename = "model/MACRelayUnitBase.cc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.cEnvir = type { i32 (...)**, i8, i8, i8, %"class.std::basic_ostream" }
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
%class.cSimulation = type { %class.cNoncopyableOwnedObject.base, i32, i32, %class.cModule**, i32, %class.cEnvir*, %class.cModule*, %class.cSimpleModule*, %class.cComponent*, i32, %class.cModuleType*, %class.cScheduler*, %class.SimTime, i64, %class.cMessage*, %class.cException*, %class.cHasher*, %class.cMessageHeap }
%class.cNoncopyableOwnedObject.base = type { %class.cOwnedObject.base }
%class.cOwnedObject.base = type <{ %class.cNamedObject.base, [4 x i8], %class.cObject*, i32 }>
%class.cNamedObject.base = type <{ %class.cObject, i8*, i16, i16 }>
%class.cObject = type { i32 (...)** }
%class.cModule = type { %class.cComponent, i8*, i32, %class.cModule*, %class.cModule*, %class.cModule*, %class.cModule*, i32, %"struct.cGate::Desc"*, i32, i32 }
%class.cComponent = type { %class.cDefaultList, %class.cComponentType*, i16, i32*, i16, i16, %class.cPar*, %class.cDisplayString* }
%class.cDefaultList = type { %class.cNoncopyableOwnedObject.base, %class.cOwnedObject**, i32, i32 }
%class.cOwnedObject = type <{ %class.cNamedObject.base, [4 x i8], %class.cObject*, i32, [4 x i8] }>
%class.cComponentType = type { %class.cNoncopyableOwnedObject.base, %"class.std::basic_string", %"class.std::map", %"class.std::set" }
%"class.std::basic_string" = type { %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" }
%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" = type { i8* }
%"class.std::map" = type { %"class.std::_Rb_tree" }
%"class.std::_Rb_tree" = type { %"struct.std::_Rb_tree<std::basic_string<char>, std::pair<const std::basic_string<char>, cParImpl *>, std::_Select1st<std::pair<const std::basic_string<char>, cParImpl *> >, std::less<std::basic_string<char> >, std::allocator<std::pair<const std::basic_string<char>, cParImpl *> > >::_Rb_tree_impl" }
%"struct.std::_Rb_tree<std::basic_string<char>, std::pair<const std::basic_string<char>, cParImpl *>, std::_Select1st<std::pair<const std::basic_string<char>, cParImpl *> >, std::less<std::basic_string<char> >, std::allocator<std::pair<const std::basic_string<char>, cParImpl *> > >::_Rb_tree_impl" = type { %"struct.std::less", %"struct.std::_Rb_tree_node_base", i64 }
%"struct.std::less" = type { i8 }
%"struct.std::_Rb_tree_node_base" = type { i32, %"struct.std::_Rb_tree_node_base"*, %"struct.std::_Rb_tree_node_base"*, %"struct.std::_Rb_tree_node_base"* }
%"class.std::set" = type { %"class.std::_Rb_tree.3" }
%"class.std::_Rb_tree.3" = type { %"struct.std::_Rb_tree<cParImpl *, cParImpl *, std::_Identity<cParImpl *>, cComponentType::Less, std::allocator<cParImpl *> >::_Rb_tree_impl" }
%"struct.std::_Rb_tree<cParImpl *, cParImpl *, std::_Identity<cParImpl *>, cComponentType::Less, std::allocator<cParImpl *> >::_Rb_tree_impl" = type { %"struct.cComponentType::Less", %"struct.std::_Rb_tree_node_base", i64 }
%"struct.cComponentType::Less" = type { i8 }
%class.cPar = type { %class.cObject, %class.cComponent*, %class.cParImpl* }
%class.cParImpl = type { %class.cNamedObject.base, i8* }
%class.cDisplayString = type { i8*, i8*, %"struct.cDisplayString::Tag"*, i32, i8*, i8, %class.cComponent* }
%"struct.cDisplayString::Tag" = type { i8*, i32, [16 x i8*] }
%"struct.cGate::Desc" = type { %class.cModule*, %"struct.cGate::Name"*, i32, %union.anon, %union.anon.7 }
%"struct.cGate::Name" = type <{ %class.opp_string, %class.opp_string, %class.opp_string, i32, [4 x i8] }>
%class.opp_string = type { i8* }
%union.anon = type { %class.cGate* }
%class.cGate = type { %class.cObject, %"struct.cGate::Desc"*, i32, %class.cChannel*, %class.cGate*, %class.cGate* }
%class.cChannel = type <{ %class.cComponent, %class.cGate*, i32, [4 x i8] }>
%union.anon.7 = type { %class.cGate* }
%class.cSimpleModule = type { %class.cModule, %class.cMessage*, %class.cCoroutine* }
%class.cCoroutine = type { i32 (...)**, %struct._Task* }
%struct._Task = type opaque
%class.cModuleType = type { %class.cComponentType }
%class.cScheduler = type { %class.cObject, %class.cSimulation* }
%class.SimTime = type { i64 }
%class.cMessage = type { %class.cOwnedObject.base, i16, i16, i16, %class.cArray*, %class.cObject*, i8*, i32, i32, i32, i32, %class.SimTime, %class.SimTime, %class.SimTime, %class.SimTime, i32, i64, i64, i64, i64 }
%class.cArray = type { %class.cOwnedObject.base, %class.cObject**, i32, i32, i32, i32 }
%class.cException = type <{ %"class.std::exception", i32, [4 x i8], %"class.std::basic_string", i8, [7 x i8], %"class.std::basic_string", %"class.std::basic_string", i32, [4 x i8] }>
%"class.std::exception" = type { i32 (...)** }
%class.cHasher = type { i32 }
%class.cMessageHeap = type { %class.cOwnedObject.base, %class.cMessage**, i32, i32, i64 }
%class.MACAddress = type { [6 x i8] }
%class.MACRelayUnitBase = type <{ %class.cSimpleModule, i32, i32, %class.SimTime, %"class.std::map.8", i32, [4 x i8] }>
%"class.std::map.8" = type { %"class.std::_Rb_tree.9" }
%"class.std::_Rb_tree.9" = type { %"struct.std::_Rb_tree<MACAddress, std::pair<const MACAddress, MACRelayUnitBase::AddressEntry>, std::_Select1st<std::pair<const MACAddress, MACRelayUnitBase::AddressEntry> >, MACRelayUnitBase::MAC_compare, std::allocator<std::pair<const MACAddress, MACRelayUnitBase::AddressEntry> > >::_Rb_tree_impl" }
%"struct.std::_Rb_tree<MACAddress, std::pair<const MACAddress, MACRelayUnitBase::AddressEntry>, std::_Select1st<std::pair<const MACAddress, MACRelayUnitBase::AddressEntry> >, MACRelayUnitBase::MAC_compare, std::allocator<std::pair<const MACAddress, MACRelayUnitBase::AddressEntry> > >::_Rb_tree_impl" = type { %"struct.MACRelayUnitBase::MAC_compare", %"struct.std::_Rb_tree_node_base", i64 }
%"struct.MACRelayUnitBase::MAC_compare" = type { i8 }
%"struct.std::_Rb_tree_node" = type { %"struct.std::_Rb_tree_node_base", %"struct.std::pair" }
%"struct.std::pair" = type { %class.MACAddress, %"struct.MACRelayUnitBase::AddressEntry" }
%"struct.MACRelayUnitBase::AddressEntry" = type { i32, %class.SimTime }

@.str.16 = external hidden unnamed_addr constant [16 x i8], align 1
@.str.17 = external hidden unnamed_addr constant [12 x i8], align 1
@.str.18 = external hidden unnamed_addr constant [3 x i8], align 1
@.str.19 = external hidden unnamed_addr constant [10 x i8], align 1
@.str.20 = external hidden unnamed_addr constant [8 x i8], align 1
@.str.21 = external hidden unnamed_addr constant [1 x i8], align 1
@_ZN11cSimulation5evPtrE = external local_unnamed_addr global %class.cEnvir*, align 8
@_ZN11cSimulation6simPtrE = external local_unnamed_addr global %class.cSimulation*, align 8

; Function Attrs: nounwind memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #0

; Function Attrs: nounwind memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #0

; Function Attrs: uwtable
declare dereferenceable(288) %class.cEnvir* @_ZN6cEnvirlsI10MACAddressEERS_RKT_(%class.cEnvir*, %class.MACAddress* dereferenceable(6)) local_unnamed_addr #1 align 2

; Function Attrs: uwtable
define void @_ZN16MACRelayUnitBase17printAddressTableEv(%class.MACRelayUnitBase* %this) unnamed_addr #1 align 2 {
entry:
  %tmp.i = alloca %class.SimTime, align 8
  %0 = load %class.cEnvir*, %class.cEnvir** @_ZN11cSimulation5evPtrE, align 8
  %disable_tracing.i = getelementptr inbounds %class.cEnvir, %class.cEnvir* %0, i64 0, i32 1
  %1 = load i8, i8* %disable_tracing.i, align 8
  %tobool.i = icmp eq i8 %1, 0
  br i1 %tobool.i, label %cond.false, label %cleanup.done

cond.false:                                       ; preds = %entry
  %out.i56 = getelementptr inbounds %class.cEnvir, %class.cEnvir* %0, i64 0, i32 4
  %call1.i.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %out.i56, i8* nonnull getelementptr inbounds ([16 x i8], [16 x i8]* @.str.16, i64 0, i64 0), i64 15)
  %_M_node_count.i.i = getelementptr inbounds %class.MACRelayUnitBase, %class.MACRelayUnitBase* %this, i64 0, i32 4, i32 0, i32 0, i32 2
  %2 = load i64, i64* %_M_node_count.i.i, align 8
  %call.i.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertImEERSoT_(%"class.std::basic_ostream"* nonnull %out.i56, i64 %2)
  %call1.i.i61 = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %out.i56, i8* nonnull getelementptr inbounds ([12 x i8], [12 x i8]* @.str.17, i64 0, i64 0), i64 11)
  br label %cleanup.done

cleanup.done:                                     ; preds = %cond.false, %entry
  %_M_left.i.i = getelementptr inbounds %class.MACRelayUnitBase, %class.MACRelayUnitBase* %this, i64 0, i32 4, i32 0, i32 0, i32 1, i32 2
  %3 = bitcast %"struct.std::_Rb_tree_node_base"** %_M_left.i.i to %"struct.std::_Rb_tree_node"**
  %4 = load %"struct.std::_Rb_tree_node"*, %"struct.std::_Rb_tree_node"** %3, align 8
  %5 = getelementptr inbounds %"struct.std::_Rb_tree_node", %"struct.std::_Rb_tree_node"* %4, i64 0, i32 0
  %_M_header.i.i = getelementptr inbounds %class.MACRelayUnitBase, %class.MACRelayUnitBase* %this, i64 0, i32 4, i32 0, i32 0, i32 1
  %cmp.i89 = icmp eq %"struct.std::_Rb_tree_node_base"* %5, %_M_header.i.i
  br i1 %cmp.i89, label %for.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %cleanup.done
  %agingTime = getelementptr inbounds %class.MACRelayUnitBase, %class.MACRelayUnitBase* %this, i64 0, i32 3
  %6 = bitcast %class.SimTime* %tmp.i to i8*
  %t2.i.i.i = getelementptr inbounds %class.SimTime, %class.SimTime* %tmp.i, i64 0, i32 0
  %t2.i.i4.i = getelementptr inbounds %class.SimTime, %class.SimTime* %agingTime, i64 0, i32 0
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %iter.sroa.0.090 = phi %"struct.std::_Rb_tree_node_base"* [ %5, %for.body.lr.ph ], [ %call.i, %for.inc ]
  %7 = load %class.cEnvir*, %class.cEnvir** @_ZN11cSimulation5evPtrE, align 8
  %disable_tracing.i66 = getelementptr inbounds %class.cEnvir, %class.cEnvir* %7, i64 0, i32 1
  %8 = load i8, i8* %disable_tracing.i66, align 8
  %tobool.i67 = icmp eq i8 %8, 0
  br i1 %tobool.i67, label %cond.false21, label %for.inc

cond.false21:                                     ; preds = %for.body
  %out.i68 = getelementptr inbounds %class.cEnvir, %class.cEnvir* %7, i64 0, i32 4
  %call1.i.i70 = call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %out.i68, i8* nonnull getelementptr inbounds ([3 x i8], [3 x i8]* @.str.18, i64 0, i64 0), i64 2)
  %_M_value_field.i = getelementptr inbounds %"struct.std::_Rb_tree_node_base", %"struct.std::_Rb_tree_node_base"* %iter.sroa.0.090, i64 1
  %first = bitcast %"struct.std::_Rb_tree_node_base"* %_M_value_field.i to %class.MACAddress*
  %call25 = call dereferenceable(288) %class.cEnvir* @_ZN6cEnvirlsI10MACAddressEERS_RKT_(%class.cEnvir* nonnull %7, %class.MACAddress* nonnull dereferenceable(6) %first)
  %out.i71 = getelementptr inbounds %class.cEnvir, %class.cEnvir* %call25, i64 0, i32 4
  %call1.i.i73 = call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %out.i71, i8* nonnull getelementptr inbounds ([10 x i8], [10 x i8]* @.str.19, i64 0, i64 0), i64 9)
  %portno = getelementptr inbounds %"struct.std::_Rb_tree_node_base", %"struct.std::_Rb_tree_node_base"* %iter.sroa.0.090, i64 1, i32 1
  %9 = bitcast %"struct.std::_Rb_tree_node_base"** %portno to i32*
  %10 = load i32, i32* %9, align 4
  %call.i76 = call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"* nonnull %out.i71, i32 %10)
  %insertionTime = getelementptr inbounds %"struct.std::_Rb_tree_node_base", %"struct.std::_Rb_tree_node_base"* %iter.sroa.0.090, i64 1, i32 2
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %6)
  %t.i.i.i = bitcast %"struct.std::_Rb_tree_node_base"** %insertionTime to i64*
  %11 = load i64, i64* %t.i.i.i, align 8
  %12 = load i64, i64* %t2.i.i4.i, align 8
  %xor.i.i.i.i = xor i64 %12, %11
  %cmp.i.i.i.i = icmp slt i64 %xor.i.i.i.i, 0
  %add.i.i.i = add nsw i64 %12, %11
  store i64 %add.i.i.i, i64* %t2.i.i.i, align 8
  %xor.i11.i.i.i = xor i64 %add.i.i.i, %12
  %cmp.i12.i.i.i = icmp sgt i64 %xor.i11.i.i.i, -1
  %or.cond.i = or i1 %cmp.i.i.i.i, %cmp.i12.i.i.i
  br i1 %or.cond.i, label %_ZN6cEnvirlsIPKcEERS_RKT_.exit, label %if.then.i.i.i79

if.then.i.i.i79:                                  ; preds = %cond.false21
  call void @_ZN7SimTime14overflowAddingERKS_(%class.SimTime* nonnull %tmp.i, %class.SimTime* nonnull dereferenceable(8) %agingTime)
  %.pre.i = load i64, i64* %t2.i.i.i, align 8
  br label %_ZN6cEnvirlsIPKcEERS_RKT_.exit

_ZN6cEnvirlsIPKcEERS_RKT_.exit:                   ; preds = %if.then.i.i.i79, %cond.false21
  %13 = phi i64 [ %add.i.i.i, %cond.false21 ], [ %.pre.i, %if.then.i.i.i79 ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %6)
  %14 = load %class.cSimulation*, %class.cSimulation** @_ZN11cSimulation6simPtrE, align 8
  %t.i.i.i.i = getelementptr inbounds %class.cSimulation, %class.cSimulation* %14, i64 0, i32 12, i32 0
  %15 = load i64, i64* %t.i.i.i.i, align 8
  %cmp.i78 = icmp sle i64 %13, %15
  %cond = select i1 %cmp.i78, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.20, i64 0, i64 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.21, i64 0, i64 0)
  %16 = select i1 %cmp.i78, i64 7, i64 0
  %call1.i.i65 = call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %out.i71, i8* nonnull %cond, i64 %16)
  %17 = bitcast %"class.std::basic_ostream"* %out.i71 to i8**
  %vtable.i = load i8*, i8** %17, align 8
  %vbase.offset.ptr.i = getelementptr i8, i8* %vtable.i, i64 -24
  %18 = bitcast i8* %vbase.offset.ptr.i to i64*
  %vbase.offset.i = load i64, i64* %18, align 8
  %19 = bitcast %"class.std::basic_ostream"* %out.i71 to i8*
  %add.ptr.i = getelementptr inbounds i8, i8* %19, i64 %vbase.offset.i
  %_M_ctype.i.i = getelementptr inbounds i8, i8* %add.ptr.i, i64 240
  %20 = bitcast i8* %_M_ctype.i.i to %"class.std::ctype"**
  %21 = load %"class.std::ctype"*, %"class.std::ctype"** %20, align 8
  %tobool.i.i.i = icmp eq %"class.std::ctype"* %21, null
  br i1 %tobool.i.i.i, label %if.then.i.i.i, label %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i

if.then.i.i.i:                                    ; preds = %_ZN6cEnvirlsIPKcEERS_RKT_.exit
  call void @_ZSt16__throw_bad_castv() #5
  unreachable

_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i: ; preds = %_ZN6cEnvirlsIPKcEERS_RKT_.exit
  %_M_widen_ok.i.i.i = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %21, i64 0, i32 8
  %22 = load i8, i8* %_M_widen_ok.i.i.i, align 8
  %tobool.i3.i.i = icmp eq i8 %22, 0
  br i1 %tobool.i3.i.i, label %if.end.i.i.i, label %if.then.i4.i.i

if.then.i4.i.i:                                   ; preds = %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i
  %arrayidx.i.i.i = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %21, i64 0, i32 9, i64 10
  %23 = load i8, i8* %arrayidx.i.i.i, align 1
  br label %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit

if.end.i.i.i:                                     ; preds = %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i
  call void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"* nonnull %21)
  %24 = bitcast %"class.std::ctype"* %21 to i8 (%"class.std::ctype"*, i8)***
  %vtable.i.i.i = load i8 (%"class.std::ctype"*, i8)**, i8 (%"class.std::ctype"*, i8)*** %24, align 8
  %vfn.i.i.i = getelementptr inbounds i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %vtable.i.i.i, i64 6
  %25 = load i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %vfn.i.i.i, align 8
  %call.i.i.i = call signext i8 %25(%"class.std::ctype"* nonnull %21, i8 signext 10)
  br label %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit

_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit: ; preds = %if.end.i.i.i, %if.then.i4.i.i
  %retval.0.i.i.i = phi i8 [ %23, %if.then.i4.i.i ], [ %call.i.i.i, %if.end.i.i.i ]
  %call1.i = call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"* nonnull %out.i71, i8 signext %retval.0.i.i.i)
  %call.i.i55 = call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"* nonnull %call1.i)
  br label %for.inc

for.inc:                                          ; preds = %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit, %for.body
  %call.i = call %"struct.std::_Rb_tree_node_base"* @_ZSt18_Rb_tree_incrementPSt18_Rb_tree_node_base(%"struct.std::_Rb_tree_node_base"* %iter.sroa.0.090) #6
  %cmp.i = icmp eq %"struct.std::_Rb_tree_node_base"* %call.i, %_M_header.i.i
  br i1 %cmp.i, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %cleanup.done
  ret void
}

declare void @_ZN7SimTime14overflowAddingERKS_(%class.SimTime*, %class.SimTime* dereferenceable(8)) local_unnamed_addr #2

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"*, i32) local_unnamed_addr #2

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* dereferenceable(272), i8*, i64) local_unnamed_addr #2

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"*, i8 signext) local_unnamed_addr #2

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"*) local_unnamed_addr #2

; Function Attrs: noreturn
declare void @_ZSt16__throw_bad_castv() local_unnamed_addr #3

declare void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"*) local_unnamed_addr #2

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertImEERSoT_(%"class.std::basic_ostream"*, i64) local_unnamed_addr #2

; Function Attrs: nounwind memory(read)
declare %"struct.std::_Rb_tree_node_base"* @_ZSt18_Rb_tree_incrementPSt18_Rb_tree_node_base(%"struct.std::_Rb_tree_node_base"*) local_unnamed_addr #4

attributes #0 = { nounwind memory(argmem: readwrite) }
attributes #1 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind memory(read) "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { noreturn }
attributes #6 = { nounwind memory(read) willreturn }


