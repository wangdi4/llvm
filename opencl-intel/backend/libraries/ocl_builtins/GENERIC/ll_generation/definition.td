// Basic tablegen definition classes for .ll builtin generation

include "GENERIC/ll_generation/declaration.td"

// A macro can be referenced in the code by its name,
// and the template engine will replace it with its value.
// e.g.
// code impl = [{
//   call {SomeType} @{SomeFunc}(i32 0)
// }];
// and given
// list<Macro> defines = [Macro<"SomeType", i32.Name>, Macro<"SomeFunc", "foo">]
// both impl and defines are binded to the same LLBaseDefine def.
// After template engine, we'll get:
// [{
//   call i32 @foo(i32 0)
// }]
class Macro<string name, string value> {
  string Name = name;
  string Value = value;
}

class LLBaseDefine<LLDeclare declare, list<Value> params, code impl, list<Macro> defines = []> {
  LLDeclare Proto = declare;
  list<Value> Params = params;
  code Impl = impl;
  list<Macro> Defines = defines;

  // Dummy field
  bit EmitMangledName = false;
  list<string> FuncAttrs = [];
}

multiclass LLDefine<string name, list<Value> params, Type return_type, code impl, list<Macro> defines = [], string mangle_suffix = ""> {
  let HasDefine = true in {
    def __declare : LLDeclare<name, mapValueToType<params>.ret, return_type, mangle_suffix>;
  }
  def __define : LLBaseDefine<!cast<LLDeclare>(NAME # __declare), params, impl, defines>;
}
