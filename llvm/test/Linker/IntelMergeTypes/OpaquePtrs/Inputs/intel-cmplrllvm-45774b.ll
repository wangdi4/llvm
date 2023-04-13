; INTEL_FEATURE_SW_DTRANS
target datalayout = "e-m:x-p:32:32-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32-a:0:32-S32"
target triple = "i386-pc-windows-msvc19.33.31629"

%eh.CatchableTypeArray.3 = type { i32, [3 x ptr] }
%eh.ThrowInfo = type { i32, ptr, ptr, ptr }
%eh.CatchableType = type { i32, ptr, i32, i32, i32, i32, ptr }
%rtti.TypeDescriptor43 = type { ptr, ptr, [44 x i8] }
%"class..?AVXMLPlatformUtilsException@xercesc_2_7@@.xercesc_2_7::XMLPlatformUtilsException" = type { %"class..?AVXMLException@xercesc_2_7@@.xercesc_2_7::XMLException" }
%"class..?AVXMLException@xercesc_2_7@@.xercesc_2_7::XMLException" = type { ptr, i32, ptr, i32, ptr, ptr }
%"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager" = type { ptr }

$"??0XMLPlatformUtilsException@xercesc_2_7@@QAE@ABV01@@Z" = comdat any

$"_CTA3?AVXMLPlatformUtilsException@xercesc_2_7@@" = comdat any

$"_TI3?AVXMLPlatformUtilsException@xercesc_2_7@@" = comdat any

$"_CT??_R0?AVXMLPlatformUtilsException@xercesc_2_7@@@8??0XMLPlatformUtilsException@xercesc_2_7@@QAE@ABV01@@Z24" = comdat any

$"_CT??_R0?AVXMLException@xercesc_2_7@@@8??0XMLException@xercesc_2_7@@QAE@ABV01@@Z24" = comdat any

$"_CT??_R0?AVXMemory@xercesc_2_7@@@814" = comdat any

$"??_R0?AVXMLPlatformUtilsException@xercesc_2_7@@@8" = comdat any

@"??_7type_info@@6B@" = external constant ptr, !intel_dtrans_type !0
@"_CTA3?AVXMLPlatformUtilsException@xercesc_2_7@@" = linkonce_odr unnamed_addr constant %eh.CatchableTypeArray.3 { i32 3, [3 x ptr] [ptr @"_CT??_R0?AVXMLPlatformUtilsException@xercesc_2_7@@@8??0XMLPlatformUtilsException@xercesc_2_7@@QAE@ABV01@@Z24", ptr @"_CT??_R0?AVXMLException@xercesc_2_7@@@8??0XMLException@xercesc_2_7@@QAE@ABV01@@Z24", ptr @"_CT??_R0?AVXMemory@xercesc_2_7@@@814"] }, section ".xdata", comdat
@"_TI3?AVXMLPlatformUtilsException@xercesc_2_7@@" = linkonce_odr unnamed_addr constant %eh.ThrowInfo { i32 0, ptr @"??1XMLException@xercesc_2_7@@UAE@XZ", ptr null, ptr @"_CTA3?AVXMLPlatformUtilsException@xercesc_2_7@@" }, section ".xdata", comdat
@"_CT??_R0?AVXMLPlatformUtilsException@xercesc_2_7@@@8??0XMLPlatformUtilsException@xercesc_2_7@@QAE@ABV01@@Z24" = linkonce_odr unnamed_addr constant %eh.CatchableType { i32 0, ptr @"??_R0?AVXMLPlatformUtilsException@xercesc_2_7@@@8", i32 0, i32 -1, i32 0, i32 24, ptr @"??0XMLPlatformUtilsException@xercesc_2_7@@QAE@ABV01@@Z" }, section ".xdata", comdat
@"_CT??_R0?AVXMLException@xercesc_2_7@@@8??0XMLException@xercesc_2_7@@QAE@ABV01@@Z24" = linkonce_odr unnamed_addr constant %eh.CatchableType { i32 0, ptr null, i32 0, i32 -1, i32 0, i32 24, ptr null }, section ".xdata", comdat
@"_CT??_R0?AVXMemory@xercesc_2_7@@@814" = linkonce_odr unnamed_addr constant %eh.CatchableType { i32 0, ptr null, i32 4, i32 -1, i32 0, i32 1, ptr null }, section ".xdata", comdat
@"??_R0?AVXMLPlatformUtilsException@xercesc_2_7@@@8" = linkonce_odr global %rtti.TypeDescriptor43 { ptr @"??_7type_info@@6B@", ptr null, [44 x i8] c".?AVXMLPlatformUtilsException@xercesc_2_7@@\00" }, comdat

