// RUN: %clang_cc1 %s -emit-spirv -triple spir-unknown-unknown -O0 -cl-std=CL2.0 -cl-ext=+cl_khr_subgroups -o %t.spv
// RUN: llvm-spirv -to-text %t.spv -o - | FileCheck %s

#pragma OPENCL EXTENSION cl_khr_subgroups : enable

// CHECK: TypePipe [[PipeROTy:[0-9]+]] 0
// CHECK: TypePipe [[PipeWOTy:[0-9]+]] 1

// CHECK-LABEL: 5 Function
void test1(read_only pipe int p, global int *ptr) {

// CHECK: Load [[PipeROTy]] [[ReadPipe11:[0-9]+]]
// CHECK: GetNumPipePackets {{[0-9]+}} {{[0-9]+}} [[ReadPipe11]]
  *ptr = get_pipe_num_packets(p);

// CHECK: Load [[PipeROTy]] [[ReadPipe12:[0-9]+]]
// CHECK: GetMaxPipePackets {{[0-9]+}} {{[0-9]+}} [[ReadPipe12]]
  *ptr = get_pipe_max_packets(p);

// CHECK: Load [[PipeROTy]] [[ReadPipe13:[0-9]+]]
// CHECK: ReadPipe {{[0-9]+}} {{[0-9]+}} [[ReadPipe13]]
  read_pipe(p, ptr);

// CHECK: Load [[PipeROTy]] [[ReadPipe14:[0-9]+]]
// CHECK: ReserveReadPipePackets {{[0-9]+}} {{[0-9]+}} [[ReadPipe14]]
  reserve_id_t rid = reserve_read_pipe(p, 2);

// CHECK: Load [[PipeROTy]] [[ReadPipe15:[0-9]+]]
// CHECK: ReservedReadPipe {{[0-9]+}} {{[0-9]+}} [[ReadPipe15]]
  read_pipe(p, rid, 2, ptr);

// CHECK: Load [[PipeROTy]] [[ReadPipe16:[0-9]+]]
// CHECK: CommitReadPipe [[ReadPipe16]]
  commit_read_pipe(p, rid);
}

// CHECK-LABEL: 5 Function
void test2(write_only pipe int p, global int *ptr) {

// CHECK: Load [[PipeWOTy]] [[WritePipe21:[0-9]+]]
// CHECK: GetNumPipePackets {{[0-9]+}} {{[0-9]+}} [[WritePipe21]]
  *ptr = get_pipe_num_packets(p);

// CHECK: Load [[PipeWOTy]] [[WritePipe22:[0-9]+]]
// CHECK: GetMaxPipePackets {{[0-9]+}} {{[0-9]+}}  [[WritePipe22]]
  *ptr = get_pipe_max_packets(p);

// CHECK: Load [[PipeWOTy]] [[WritePipe23:[0-9]+]]
// CHECK: WritePipe {{[0-9]+}} {{[0-9]+}} [[WritePipe23]]
  write_pipe(p, ptr);

// CHECK: Load [[PipeWOTy]] [[WritePipe24:[0-9]+]]
// CHECK: ReserveWritePipePackets {{[0-9]+}} {{[0-9]+}} [[WritePipe24]]
  reserve_id_t rid = reserve_write_pipe(p, 2);

// CHECK: Load [[PipeWOTy]] [[WritePipe25:[0-9]+]]
// CHECK: ReservedWritePipe {{[0-9]+}} {{[0-9]+}} [[WritePipe25]]
  write_pipe(p, rid, 2, ptr);

// CHECK: Load [[PipeWOTy]] [[WritePipe26:[0-9]+]]
// CHECK: CommitWritePipe [[WritePipe26]]
  commit_write_pipe(p, rid);
}

// CHECK-LABEL: 5 Function
void test3(read_only pipe int p, global int *ptr) {

// CHECK: Load [[PipeROTy]] [[ReadPipe31:[0-9]+]]
// CHECK: GroupReserveReadPipePackets {{[0-9]+}} {{[0-9]+}} {{[0-9]+}} [[ReadPipe31]]
  reserve_id_t rid = work_group_reserve_read_pipe(p, 2);

// CHECK: Load [[PipeROTy]] [[ReadPipe32:[0-9]+]]
// CHECK: GroupCommitReadPipe {{[0-9]+}} [[ReadPipe32]]
  work_group_commit_read_pipe(p, rid);
}

// CHECK-LABEL: 5 Function
void test4(write_only pipe int p, global int *ptr) {

// CHECK: Load [[PipeWOTy]] [[WritePipe41:[0-9]+]]
// CHECK: GroupReserveWritePipePackets {{[0-9]+}} {{[0-9]+}} {{[0-9]+}} [[WritePipe41]]
  reserve_id_t rid = work_group_reserve_write_pipe(p, 2);

// CHECK: Load [[PipeWOTy]] [[WritePipe42:[0-9]+]]
// CHECK: GroupCommitWritePipe {{[0-9]+}} [[WritePipe42]]
  work_group_commit_write_pipe(p, rid);
}

// CHECK-LABEL: 5 Function
void test5(read_only pipe int p, global int *ptr) {

// CHECK: Load [[PipeROTy]] [[ReadPipe51:[0-9]+]]
// CHECK: GroupReserveReadPipePackets {{[0-9]+}} {{[0-9]+}} {{[0-9]+}} [[ReadPipe51]]
  reserve_id_t rid = sub_group_reserve_read_pipe(p, 2);

// CHECK: Load [[PipeROTy]] [[ReadPipe52:[0-9]+]]
// CHECK: GroupCommitReadPipe {{[0-9]+}} [[ReadPipe52]]
  sub_group_commit_read_pipe(p, rid);
}

// CHECK-LABEL: 5 Function
void test6(write_only pipe int p, global int *ptr) {

// CHECK: Load [[PipeWOTy]] [[WritePipe61:[0-9]+]]
// CHECK: GroupReserveWritePipePackets {{[0-9]+}} {{[0-9]+}} {{[0-9]+}} [[WritePipe61]]
  reserve_id_t rid = sub_group_reserve_write_pipe(p, 2);

// CHECK: Load [[PipeWOTy]] [[WritePipe62:[0-9]+]]
// CHECK: GroupCommitWritePipe {{[0-9]+}} [[WritePipe62]]
  sub_group_commit_write_pipe(p, rid);
}
