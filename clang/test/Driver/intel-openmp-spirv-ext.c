// INTEL

// RUN: %clangxx -target x86_64-unknown-linux-gnu --intel -fiopenmp -fopenmp-targets=spir64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=CHECK-DEFAULT
// RUN: %clangxx -target x86_64-unknown-linux-gnu --intel -fiopenmp -fopenmp-targets=spir64 -fopenmp-target-simd %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=CHECK-SIMD

// CHECK-DEFAULT: llvm-spirv{{.*}}"-spirv-ext=-all
// CHECK-DEFAULT-SAME:,+SPV_EXT_shader_atomic_float_add
// CHECK-DEFAULT-SAME:,+SPV_EXT_shader_atomic_float_min_max
// CHECK-DEFAULT-SAME:,+SPV_KHR_no_integer_wrap_decoration,+SPV_KHR_float_controls
// CHECK-DEFAULT-SAME:,+SPV_KHR_expect_assume,+SPV_KHR_linkonce_odr,+SPV_INTEL_subgroups
// CHECK-DEFAULT-SAME:,+SPV_INTEL_media_block_io
// CHECK-DEFAULT-SAME:,+SPV_INTEL_device_side_avc_motion_estimation
// CHECK-DEFAULT-SAME:,+SPV_INTEL_fpga_loop_controls
// CHECK-DEFAULT-SAME:,+SPV_INTEL_unstructured_loop_controls,+SPV_INTEL_fpga_reg
// CHECK-DEFAULT-SAME:,+SPV_INTEL_blocking_pipes,+SPV_INTEL_function_pointers
// CHECK-DEFAULT-SAME:,+SPV_INTEL_kernel_attributes,+SPV_INTEL_io_pipes
// CHECK-DEFAULT-SAME:,+SPV_INTEL_inline_assembly,+SPV_INTEL_arbitrary_precision_integers
// CHECK-DEFAULT-SAME:,+SPV_INTEL_float_controls2,+SPV_INTEL_vector_compute
// CHECK-DEFAULT-SAME:,+SPV_INTEL_fast_composite
// CHECK-DEFAULT-SAME:,+SPV_INTEL_joint_matrix
// CHECK-DEFAULT-SAME:,+SPV_INTEL_arbitrary_precision_fixed_point
// CHECK-DEFAULT-SAME:,+SPV_INTEL_arbitrary_precision_floating_point
// CHECK-DEFAULT-SAME:,+SPV_INTEL_variable_length_array,+SPV_INTEL_fp_fast_math_mode
// CHECK-DEFAULT-SAME:,+SPV_INTEL_long_constant_composite
// CHECK-DEFAULT-SAME:,+SPV_INTEL_arithmetic_fence
// CHECK-DEFAULT-SAME:,+SPV_INTEL_task_sequence
// CHECK-DEFAULT-SAME:,+SPV_INTEL_optnone
// CHECK-DEFAULT-SAME:,+SPV_INTEL_token_type
// CHECK-DEFAULT-SAME:,+SPV_INTEL_bfloat16_conversion
// CHECK-DEFAULT-SAME:,+SPV_INTEL_joint_matrix
// CHECK-DEFAULT-SAME:,+SPV_INTEL_hw_thread_queries
// CHECK-DEFAULT-SAME:,+SPV_INTEL_memory_access_aliasing
// CHECK-DEFAULT-SAME:,+SPV_KHR_uniform_group_instructions"
// CHECK-SIMD: llvm-spirv{{.*}}"-spirv-ext=-all
// CHECK-SIMD-SAME:,+SPV_EXT_shader_atomic_float_add
// CHECK-SIMD-SAME:,+SPV_EXT_shader_atomic_float_min_max
// CHECK-SIMD-SAME:,+SPV_KHR_no_integer_wrap_decoration,+SPV_KHR_float_controls
// CHECK-SIMD-SAME:,+SPV_KHR_expect_assume,+SPV_KHR_linkonce_odr,+SPV_INTEL_subgroups
// CHECK-SIMD-SAME:,+SPV_INTEL_media_block_io
// CHECK-SIMD-SAME:,+SPV_INTEL_device_side_avc_motion_estimation
// CHECK-SIMD-SAME:,+SPV_INTEL_fpga_loop_controls
// CHECK-SIMD-SAME:,+SPV_INTEL_unstructured_loop_controls,+SPV_INTEL_fpga_reg
// CHECK-SIMD-SAME:,+SPV_INTEL_blocking_pipes,+SPV_INTEL_function_pointers
// CHECK-SIMD-SAME:,+SPV_INTEL_kernel_attributes,+SPV_INTEL_io_pipes
// CHECK-SIMD-SAME:,+SPV_INTEL_inline_assembly,+SPV_INTEL_arbitrary_precision_integers
// CHECK-SIMD-SAME:,+SPV_INTEL_float_controls2,+SPV_INTEL_vector_compute
// CHECK-SIMD-SAME:,+SPV_INTEL_fast_composite
// CHECK-SIMD-SAME:,+SPV_INTEL_joint_matrix
// CHECK-SIMD-SAME:,+SPV_INTEL_arbitrary_precision_fixed_point
// CHECK-SIMD-SAME:,+SPV_INTEL_arbitrary_precision_floating_point
// CHECK-SIMD-SAME:,+SPV_INTEL_variable_length_array,+SPV_INTEL_fp_fast_math_mode
// CHECK-SIMD-SAME:,+SPV_INTEL_long_constant_composite
// CHECK-SIMD-SAME:,+SPV_INTEL_arithmetic_fence
// CHECK-SIMD-SAME:,+SPV_INTEL_task_sequence
// CHECK-SIMD-SAME:,+SPV_INTEL_token_type
// CHECK-SIMD-SAME:,+SPV_INTEL_bfloat16_conversion
// CHECK-SIMD-SAME:,+SPV_INTEL_joint_matrix
// CHECK-SIND-SAME:,+SPV_INTEL_hw_thread_queries
// CHECK-SIMD-SAME:,+SPV_INTEL_memory_access_aliasing
// CHECK-SIMD-SAME:,+SPV_KHR_uniform_group_instructions"
