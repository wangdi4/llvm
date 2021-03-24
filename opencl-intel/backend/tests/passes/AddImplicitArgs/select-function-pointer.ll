; RUN: %oclopt -add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -add-implicit-args %s -S | FileCheck %s
;
; CHECK: define spir_func i32 @_Z3addii(i32 %a, i32 %b,
; CHECK: define spir_func i32 @_Z3subii(i32 %a, i32 %b,
; CHECK: define spir_kernel void @_ZTS1K
; CHECK: %[[ADD:.*]] = bitcast i32 (i32, i32, {{.*}})* @_Z3addii to i32 (i32, i32)*
; CHECK: %[[SUB:.*]] = bitcast i32 (i32, i32, {{.*}})* @_Z3subii to i32 (i32, i32)*
; CHECK: %[[SELECT:.*]] = select i1 %{{.*}}, i32 (i32, i32)* %[[ADD]], i32 (i32, i32)* %[[SUB]]
; CHECK: %[[BITCAST:.*]] = bitcast i32 (i32, i32)* %[[SELECT]] to i32 (i32, i32,
; CHECK: call i32 %[[BITCAST]]
; CHECK: %[[ADD2:.*]] = bitcast i32 (i32, i32, {{.*}})* @_Z3addii to i32 (i32, i32)*
; CHECK: %[[SUB2:.*]] = bitcast i32 (i32, i32, {{.*}})* @_Z3subii to i32 (i32, i32)*
; CHECK: %[[SELECT2:.*]] = select i1 %{{.*}}, i32 (i32, i32)* %[[ADD2]], i32 (i32, i32)* %[[SUB2]]
; CHECK: %[[BITCAST2:.*]] = bitcast i32 (i32, i32)* %[[SELECT2]] to i32 (i32, i32,
; CHECK: call i32 %[[BITCAST2]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-linux-sycldevice"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

@__spirv_BuiltInGlobalInvocationId = external dso_local local_unnamed_addr addrspace(2) constant <3 x i64>, align 32

; Function Attrs: norecurse nounwind readnone
define spir_func i32 @_Z3addii(i32 %a, i32 %b) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}

; Function Attrs: norecurse nounwind readnone
define spir_func i32 @_Z3subii(i32 %a, i32 %b) local_unnamed_addr #0 {
entry:
  %sub = sub nsw i32 %a, %b
  ret i32 %sub
}

define spir_kernel void @_ZTS1K(i32 %_arg_, i32 addrspace(1)* %_arg_1, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset, i32 addrspace(1)* %_arg_3, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange5, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange6, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset7, i32 addrspace(1)* %_arg_8, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_AccessRange10, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_MemRange11, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") nocapture readonly align 8 %_arg_Offset12) local_unnamed_addr #1 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 {
entry:
  %0 = getelementptr inbounds %"class.cl::sycl::range", %"class.cl::sycl::range"* %_arg_Offset, i64 0, i32 0, i32 0, i64 0
  %1 = load i64, i64* %0, align 8
  %add.ptr.i = getelementptr inbounds i32, i32 addrspace(1)* %_arg_1, i64 %1
  %2 = getelementptr inbounds %"class.cl::sycl::range", %"class.cl::sycl::range"* %_arg_Offset7, i64 0, i32 0, i32 0, i64 0
  %3 = load i64, i64* %2, align 8
  %add.ptr.i15 = getelementptr inbounds i32, i32 addrspace(1)* %_arg_3, i64 %3
  %4 = getelementptr inbounds %"class.cl::sycl::range", %"class.cl::sycl::range"* %_arg_Offset12, i64 0, i32 0, i32 0, i64 0
  %5 = load i64, i64* %4, align 8
  %add.ptr.i26 = getelementptr inbounds i32, i32 addrspace(1)* %_arg_8, i64 %5
  %6 = load <3 x i64>, <3 x i64> addrspace(2)* @__spirv_BuiltInGlobalInvocationId, align 32
  %7 = extractelement <3 x i64> %6, i64 0
  %cmp.i = icmp eq i32 %_arg_, 0
  %_Z3addii._Z3subii.i = select i1 %cmp.i, i32 (i32, i32)* @_Z3addii, i32 (i32, i32)* @_Z3subii
  %arrayidx.i.i = getelementptr inbounds i32, i32 addrspace(1)* %add.ptr.i, i64 %7
  %8 = load i32, i32 addrspace(1)* %arrayidx.i.i, align 4, !tbaa !9
  %arrayidx.i10.i = getelementptr inbounds i32, i32 addrspace(1)* %add.ptr.i15, i64 %7
  %9 = load i32, i32 addrspace(1)* %arrayidx.i10.i, align 4, !tbaa !9
  %call4.i = tail call spir_func i32 %_Z3addii._Z3subii.i(i32 %8, i32 %9), !callees !13
  %cmp.i2 = icmp ne i32 %_arg_, 0
  %_Z3addii._Z3subii.i2 = select i1 %cmp.i2, i32 (i32, i32)* @_Z3addii, i32 (i32, i32)* @_Z3subii
  %call4.i2 = tail call spir_func i32 %_Z3addii._Z3subii.i2(i32 %8, i32 %9), !callees !13
  %result = add i32 %call4.i, %call4.i2
  store i32 %result, i32 addrspace(1)* %arrayidx.i.i, align 4, !tbaa !9
  store i32 %_arg_, i32 addrspace(1)* %add.ptr.i26, align 4, !tbaa !9
  ret void
}

attributes #0 = { norecurse nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "referenced-indirectly" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.spir.version = !{!0}
!spirv.Source = !{!1}
!llvm.ident = !{!2}
!llvm.module.flags = !{!3}

!0 = !{i32 1, i32 2}
!1 = !{i32 4, i32 100000}
!2 = !{!"clang version 9.0.0 "}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 0, i32 1, i32 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0}
!5 = !{!"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!6 = !{!"int", !"int*", !"range<1>", !"range<1>", !"id<1>", !"int*", !"range<1>", !"range<1>", !"id<1>", !"int*", !"range<1>", !"range<1>", !"id<1>"}
!7 = !{!"int", !"int*", !"cl::sycl::range<1>", !"cl::sycl::range<1>", !"cl::sycl::id<1>", !"int*", !"cl::sycl::range<1>", !"cl::sycl::range<1>", !"cl::sycl::id<1>", !"int*", !"cl::sycl::range<1>", !"cl::sycl::range<1>", !"cl::sycl::id<1>"}
!8 = !{!"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !""}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C++ TBAA"}
!13 = !{i32 (i32, i32)* @_Z3addii, i32 (i32, i32)* @_Z3subii}


; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZTS1K -- {{.*}} bitcast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZTS1K -- {{.*}} bitcast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZTS1K -- {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZTS1K -- {{.*}} bitcast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZTS1K -- {{.*}} bitcast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZTS1K -- {{.*}} bitcast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZTS1K -- {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZTS1K -- {{.*}} bitcast
; DEBUGIFY-NOT: WARNING
