// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown-intelfpga -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple x86_64-unknown-unknown-intelfpga -fsyntax-only -verify %s

__attribute__((max_global_work_dim(0)))
__attribute__((max_work_group_size(1024, 1, 1))) //expected-error{{'max_work_group_size' X-, Y- and Z- sizes must be 1 when 'max_global_work_dim' attribute is used and equals to 0}}
__kernel void kernel_1a() {
}

__attribute__((max_work_group_size(1024, 1, 1))) //expected-error{{'max_work_group_size' X-, Y- and Z- sizes must be 1 when 'max_global_work_dim' attribute is used and equals to 0}}
__attribute__((max_global_work_dim(0)))
__kernel void kernel_1b() {
}

__attribute__((reqd_work_group_size(16,16,16))) //expected-error{{'reqd_work_group_size' X-, Y- and Z- sizes must be 1 when 'max_global_work_dim' attribute is used and equals to 0}}
__attribute__((max_global_work_dim(0)))
__kernel void kernel_2a() {
}

__attribute__((max_global_work_dim(0)))
__attribute__((reqd_work_group_size(16,16,16))) //expected-error{{'reqd_work_group_size' X-, Y- and Z- sizes must be 1 when 'max_global_work_dim' attribute is used and equals to 0}}
__kernel void kernel_2b() {
}

__attribute__((reqd_work_group_size(64,64,64)))
__attribute__((max_work_group_size(16,16,16))) //expected-error{{'max_work_group_size' attribute conflicts with 'reqd_work_group_size' attribute}}
__kernel void kernel_3a() {
}

__attribute__((max_work_group_size(16,16,16)))
__attribute__((reqd_work_group_size(64,64,64))) //expected-error{{'reqd_work_group_size' attribute conflicts with 'max_work_group_size' attribute}}
__kernel void kernel_3b() {
}

__attribute__((autorun)) //expected-error{{'autorun' attribute requires 'reqd_work_group_size' or 'max_global_work_dim' attribute to be specified}}
__kernel void kernel_4a() {
}

__attribute__((autorun))
__attribute__((max_global_work_dim(0)))
__kernel void kernel_4b() {
}

__attribute__((autorun))
__attribute__((reqd_work_group_size(16, 16, 16)))
__kernel void kernel_4c() {
}

__attribute__((autorun)) //expected-error{{'autorun' attribute requires 'reqd_work_group_size' or 'max_global_work_dim' attribute to be specified}}
__attribute__((reqd_work_group_size(10, 10, 10))) //expected-error{{Autorun kernel functions must have work group sizes that are divisors of 2^32}}
__kernel void kernel_4d() {
}

__attribute__((reqd_work_group_size(10, 10, 10))) //expected-error{{Autorun kernel functions must have work group sizes that are divisors of 2^32}}
__attribute__((autorun))
__kernel void kernel_4e() {
}

__attribute__((autorun)) //expected-error{{Autorun kernel functions cannot have any arguments}}
__attribute__((max_global_work_dim(0)))
__kernel void kernel_4f(int a) {
}

__attribute__((autorun))
__attribute__((max_global_work_dim(2)))
__kernel void kernel_4g() {
}

__attribute__((max_global_work_dim(2)))
__attribute__((autorun))
__kernel void kernel_4h() {
}

__attribute__((stall_free(0))) //expected-error{{'stall_free' attribute takes no arguments}}
__kernel void kernel_4i(int a) {
}

__attribute__((stall_free)) //expected-error{{'stall_free' attribute only applies to functions}}
__attribute__((autorun)) //expected-warning{{'autorun' attribute only applies to functions}}
constant int i = 10;

__attribute__((max_global_work_dim(4))) //expected-error{{'max_global_work_dim' attribute must be in range from 0 to 3}}
__kernel void kernel_5a() {
}

__attribute__((max_global_work_dim(0)))
__attribute__((max_work_group_size(10, 10, 10))) //expected-error{{'max_work_group_size' X-, Y- and Z- sizes must be 1 when 'max_global_work_dim' attribute is used}}
__kernel void kernel_5b() {
}

__attribute__((max_global_work_dim(0)))
__attribute__((reqd_work_group_size(10, 10, 10))) //expected-error{{'reqd_work_group_size' X-, Y- and Z- sizes must be 1 when 'max_global_work_dim' attribute is used}}
__kernel void kernel_5c() {
}

__attribute__((num_compute_units(5)))
__kernel void kernel_6a() {
}

__attribute__((autorun))
__attribute__((max_global_work_dim(0)))
__attribute__((num_compute_units(2, 3, 4)))
__kernel void kernel_6b() {
}

