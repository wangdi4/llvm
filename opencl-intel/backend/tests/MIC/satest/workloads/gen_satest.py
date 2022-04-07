import re
import argparse
import os.path

config_template = r"""<?xml version="1.0" encoding="UTF-8" ?>
<RunConfiguration>
  <ProgramFile compilation_flags="{compilation_options}">{src_file}</ProgramFile>
  <ProgramFileType>CL</ProgramFileType>
  <UseVTune>0</UseVTune>
  <UseVectorizer>1</UseVectorizer>
  <KernelConfiguration Name="Foo">
    <WorkDimention>1</WorkDimention>
    <LocalWorkSize>16</LocalWorkSize>
    <GlobalWorkSize>16</GlobalWorkSize>
    <GlobalWorkOffset>0</GlobalWorkOffset>
    <InputDataFile>{src_file}.dat.xml</InputDataFile>
    <InputDataFileType>xml</InputDataFileType>
    <ReferenceDataFile>{src_file}.ref.xml</ReferenceDataFile>
    <ReferenceDataFileType>xml</ReferenceDataFileType>
    <NeatDataFile>{src_file}.neat.xml</NeatDataFile>
    <NeatDataFileType>xml</NeatDataFileType>
  </KernelConfiguration>
</RunConfiguration>"""


def get_command(line):
    # Appears in Volcan log files
    m = re.match("Command:(\w+).*", line)
    if m:
        return m.group(1)
    return None

def get_source_start(line):
    if line == "Source:":
        return line
    return None

def get_source_end(line):
    if line == "/Source:":
        return line
    return None

def get_options_start(line):
    if line == "Options:":
        return line
    return None

def get_options_end(line):
    if line == "/Options:":
        return line
    return None

def get_process_start(line):
    if line == "Process:":
        return line
    return None

def get_process_end(line):
    if line == "/Process:":
        return line
    return None

def get_interesting_line(line):
#    m = re.match(r"[0-9]+>(.+)", line)
    m = re.match(r"stderr:  (.+)", line)
    if m:
        return m.group(1)
    return None


def main():
    # Maps a kernel source string to filename containing the string
    src_to_c = {}
    # Set of tuple([kernel_filename, compile_options])
    cfg_lookup = set()
    parser = argparse.ArgumentParser()
    parser.add_argument("logfile", type=file, help="File to parse")
    options = parser.parse_args()
    kernel_name = ''
    config_name = ''
    cl_filename = ''
    in_source = False
    in_options = False
    in_process = False
    src = ''
    compile_options = ''
    for line in options.logfile:
        line = get_interesting_line(line.strip())
        if line is None:
            continue
        cmd = get_command(line)
        if cmd:
            kernel_name = os.path.basename(cmd)
            continue
        if get_process_start(line):
            in_process = True
            continue
        if get_process_end(line):
            in_process = False
            continue
        if get_source_start(line):
            in_source = True
            continue
        if get_source_end(line):
            in_source = False
            continue
        if get_options_start(line):
            in_options = True
            continue
        if get_options_end(line):
            in_options = False
            kernel_filename = ''
            # First see if we need to create a new src file or we can reuse one
            if src in src_to_c:
                kernel_filename = src_to_c[src]
            else:
                for kernel_num in range(10000):
                    candidate = kernel_name + '.' + str(kernel_num) + '.c'
                    if not os.path.exists(candidate):
                        kernel_filename = candidate
                        break
                src_to_c[src] = kernel_filename
            with open(kernel_filename, 'w') as src_file:
                src_file.write(src)
            src = ''
            # Next see if we need to create a new config or this is the same as a previous one
            key = tuple([kernel_filename, compile_options.strip()])
            if key in cfg_lookup:
                continue
            for cfg_num in range(10000):
                candidate = kernel_name + '.' + str(cfg_num) + '.cfg.xml'
                if not os.path.exists(candidate):
                    config_name = candidate
                    break
            cfg_lookup.add(key)
            with open(config_name , 'w') as cfg_file:
                cfg_file.write(config_template.format(src_file=kernel_filename, compilation_options=compile_options))
            continue
        if in_source:
            src = src + line.rstrip() + '\n';
        if in_options:
            compile_options = line.strip()
        if in_process:
            kernel_name = line.strip()

if __name__ == "__main__":
    main()
