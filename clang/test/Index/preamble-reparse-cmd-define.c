// RUN: c-index-test -write-pch %t.h.pch %s.h
// RUN: env CINDEXTEST_EDITING=1 CINDEXTEST_REMAP_AFTER_TRIAL=1 c-index-test -test-load-source-reparse 3 local \ 
// INTEL RUN:           "-remap-file=%s,%s.remap" %s -include-pch %t.h.pch -D CMD_MACRO=1 2>&1 | FileCheck %s

// CHECK-NOT: error:

int foo(void) {
  return x;
}
