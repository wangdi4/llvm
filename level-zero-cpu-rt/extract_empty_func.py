import re
def stripComments(code):
    code = str(code)
    return re.sub(r'(?m) *\/\/\/.*\n?', '', code)

def get_existing_api(file):
    funcname_list= list()
    with open(file, "r") as file_in:
        out = False
        for line in file_in:
            #print(line)
            code = line.strip()
            if code.startswith("ZE_APIEXPORT ze_result_t ZE_APICALL"):
                out = True
                block = list()
            
            if out:
                strip_line=stripComments(line)
                block.append(strip_line) 

            if out and code.endswith(") {"):
                out = False
                bstr = ''.join(block)
                bstr = bstr.split("ZE_APICALL",1)[1].strip()
                bstr = bstr.split("(",1)[0]
                #print(block)
                funcname_list.append(bstr)

    return funcname_list

def parse_ze(header_file, existing_api,output_file):

    with open(header_file, "r") as file_in, open(output_file,"w") as file_out:
        functions = list()
        out = False
        for line in file_in:
            code = line.strip()
            if code.startswith("ZE_APIEXPORT ze_result_t ZE_APICALL"):
                out = True
                block = list()
            
            if out:
                strip_line=stripComments(line)
                block.append(strip_line) 

            if out and code.startswith(");"):
                out = False
                block[-1]=")"
                functions.append(block)

        file_out.write(f'#include "{header_file}"\n')

        for b in functions:          
            b.append("{\n")
            # extract function parameters
            func_name = b[1].strip().replace("(","")

            if func_name in existing_api:
                #print(f"{func_name} implemented")
                continue
            params = b[2:-1]
            for pline in params:
                pline = pline.replace(',', '')
                pline = pline.replace('*', '')
                pline = pline.replace(')', '')
                param_list = re.split('\s+',pline.strip())
                    
                if len(param_list) > 1:
                    b.append(f"(void){param_list[-1]};\n")
            b.append("return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;\n")
            b.append("}\n")
            file_out.writelines(b)
            file_out.write("\n")


if __name__ == "__main__":
    existing_api = get_existing_api("src/ze_api.cpp")
    parse_ze("level_zero/include/ze_api.h",existing_api,"src/ze_api_empty.cpp")
    parse_ze("level_zero/include/zes_api.h",existing_api,"src/zes_api_empty.cpp")
    parse_ze("level_zero/include/zet_api.h",existing_api,"src/zet_api_empty.cpp")
