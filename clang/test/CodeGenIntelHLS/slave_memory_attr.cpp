//RUN: %clang_cc1 -fhls -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s
//
#define component __attribute__((ihc_component))
#define slave_arg __attribute__((local_mem_size(32), slave_memory_argument))

//CHECK: define{{.*}}foo0a
//CHECK-SAME: !ihc_component [[CFOO0A:![0-9]+]]
//CHECK-SAME: !arg_type [[ATFOO0A:![0-9]+]]
//CHECK-SAME: !memory [[MFOO0A:![0-9]+]]
//CHECK-SAME: !local_mem_size [[LMSFOO0A:![0-9]+]]

component int foo0a(int *j0,
                    slave_arg int *j1,
                    slave_arg __attribute__((memory("MLAB")))      int *i0,
                    slave_arg __attribute__((memory("BLOCK_RAM"))) int *i1,
                    slave_arg __attribute__((numbanks(4)))         int *i2,
                    slave_arg __attribute__((bankwidth(4)))        int *i3,
                    slave_arg __attribute__((singlepump))          int *i4,
                    slave_arg __attribute__((doublepump))          int *i5,
                    slave_arg __attribute__((bank_bits(4,3,2)))    int *i7,
                    slave_arg __attribute__((internal_max_block_ram_depth(32)))
                    int *i10,
                    slave_arg __attribute__((readwrite_mode("readonly")))
                    int *i11,
                    slave_arg __attribute__((readwrite_mode("writeonly")))
                    int *i12,
                    slave_arg __attribute__((readwrite_mode("readwrite")))
                    int *i13)
{
  return 0;
}

//CHECK: define{{.*}}foo0b
//CHECK-SAME: !ihc_component [[CFOO1A:![0-9]+]]
//CHECK-SAME: !arg_type [[ATFOO1A:![0-9]+]]
//CHECK-SAME: !memory [[MFOO1A:![0-9]+]]
//CHECK-SAME: !local_mem_size [[LMSFOO1A:![0-9]+]]
component
int foo0b(slave_arg __attribute__((memory("MLAB")))
                    __attribute__((singlepump))
                    __attribute__((bankwidth(4)))
                    __attribute__((numbanks(8)))
                    __attribute__((bank_bits(4,3,2)))
                    __attribute__((internal_max_block_ram_depth(64)))
                    __attribute__((readwrite_mode("readonly")))
          int *i0)
{
  return 0;
}
//CHECK: [[MFOO0A]] =
//CHECK-SAME: !{!"",
//CHECK-SAME: !"",
//CHECK-SAME: !"{memory:MLAB}{sizeinfo:8}",
//CHECK-SAME: !"{memory:BLOCK_RAM}{sizeinfo:8}",
//CHECK-SAME: !"{memory:DEFAULT}{sizeinfo:8}{numbanks:4}",
//CHECK-SAME: !"{memory:DEFAULT}{sizeinfo:8}{bankwidth:4}",
//CHECK-SAME: !"{memory:DEFAULT}{sizeinfo:8}{pump:1}",
//CHECK-SAME: !"{memory:DEFAULT}{sizeinfo:8}{pump:2}",
//CHECK-SAME: !"{memory:DEFAULT}{sizeinfo:8}{numbanks:8}{bank_bits:4,3,2}",
//CHECK-SAME: !"{readwritememory:READONLY}",
//CHECK-SAME: !"{readwritememory:WRITEONLY}",
//CHECK-SAME: !"{readwritememory:READWRITE}"}

//CHECK: [[CFOO1A]] = !{!"_Z5foo0bPi", i32 undef}
//CHECK: [[ATFOO1A]] = !{!"mm_slave"}
//CHECK: [[MFOO1A]] = !{!"{memory:MLAB}{sizeinfo:8}{pump:1}{bankwidth:4}{numbanks:8}
//CHECK-SAME: {internal_max_block_ram_depth:64}{bank_bits:4,3,2}{readwritememory:READONLY}"
//CHECK: [[LMSFOO1A]] = !{i32 32}
