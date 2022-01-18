// RUN: %clang -g -### -S  %s 2>&1 | FileCheck -check-prefix=TEST1 %s
// TEST1: "-dwarf-debug-flags"
// RUN: %clang  -### -S  %s 2>&1 | FileCheck -check-prefix=TEST2 %s
// TEST2-NOT: "-dwarf-debug-flags"
// RUN: %clang -g -O0 -### -S  %s 2>&1 | FileCheck -check-prefix=TEST3 %s
// TEST3: "-dwarf-debug-flags" " -g -O0   {{.*}}"
// RUN: %clang -g -O0 -grecord-gcc-switches -### -S  %s 2>&1 | FileCheck -check-prefix=TEST4 %s
// TEST4: "-dwarf-debug-flags" "ProducerFlags_{{.*}} -g -O0 -grecord-command-line  {{.*}} -g -O0 -grecord-command-line  {{.*}}"
// RUN: %clang  -### -S -g -sox %s 2>&1 | FileCheck -check-prefix=TEST5 %s
// TEST5-NOT: "-dwarf-debug-flags" "ProducerFlags_207{{.*}} -g -O0 -grecord-command-line  {{.*}} -g -O0 -grecord-command-line  {{.*}}"
