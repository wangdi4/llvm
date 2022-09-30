// REQUIRES: system-linux

// RUN: rm -rf %t_dir
// RUN: mkdir -p %t_dir
// RUN: touch %t_dir/empty_file
// RUN: env TMP=%t_dir TMP=%t_dir TMPDIR=%t_dir %clangxx %s
// RUN: env TMP=%t_dir TMP=%t_dir TMPDIR=%t_dir %clangxx -c %s
// RUN: ls -r %t_dir 2>&1 | FileCheck %s -check-prefix=NOFILES
// NOFILES-NOT: clang
// NOFILES-NOT: intel

int main() {
  return 0;
}