define linkonce_odr dso_local x86_thiscallcc noundef "intel_dtrans_func_index"="1" ptr @"??0XMLPlatformUtilsException@xercesc_2_7@@QAE@ABV01@@Z"(ptr noundef nonnull returned align 4 dereferenceable(24) "intel_dtrans_func_index"="2" %this, ptr noundef nonnull align 4 dereferenceable(24) "intel_dtrans_func_index"="3" %toCopy) unnamed_addr comdat align 2 !intel.dtrans.func.type !27 {
entry:
  ret ptr null
}

declare !intel.dtrans.func.type !29 dso_local x86_thiscallcc void @"??1XMLException@xercesc_2_7@@UAE@XZ"(ptr noundef nonnull align 4 dereferenceable(24) "intel_dtrans_func_index"="1") unnamed_addr

declare !intel.dtrans.func.type !31 dso_local x86_stdcallcc void @_CxxThrowException(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2") local_unnamed_addr

define dso_local noundef i32 @"?curFilePos@XMLPlatformUtils@xercesc_2_7@@SAIPAXQAVMemoryManager@2@@Z"(ptr noundef "intel_dtrans_func_index"="1" %theFile, ptr noundef "intel_dtrans_func_index"="2" %manager) local_unnamed_addr align 2 !intel.dtrans.func.type !33 {
entry:
  %tmp = alloca %"class..?AVXMLPlatformUtilsException@xercesc_2_7@@.xercesc_2_7::XMLPlatformUtilsException", align 4
  call void @_CxxThrowException(ptr nonnull %tmp, ptr nonnull @"_TI3?AVXMLPlatformUtilsException@xercesc_2_7@@")
  unreachable
}

!llvm.linker.options = !{!1, !1, !2, !3, !4, !5, !6, !7, !8, !9, !10, !11, !12}
!llvm.module.flags = !{!13, !14, !15, !16, !17, !18}
!intel.dtrans.types = !{!19, !23}
!llvm.ident = !{!26}

!0 = !{i8 0, i32 1}
!1 = !{!"/DEFAULTLIB:uuid.lib"}
!2 = !{!"/DEFAULTLIB:libcpmt.lib"}
!3 = !{!"/DEFAULTLIB:libcmt.lib"}
!4 = !{!"/DEFAULTLIB:libircmt.lib"}
!5 = !{!"/DEFAULTLIB:svml_dispmt.lib"}
!6 = !{!"/DEFAULTLIB:libdecimal.lib"}
!7 = !{!"/DEFAULTLIB:libmmt.lib"}
!8 = !{!"/DEFAULTLIB:oldnames.lib"}
!9 = !{!"/FAILIFMISMATCH:\22_CRT_STDIO_ISO_WIDE_SPECIFIERS=0\22"}
!10 = !{!"/FAILIFMISMATCH:\22_MSC_VER=1900\22"}
!11 = !{!"/FAILIFMISMATCH:\22_ITERATOR_DEBUG_LEVEL=0\22"}
!12 = !{!"/FAILIFMISMATCH:\22RuntimeLibrary=MT_StaticRelease\22"}
!13 = !{i32 1, !"NumRegisterParameters", i32 0}
!14 = !{i32 1, !"wchar_size", i32 2}
!15 = !{i32 1, !"Virtual Function Elim", i32 0}
!16 = !{i32 1, !"MaxTLSAlign", i32 65536}
!17 = !{i32 1, !"ThinLTO", i32 0}
!18 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!19 = !{!"S", %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager" zeroinitializer, i32 1, !20}
!20 = !{!21, i32 2}
!21 = !{!"F", i1 true, i32 0, !22}
!22 = !{i32 0, i32 0}
!23 = !{!"S", %"class..?AVXMLException@xercesc_2_7@@.xercesc_2_7::XMLException" zeroinitializer, i32 6, !20, !22, !0, !22, !24, !25}
!24 = !{i16 0, i32 1}
!25 = !{%"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager" zeroinitializer, i32 1}
!26 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!27 = distinct !{!28, !28, !28}
!28 = !{%"class..?AVXMLPlatformUtilsException@xercesc_2_7@@.xercesc_2_7::XMLPlatformUtilsException" zeroinitializer, i32 1}
!29 = distinct !{!30}
!30 = !{%"class..?AVXMLException@xercesc_2_7@@.xercesc_2_7::XMLException" zeroinitializer, i32 1}
!31 = distinct !{!0, !32}
!32 = !{%eh.ThrowInfo zeroinitializer, i32 1}
!33 = distinct !{!0, !25}
; end INTEL_FEATURE_SW_DTRANS
