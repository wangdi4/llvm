// -fintel-compatibility-doc is not valid by itself
// RUN: not %clang -c -Xclang -fintel-compatibility-doc %s 2>&1 | FileCheck --check-prefix=CHECK_WRONG1 %s
// CHECK_WRONG1: unknown argument: '-fintel-compatibility-doc'

// -fintel-compatibility-doc= must be followed by at least one option
// RUN: %clang -c -Xclang -fintel-compatibility-doc= %s 2>&1 | FileCheck %s
// CHECK: invalid value '' in '-fintel-compatibility-doc='

// -fintel-compatibility-doc= must be followed by at least one valid option
// RUN: %clang -c -Xclang -fintel-compatibility-doc=foobar %s 2>&1 | FileCheck --check-prefix=CHECK_WRONG2 %s
// CHECK_WRONG2: invalid value 'foobar' in '-fintel-compatibility-doc='
