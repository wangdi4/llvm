; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-cg"  -force-hir-cg -hir-cost-model-throttling=0 -S 2>&1 | FileCheck %s

; Verify that we are able to parse indirect call using %25 as the function pointer.
; CHECK: UNKNOWN LOOP
; CHECK: %25 = (%vtable.i.i.i)[6];
; CHECK: %call.i.i.i = %25(&((%21)[0]),  10);
; CHECK: END LOOP

; Verify that we are able to generate the indirect call successfully.

; CHECK: region{{.*}}:
; CHECK: call signext i8 %t{{.*}}({{.*}} nonnull %t{{.*}}, i8 signext 10)


; ModuleID = 'module.ll'
source_filename = "model/MACRelayUnitBase.cc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.cEnvir = type { ptr, i8, i8, i8, %"class.std::basic_ostream" }
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
%class.cSimulation = type { %class.cNoncopyableOwnedObject.base, i32, i32, ptr, i32, ptr, ptr, ptr, ptr, i32, ptr, ptr, %class.SimTime, i64, ptr, ptr, ptr, %class.cMessageHeap }
%class.cNoncopyableOwnedObject.base = type { %class.cOwnedObject.base }
%class.cOwnedObject.base = type <{ %class.cNamedObject.base, [4 x i8], ptr, i32 }>
%class.cNamedObject.base = type <{ %class.cObject, ptr, i16, i16 }>
%class.cObject = type { ptr }
%class.cModule = type { %class.cComponent, ptr, i32, ptr, ptr, ptr, ptr, i32, ptr, i32, i32 }
%class.cComponent = type { %class.cDefaultList, ptr, i16, ptr, i16, i16, ptr, ptr }
%class.cDefaultList = type { %class.cNoncopyableOwnedObject.base, ptr, i32, i32 }
%class.cOwnedObject = type <{ %class.cNamedObject.base, [4 x i8], ptr, i32, [4 x i8] }>
%class.cComponentType = type { %class.cNoncopyableOwnedObject.base, %"class.std::basic_string", %"class.std::map", %"class.std::set" }
%"class.std::basic_string" = type { %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" }
%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" = type { ptr }
%"class.std::map" = type { %"class.std::_Rb_tree" }
%"class.std::_Rb_tree" = type { %"struct.std::_Rb_tree<std::basic_string<char>, std::pair<const std::basic_string<char>, cParImpl *>, std::_Select1st<std::pair<const std::basic_string<char>, cParImpl *> >, std::less<std::basic_string<char> >, std::allocator<std::pair<const std::basic_string<char>, cParImpl *> > >::_Rb_tree_impl" }
%"struct.std::_Rb_tree<std::basic_string<char>, std::pair<const std::basic_string<char>, cParImpl *>, std::_Select1st<std::pair<const std::basic_string<char>, cParImpl *> >, std::less<std::basic_string<char> >, std::allocator<std::pair<const std::basic_string<char>, cParImpl *> > >::_Rb_tree_impl" = type { %"struct.std::less", %"struct.std::_Rb_tree_node_base", i64 }
%"struct.std::less" = type { i8 }
%"struct.std::_Rb_tree_node_base" = type { i32, ptr, ptr, ptr }
%"class.std::set" = type { %"class.std::_Rb_tree.3" }
%"class.std::_Rb_tree.3" = type { %"struct.std::_Rb_tree<cParImpl *, cParImpl *, std::_Identity<cParImpl *>, cComponentType::Less, std::allocator<cParImpl *> >::_Rb_tree_impl" }
%"struct.std::_Rb_tree<cParImpl *, cParImpl *, std::_Identity<cParImpl *>, cComponentType::Less, std::allocator<cParImpl *> >::_Rb_tree_impl" = type { %"struct.cComponentType::Less", %"struct.std::_Rb_tree_node_base", i64 }
%"struct.cComponentType::Less" = type { i8 }
%class.cPar = type { %class.cObject, ptr, ptr }
%class.cParImpl = type { %class.cNamedObject.base, ptr }
%class.cDisplayString = type { ptr, ptr, ptr, i32, ptr, i8, ptr }
%"struct.cDisplayString::Tag" = type { ptr, i32, [16 x ptr] }
%"struct.cGate::Desc" = type { ptr, ptr, i32, %union.anon, %union.anon.7 }
%"struct.cGate::Name" = type <{ %class.opp_string, %class.opp_string, %class.opp_string, i32, [4 x i8] }>
%class.opp_string = type { ptr }
%union.anon = type { ptr }
%class.cGate = type { %class.cObject, ptr, i32, ptr, ptr, ptr }
%class.cChannel = type <{ %class.cComponent, ptr, i32, [4 x i8] }>
%union.anon.7 = type { ptr }
%class.cSimpleModule = type { %class.cModule, ptr, ptr }
%class.cCoroutine = type { ptr, ptr }
%struct._Task = type opaque
%class.cModuleType = type { %class.cComponentType }
%class.cScheduler = type { %class.cObject, ptr }
%class.SimTime = type { i64 }
%class.cMessage = type { %class.cOwnedObject.base, i16, i16, i16, ptr, ptr, ptr, i32, i32, i32, i32, %class.SimTime, %class.SimTime, %class.SimTime, %class.SimTime, i32, i64, i64, i64, i64 }
%class.cArray = type { %class.cOwnedObject.base, ptr, i32, i32, i32, i32 }
%class.cException = type <{ %"class.std::exception", i32, [4 x i8], %"class.std::basic_string", i8, [7 x i8], %"class.std::basic_string", %"class.std::basic_string", i32, [4 x i8] }>
%"class.std::exception" = type { ptr }
%class.cHasher = type { i32 }
%class.cMessageHeap = type { %class.cOwnedObject.base, ptr, i32, i32, i64 }
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
@_ZN11cSimulation5evPtrE = external local_unnamed_addr global ptr, align 8
@_ZN11cSimulation6simPtrE = external local_unnamed_addr global ptr, align 8

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #0

; Function Attrs: uwtable
declare dereferenceable(288) ptr @_ZN6cEnvirlsI10MACAddressEERS_RKT_(ptr, ptr dereferenceable(6)) local_unnamed_addr #1 align 2

; Function Attrs: uwtable
define void @_ZN16MACRelayUnitBase17printAddressTableEv(ptr %this) unnamed_addr #1 align 2 {
entry:
  %tmp.i = alloca %class.SimTime, align 8
  %0 = load ptr, ptr @_ZN11cSimulation5evPtrE, align 8
  %disable_tracing.i = getelementptr inbounds %class.cEnvir, ptr %0, i64 0, i32 1
  %1 = load i8, ptr %disable_tracing.i, align 8
  %tobool.i = icmp eq i8 %1, 0
  br i1 %tobool.i, label %cond.false, label %cleanup.done

cond.false:                                       ; preds = %entry
  %out.i56 = getelementptr inbounds %class.cEnvir, ptr %0, i64 0, i32 4
  %call1.i.i = tail call dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull dereferenceable(272) %out.i56, ptr nonnull @.str.16, i64 15)
  %_M_node_count.i.i = getelementptr inbounds %class.MACRelayUnitBase, ptr %this, i64 0, i32 4, i32 0, i32 0, i32 2
  %2 = load i64, ptr %_M_node_count.i.i, align 8
  %call.i.i = tail call dereferenceable(272) ptr @_ZNSo9_M_insertImEERSoT_(ptr nonnull %out.i56, i64 %2)
  %call1.i.i61 = tail call dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull dereferenceable(272) %out.i56, ptr nonnull @.str.17, i64 11)
  br label %cleanup.done

