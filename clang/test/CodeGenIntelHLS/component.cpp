//RUN: %clang_cc1 -fhls -emit-llvm -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -debug-info-kind=limited -emit-llvm -triple x86_64-unknown-linux-gnu -o - %s

//CHECK: [[IMD:@.str[\.]*[0-9]*]] = {{.*}}{internal_max_block_ram_depth:64}
//CHECK: [[IMD2:@.str[\.]*[0-9]*]] = {{.*}}{internal_max_block_ram_depth:512}

//CHECK: @_Z4foo1iiiiPiS_Ri{{.*}}!ihc_component [[CFOO1:![0-9]+]]
//CHECK-SAME: !arg_type [[ATFOO1:![0-9]+]]
//CHECK-SAME: !impl_type [[ITFOO1:![0-9]+]]
//CHECK-SAME: !stable [[SFOO1:![0-9]+]]
//CHECK-SAME: !cosim_name [[CNFOO1:![0-9]+]]
//CHECK-SAME: !component_interface [[A_STREAMING:![0-9]+]]
//CHECK-SAME: !stall_free_return [[FALSE:![0-9]+]]
//CHECK-SAME: !use_single_clock [[FALSE]]
//CHECK-SAME: !local_mem_size [[LMSFOO1:![0-9]+]]
__attribute__((ihc_component))
__attribute__((component_interface("avalon_streaming")))
int foo1(
     int i1,
   __attribute__((argument_interface("wire")))
   __attribute__((stable_argument))
     int i2,
   __attribute__((argument_interface("avalon_mm_slave")))
     int i3,
   __attribute__((stable_argument))
   __attribute__((argument_interface("avalon_streaming")))
     int i4,
   __attribute__((slave_memory_argument))
   __attribute__((local_mem_size(32)))
     int *i5,
     int *i6,
     int &i7
)
{
  return 0;
}

//CHECK: @_Z4foo2v{{.*}}!ihc_component [[CFOO2:![0-9]+]]
//CHECK-SAME: !component_interface [[A_MM_SLAVE:![0-9]+]]
//CHECK-SAME: !stall_free_return [[FALSE]] !use_single_clock [[FALSE]]
__attribute__((ihc_component))
__attribute__((component_interface("avalon_mm_slave")))
int foo2() { return 0; }

//CHECK: @_Z4foo3v{{.*}}!ihc_component [[CFOO3:![0-9]+]]
//CHECK-SAME: !component_interface [[ALWAYS_R:![0-9]+]]
//CHECK-SAME: !stall_free_return [[FALSE]] !use_single_clock [[FALSE]]
//CHECK-SAME: !max_concurrency [[MCFOO3:![0-9]+]]
__attribute__((ihc_component))
__attribute__((max_concurrency(2048)))
__attribute__((component_interface("always_run")))
int foo3() { return 0; }

template <int N>
__attribute__((ihc_component))
__attribute__((max_concurrency(N)))
__attribute__((scheduler_pipelining_effort_pct(N)))
__attribute__((component_interface("always_run")))
int tfoo3() {
    int stuff[100] __attribute__((__internal_max_block_ram_depth__(N)));
    return 0;
}

//CHECK: @_Z5tfoo3ILi512EEiv{{.*}}!ihc_component [[CTFOO3:![0-9]+]]
//CHECK-SAME: !component_interface [[ALWAYS_R]]
//CHECK-SAME: !stall_free_return [[FALSE]] !use_single_clock [[FALSE]]
//CHECK-SAME: !max_concurrency [[MCTFOO3:![0-9]+]]
//CHECK-SAME: !scheduler_pipelining_effort_pct [[MCTFOO3:![0-9]+]]
void call_it()
{
  tfoo3<512>();
// CHECK: stuff = alloca [100 x i32]
// CHECK: %[[STUFFBC2:[a-z0-9]+]] = bitcast [100 x i32]* %stuff to i8*
    int stuff[100] __attribute__((__internal_max_block_ram_depth__(64)));
// CHECK: llvm.var.annotation{{.*}}[[STUFFBC2]]{{.*}}[[IMD2]]
}

