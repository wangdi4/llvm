//RUN: %clang_cc1 -fhls -emit-llvm -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s

//CHECK: @_Z4foo1iiiiPiS_Ri{{.*}}!ihc_component [[CFOO1:![0-9]+]] !ihc_attrs [[AFOO1:![0-9]+]]
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

//CHECK: @_Z4foo2v{{.*}}!ihc_component [[CFOO2:![0-9]+]] !ihc_attrs [[AFOO2:![0-9]+]]
__attribute__((ihc_component))
__attribute__((component_interface("avalon_mm_slave")))
int foo2() { return 0; }

//CHECK: @_Z4foo3v{{.*}}!ihc_component [[CFOO3:![0-9]+]] !ihc_attrs [[AFOO3:![0-9]+]]
__attribute__((ihc_component))
__attribute__((max_concurrency(2048)))
__attribute__((component_interface("always_run")))
int foo3() { return 0; }

//CHECK: @_Z4foo4v{{.*}}!ihc_component [[CFOO4:![0-9]+]] !ihc_attrs [[AFOO4:![0-9]+]]
__attribute__((ihc_component))
__attribute__((stall_free_return))
int foo4() { return 0; }

//CHECK: [[CFOO1]] = !{!"_Z4foo1iiiiPiS_Ri", i32 undef, [[FOO1A1:![0-9]+]], [[FOO1A2:![0-9]+]], [[FOO1A3:![0-9]+]], [[FOO1A4:![0-9]+]], [[FOO1A5:![0-9]+]], [[FOO1A6:![0-9]+]], [[FOO1A7:![0-9]+]]}
//CHECK: [[FOO1A1]] = !{!"arg_type", !"default", !"impl_type", !"wire", !"stable", i32 0, !"cosim_name", !"i1"}
//CHECK: [[FOO1A2]] = !{!"arg_type", !"default", !"impl_type", !"wire", !"stable", i32 1, !"cosim_name", !"i2"}
//CHECK: [[FOO1A3]] = !{!"arg_type", !"default", !"impl_type", !"avalon_mm_slave", !"stable", i32 0, !"cosim_name", !"i3"}
//CHECK: [[FOO1A4]] = !{!"arg_type", !"default", !"impl_type", !"avalon_streaming", !"stable", i32 1, !"cosim_name", !"i4"}
//CHECK: [[FOO1A5]] = !{!"arg_type", !"mm_slave", !"impl_type", !"wire", !"stable", i32 0, !"cosim_name", !"i5", !"local_mem_size", i32 32}
//CHECK: [[FOO1A6]] = !{!"arg_type", !"pointer", !"impl_type", !"wire", !"stable", i32 0, !"cosim_name", !"i6"}
//CHECK: [[FOO1A7]] = !{!"arg_type", !"pointer", !"impl_type", !"wire", !"stable", i32 0, !"cosim_name", !"i7"}
//CHECK: [[AFOO1]] = !{!"cosim_name", !"_Z4foo1iiiiPiS_Ri", !"component_interface", !"avalon_streaming", !"stall_free_return", i32 0

//CHECK: [[CFOO2]] = !{!"_Z4foo2v", i32 undef}
//CHECK: [[AFOO2]] = !{!"cosim_name", !"_Z4foo2v", !"component_interface", !"avalon_mm_slave", !"stall_free_return", i32 0

//CHECK: [[CFOO3]] = !{!"_Z4foo3v", i32 undef}
//CHECK: [[AFOO3]] = !{!"cosim_name", !"_Z4foo3v", !"component_interface", !"always_run", !"stall_free_return", i32 0, !"max_concurrency", i32 2048

//CHECK: [[CFOO4]] = !{!"_Z4foo4v", i32 undef}
//CHECK: [[AFOO4]] = !{!"cosim_name", !"_Z4foo4v", !"component_interface", !"avalon_streaming", !"stall_free_return", i32 1