cleanup.done:                                     ; preds = %cond.false, %entry
  %_M_left.i.i = getelementptr inbounds %class.MACRelayUnitBase, ptr %this, i64 0, i32 4, i32 0, i32 0, i32 1, i32 2
  %3 = bitcast ptr %_M_left.i.i to ptr
  %4 = load ptr, ptr %3, align 8
  %5 = getelementptr inbounds %"struct.std::_Rb_tree_node", ptr %4, i64 0, i32 0
  %_M_header.i.i = getelementptr inbounds %class.MACRelayUnitBase, ptr %this, i64 0, i32 4, i32 0, i32 0, i32 1
  %cmp.i89 = icmp eq ptr %5, %_M_header.i.i
  br i1 %cmp.i89, label %for.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %cleanup.done
  %agingTime = getelementptr inbounds %class.MACRelayUnitBase, ptr %this, i64 0, i32 3
  %6 = bitcast ptr %tmp.i to ptr
  %t2.i.i.i = getelementptr inbounds %class.SimTime, ptr %tmp.i, i64 0, i32 0
  %t2.i.i4.i = getelementptr inbounds %class.SimTime, ptr %agingTime, i64 0, i32 0
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %iter.sroa.0.090 = phi ptr [ %5, %for.body.lr.ph ], [ %call.i, %for.inc ]
  %7 = load ptr, ptr @_ZN11cSimulation5evPtrE, align 8
  %disable_tracing.i66 = getelementptr inbounds %class.cEnvir, ptr %7, i64 0, i32 1
  %8 = load i8, ptr %disable_tracing.i66, align 8
  %tobool.i67 = icmp eq i8 %8, 0
  br i1 %tobool.i67, label %cond.false21, label %for.inc

