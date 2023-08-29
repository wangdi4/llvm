; RUN: sycl-post-link --ir-output-only -ompoffload-link-entries %s -S -o - | \
; RUN:   FileCheck %s
; RUN: sycl-post-link --ir-output-only -ompoffload-link-entries -fixup-vtables=false %s -S -o - | \
; RUN:   FileCheck %s -check-prefixes=NEG

; Original code:
; #include <new>
; #include <omp.h>
;
; struct Base {
;   virtual ~Base() = default;
; };
;
; struct Derived : public Base {
; #pragma omp declare target
;   Derived() {}
; #pragma omp end declare target
;   ~Derived() {}
; };
;
; int main(void) {
;   Base *Ptr = reinterpret_cast<Base *>(omp_target_alloc(sizeof(Derived), 0));
;   Derived Obj;
; #pragma omp target firstprivate(Ptr)
;   {
;     new (reinterpret_cast<Derived *>(Ptr)) Derived(Obj);
;   }
;   return 0;
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.__tgt_offload_entry = type { ptr addrspace(4), ptr addrspace(2), i64, i32, i32, i64 }
%struct.Derived = type { %struct.Base }
%struct.Base = type { ptr addrspace(4) }

$_ZTV7Derived = comdat any

$_ZTS7Derived = comdat any

$_ZTS4Base = comdat any

$_ZTI4Base = comdat any

$_ZTI7Derived = comdat any

$_ZTV4Base = comdat any

@_ZTV7Derived = linkonce_odr hidden unnamed_addr addrspace(1) constant { [4 x ptr addrspace(4)] } { [4 x ptr addrspace(4)] [ ptr addrspace(4) null, ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTI7Derived to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @_ZN7DerivedD1Ev to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @_ZN7DerivedD0Ev to ptr addrspace(4)) ] }, comdat, align 8
; Check vtables and typeinfo structures do not reference undefined symbols:
; CHECK: @_ZTV7Derived = linkonce_odr hidden unnamed_addr addrspace(1)
; CHECK-SAME: constant { [4 x ptr addrspace(4)] } { [4 x ptr addrspace(4)] [
; CHECK-SAME: ptr addrspace(4) null,
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTI7Derived to ptr addrspace(4)),
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr null to ptr addrspace(4)),
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr null to ptr addrspace(4))] }, comdat, align 8

; Check that fixup may be disabled by -fixup-vtables=false:
; NEG: @_ZTV7Derived = linkonce_odr hidden unnamed_addr addrspace(1)
; NEG-SAME: constant { [4 x ptr addrspace(4)] } { [4 x ptr addrspace(4)] [
; NEG-SAME: ptr addrspace(4) null,
; NEG-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTI7Derived to ptr addrspace(4)),
; NEG-SAME: ptr addrspace(4) addrspacecast (ptr @_ZN7DerivedD1Ev to ptr addrspace(4)),
; NEG-SAME: ptr addrspace(4) addrspacecast (ptr @_ZN7DerivedD0Ev to ptr addrspace(4))] }, comdat, align 8

@_ZTVN10__cxxabiv120__si_class_type_infoE = external addrspace(1) global ptr addrspace(4)
@_ZTS7Derived = linkonce_odr hidden addrspace(1) constant [9 x i8] c"7Derived\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external addrspace(1) global ptr addrspace(4)
@_ZTS4Base = linkonce_odr hidden addrspace(1) constant [6 x i8] c"4Base\00", comdat, align 1
@_ZTI4Base = linkonce_odr hidden addrspace(1) constant { ptr addrspace(4), ptr addrspace(4) } { ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv117__class_type_infoE to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTS4Base to ptr addrspace(4)) }, comdat, align 8
; CHECK: @_ZTI4Base = linkonce_odr hidden addrspace(1)
; CHECK-SAME: constant { ptr addrspace(4), ptr addrspace(4) } {
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) null to ptr addrspace(4)),
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTS4Base to ptr addrspace(4)) }, comdat, align 8

