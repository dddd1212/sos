import os
import sys
import make_class


class KernelProjectMake(make_class.Make):
    def __init__(self, project_path,
                       additional_cflags = None, additional_lflags = None, bin_subdir = "system", **kargs):
        
        additional_cflags = "-fpic",
        additional_lflags = "-fpic -fvisibility=hidden -shared")
                if additional_cflags != None:
                    
                make_class.Make.__init__(self, project_path, " ".join(additional_cflags,

def get_make_class():
    ret = make_class.Make(project_path = this_file_dir,
                          bin_subdir = "system",
                          additional_cflags = "-fpic",
                          additional_lflags = "-fpic -fvisibility=hidden -shared")
    raise Exception("Not implemented")
    return ret
    
if __name__ == '__main__':
    get_make_class().run(sys.argv)
