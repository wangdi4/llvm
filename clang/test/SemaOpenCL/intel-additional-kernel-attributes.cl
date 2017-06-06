// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown-intelfpga -fsyntax-only -verify %s

__attribute__((task)) //expected-error{{'task' attribute conflicts with 'max_work_group_size' attribute}}
__attribute__((max_work_group_size(1024, 1, 1)))
__kernel void kernel_1a() {
}

__attribute__((max_work_group_size(1024, 1, 1))) //expected-error{{'max_work_group_size' attribute conflicts with 'task' attribute}}
__attribute__((task))
__kernel void kernel_1b() {
}

__attribute__((reqd_work_group_size(16,16,16))) //expected-error{{'reqd_work_group_size' attribute conflicts with 'task' attribute}}
__attribute__((task))
__kernel void kernel_2a() {
}

__attribute__((task)) //expected-error{{'task' attribute conflicts with 'reqd_work_group_size' attribute}}
__attribute__((reqd_work_group_size(16,16,16)))
__kernel void kernel_2b() {
}

__attribute__((reqd_work_group_size(64,64,64))) //expected-error{{'reqd_work_group_size' attribute conflicts with 'max_work_group_size' attribute}}
__attribute__((max_work_group_size(16,16,16)))
__kernel void kernel_3a() {
}

__attribute__((max_work_group_size(16,16,16))) //expected-error{{'max_work_group_size' attribute conflicts with 'reqd_work_group_size' attribute}}
__attribute__((reqd_work_group_size(64,64,64)))
__kernel void kernel_3b() {
}
