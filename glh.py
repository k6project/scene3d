import sys
if len(sys.argv) < 2:
    sys.exit(-1)
result = []
max_major = 1
max_minor = 0
if len(sys.argv) > 2:
    max_major = int(sys.argv[2])
if len(sys.argv) > 3:
    max_minor = int(sys.argv[3])
with open(sys.argv[1]) as f:
    version = -1
    for line in f:
        if line.startswith("#define GL_VERSION_"):
            version = len(result)
            ver = line[:-3].split("_")[-2:]
	    result.append({ "version" : ver, "functions":[] })
        elif line.startswith("#endif /* GL_VERSION_"):
            version = -1
	else:
            token = "APIENTRY gl"
            pos = line.find(token)
            if version > -1 and pos > 0:
                name = line[pos+len(token):line.find(" (")]
                result[version]["functions"].append([name, name.upper()])
print("#ifndef GLFUNCTION\n#define GLFUNCTION(a,b)\n#endif")
for r in result:
    major = int(r["version"][0])
    minor = int(r["version"][1])
    if major <= max_major and minor <= max_minor:
        for f in r["functions"]:
            print("GLFUNCTION(%s,%s)" %(f[0], f[1]))
print("#undef GLFUNCTION\n") 

    