cond.false21:                                     ; preds = %for.body
  %out.i68 = getelementptr inbounds %class.cEnvir, ptr %7, i64 0, i32 4
  %call1.i.i70 = call dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull dereferenceable(272) %out.i68, ptr nonnull @.str.18, i64 2)
  %_M_value_field.i = getelementptr inbounds %"struct.std::_Rb_tree_node_base", ptr %iter.sroa.0.090, i64 1
  %first = bitcast ptr %_M_value_field.i to ptr
  %call25 = call dereferenceable(288) ptr @_ZN6cEnvirlsI10MACAddressEERS_RKT_(ptr nonnull %7, ptr dereferenceable(6) %first)
  %out.i71 = getelementptr inbounds %class.cEnvir, ptr %call25, i64 0, i32 4
  %call1.i.i73 = call dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull dereferenceable(272) %out.i71, ptr nonnull @.str.19, i64 9)
  %portno = getelementptr inbounds %"struct.std::_Rb_tree_node_base", ptr %iter.sroa.0.090, i64 1, i32 1
  %9 = bitcast ptr %portno to ptr
  %10 = load i32, ptr %9, align 4
  %call.i76 = call dereferenceable(272) ptr @_ZNSolsEi(ptr nonnull %out.i71, i32 %10)
  %insertionTime = getelementptr inbounds %"struct.std::_Rb_tree_node_base", ptr %iter.sroa.0.090, i64 1, i32 2
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %6)
  %t.i.i.i = bitcast ptr %insertionTime to ptr
  %11 = load i64, ptr %t.i.i.i, align 8
  %12 = load i64, ptr %t2.i.i4.i, align 8
  %xor.i.i.i.i = xor i64 %12, %11
  %cmp.i.i.i.i = icmp slt i64 %xor.i.i.i.i, 0
  %add.i.i.i = add nsw i64 %12, %11
  store i64 %add.i.i.i, ptr %t2.i.i.i, align 8
  %xor.i11.i.i.i = xor i64 %add.i.i.i, %12
  %cmp.i12.i.i.i = icmp sgt i64 %xor.i11.i.i.i, -1
  %or.cond.i = or i1 %cmp.i.i.i.i, %cmp.i12.i.i.i
  br i1 %or.cond.i, label %_ZN6cEnvirlsIPKcEERS_RKT_.exit, label %if.then.i.i.i79

if.then.i.i.i79:                                  ; preds = %cond.false21
  call void @_ZN7SimTime14overflowAddingERKS_(ptr nonnull %tmp.i, ptr nonnull dereferenceable(8) %agingTime)
  %.pre.i = load i64, ptr %t2.i.i.i, align 8
  br label %_ZN6cEnvirlsIPKcEERS_RKT_.exit

