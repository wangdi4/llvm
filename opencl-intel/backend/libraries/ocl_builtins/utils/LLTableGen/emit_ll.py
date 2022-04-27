import argparse
import json
from typing import Any, List

parser = argparse.ArgumentParser("Emit .ll from TableGen'ed json file")
parser.add_argument('jsonfile')
parser.add_argument('-o', '--output-file', dest='output', required=True)


class PrintableData(object):

    def __init__(self, data: dict) -> None:
        self.data = data

    def emit(self, key: str = "Name") -> str:
        return self.data[key]

    def __repr__(self) -> str:
        return self.emit()

    def __str__(self) -> str:
        return self.__repr__()


class LLBuilder(object):

    def __init__(self, records: dict) -> None:
        self.records = records

    @staticmethod
    def from_file(filename: str):
        with open(filename, 'r') as f:
            return LLBuilder(json.load(f))

    def get(self, name: str) -> Any:
        return self.records[name]

    def build_opaque_types(self) -> List[str]:
        names = self.records["!instanceof"]["OpaqueStructType"]
        return [LLOpaqueType(self, name).emit() for name in names]

    def build_declares(self) -> List[str]:
        names = self.records["!instanceof"]["LLDeclare"]
        return [LLDeclare(self, name).emit() for name in names]

    def build_defines(self) -> List[str]:
        names = self.records["!instanceof"]["LLBaseDefine"]
        return [LLDefine(self, name).emit() for name in names]

    def build(self, filename: str) -> None:
        opaque_type_defs = self.build_opaque_types()
        declares = self.build_declares()
        defines = self.build_defines()
        with open(filename, "w") as f:
            f.writelines(line + '\n\n' for line in opaque_type_defs if line)
            f.writelines(line + '\n\n' for line in declares if line)
            f.writelines(line + '\n\n' for line in defines if line)


class Record(PrintableData):

    def __init__(self, builder: LLBuilder, name: str) -> None:
        self.builder = builder
        super().__init__(self.builder.get(name))


class LLType(Record):
    pass


class LLOpaqueType(LLType):

    def emit(self) -> str:
        return super().emit() + " = type opaque"


class LLValue(Record):

    @property
    def name(self) -> str:
        return super().emit()

    @property
    def type(self) -> LLType:
        return LLType(self.builder, self.data["Ty"]["def"])

    def emit(self) -> str:
        return "{} {}".format(self.type, self.name)


class LLDeclare(Record):

    @property
    def ret_type(self) -> LLType:
        return LLType(self.builder, self.data["RetType"]["def"])

    @property
    def param_types(self) -> List[LLType]:
        return [
            LLType(self.builder, t["def"]) for t in self.data["ParamTypes"]
        ]

    @property
    def name(self) -> str:
        return self.data[
            "MangledName"] if self.emit_mangled_name else self.data["Name"]

    @property
    def has_define(self) -> bool:
        return self.data["HasDefine"]

    @property
    def emit_mangled_name(self) -> bool:
        return self.data["EmitMangledName"]

    @property
    def func_attrs(self) -> List[str]:
        return self.data["FuncAttrs"]

    def get_attrs_suffix(self) -> str:
        if self.func_attrs:
            return " {}".format(" ".join(self.func_attrs))
        return ""

    def emit(self) -> str:
        if self.has_define:
            return ""
        return "declare {ret_type} @{func_name}({param_list}){attr_suffix}".format(
            ret_type=self.ret_type.emit(),
            func_name=self.name,
            param_list=", ".join(t.emit() for t in self.param_types),
            attr_suffix=self.get_attrs_suffix())


class LLDefine(Record):

    @property
    def params(self) -> List[LLValue]:
        return [LLValue(self.builder, p["def"]) for p in self.data["Params"]]

    @property
    def prototype(self) -> LLDeclare:
        return LLDeclare(self.builder, self.data["Proto"]["def"])

    @property
    def code(self) -> str:
        return self.data["Impl"]

    def get_macros(self) -> dict:
        macros = {}
        for macro in self.data["Defines"]:
            macro_data = self.builder.get(macro["def"])
            macros[macro_data["Name"]] = macro_data["Value"]
        return macros

    def render_code(self) -> str:
        # predefined render args
        render_args = {'Args': self.params, 'RetType': self.prototype.ret_type}
        # extra macros defined in *.td
        render_args.update(self.get_macros())
        return self.code.format(**render_args)

    def emit(self) -> str:
        return "define {ret_type} @{func_name}({param_list}){attr_suffix} {{{func_body}}}".format(
            ret_type=self.prototype.ret_type,
            func_name=self.prototype.name,
            param_list=", ".join(v.emit() for v in self.params),
            attr_suffix=self.prototype.get_attrs_suffix(),
            func_body=self.render_code())


if __name__ == "__main__":
    args = parser.parse_args()
    builder = LLBuilder.from_file(args.jsonfile)
    builder.build(args.output)
