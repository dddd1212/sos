# This file cannot be run as is.
# It must be execute by open(exec()) by a builder script.
# In order to run properly, __file__ needs to point to the project directory and not to this file directory.
# In addition, the caller script must import the make_class python script.
import os
import sys
this_file_dir = os.path.dirname(os.path.abspath(os.path.realpath(__file__)))


class KernelProjectMake(make_class.Make):
    def __init__(self, project_path,
                       additional_cflags = None, additional_lflags = None, bin_subdir = "system", **kargs):
        
        cflags = "-fPIC"
        lflags = "-fPIC -fvisibility=hidden -shared"
        if additional_cflags != None:
            cflags = " ".join(cflags, additional_cflags)
        if additional_lflags != None:
            lflags = " ".join(cflags, additional_lflags)
        if os.path.basename(project_path).startswith("qkr_"):
            bin_filename = os.path.basename(project_path)[4:]+".qkr"
        else:
            bin_filename = None
        make_class.Make.__init__(self, project_path, additional_cflags = cflags, additional_lflags = lflags, bin_subdir = bin_subdir, bin_filename = bin_filename, **kargs)

def get_make_class():
    ret = KernelProjectMake(project_path = this_file_dir)
    return ret

if __name__ == '__main__':
    get_make_class().run(sys.argv)
