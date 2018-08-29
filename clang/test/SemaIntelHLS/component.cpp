//RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

__attribute__((ihc_component))
void foo1(int i) {}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: ComponentAttr

__attribute__((stall_free_return))
__attribute__((ihc_component))
void foo2(int i) {}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK: StallFreeReturnAttr

__attribute__((__hls_component_use_single_clock__))
__attribute__((ihc_component))
void foo2a(int i) {}
// CHECK: FunctionDecl{{.*}}foo2a
// CHECK: UseSingleClockAttr

__attribute__((hls_component_use_single_clock))
__attribute__((ihc_component))
void foo2aa(int i) {}
// CHECK: FunctionDecl{{.*}}foo2a
// CHECK: UseSingleClockAttr

__attribute__((component_interface("avalon_streaming")))
__attribute__((ihc_component))
void foo3(int i) {}
// CHECK: FunctionDecl{{.*}}foo3
// CHECK: ComponentInterfaceAttr{{.*}}Streaming

__attribute__((component_interface("avalon_mm_slave")))
__attribute__((ihc_component))
void foo4(int i) {}
// CHECK: FunctionDecl{{.*}}foo4
// CHECK: ComponentInterfaceAttr{{.*}}Slave

__attribute__((component_interface("always_run")))
__attribute__((ihc_component))
void foo5(int i) {}
// CHECK: FunctionDecl{{.*}}foo5
// CHECK: ComponentInterfaceAttr{{.*}}AlwaysRun

__attribute__((ihc_component))
void foo6(__attribute__((argument_interface("avalon_streaming"))) int i6) {}
// CHECK: FunctionDecl{{.*}}foo6
// CHECK-NEXT: ParmVarDecl{{.*}}i6
// CHECK-NEXT: ArgumentInterfaceAttr{{.*}}Streaming

__attribute__((ihc_component))
void foo7(__attribute__((argument_interface("avalon_mm_slave"))) int i7) {}
// CHECK: FunctionDecl{{.*}}foo7
// CHECK-NEXT: ParmVarDecl{{.*}}i7
// CHECK-NEXT: ArgumentInterfaceAttr{{.*}}Slave

__attribute__((ihc_component))
void foo8(__attribute__((argument_interface("wire"))) int i8) {}
// CHECK: FunctionDecl{{.*}}foo8
// CHECK-NEXT: ParmVarDecl{{.*}}i8
// CHECK-NEXT: ArgumentInterfaceAttr{{.*}}Wire

__attribute__((ihc_component))
void foo9(__attribute__((stable_argument)) int i9) {}
// CHECK: FunctionDecl{{.*}}foo9
// CHECK-NEXT: ParmVarDecl{{.*}}i9
// CHECK-NEXT: StableArgumentAttr

__attribute__((ihc_component))
void foo10(
   __attribute__((slave_memory_argument)) int i10) {}
// CHECK: FunctionDecl{{.*}}foo10
// CHECK-NEXT: ParmVarDecl{{.*}}i10
// CHECK-NEXT: SlaveMemoryArgumentAttr

__attribute__((ihc_component))
void foo11(
   __attribute__((local_mem_size(32))) int *i11p) {}
// CHECK: FunctionDecl{{.*}}foo11
// CHECK-NEXT: ParmVarDecl{{.*}}i11p
// CHECK-NEXT: OpenCLLocalMemSizeAttr

// Diagnostics

__attribute__((ihc_component)) int var; // expected-error{{attribute only applies to functions}}

__attribute__((ihc_component("avalon_streaming"))) // expected-error{{'ihc_component' attribute takes no arguments}}
void bar1() {}

__attribute__((ihc_component))
__attribute__((component_interface("always_run", 6))) // expected-error{{'component_interface' attribute takes one argument}}
void bar2(int i) {}

__attribute__((ihc_component))
__attribute__((component_interface(6))) // expected-error{{'component_interface' attribute requires a string}}
void bar3(int i) {}

__attribute__((ihc_component))
__attribute__((component_interface("other_string")))
void bar4(int i) {} // expected-error{{string passed to 'component_interface' attribute is not a valid interface type}}

__attribute__((ihc_component))
__attribute__((argument_interface("wire"))) // expected-error{{'argument_interface' attribute only applies to parameters}}
void bar5(int i) {}

__attribute__((ihc_component))
void bar6(__attribute__((argument_interface("wire", "wire"))) int i) {} // expected-error{{'argument_interface' attribute takes one argument}}

__attribute__((ihc_component))
void bar7(__attribute__((argument_interface(1.0))) int i) {} // expected-error{{'argument_interface' attribute requires a string}}

__attribute__((ihc_component))
void bar8(__attribute__((argument_interface("something-else"))) int i) {} // expected-error{{string passed to 'argument_interface' attribute is not a valid interface type}}

__attribute__((ihc_component))
void bar9(__attribute__((stable_argument(0))) int i) {} // expected-error{{'stable_argument' attribute takes no arguments}}

__attribute__((ihc_component))
void bar10(__attribute__((slave_memory_argument(0))) int i) {} // expected-error{{'slave_memory_argument' attribute takes no arguments}}

__attribute__((stall_free)) //expected-error{{'stall_free' attribute only applies to functions}}
__attribute__((scheduler_pipelining_effort_pct(1))) //expected-error{{'scheduler_pipelining_effort_pct' attribute only applies to functions}}
const int i = 10;

__attribute__((stall_free(0))) //expected-error{{'stall_free' attribute takes no arguments}}
void bar11(int a) {
}
__attribute__((scheduler_pipelining_effort_pct)) //expected-error{{'scheduler_pipelining_effort_pct' attribute takes one argument}}
void bar12() {
}

__attribute__((scheduler_pipelining_effort_pct(12)))
void bar13() {
}

__attribute__((scheduler_pipelining_effort_pct("sch"))) //expected-error{{integral constant expression must have integral or unscoped enumeration type, not 'const char [4]'}}
void bar14() {
}

__attribute__((scheduler_pipelining_effort_pct(-12))) // expected-error{{'scheduler_pipelining_effort_pct' attribute requires integer constant between 0 and 1048576 inclusive}}
void bar15() {
}

__attribute__((scheduler_pipelining_effort_pct(0)))
void bar16() {
}

void bar17() {
    int stuff[100] __attribute__((__internal_max_block_ram_depth__(64)));
    int __attribute__((__internal_max_block_ram_depth__(64))) s;
    int __attribute__((__internal_max_block_ram_depth__("sch"))) s1; // expected-error{{integral constant expression must have integral or unscoped enumeration type, not 'const char [4]'}}
    int __attribute__((__internal_max_block_ram_depth__(0))) s2;
    int __attribute__((__internal_max_block_ram_depth__(-64))) s3; // expected-error{{'internal_max_block_ram_depth' attribute requires integer constant between 0 and 1048576 inclusive}}
    // expected-error@+1{{attributes are not compatible}}
    int __attribute__((__internal_max_block_ram_depth__(64))) __attribute__((register)) s4;
// expected-note@-1{{conflicting attribute is here}}
    // expected-error@+1{{attributes are not compatible}}
    int __attribute__((register)) __attribute__((__internal_max_block_ram_depth__(64))) s5;
// expected-note@-1{{conflicting attribute is here}}
}

__attribute__((__internal_max_block_ram_depth__(64))) // expected-error{{'__internal_max_block_ram_depth__' attribute only applies to local or static variables}}
void bar18() {
}

