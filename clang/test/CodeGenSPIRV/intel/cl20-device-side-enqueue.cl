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
// CHECK: EntryPoint {{[0-9]+}} [[BlockKer4:[0-9]+]] "__device_side_enqueue_block_invoke_4_kernel"
// CHECK: Name [[BlockGl1:[0-9]+]] "__block_literal_global"
// CHECK: Name [[BlockGl2:[0-9]+]] "__block_literal_global.1"
// CHECK: Name [[BlockGl3:[0-9]+]] "__block_literal_global.2"

// CHECK-DAG: TypeInt [[Int32Ty:[0-9]+]] 32
// CHECK-DAG: TypeInt [[Int8Ty:[0-9]+]] 8

// CHECK-DAG: Constant [[Int32Ty]] [[ConstInt8:[0-9]+]] 8
// CHECK-DAG: Constant [[Int32Ty]] [[ConstInt4:[0-9]+]] 4
// CHECK-DAG: Constant [[Int32Ty]] [[ConstInt1:[0-9]+]] 1
// CHECK-DAG: Constant [[Int32Ty]] [[Int32Null:[0-9]+]] 0
// CHECK-DAG: Constant [[Int32Ty]] [[ConstInt20:[0-9]+]] 20
// CHECK-DAG: Constant [[Int32Ty]] [[ConstInt2:[0-9]+]] 2
// CHECK-DAG: Constant [[Int32Ty]] [[ConstInt3:[0-9]+]] 3

// CHECK-DAG: TypeVoid [[VoidTy:[0-9]+]]

// CHECK-DAG: TypePointer [[Int32PtrTy:[0-9]+]] 5 [[Int32Ty]]
// CHECK-DAG: TypePointer [[Int32PtrLocTy:[0-9]+]] 7 [[Int32Ty]]

// CHECK-DAG: TypeQueue [[QueueTy:[0-9]+]]
// CHECK-DAG: 3 TypeStruct [[NDRangeTy:[0-9]+]] [[Int32Ty]]
// CHECK-DAG: TypePointer [[NDRangePtrTy:[0-9]+]] 7 [[NDRangeTy]]
// CHECK-DAG: TypeDeviceEvent [[EventTy:[0-9]+]]
// CHECK-DAG: TypeStruct [[BlockLit1Ty:[0-9]+]] [[Int32Ty]] [[Int32Ty]] [[Int32PtrTy]] [[Int32Ty]] [[Int32PtrTy]]
// CHECK-DAG: TypePointer [[BlockLit1PtrTy:[0-9]+]] 7 [[BlockLit1Ty]]
// CHECK-DAG: TypePointer [[Int8PtrGenTy:[0-9]+]] 8 [[Int8Ty]]
// CHECK-DAG: TypePointer [[EventPtrTy:[0-9]+]] 8 [[EventTy]]

// CHECK-DAG: 4 TypeFunction [[BlockTy1:[0-9]+]] [[VoidTy]] [[Int8PtrGenTy]]

// CHECK-DAG: TypeArray [[LocalBufs3Ty:[0-9]+]] [[Int32Ty]] [[ConstInt1]]
// CHECK-DAG: TypePointer [[LocalBufs3PtrTy:[0-9]+]] 7 [[LocalBufs3Ty]]

// CHECK-DAG: 5 TypeFunction [[BlockTy2:[0-9]+]] [[VoidTy]] [[Int8PtrGenTy]]

// CHECK-DAG: TypeArray [[LocalBufs4Ty:[0-9]+]] [[Int32Ty]] [[ConstInt3]]
// CHECK-DAG: TypePointer [[LocalBufs4PtrTy:[0-9]+]] 7 [[LocalBufs4Ty]]

// CHECK-DAG: 7 TypeFunction [[BlockTy3:[0-9]+]] [[VoidTy]] [[Int8PtrGenTy]]

// CHECK-DAG: ConstantNull [[EventPtrTy]] [[EventNull:[0-9]+]]

