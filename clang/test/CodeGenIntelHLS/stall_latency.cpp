//RUN: %clang_cc1 -fhls -emit-llvm -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -debug-info-kind=limited -emit-llvm -triple x86_64-unknown-linux-gnu -o %t %s

__attribute__((ihc_component))
__attribute__((stall_latency_enabled))
void foo1(){}
//CHECK: define{{.*}}foo1
//CHECK-SAME: !stall_latency [[TRUE:![0-9]+]]

__attribute__((ihc_component))
__attribute__((stall_latency_disabled))
void foo2(){}
//CHECK: define{{.*}}foo2
//CHECK-SAME: !stall_latency [[FALSE:![0-9]+]]

//CHECK: [[FALSE]] = !{i32 0}
//CHECK: [[TRUE]] = !{i32 1}
