; Compiled from:
; --------------
; #include <CL/sycl.hpp>
; #include <CL/sycl/intel/fpga_extensions.hpp>
; #include <iostream>
;
; using namespace cl::sycl;
;
; constexpr size_t N = 10;
; constexpr size_t D = N * 2;
;
; // Specialize a pipe type
; using my_m2s_pipe = pipe<class m2s_pipe, int, D>;
; using my_s2m_pipe = pipe<class s2m_pipe, int, D>;
;
; class master_task;
; class slave_task;
;
; void master(queue &q, buffer<int, 1> &src_buf, buffer<int, 1> &dst_buf) {
;   q.submit([&](handler &cgh) {
;     // Get read access to src array
;     auto src_buf_acc = src_buf.get_access<access::mode::read>(cgh);
;     // Get write access to dst array
;     auto dst_buf_acc = dst_buf.get_access<access::mode::write>(cgh);
;     cgh.single_task<master_task>([=]() {
;       for (int i = 0; i < N; i++) {
;         // Blocking write an int to the m2s_pipe
;         my_m2s_pipe::write(src_buf_acc[i]);
;       }
;
;       for (int i = N; i < D; i++) {
;         // Non-blocking write an int to the m2s_pipe
;         bool success = false;
;         while (!success) {
;           my_m2s_pipe::write(src_buf_acc[i], success);
;         }
;       }
;
;       for (int i = 0; i < N; i++) {
;         // Blocking read an int from the s2m_pipe
;         dst_buf_acc[i] = my_s2m_pipe::read();
;       }
;
;       for (int i = N; i < D; i++) {
;         // Non-blocking read an int from the s2m_pipe
;         bool success = false;
;         while (!success) {
;           dst_buf_acc[i] = my_s2m_pipe::read(success);
;         }
;       }
;     });
;   });
; }
;
; void slave(queue &q) {
;   q.submit([&](handler &cgh) {
;     cgh.single_task<slave_task>([=]() {
;       std::array<int, D> tmp;
;       for (int i = 0; i < N; i++) {
;         // Blocking read an int from the m2s_pipe
;         tmp[i] = my_m2s_pipe::read();
;       }
;
;       for (int i = N; i < D; i++) {
;         // Non-blocking read an int from the m2s_pipe
;         bool success = false;
;         while (!success) {
;           tmp[i] = my_m2s_pipe::read(success);
;         }
;       }
;
;       for (int i = 0; i < N; i++) {
;         // Blocking write an int to the s2m_pipe
;         my_s2m_pipe::write(tmp[i]);
;       }
;
;       for (int i = N; i < D; i++) {
;         // Non-blocking write an int to the s2m_pipe
;         bool success = false;
;         while (!success) {
;           my_s2m_pipe::write(tmp[i], success);
;         }
;       }
;     });
;   });
; }
;
; int main() {
;   std::array<int, D> src, dst;
;   for (int i = 0; i < D; i++) {
;     src[i] = rand();
;   }
;
;   {
; // #ifdef FPGA_EMULATOR
; //     intel::fpga_emulator_selector device_selector;
; // #else
; //     intel::fpga_selector device_selector;
; // #endif
;     queue q;
;     buffer<int, 1> src_buf(src.data(), D);
;     buffer<int, 1> dst_buf(dst.data(), D);
;
;     master(q, src_buf, dst_buf);
;     slave(q);
;   }
;
;   bool passed = true;
;   for (int i = 0; i < D; i++) {
;     passed &= (src[i] == dst[i]);
;     if (!passed) {
;       printf("src[%d] = %d, dst[%d] = %d\n", i, src[i], i, dst[i]);
;     }
;   }
;
;   if (passed) {
;     printf("PASSED\n");
;   } else {
;     printf("FAILED\n");
;   }
;
;   return 0;
; }
; --------------

; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage = type { i32, i32, i32 }
%opencl.pipe_ro_t.6 = type opaque

@_ZN2cl4sycl4pipeI8s2m_pipeiLi20EE9m_StorageE = available_externally addrspace(1) constant %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage { i32 4, i32 4, i32 20 }, align 4

; Function Attrs: nounwind
define internal i32 @_ZN2cl4sycl4pipeI8s2m_pipeiLi20EE4readEv() #0 {
entry:
  %TempData = alloca i32, align 4
  ;; CHECK: %call = call %opencl.pipe_ro_t.6 addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_read{{.*}}({{.*}}addrspacecast{{.*}}bitcast (%opencl.pipe_rw_t addrspace(1)* addrspace(1)* @_ZN2cl4sycl4pipeI8s2m_pipeiLi20EE9m_StorageE.syclpipe to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)*)
  %call = call %opencl.pipe_ro_t.6 addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* addrspacecast (%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* @_ZN2cl4sycl4pipeI8s2m_pipeiLi20EE9m_StorageE to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*)) #0
  %0 = bitcast i32* %TempData to i8*
  %1 = bitcast i32* %TempData to i8*
  %2 = addrspacecast i8* %1 to i8 addrspace(4)*
  %3 = call i32 @__read_pipe_2_bl(%opencl.pipe_ro_t.6 addrspace(1)* %call, i8 addrspace(4)* %2, i32 4, i32 4) #0
  %4 = load i32, i32* %TempData, align 4
  %5 = bitcast i32* %TempData to i8*
  ret i32 %4
}

; Function Attrs: nounwind
declare %opencl.pipe_ro_t.6 addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %0) #0


; Function Attrs: convergent nounwind
declare i32 @__read_pipe_2_bl(%opencl.pipe_ro_t.6 addrspace(1)* %0, i8 addrspace(4)* nocapture %1, i32 %2, i32 %3) #1

; Function Attrs: convergent nounwind
declare i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t.6 addrspace(1)* %0, i8 addrspace(4)* nocapture %1, i32 %2, i32 %3) #1


attributes #0 = { nounwind }
attributes #1 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!3}
!opencl.spir.version = !{!4}
!opencl.ocl.version = !{!5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!spirv.Generator = !{!7}

!0 = !{i32 4}
!1 = !{i32 20}
!2 = !{!""}
!3 = !{i32 4, i32 100000}
!4 = !{i32 1, i32 2}
!5 = !{i32 1, i32 0}
!6 = !{}
!7 = !{i16 6, i16 14}
