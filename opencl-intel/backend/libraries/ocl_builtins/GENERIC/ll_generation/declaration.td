// Basic tablegen declaration classes for .ll builtin generation

include "GENERIC/ll_generation/type_value.td"

class LLDeclare<string name, list<Type> param_types, Type return_type, string mangle_suffix = ""> {
  string Name = name;
  list<Type> ParamTypes = param_types;
  Type RetType = return_type;

  bit EmitMangledName = false;
  string MangledName = "_Z" # !size(name) # name # mangle_suffix;

  bit HasDefine = false;

  list<string> FuncAttrs = [];
}
