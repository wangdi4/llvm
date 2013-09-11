import sys

def parse_dependecy():
    data = sys.stdin.readlines()
    skipPrefix = True
    result = ""
    for line in data:
        if (line.find(".obj:") >= 0):
            skipPrefix = False
            line = line[line.find(".obj:") + 5 :]
        elif (line.find(".o:") >= 0):
            skipPrefix = False
            line = line[line.find(".o:") + 3 :]
        if (skipPrefix == True):
            continue
        line = line.rstrip('\n')
        if (line.rfind("\\") >= 0):
            if (len(line) == 1):
                continue
            if ((line.rfind("\\")) == (len(line) -1)):
                line = line[0:(line.rfind("\\") -1)]
        result = result + line
    result = result.replace("\\ ", "#")
    resultList = result.split(" ")
    result = ""
    for r in resultList:
        if (len(r) == 0):
            continue
        if r.find("#") >= 0:
#            r = "\"" + r.replace("#", " ") + "\""
            r = r.replace("#", " ")
        result = result + "#" + r
    if ((len(result) > 0) and (result[0] == "#")):
        result = result[1:]
        result = result.replace("/", "\\")
        sys.stdout.write(result)

def main():
    parse_dependecy()

main()
