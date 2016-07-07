// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -fsyntax-only -verify %s

namespace std {
class type_info {
  bool operator==(const type_info &__arg) const; // expected-note{{implicitly declared private here}}
};
}

template <class D> int get_deleter(std::type_info const &ti) {
  return ti == typeid(D); // expected-warning{{'operator==' is a private member of 'std::type_info'}}
}