; NEG: @_ZTI4Base = linkonce_odr hidden addrspace(1)
; NEG-SAME: constant { ptr addrspace(4), ptr addrspace(4) } {
; NEG-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv117__class_type_infoE to ptr addrspace(4)),
; NEG-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTS4Base to ptr addrspace(4)) }, comdat, align 8

@_ZTI7Derived = linkonce_odr hidden addrspace(1) constant { ptr addrspace(4), ptr addrspace(4), ptr addrspace(4) } { ptr addrspace(4) addrspacecast ( ptr addrspace(1) @_ZTVN10__cxxabiv120__si_class_type_infoE to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTS7Derived to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTI4Base to ptr addrspace(4)) }, comdat, align 8
; CHECK: @_ZTI7Derived = linkonce_odr hidden addrspace(1)
; CHECK-SAME: constant { ptr addrspace(4), ptr addrspace(4), ptr addrspace(4) } {
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) null to ptr addrspace(4)),
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTS7Derived to ptr addrspace(4)),
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTI4Base to ptr addrspace(4)) }, comdat, align 8

; NEG: @_ZTI7Derived = linkonce_odr hidden addrspace(1)
; NEG-SAME: constant { ptr addrspace(4), ptr addrspace(4), ptr addrspace(4) } {
; NEG-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv120__si_class_type_infoE to ptr addrspace(4)),
; NEG-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTS7Derived to ptr addrspace(4)),
; NEG-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTI4Base to ptr addrspace(4)) }, comdat, align 8

@_ZTV4Base = linkonce_odr hidden unnamed_addr addrspace(1) constant { [4 x ptr addrspace(4)] } { [4 x ptr addrspace(4)] [ ptr addrspace(4) null, ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTI4Base to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @_ZN4BaseD1Ev to ptr addrspace(4)) , ptr addrspace(4) addrspacecast (ptr  @_ZN4BaseD0Ev to ptr addrspace(4)) ]}, comdat, align 8
; CHECK: @_ZTV4Base = linkonce_odr hidden unnamed_addr addrspace(1)
; CHECK-SAME: constant { [4 x ptr addrspace(4)] } { [4 x ptr addrspace(4)] [
; CHECK-SAME: ptr addrspace(4) null,
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTI4Base to ptr addrspace(4)),
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr null to ptr addrspace(4)),
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr null to ptr addrspace(4))] }, comdat, align 8

; NEG: @_ZTV4Base = linkonce_odr hidden unnamed_addr addrspace(1)
; NEG-SAME: constant { [4 x ptr addrspace(4)] } { [4 x ptr addrspace(4)] [
; NEG-SAME: ptr addrspace(4) null,
; NEG-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTI4Base to ptr addrspace(4)),
; NEG-SAME: ptr addrspace(4) addrspacecast (ptr @_ZN4BaseD1Ev to ptr addrspace(4)),
; NEG-SAME: ptr addrspace(4) addrspacecast (ptr @_ZN4BaseD0Ev to ptr addrspace(4))] }, comdat, align 8

@.omp_offloading.entry_name = internal target_declare unnamed_addr addrspace(2) constant [40 x i8] c"__omp_offloading_805_e20624__Z4main_l18\00"
@.omp_offloading.entry.__omp_offloading_805_e20624__Z4main_l18 = weak target_declare local_unnamed_addr addrspace(2) constant %struct.__tgt_offload_entry { ptr addrspace(4) null, ptr addrspace(2) getelementptr inbounds ([40 x i8], ptr addrspace(2) @.omp_offloading.entry_name, i32 0, i32 0), i64 0, i32 0, i32 0, i64 40 }, section "omp_offloading_entries"

; Function Attrs: convergent nounwind
declare spir_func void @_ZN7DerivedD1Ev(ptr addrspace(4) dereferenceable_or_null(8)) unnamed_addr #0

; Function Attrs: convergent nounwind
declare spir_func void @_ZN7DerivedD0Ev(ptr addrspace(4) dereferenceable_or_null(8)) unnamed_addr #0

