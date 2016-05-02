import os
import sys
EXPORTS_FILE_NAME = "exports.h"
IMPORTS_FILE_NAME = "exports_auto_gen.h"
linesep = "\r\n"
if not os.path.exists(EXPORTS_FILE_NAME):
    print "No exports.h file found. Finished."
    sys.exit(0)
exports_file = open(EXPORTS_FILE_NAME,"rb")
module_name = None
auto_exports_include = False
exports = []
first_line = True
add_include_imports = False
while True:
    line = exports_file.readline()
    if line == '':
        break
    line2 = line.strip()
    if first_line and not line2.startswith('#include "%s"'%IMPORTS_FILE_NAME):
        add_include_imports = True
    if line2 != '':
        first_line = False
    if line2.startswith("#define MODULE_NAME"):
        if auto_exports_include:
            print "Error in auto exports: The #define MODULE_NAME must be before the include to 'auto_exports.h'!"
            sys.exit(1)
        module_name = line2[line2.find("#define MODULE_NAME")+19:].strip()
        continue
    if line2.startswith('#include "../common/auto_exports.h"'):
        auto_exports_include = True
        continue
    if line2.startswith("EXPORT("):
        assert line2.count("(") == 1
        assert line2.count(")") == 1
        func_name = line[line2.find("(")+1:line2.find(")")]
        exports.append(func_name)
        continue

exports_file.close()

if not module_name:
    print "Error in exports.h! no #define MODULE_NAME my_module_name!"
    sys.exit(1)
if not auto_exports_include:
    print "Error in exports.h! no '#include \"../common/auto_exports.h\"'!"
    sys.exit(1)
if add_include_imports:
    exports_data = open(EXPORTS_FILE_NAME,"rb").read()
    exports_file = open(EXPORTS_FILE_NAME,"wb")
    exports_file.write("#include \"%s\""%IMPORTS_FILE_NAME + linesep)
    exports_file.write(exports_data)
    exports_file.close()
imports_data = []
for export in exports:
    imports_data.append("#define %s EXP_%s_%s"%(export, module_name, export))
imports_data = "// Auto generated file!" + linesep + linesep.join(imports_data)
if os.path.exists(IMPORTS_FILE_NAME):
    orig_imports_data = open(IMPORTS_FILE_NAME,"rb").read()
    if orig_imports_data == imports_data:
        sys.exit(0)
open(IMPORTS_FILE_NAME,"wb").write(imports_data)
