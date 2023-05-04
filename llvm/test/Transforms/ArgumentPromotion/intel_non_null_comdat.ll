; CMPLRLLVM-35006: Test to make sure compiler doesn't fail with assertion.

; RUN: opt < %s -passes='cgscc(argpromotion),function(instcombine)' -disable-output

%"struct.omnetpp::cComponent::SignalListenerList" = type { i32, ptr }
%"class.omnetpp::cIListener" = type { ptr, %"class.std::vector.52" }
%"class.std::vector.52" = type { %"struct.std::_Vector_base.53" }
%"struct.std::_Vector_base.53" = type { %"struct.std::_Vector_base<std::pair<omnetpp::cComponent *, int>, std::allocator<std::pair<omnetpp::cComponent *, int>>>::_Vector_impl" }
%"struct.std::_Vector_base<std::pair<omnetpp::cComponent *, int>, std::allocator<std::pair<omnetpp::cComponent *, int>>>::_Vector_impl" = type { %"struct.std::_Vector_base<std::pair<omnetpp::cComponent *, int>, std::allocator<std::pair<omnetpp::cComponent *, int>>>::_Vector_impl_data" }
%"struct.std::_Vector_base<std::pair<omnetpp::cComponent *, int>, std::allocator<std::pair<omnetpp::cComponent *, int>>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"struct.std::pair" = type <{ ptr, i32, [4 x i8] }>
%"class.omnetpp::cComponent" = type { %"class.omnetpp::cSoftOwner", ptr, ptr, i32, i16, i16, ptr, i16, i16, ptr, ptr, ptr, ptr }
%"class.omnetpp::cSoftOwner" = type { %"class.omnetpp::cNoncopyableOwnedObject.base", ptr, i32, i32 }
%"class.omnetpp::cNoncopyableOwnedObject.base" = type { %"class.omnetpp::cOwnedObject.base" }
%"class.omnetpp::cOwnedObject.base" = type <{ %"class.omnetpp::cNamedObject.base", [4 x i8], ptr, i32 }>
%"class.omnetpp::cNamedObject.base" = type <{ %"class.omnetpp::cObject", ptr, i32 }>
%"class.omnetpp::cObject" = type { ptr }
%"class.omnetpp::cOwnedObject" = type <{ %"class.omnetpp::cNamedObject.base", [4 x i8], ptr, i32, [4 x i8] }>
%"class.omnetpp::cComponentType" = type { %"class.omnetpp::cNoncopyableOwnedObject.base", %"class.std::__cxx11::basic_string", i8, i8, %"class.std::map.29", %"class.std::set", %"class.std::map.39", i8, %"class.std::__cxx11::basic_string" }
%"class.std::map.29" = type { %"class.std::_Rb_tree.30" }
%"class.std::_Rb_tree.30" = type { %"struct.std::_Rb_tree<std::__cxx11::basic_string<char>, std::pair<const std::__cxx11::basic_string<char>, omnetpp::cParImpl *>, std::_Select1st<std::pair<const std::__cxx11::basic_string<char>, omnetpp::cParImpl *>>, std::less<std::__cxx11::basic_string<char>>>::_Rb_tree_impl" }
%"struct.std::_Rb_tree<std::__cxx11::basic_string<char>, std::pair<const std::__cxx11::basic_string<char>, omnetpp::cParImpl *>, std::_Select1st<std::pair<const std::__cxx11::basic_string<char>, omnetpp::cParImpl *>>, std::less<std::__cxx11::basic_string<char>>>::_Rb_tree_impl" = type { %"struct.std::_Rb_tree_key_compare.11", %"struct.std::_Rb_tree_header" }
%"struct.std::_Rb_tree_key_compare.11" = type { %"struct.std::less.12" }
%"struct.std::less.12" = type { i8 }
%"struct.std::_Rb_tree_header" = type { %"struct.std::_Rb_tree_node_base", i64 }
%"struct.std::_Rb_tree_node_base" = type { i32, ptr, ptr, ptr }
%"class.std::set" = type { %"class.std::_Rb_tree.34" }
%"class.std::_Rb_tree.34" = type { %"struct.std::_Rb_tree<omnetpp::cParImpl *, omnetpp::cParImpl *, std::_Identity<omnetpp::cParImpl *>, omnetpp::cComponentType::Less>::_Rb_tree_impl" }
%"struct.std::_Rb_tree<omnetpp::cParImpl *, omnetpp::cParImpl *, std::_Identity<omnetpp::cParImpl *>, omnetpp::cComponentType::Less>::_Rb_tree_impl" = type { %"struct.std::_Rb_tree_key_compare.38", %"struct.std::_Rb_tree_header" }
%"struct.std::_Rb_tree_key_compare.38" = type { %"struct.omnetpp::cComponentType::Less" }
%"struct.omnetpp::cComponentType::Less" = type { i8 }
%"class.std::map.39" = type { %"class.std::_Rb_tree.40" }
%"class.std::_Rb_tree.40" = type { %"struct.std::_Rb_tree<int, std::pair<const int, omnetpp::cComponentType::SignalDesc>, std::_Select1st<std::pair<const int, omnetpp::cComponentType::SignalDesc>>, std::less<int>>::_Rb_tree_impl" }
%"struct.std::_Rb_tree<int, std::pair<const int, omnetpp::cComponentType::SignalDesc>, std::_Select1st<std::pair<const int, omnetpp::cComponentType::SignalDesc>>, std::less<int>>::_Rb_tree_impl" = type { %"struct.std::_Rb_tree_key_compare.11", %"struct.std::_Rb_tree_header" }
%"class.std::__cxx11::basic_string" = type { %"struct.std::__cxx11::basic_string<char>::_Alloc_hider", i64, %union.anon }
%"struct.std::__cxx11::basic_string<char>::_Alloc_hider" = type { ptr }
%union.anon = type { i64, [8 x i8] }
%"class.omnetpp::cSimulation" = type { %"class.omnetpp::cNamedObject.base", i32, i32, ptr, i32, ptr, ptr, ptr, ptr, i32, ptr, ptr, ptr, %"class.omnetpp::SimTime", i32, %"class.omnetpp::SimTime", i64, ptr, i8, ptr }
%"class.omnetpp::cEnvir" = type { ptr, i8, i8, i8, %"class.std::vector" }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<omnetpp::cISimulationLifecycleListener *, std::allocator<omnetpp::cISimulationLifecycleListener *>>::_Vector_impl" }
%"struct.std::_Vector_base<omnetpp::cISimulationLifecycleListener *, std::allocator<omnetpp::cISimulationLifecycleListener *>>::_Vector_impl" = type { %"struct.std::_Vector_base<omnetpp::cISimulationLifecycleListener *, std::allocator<omnetpp::cISimulationLifecycleListener *>>::_Vector_impl_data" }
%"struct.std::_Vector_base<omnetpp::cISimulationLifecycleListener *, std::allocator<omnetpp::cISimulationLifecycleListener *>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"class.omnetpp::cISimulationLifecycleListener" = type { ptr }
%"class.omnetpp::cModule" = type { %"class.omnetpp::cComponent", ptr, %"class.omnetpp::opp_pooledstring", %"class.omnetpp::opp_pooledstring", ptr, ptr, ptr, i32, i32, ptr, ptr }
%"class.omnetpp::opp_pooledstring" = type { ptr }
%"struct.omnetpp::cModule::SubcomponentData" = type <{ %"class.std::vector.65", %"class.std::vector.70", %"class.std::vector.75", i32, [4 x i8] }>
%"class.std::vector.65" = type { %"struct.std::_Vector_base.66" }
%"struct.std::_Vector_base.66" = type { %"struct.std::_Vector_base<omnetpp::cModule *, std::allocator<omnetpp::cModule *>>::_Vector_impl" }
%"struct.std::_Vector_base<omnetpp::cModule *, std::allocator<omnetpp::cModule *>>::_Vector_impl" = type { %"struct.std::_Vector_base<omnetpp::cModule *, std::allocator<omnetpp::cModule *>>::_Vector_impl_data" }
%"struct.std::_Vector_base<omnetpp::cModule *, std::allocator<omnetpp::cModule *>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"class.std::vector.70" = type { %"struct.std::_Vector_base.71" }
%"struct.std::_Vector_base.71" = type { %"struct.std::_Vector_base<omnetpp::cModule::SubmoduleVector, std::allocator<omnetpp::cModule::SubmoduleVector>>::_Vector_impl" }
%"struct.std::_Vector_base<omnetpp::cModule::SubmoduleVector, std::allocator<omnetpp::cModule::SubmoduleVector>>::_Vector_impl" = type { %"struct.std::_Vector_base<omnetpp::cModule::SubmoduleVector, std::allocator<omnetpp::cModule::SubmoduleVector>>::_Vector_impl_data" }
%"struct.std::_Vector_base<omnetpp::cModule::SubmoduleVector, std::allocator<omnetpp::cModule::SubmoduleVector>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"struct.omnetpp::cModule::SubmoduleVector" = type { %"class.std::__cxx11::basic_string", %"class.std::vector.65" }
%"class.std::vector.75" = type { %"struct.std::_Vector_base.76" }
%"struct.std::_Vector_base.76" = type { %"struct.std::_Vector_base<omnetpp::cChannel *, std::allocator<omnetpp::cChannel *>>::_Vector_impl" }
%"struct.std::_Vector_base<omnetpp::cChannel *, std::allocator<omnetpp::cChannel *>>::_Vector_impl" = type { %"struct.std::_Vector_base<omnetpp::cChannel *, std::allocator<omnetpp::cChannel *>>::_Vector_impl_data" }
%"struct.std::_Vector_base<omnetpp::cChannel *, std::allocator<omnetpp::cChannel *>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"class.omnetpp::cChannel" = type <{ %"class.omnetpp::cComponent", ptr, i32, [4 x i8] }>
%"class.omnetpp::cGate" = type { %"class.omnetpp::cObject", ptr, i32, i32, ptr, ptr, ptr }
%"struct.omnetpp::cGate::Desc" = type { ptr, ptr, i32, %"union.omnetpp::cGate::Desc::Gates", %"union.omnetpp::cGate::Desc::Gates" }
%"struct.omnetpp::cGate::Name" = type <{ %"class.omnetpp::opp_string", %"class.omnetpp::opp_string", %"class.omnetpp::opp_string", i32, [4 x i8] }>
%"class.omnetpp::opp_string" = type { ptr }
%"union.omnetpp::cGate::Desc::Gates" = type { ptr }
%"class.omnetpp::cCanvas" = type { %"class.omnetpp::cOwnedObject.base", %"struct.omnetpp::cFigure::Color", ptr, %"class.std::map.85", %"class.std::map.90", double, double }
%"struct.omnetpp::cFigure::Color" = type { i8, i8, i8 }
%"class.omnetpp::cFigure" = type { %"class.omnetpp::cOwnedObject.base", i32, i8, i8, double, %"class.omnetpp::opp_pooledstring", ptr, %"struct.omnetpp::cFigure::Transform", %"class.std::vector.80", %"class.omnetpp::opp_pooledstring", i64, i8, i8, i8, i32 }
%"struct.omnetpp::cFigure::Transform" = type { double, double, double, double, double, double }
%"class.std::vector.80" = type { %"struct.std::_Vector_base.81" }
%"struct.std::_Vector_base.81" = type { %"struct.std::_Vector_base<omnetpp::cFigure *, std::allocator<omnetpp::cFigure *>>::_Vector_impl" }
%"struct.std::_Vector_base<omnetpp::cFigure *, std::allocator<omnetpp::cFigure *>>::_Vector_impl" = type { %"struct.std::_Vector_base<omnetpp::cFigure *, std::allocator<omnetpp::cFigure *>>::_Vector_impl_data" }
%"struct.std::_Vector_base<omnetpp::cFigure *, std::allocator<omnetpp::cFigure *>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"class.std::map.85" = type { %"class.std::_Rb_tree.86" }
%"class.std::_Rb_tree.86" = type { %"struct.std::_Rb_tree<std::__cxx11::basic_string<char>, std::pair<const std::__cxx11::basic_string<char>, int>, std::_Select1st<std::pair<const std::__cxx11::basic_string<char>, int>>, std::less<std::__cxx11::basic_string<char>>>::_Rb_tree_impl" }
%"struct.std::_Rb_tree<std::__cxx11::basic_string<char>, std::pair<const std::__cxx11::basic_string<char>, int>, std::_Select1st<std::pair<const std::__cxx11::basic_string<char>, int>>, std::less<std::__cxx11::basic_string<char>>>::_Rb_tree_impl" = type { %"struct.std::_Rb_tree_key_compare.11", %"struct.std::_Rb_tree_header" }
%"class.std::map.90" = type { %"class.std::_Rb_tree.91" }
%"class.std::_Rb_tree.91" = type { %"struct.std::_Rb_tree<const omnetpp::cObject *, std::pair<const omnetpp::cObject *const, double>, std::_Select1st<std::pair<const omnetpp::cObject *const, double>>, std::less<const omnetpp::cObject *>>::_Rb_tree_impl" }
%"struct.std::_Rb_tree<const omnetpp::cObject *, std::pair<const omnetpp::cObject *const, double>, std::_Select1st<std::pair<const omnetpp::cObject *const, double>>, std::less<const omnetpp::cObject *>>::_Rb_tree_impl" = type { %"struct.std::_Rb_tree_key_compare.11", %"struct.std::_Rb_tree_header" }
%"class.omnetpp::cOsgCanvas" = type { %"class.omnetpp::cOwnedObject.base", ptr, i32, %"struct.omnetpp::cFigure::Color", i32, double, double, double, ptr, ptr }
%"class.osg::Node" = type opaque
%"struct.omnetpp::cOsgCanvas::Viewpoint" = type <{ %"struct.omnetpp::cOsgCanvas::Vec3d", %"struct.omnetpp::cOsgCanvas::Vec3d", %"struct.omnetpp::cOsgCanvas::Vec3d", i8, [7 x i8] }>
%"struct.omnetpp::cOsgCanvas::Vec3d" = type { double, double, double }
%"struct.omnetpp::cOsgCanvas::EarthViewpoint" = type <{ double, double, double, double, double, double, i8, [7 x i8] }>
%"class.omnetpp::cSimpleModule" = type { %"class.omnetpp::cModule", ptr, ptr }
%"class.omnetpp::cMessage" = type { %"class.omnetpp::cEvent", i16, i16, ptr, ptr, ptr, i32, i32, i32, i32, %"class.omnetpp::SimTime", %"class.omnetpp::SimTime", %"class.omnetpp::SimTime", i64, i64 }
%"class.omnetpp::cEvent" = type { %"class.omnetpp::cOwnedObject.base", %"class.omnetpp::SimTime", i16, i32, i64, i64 }
%"class.omnetpp::cArray" = type { %"class.omnetpp::cOwnedObject.base", ptr, i32, i32, i32, i32 }
%"class.omnetpp::cCoroutine" = type { ptr, ptr }
%"struct.omnetpp::_Task" = type { i64, [1 x %struct.__jmp_buf_tag.4815], [1 x %struct.__jmp_buf_tag.4815], i32, i32, ptr, void (ptr)*, ptr, i32, ptr, i64 }
%struct.__jmp_buf_tag.4815 = type { [8 x i64], i32, %struct.__sigset_t.4814 }
%struct.__sigset_t.4814 = type { [16 x i64] }
%"class.omnetpp::cModuleType" = type { %"class.omnetpp::cComponentType" }
%"class.omnetpp::cFutureEventSet" = type { %"class.omnetpp::cOwnedObject.base", [4 x i8] }
%"class.omnetpp::cScheduler" = type { %"class.omnetpp::cObject", %"class.omnetpp::cISimulationLifecycleListener", ptr }
%"class.omnetpp::SimTime" = type { i64 }
%"class.omnetpp::cException" = type { %"class.std::exception", i32, %"class.std::__cxx11::basic_string", i32, i64, %"class.omnetpp::SimTime", i8, %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string", i32, i32 }
%"class.std::exception" = type { ptr }
%"class.omnetpp::cFingerprintCalculator" = type { %"class.omnetpp::cObject" }
%"class.omnetpp::cPar" = type { %"class.omnetpp::cObject", ptr, ptr, ptr }
%"class.omnetpp::cParImpl" = type { %"class.omnetpp::cNamedObject.base", %"class.omnetpp::opp_staticpooledstring", %"class.omnetpp::opp_staticpooledstring" }
%"class.omnetpp::opp_staticpooledstring" = type { ptr }
%"class.omnetpp::cDisplayString" = type { ptr, ptr, ptr, i32, ptr, i8, ptr }
%"struct.omnetpp::cDisplayString::Tag" = type { ptr, i32, [16 x ptr] }
%"class.std::vector.47" = type { %"struct.std::_Vector_base.48" }
%"struct.std::_Vector_base.48" = type { %"struct.std::_Vector_base<omnetpp::cComponent::SignalListenerList, std::allocator<omnetpp::cComponent::SignalListenerList>>::_Vector_impl" }
%"struct.std::_Vector_base<omnetpp::cComponent::SignalListenerList, std::allocator<omnetpp::cComponent::SignalListenerList>>::_Vector_impl" = type { %"struct.std::_Vector_base<omnetpp::cComponent::SignalListenerList, std::allocator<omnetpp::cComponent::SignalListenerList>>::_Vector_impl_data" }
%"struct.std::_Vector_base<omnetpp::cComponent::SignalListenerList, std::allocator<omnetpp::cComponent::SignalListenerList>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"class.std::unordered_set" = type { %"class.std::_Hashtable" }
%"class.std::_Hashtable" = type { ptr, i64, %"struct.std::__detail::_Hash_node_base", i64, %"struct.std::__detail::_Prime_rehash_policy", ptr }
%"struct.std::__detail::_Hash_node_base" = type { ptr }
%"struct.std::__detail::_Prime_rehash_policy" = type { float, i64 }

$_ZN7omnetpp10cComponent18SignalListenerList2gtERKS1_S3_ = comdat any

@_ZTVN7omnetpp8cChannelE = internal constant { [136 x ptr] } { [136 x ptr] [ptr null, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr @_ZN7omnetpp10cComponent9subscribeEiPNS_10cIListenerE, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef] }

define internal void @_ZN7omnetpp10cComponent9subscribeEiPNS_10cIListenerE() personality ptr undef {
  call fastcc void @_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPN7omnetpp10cComponent18SignalListenerListESt6vectorIS4_SaIS4_EEEElNS0_5__ops15_Iter_comp_iterIPFbRKS4_SD_EEEEvT_SH_T0_T1_()
  ret void
}

define internal i1 @_ZN7omnetpp10cComponent18SignalListenerList2gtERKS1_S3_(ptr %0, ptr %1) comdat {
  ret i1 false
}

define internal fastcc void @_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPN7omnetpp10cComponent18SignalListenerListESt6vectorIS4_SaIS4_EEEElNS0_5__ops15_Iter_comp_iterIPFbRKS4_SD_EEEEvT_SH_T0_T1_() {
  ret void

1:                                                ; No predecessors!
  store ptr @_ZN7omnetpp10cComponent18SignalListenerList2gtERKS1_S3_, ptr null, align 8
  ret void
}