; Function Attrs: convergent nounwind
declare spir_func void @_ZN4BaseD1Ev(ptr addrspace(4) dereferenceable_or_null(8)) unnamed_addr #0

; Function Attrs: convergent nounwind
declare spir_func void @_ZN4BaseD0Ev(ptr addrspace(4) dereferenceable_or_null(8)) unnamed_addr #0

; Function Attrs: noinline norecurse nounwind
define weak dso_local spir_kernel void @__omp_offloading_805_e20624__Z4main_l18(ptr addrspace(1) %0, ptr addrspace(1) %Obj.ascast, ptr addrspace(1) %1) local_unnamed_addr #1 {
DIR.OMP.TARGET.3:
  call void @__itt_offload_wi_start_wrapper()
  %2 = tail call spir_func i64 @_Z12get_local_idj(i32 0) #3
  %3 = tail call spir_func i64 @_Z12get_local_idj(i32 1) #3
  %4 = or i64 %3, %2
  %5 = tail call spir_func i64 @_Z12get_local_idj(i32 2) #3
  %6 = or i64 %4, %5
  %7 = icmp eq i64 %6, 0
  call void @__itt_offload_wg_barrier_wrapper()
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784) #3
  call void @__itt_offload_wi_resume_wrapper()
  br i1 %7, label %master.thread.code, label %master.thread.fallthru

DIR.OMP.END.TARGET.57.exitStub:                   ; preds = %master.thread.code1, %master.thread.fallthru
  call void @__itt_offload_wg_barrier_wrapper()
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784) #3
  call void @__itt_offload_wi_resume_wrapper()
  call void @__itt_offload_wi_finish_wrapper()
  ret void

master.thread.code:                               ; preds = %DIR.OMP.TARGET.3
  %8 = getelementptr %struct.Base, ptr addrspace(1) %0, i64 0, i32 0
  store ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTV4Base to ptr addrspace(4)), ptr addrspace(1) %8, align 8, !tbaa !6, !alias.scope !9, !noalias !12
  br label %master.thread.fallthru

master.thread.fallthru:                           ; preds = %DIR.OMP.TARGET.3, %master.thread.code
  call void @__itt_offload_wg_barrier_wrapper()
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784) #3
  call void @__itt_offload_wi_resume_wrapper()
  call void @__itt_offload_wg_barrier_wrapper()
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784) #3
  call void @__itt_offload_wi_resume_wrapper()
  br i1 %7, label %master.thread.code1, label %DIR.OMP.END.TARGET.57.exitStub

master.thread.code1:                              ; preds = %master.thread.fallthru
  %9 = getelementptr %struct.Base, ptr addrspace(1) %0, i64 0, i32 0
  store ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTV7Derived to ptr addrspace(4)), ptr addrspace(1) %9, align 8, !tbaa !6, !alias.scope !9, !noalias !12
  br label %DIR.OMP.END.TARGET.57.exitStub
}

declare spir_func i64 @_Z12get_local_idj(i32) local_unnamed_addr

; Function Attrs: convergent
declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32) local_unnamed_addr #2

declare void @__itt_offload_wi_start_wrapper()

declare void @__itt_offload_wg_barrier_wrapper()

declare void @__itt_offload_wi_resume_wrapper()

declare void @__itt_offload_wi_finish_wrapper()

attributes #0 = { convergent nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { noinline norecurse nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target.declare"="true" "unsafe-fp-math"="true" }
attributes #2 = { convergent }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!spirv.Source = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{}
!4 = !{!"clang version 12.0.0"}
!5 = !{i32 4, i32 200000}
!6 = !{!7, !7, i64 0}
!7 = !{!"vtable pointer", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!10}
!10 = distinct !{!10, !11, !"OMPAliasScope"}
!11 = distinct !{!11, !"OMPDomain"}
!12 = !{!13}
!13 = distinct !{!13, !11, !"OMPAliasScope"}