_ZN6cEnvirlsIPKcEERS_RKT_.exit:                   ; preds = %if.then.i.i.i79, %cond.false21
  %13 = phi i64 [ %add.i.i.i, %cond.false21 ], [ %.pre.i, %if.then.i.i.i79 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %6)
  %14 = load ptr, ptr @_ZN11cSimulation6simPtrE, align 8
  %t.i.i.i.i = getelementptr inbounds %class.cSimulation, ptr %14, i64 0, i32 12, i32 0
  %15 = load i64, ptr %t.i.i.i.i, align 8
  %cmp.i78 = icmp sle i64 %13, %15
  %cond = select i1 %cmp.i78, ptr @.str.20, ptr @.str.21
  %16 = select i1 %cmp.i78, i64 7, i64 0
  %call1.i.i65 = call dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull dereferenceable(272) %out.i71, ptr nonnull %cond, i64 %16)
  %17 = bitcast ptr %out.i71 to ptr
  %vtable.i = load ptr, ptr %17, align 8
  %vbase.offset.ptr.i = getelementptr i8, ptr %vtable.i, i64 -24
  %18 = bitcast ptr %vbase.offset.ptr.i to ptr
  %vbase.offset.i = load i64, ptr %18, align 8
  %19 = bitcast ptr %out.i71 to ptr
  %add.ptr.i = getelementptr inbounds i8, ptr %19, i64 %vbase.offset.i
  %_M_ctype.i.i = getelementptr inbounds i8, ptr %add.ptr.i, i64 240
  %20 = bitcast ptr %_M_ctype.i.i to ptr
  %21 = load ptr, ptr %20, align 8
  %tobool.i.i.i = icmp eq ptr %21, null
  br label %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i

_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i: ; preds = %_ZN6cEnvirlsIPKcEERS_RKT_.exit
  %_M_widen_ok.i.i.i = getelementptr inbounds %"class.std::ctype", ptr %21, i64 0, i32 8
  %22 = load i8, ptr %_M_widen_ok.i.i.i, align 8
  %tobool.i3.i.i = icmp eq i8 %22, 0
  br i1 %tobool.i3.i.i, label %if.end.i.i.i, label %if.then.i4.i.i

if.then.i4.i.i:                                   ; preds = %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i
  %arrayidx.i.i.i = getelementptr inbounds %"class.std::ctype", ptr %21, i64 0, i32 9, i64 10
  %23 = load i8, ptr %arrayidx.i.i.i, align 1
  br label %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit

if.end.i.i.i:                                     ; preds = %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i
  call void @_ZNKSt5ctypeIcE13_M_widen_initEv(ptr nonnull %21)
  %24 = bitcast ptr %21 to ptr
  %vtable.i.i.i = load ptr, ptr %24, align 8
  %vfn.i.i.i = getelementptr inbounds ptr, ptr %vtable.i.i.i, i64 6
  %25 = load ptr, ptr %vfn.i.i.i, align 8
  %call.i.i.i = call signext i8 %25(ptr nonnull %21, i8 signext 10)
  br label %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit

_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit: ; preds = %if.end.i.i.i, %if.then.i4.i.i
  %retval.0.i.i.i = phi i8 [ %23, %if.then.i4.i.i ], [ %call.i.i.i, %if.end.i.i.i ]
  %call1.i = call dereferenceable(272) ptr @_ZNSo3putEc(ptr nonnull %out.i71, i8 signext %retval.0.i.i.i)
  %call.i.i55 = call dereferenceable(272) ptr @_ZNSo5flushEv(ptr nonnull %call1.i)
  br label %for.inc

for.inc:                                          ; preds = %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit, %for.body
  %call.i = call ptr @_ZSt18_Rb_tree_incrementPSt18_Rb_tree_node_base(ptr %iter.sroa.0.090) #6
  %cmp.i = icmp eq ptr %call.i, %_M_header.i.i
  br i1 %cmp.i, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %cleanup.done
  ret void
}

declare void @_ZN7SimTime14overflowAddingERKS_(ptr, ptr dereferenceable(8)) local_unnamed_addr #2

declare dereferenceable(272) ptr @_ZNSolsEi(ptr, i32) local_unnamed_addr #2

declare dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr dereferenceable(272), ptr, i64) local_unnamed_addr #2

declare dereferenceable(272) ptr @_ZNSo3putEc(ptr, i8 signext) local_unnamed_addr #2

declare dereferenceable(272) ptr @_ZNSo5flushEv(ptr) local_unnamed_addr #2

; Function Attrs: noreturn
declare void @_ZSt16__throw_bad_castv() local_unnamed_addr #3

declare void @_ZNKSt5ctypeIcE13_M_widen_initEv(ptr) local_unnamed_addr #2

declare dereferenceable(272) ptr @_ZNSo9_M_insertImEERSoT_(ptr, i64) local_unnamed_addr #2

; Function Attrs: nounwind readonly
declare ptr @_ZSt18_Rb_tree_incrementPSt18_Rb_tree_node_base(ptr) local_unnamed_addr #4