kernel void device_side_enqueue(global int *a, global int *b, int i) {
  queue_t default_queue;
  unsigned flags = 0;
  ndrange_t ndrange;
  clk_event_t clk_event;
  clk_event_t event_wait_list;
  clk_event_t event_wait_list2[] = {clk_event};

// CHECK: Variable [[BlockLit1PtrTy]] [[BlockLit1Var:[0-9]+]]
// CHECK: Variable [[NDRangePtrTy]] [[NDRange1:[0-9]+]]

// CHECK: Variable [[BlockLit1PtrTy]] [[BlockLit2Var:[0-9]+]]
// CHECK: Variable [[NDRangePtrTy]] [[NDRange2:[0-9]+]]

// CHECK: Variable [[NDRangePtrTy]] [[NDRange3:[0-9]+]]
// CHECK: Variable [[LocalBufs3PtrTy]] [[LocalBufsVar3:[0-9]+]]
//
// CHECK: Variable [[NDRangePtrTy]] [[NDRange4:[0-9]+]]
// CHECK: Variable [[LocalBufs4PtrTy]] [[LocalBufsVar4:[0-9]+]]

// CHECK: Load [[QueueTy]] [[Queue1:[0-9]+]]
// CHECK: Load [[Int32Ty]] [[Flags1:[0-9]+]]

// CHECK: PtrCastToGeneric [[Int8PtrGenTy]] [[BlockLit1:[0-9]+]] [[BlockLit1Var]]
// CHECK: EnqueueKernel [[Int32Ty]] {{[0-9]+}} [[Queue1]] [[Flags1]] [[NDRange1]]
//                      [[Int32Null]] [[EventNull]] [[EventNull]]
//                      [[BlockKer1]] [[BlockLit1]] [[ConstInt20]] [[ConstInt8]]

  // Emits block literal on stack and block kernel.
  enqueue_kernel(default_queue, flags, ndrange,
                 ^(void) {
                   a[i] = b[i];
                 });

// CHECK: Load [[QueueTy]] [[Queue2:[0-9]+]]
// CHECK: Load [[Int32Ty]] [[Flags2:[0-9]+]]

// CHECK: PtrCastToGeneric [[EventPtrTy]] [[Event20:[0-9]+]]
// CHECK: PtrCastToGeneric [[EventPtrTy]] [[Event21:[0-9]+]]

// CHECK: PtrCastToGeneric [[Int8PtrGenTy]] [[BlockLit2:[0-9]+]] [[BlockLit2Var]]
// CHECK: EnqueueKernel [[Int32Ty]] {{[0-9]+}} [[Queue2]] [[Flags2]] [[NDRange2]]
//                      [[ConstInt2]] [[Event20]] [[Event21]]
//                      [[BlockKer2]] [[BlockLit2]] [[ConstInt20]] [[ConstInt8]]

  // Emits block literal on stack and block kernel.
  enqueue_kernel(default_queue, flags, ndrange, 2, &event_wait_list, &clk_event,
                 ^(void) {
                   a[i] = b[i];
                 });

  char c;

// CHECK: Load [[QueueTy]] [[Queue3:[0-9]+]]
// CHECK: Load [[Int32Ty]] [[Flags3:[0-9]+]]

// CHECK: PtrCastToGeneric [[EventPtrTy]] [[Event30:[0-9]+]]
// CHECK: PtrCastToGeneric [[EventPtrTy]] [[Event31:[0-9]+]]

// CHECK: PtrAccessChain [[Int32PtrLocTy]] [[LocalBuf31:[0-9]+]] [[LocalBufsVar3]]
// CHECK: Store [[LocalBuf31]]
// CHECK: PtrAccessChain [[Int32PtrLocTy]] [[LocalBufs3:[0-9]+]] [[LocalBufsVar3]] [[Int32Null]] [[Int32Null]]

// CHECK: Bitcast {{[0-9]+}} [[BlockLit3Tmp:[0-9]+]] [[BlockGl1]]
// CHECK: PtrCastToGeneric [[Int8PtrGenTy]] [[BlockLit3:[0-9]+]] [[BlockLit3Tmp]]
// CHECK: EnqueueKernel [[Int32Ty]] {{[0-9]+}} [[Queue3]] [[Flags3]] [[NDRange3]]
//                      [[ConstInt2]] [[Event30]] [[Event31]]
//                      [[BlockKer3]] [[BlockLit3]] [[ConstInt8]] [[ConstInt8]]
//                      [[LocalBufs3]]

  // Emits global block literal and block kernel.
  enqueue_kernel(default_queue, flags, ndrange, 2, event_wait_list2, &clk_event,
                 ^(local void *p) {
                   return;
                 },
                 c);

// CHECK: Load [[QueueTy]] [[Queue4:[0-9]+]]
// CHECK: Load [[Int32Ty]] [[Flags4:[0-9]+]]

// CHECK: Store {{[0-9]+}} [[ConstInt1]]
// CHECK: Store {{[0-9]+}} [[ConstInt2]]
// CHECK: Store {{[0-9]+}} [[ConstInt4]]
// CHECK: PtrAccessChain [[Int32PtrLocTy]] [[LocalBufs41:[0-9]+]] [[LocalBufsVar4]] [[Int32Null]] [[Int32Null]]
// CHECK: PtrAccessChain [[Int32PtrLocTy]] [[LocalBufs42:[0-9]+]] [[LocalBufsVar4]] [[Int32Null]] [[ConstInt1]]
// CHECK: PtrAccessChain [[Int32PtrLocTy]] [[LocalBufs42:[0-9]+]] [[LocalBufsVar4]] [[Int32Null]] [[ConstInt2]]

// CHECK: Bitcast {{[0-9]+}} [[BlockLit4Tmp:[0-9]+]] [[BlockGl2]]
// CHECK: PtrCastToGeneric [[Int8PtrGenTy]] [[BlockLit4:[0-9]+]] [[BlockLit4Tmp]]
// CHECK: EnqueueKernel [[Int32Ty]] {{[0-9]+}} [[Queue4]] [[Flags4]] [[NDRange4]]
//                      [[Int32Null]] [[EventNull]] [[EventNull]]
//                      [[BlockKer4]] [[BlockLit4]] [[ConstInt8]] [[ConstInt8]]
//                      [[LocalBufs41]] [[LocalBufs42]] [[LocalBufs43]]

  // Emits global block literal and block kernel.
  enqueue_kernel(default_queue, flags, ndrange,
                 ^(local void *p1, local void *p2, local void *p3) {
                   return;
                 },
                 1, 2, 4);

  void (^const block_A)(void) = ^{
    return;
  };
}

// CHECK-DAG: Function [[VoidTy]] [[BlockKer1]] 0 [[BlockTy1]]
// CHECK-DAG: Function [[VoidTy]] [[BlockKer2]] 0 [[BlockTy1]]
// CHECK-DAG: Function [[VoidTy]] [[BlockKer3]] 0 [[BlockTy2]]
// CHECK-DAG: Function [[VoidTy]] [[BlockKer4]] 0 [[BlockTy3]]
