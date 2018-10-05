// RUN: %clang_cc1 %s -x cl -triple spir64-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -x cl -triple spir-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -x cl -triple x86_64-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only

typedef unsigned int uint;
typedef int reserve_id_t; // dummy typedef to avoid extra diagnostics

__kernel void k1(read_only pipe int p1) {
  // expected-error@+1{{implicit declaration of function 'get_pipe_num_packets' is invalid in OpenCL}}
  uint num_packets = get_pipe_num_packets(p1);
  // expected-note@-1{{'get_pipe_num_packets' declared here}}
  // expected-error@+1{{implicit declaration of function 'get_pipe_max_packets' is invalid in OpenCL}}
  uint max_packets = get_pipe_max_packets(p1);
  // expected-note@-1{{did you mean 'get_pipe_num_packets'?}}
}

__kernel void k2(read_only pipe int p1, write_only pipe int p2) {
  // expected-error@+1{{implicit declaration of function 'reserve_read_pipe' is invalid in OpenCL}}
  reserve_id_t r1 = reserve_read_pipe(p1, 10);
  // expected-note@-1{{'reserve_read_pipe' declared here}}
  // expected-error@+1{{implicit declaration of function 'reserve_write_pipe' is invalid in OpenCL}}
  reserve_id_t r2 = reserve_write_pipe(p2, 10);
  // expected-note@-1{{did you mean 'reserve_read_pipe'?}}

  int data = 10;
  // expected-error@+1{{implicit declaration of function 'is_valid_reserve_id' is invalid in OpenCL}}
  if (is_valid_reserve_id(r1)) {
    // expected-error@+1{{invalid number of arguments to function: 'read_pipe'}}
    read_pipe(p1, r1, 0, &data);
    // expected-error@+1{{invalid number of arguments to function: 'write_pipe'}}
    write_pipe(p2, r2, 0, &data);
  }

  // expected-error@+1{{implicit declaration of function 'commit_read_pipe' is invalid in OpenCL}}
  commit_read_pipe(p1, r1);
  // expected-note@-1{{'commit_read_pipe' declared here}}
  // expected-error@+1{{implicit declaration of function 'commit_write_pipe' is invalid in OpenCL}}
  commit_write_pipe(p2, r2);
  // expected-note@-1{{did you mean 'commit_read_pipe'?}}
}

__kernel void k3(read_only pipe int p1, write_only pipe int p2) {
  // expected-error@+1{{implicit declaration of function 'work_group_reserve_read_pipe' is invalid in OpenCL}}
  reserve_id_t r1 = work_group_reserve_read_pipe(p1, 10);
  // expected-note@-1{{'work_group_reserve_read_pipe' declared here}}
  // expected-note@-2{{'work_group_reserve_read_pipe' declared here}}
  // expected-error@+1{{implicit declaration of function 'work_group_reserve_write_pipe' is invalid in OpenCL}}
  reserve_id_t r2 = work_group_reserve_write_pipe(p2, 10);
  // expected-note@-1{{did you mean 'work_group_reserve_read_pipe'?}}

  // expected-error@+1{{implicit declaration of function 'work_group_commit_read_pipe' is invalid in OpenCL}}
  work_group_commit_read_pipe(p1, r1);
  // expected-note@-1{{did you mean 'work_group_reserve_read_pipe'?}}
  // expected-note@-2{{'work_group_commit_read_pipe' declared here}}
  // expected-error@+1{{implicit declaration of function 'work_group_commit_write_pipe' is invalid in OpenCL}}
  work_group_commit_write_pipe(p2, r2);
  // expected-note@-1{{did you mean 'work_group_commit_read_pipe'?}}
}
