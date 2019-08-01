//RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -fsyntax-only -verify %s
int main() {
  int x = get_compute_id(); // expected-error {{use of undeclared identifier 'get_compute_id'}}
  int y;
  if (read_pipe(x, &y)) { // expected-error {{use of undeclared identifier 'read_pipe'}}
    write_pipe(x, &y); // expected-error {{use of undeclared identifier 'write_pipe'}}
  }
  return 0;
}
