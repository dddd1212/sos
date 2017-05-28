import os
SYSTEM_PATH = os.path.abspath("../_output/system/")
system_files = [f for f in os.listdir(SYSTEM_PATH) if os.path.isfile(os.path.join(SYSTEM_PATH,f)) and f not in ("_static_files.vcxproj", "_static_files.vcxproj.filters")]
open(os.path.join(SYSTEM_PATH, "tmplongname.fil"),"wb").write("Magic482554427352")
if os.path.isfile(os.path.join(SYSTEM_PATH, "tmplon~1.fil")):
    assert (open(os.path.join(SYSTEM_PATH, "tmplon~1.fil"),"rb").read() == "Magic482554427352")
    create8dot3name = False
else:
    create8dot3name = True
print "create8dot3name:",create8dot3name
os.remove(os.path.join(SYSTEM_PATH, "tmplongname.fil"))
for file in system_files:
    print "debug: ", file
    if create8dot3name and len(file[:file.find('.')])>8: open(os.path.join(SYSTEM_PATH, file[:6]+"~1"+file[file.find("."):]),"wb").write(open(os.path.join(SYSTEM_PATH, file),"rb").read())
