// RUN: %clang_cc1 -fintel-compatibility  -gnu-permissive -triple x86_64-unknown-linux-gnu -verify %s

enum EN {
};

EN foo(unsigned char w) {
      return (EN)w;  // expected-warning {{illegal implicit type conversion from 'unsigned char' to 'EN' allowed in -fpermissive mode}}
}