__attribute__((num_compute_units(-5, 4, 3))) //expected-error{{'num_compute_units' attribute parameter 1 requires integer constant between 1 and 16384 inclusive}}
__kernel void kernel_6c() {
}

__attribute__((num_compute_units(5, 100500, 3))) //expected-error{{'num_compute_units' attribute parameter 2 requires integer constant between 1 and 16384 inclusive}}
__kernel void kernel_6d() {
}

__attribute__((num_compute_units(5, 4, -3))) //expected-error{{'num_compute_units' attribute parameter 3 requires integer constant between 1 and 16384 inclusive}}
__kernel void kernel_6e() {
}

__attribute__((num_compute_units(1)))
__kernel void kernel_6f() {
}

__attribute__((num_compute_units(1, 2)))
__kernel void kernel_6g() {
}

__attribute__((num_compute_units)) //expected-error{{'num_compute_units' attribute takes at least 1 argument}}
__kernel void kernel_6h() {
}

__attribute__((num_compute_units(1, 2, 3, 4))) //expected-error{{'num_compute_units' attribute takes no more than 3 arguments}}
__kernel void kernel_6i() {
}

__attribute__((scheduler_target_fmax_mhz)) //expected-error{{'scheduler_target_fmax_mhz' attribute takes one argument}}
__kernel void kernel_7a() {
}

__attribute__((scheduler_target_fmax_mhz(12)))
__kernel void kernel_7b() {
}

__attribute__((scheduler_target_fmax_mhz("sch"))) // expected-error{{expression is not an integer constant expression}}
__kernel void kernel_7c() {
}

__attribute__((scheduler_target_fmax_mhz(-12))) // expected-error{{'scheduler_target_fmax_mhz' attribute requires integer constant between 0 and 1048576 inclusive}}
__kernel void kernel_7d() {
}

__attribute__((scheduler_target_fmax_mhz(0)))
__kernel void kernel_7e() {
}

__kernel void kernel_7f() {
    int stuff[100] __attribute__((__internal_max_block_ram_depth__(64)));
    int __attribute__((__internal_max_block_ram_depth__(64))) s;
    int __attribute__((__internal_max_block_ram_depth__("sch"))) s1; // expected-error{{expression is not an integer constant expression}}
    int __attribute__((__internal_max_block_ram_depth__(0))) s2;
    int __attribute__((__internal_max_block_ram_depth__(-64))) s3; // expected-error{{'internal_max_block_ram_depth' attribute requires integer constant between 0 and 1048576 inclusive}}
    int __attribute__((__internal_max_block_ram_depth__(64))) __attribute__((register)) s4; // expected-error{{'register' and 'internal_max_block_ram_depth' attributes are not compatible}}
// expected-note@-1{{conflicting attribute is here}}
    int __attribute__((register)) __attribute__((__internal_max_block_ram_depth__(64))) s5; // expected-error{{'__internal_max_block_ram_depth__' and 'register' attributes are not compatible}}
// expected-note@-1{{conflicting attribute is here}}
}

__attribute__((__internal_max_block_ram_depth__(64))) // expected-error{{'__internal_max_block_ram_depth__' attribute only applies to constant variables, local variables, static variables, slave memory arguments, and non-static data members}}
__kernel void kernel_7g() {
}

__attribute__((max_work_group_size(1, -1, 1))) // expected-error{{'max_work_group_size' attribute requires a non-negative integral compile time constant expression}}
__kernel void kernel_8a() {}

__attribute__((num_simd_work_items(-1))) // expected-error{{'num_simd_work_items' attribute requires a non-negative integral compile time constant expression}}
__kernel void kernel_8b() {}

__attribute__((max_work_group_size(1, 1, 1)))
void fun_8b() {} // expected-error{{attribute 'max_work_group_size' can only be applied to an OpenCL kernel function}}

__attribute__((uses_global_work_offset(0)))
__kernel void kernel_9a() {}

__attribute__((uses_global_work_offset(1)))
__kernel void kernel_9b() {}

__attribute__((uses_global_work_offset(1)))
void fun_9b() {} // expected-error{{attribute 'uses_global_work_offset' can only be applied to an OpenCL kernel function}}

__attribute__((uses_global_work_offset(1,2,3))) // expected-error{{'uses_global_work_offset' attribute takes one argument}}
__kernel void kernel_9c() {}

__attribute__((uses_global_work_offset())) // expected-error{{'uses_global_work_offset' attribute takes one argument}}
__kernel void kernel_9d() {}

__attribute__((uses_global_work_offset(-1))) // expected-error{{'uses_global_work_offset' attribute requires a non-negative integral compile time constant expression}}
__kernel void kernel_9f() {}
