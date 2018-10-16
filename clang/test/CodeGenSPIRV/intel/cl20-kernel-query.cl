// RUN: %clang_cc1 -triple spir-unknown-unknown -O0 -cl-std=CL2.0 %s -emit-spirv -o %t.spv
// RUN: llvm-spirv %t.spv -to-text -o - | FileCheck %s

; XFAIL: *
; TODO: The test case fails after commit 71264769ee5. This should pass once we
; update our SPIR-V translator and possibly BE to work with newly introduced
; format of LLVM IR.

typedef struct {int a;} ndrange_t;

// CHECK: EntryPoint {{[0-9]+}} [[BlockKer1:[0-9]+]] "__device_side_enqueue_block_invoke_kernel"
// CHECK: EntryPoint {{[0-9]+}} [[BlockKer2:[0-9]+]] "__device_side_enqueue_block_invoke_2_kernel"
// CHECK: EntryPoint {{[0-9]+}} [[BlockKer3:[0-9]+]] "__device_side_enqueue_block_invoke_3_kernel"
// CHECK: Name [[BlockGl1:[0-9]+]] "__block_literal_global"
// CHECK: Name [[BlockGl2:[0-9]+]] "__block_literal_global.1"
// CHECK: Name [[BlockGl3:[0-9]+]] "__block_literal_global.2"

// CHECK-DAG: TypeInt [[Int32Ty:[0-9]+]] 32
// CHECK-DAG: TypeInt [[Int8Ty:[0-9]+]] 8

// CHECK-DAG: Constant [[Int32Ty]] [[ConstInt8:[0-9]+]] 8

// CHECK-DAG: TypeVoid [[VoidTy:[0-9]+]]

// CHECK-DAG: 3 TypeStruct [[NDRangeTy:[0-9]+]] [[Int32Ty]]
// CHECK-DAG: TypePointer [[NDRangePtrTy:[0-9]+]] 7 [[NDRangeTy]]
// CHECK-DAG: TypePointer [[Int8PtrGenTy:[0-9]+]] 8 [[Int8Ty]]

// CHECK-DAG: 4 TypeFunction [[BlockTy1:[0-9]+]] [[VoidTy]] [[Int8PtrGenTy]]

kernel void device_side_enqueue(global int *a, global int *b, int i) {
  queue_t default_queue;
  unsigned flags = 0;
  ndrange_t ndrange;
  clk_event_t clk_event;
  clk_event_t event_wait_list;
  clk_event_t event_wait_list2[] = {clk_event};

  void (^const block_A)(void) = ^{
    return;
  };

// CHECK: Variable [[NDRangePtrTy]] [[NDRange1:[0-9]+]]

// CHECK: Bitcast {{[0-9]+}} [[BlockLit1Tmp:[0-9]+]] [[BlockGl1]]
// CHECK: PtrCastToGeneric [[Int8PtrGenTy]] [[BlockLit1:[0-9]+]] [[BlockLit1Tmp]]
// CHECK: GetKernelWorkGroupSize
//        [[Int32Ty]] {{[0-9]+}} [[BlockKer1]] [[BlockLit1]] [[ConstInt8]]
//        [[ConstInt8]]

  // Uses block kernel and global block literal.
  unsigned size = get_kernel_work_group_size(block_A);

// CHECK: GetKernelPreferredWorkGroupSizeMultiple
//        [[Int32Ty]] {{[0-9]+}} [[BlockKer1]] [[BlockLit1]] [[ConstInt8]]
//        [[ConstInt8]]

  // Uses global block literal and block kernel.
  size = get_kernel_preferred_work_group_size_multiple(block_A);

#pragma OPENCL EXTENSION cl_khr_subgroups : enable

// CHECK: Bitcast {{[0-9]+}} [[BlockLit2Tmp:[0-9]+]] [[BlockGl2]]
// CHECK: PtrCastToGeneric [[Int8PtrGenTy]] [[BlockLit2:[0-9]+]] [[BlockLit2Tmp]]
// CHECK: GetKernelNDrangeMaxSubGroupSize
//        [[Int32Ty]] {{[0-9]+}} {{[0-9]+}} [[BlockKer2]] [[BlockLit2]]
//        [[ConstInt8]] [[ConstInt8]]

  // Emits global block literal and block kernel.
  size = get_kernel_max_sub_group_size_for_ndrange(ndrange, ^(){});

// CHECK: Bitcast {{[0-9]+}} [[BlockLit3Tmp:[0-9]+]] [[BlockGl3]]
// CHECK: PtrCastToGeneric [[Int8PtrGenTy]] [[BlockLit3:[0-9]+]] [[BlockLit3Tmp]]
// CHECK: GetKernelNDrangeSubGroupCount
//        [[Int32Ty]] {{[0-9]+}} [[NDRange1]] [[BlockKer1]] [[BlockLit1]]
//        [[ConstInt8]] [[ConstInt8]]

  // Emits global block literal and block kernel.
  size = get_kernel_sub_group_count_for_ndrange(ndrange, ^(){});
}

// CHECK-DAG: Function [[VoidTy]] [[BlockKer1]] 0 [[BlockTy1]]
// CHECK-DAG: Function [[VoidTy]] [[BlockKer2]] 0 [[BlockTy1]]
// CHECK-DAG: Function [[VoidTy]] [[BlockKer3]] 0 [[BlockTy1]]
