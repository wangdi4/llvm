//REQUIRES: system-windows

#import <filenotdfound.dll>

// Test that #import with just header-file token works.

//RUN: not %clang_cc1 -fintel-ms-compatibility \
//RUN:  -show-import-processing %s > %t-check.txt 2>&1
//RUN: FileCheck --input-file=%t-check.txt %s

//CHECK: warning: wrapper file generated: {{.*}}import{{.*}}cpp
//CHECK: note: added to wrapper file: #import <filenotdfound.dll>
