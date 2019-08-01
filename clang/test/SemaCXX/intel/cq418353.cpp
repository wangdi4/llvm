// RUN: %clang_cc1 -std=c++14 -fsyntax-only -verify %s
// RUN: %clang_cc1 -fintel-compatibility -std=c++14 -fsyntax-only -verify %s
// RUN: %clang_cc1 -fintel-compatibility -std=c++1z -fsyntax-only -verify %s
// expected-no-diagnostics
namespace std
{
class locale;
template < typename > bool has_facet (const locale &) throw ();
namespace
{
template < typename > class collate;
} extern template bool has_facet < collate < int >>(const locale &);
}
