// RUN: %clang_cc1 %s -x cl -triple spir64-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -x cl -triple spir-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -x cl -triple x86_64-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only

typedef unsigned int uint;
typedef int reserve_id_t; // dummy typedef to avoid extra diagnostics

__kernel void k1(read_only pipe int p1) {
  // expected-error@+1{{use of undeclared identifier 'get_pipe_num_packets'}}
  uint num_packets = get_pipe_num_packets(p1);
  // expected-error@+1{{use of undeclared identifier 'get_pipe_max_packets'}}
  uint max_packets = get_pipe_max_packets(p1);
}

__kernel void k2(read_only pipe int p1, write_only pipe int p2) {
  // expected-error@+1{{use of undeclared identifier 'reserve_read_pipe'}}
  reserve_id_t r1 = reserve_read_pipe(p1, 10);
  // expected-error@+1{{use of undeclared identifier 'reserve_write_pipe'}}
  reserve_id_t r2 = reserve_write_pipe(p2, 10);

  int data = 10;
  // expected-error@+1{{use of undeclared identifier 'is_valid_reserve_id'}}
  if (is_valid_reserve_id(r1)) {
    // expected-error@+1{{invalid number of arguments to function: 'read_pipe'}}
    read_pipe(p1, r1, 0, &data);
    // expected-error@+1{{invalid number of arguments to function: 'write_pipe'}}
    write_pipe(p2, r2, 0, &data);
  }

  // expected-error@+1{{use of undeclared identifier 'commit_read_pipe'}}
  commit_read_pipe(p1, r1);
  // expected-error@+1{{use of undeclared identifier 'commit_write_pipe'}}
  commit_write_pipe(p2, r2);
}

__kernel void k3(read_only pipe int p1, write_only pipe int p2) {
  // expected-error@+1{{use of undeclared identifier 'work_group_reserve_read_pipe}}
  reserve_id_t r1 = work_group_reserve_read_pipe(p1, 10);
  // expected-error@+1{{use of undeclared identifier 'work_group_reserve_write_pipe}}
  reserve_id_t r2 = work_group_reserve_write_pipe(p2, 10);

  // expected-error@+1{{use of undeclared identifier 'work_group_commit_read_pipe'}}
  work_group_commit_read_pipe(p1, r1);
  // expected-error@+1{{use of undeclared identifier 'work_group_commit_write_pipe'}}
  work_group_commit_write_pipe(p2, r2);
}