//CHECK: @_Z4foo4v{{.*}}!ihc_component [[CFOO4:![0-9]+]]
//CHECK-SAME: !component_interface [[A_STREAMING]]
//CHECK-SAME: !stall_free_return [[TRUE:![0-9]+]] !use_single_clock [[FALSE]]
__attribute__((ihc_component))
__attribute__((stall_free_return))
int foo4() { return 0; }

//CHECK: @_Z4foo5v{{.*}}!ihc_component [[CFOO5:![0-9]+]]
//CHECK-SAME: !component_interface [[A_STREAMING]]
//CHECK-SAME: !stall_free_return [[FALSE]] !use_single_clock [[TRUE]]
__attribute__((ihc_component))
__attribute__((hls_component_use_single_clock))
int foo5() { return 0; }

__attribute__((ihc_component))
__attribute__((stall_free))
void foo6() {}
// CHECK: define void @_Z4foo6v{{.*}} !stall_free ![[SFTRUE:[0-9]+]]

__attribute__((ihc_component))
__attribute__((scheduler_pipelining_effort_pct(12)))
void foo7() {}
// CHECK: define void @_Z4foo7v{{.*}} !scheduler_pipelining_effort_pct ![[SPEP:[0-9]+]]

void foo8() {
// CHECK: define void @_Z4foo8v{{[^{]+}}
// CHECK: stuff = alloca [100 x i32]
// CHECK: %[[STUFFBC:[a-z0-9]+]] = bitcast [100 x i32]* %stuff to i8*
    int stuff[100] __attribute__((__internal_max_block_ram_depth__(64)));
// CHECK: llvm.var.annotation{{.*}}[[STUFFBC]]{{.*}}[[IMD]]
}

//CHECK: [[CFOO1]] = !{!"_Z4foo1iiiiPiS_Ri", i32 undef}
//CHECK: [[ATFOO1]] = !{!"default", !"default", !"default", !"default", !"mm_slave", !"pointer", !"pointer"}
//CHECK: [[ITFOO1]] = !{!"wire", !"wire", !"avalon_mm_slave", !"avalon_streaming", !"wire", !"wire", !"wire"}
//CHECK: [[SFOO1]] = !{i32 0, i32 1, i32 0, i32 1, i32 0, i32 0, i32 0}
//CHECK: [[CNFOO1]] = !{!"i1", !"i2", !"i3", !"i4", !"i5", !"i6", !"i7"}
//CHECK: [[A_STREAMING]] = !{!"avalon_streaming"}
//CHECK: [[FALSE]] = !{i32 0}
//CHECK: [[LMSFOO1]] = !{i32 0, i32 0, i32 0, i32 0, i32 32, i32 0, i32 0}

//CHECK: [[CFOO2]] = !{!"_Z4foo2v", i32 undef}
//CHECK: [[A_MM_SLAVE]] = !{!"avalon_mm_slave"}

//CHECK: [[CFOO3]] = !{!"_Z4foo3v", i32 undef}
//CHECK: [[ALWAYS_R]] = !{!"always_run"}
//CHECK: [[MCFOO3]] = !{i32 2048}

//CHECK: [[CTFOO3]] = !{!"_Z5tfoo3ILi512EEiv", i32 undef}
//CHECK: [[MCTFOO3]] = !{i32 512}

//CHECK: [[CFOO4]] = !{!"_Z4foo4v", i32 undef}
//CHECK: [[TRUE]] = !{i32 1}

//CHECK: [[CFOO5]] = !{!"_Z4foo5v", i32 undef}
//CHECK: [[SFTRUE]] = !{i1 true}
//CHECK: [[SPEP]] = !{i32 12}
