###### make.py template ##############
import shutil


########## import make_class ###############
import os
import sys
this_file_dir = os.path.dirname(os.path.abspath(os.path.realpath(__file__)))
scripts_dir = os.path.join(this_file_dir,"..","..","build","scripts")
sys.path.insert(0,scripts_dir)
import make_class
sys.path.remove(scripts_dir)
############################################
class StaticFilesMake(make_class.Make):
    def __init__(self, project_path, **kargs):
        make_class.Make.__init__(self, project_path, bin_subdir = "system", 
                                                     bin_filename = "BOOT.TXT",
                                                     **kargs)
        
    def build(self):
        self.make_dirs()
        src = os.path.join(self.project_path, "BOOT.TXT")
        dst = self._get_bin_file_path()
        if self._is_build_required([src], [dst]):
            self.log("Coping: %s"%src, 1)
            shutil.copyfile(src, dst)
        return 0
    
    
def get_make_class():
    ret = StaticFilesMake(project_path = this_file_dir)
    return ret

if __name__ == '__main__':
    get_make_class().run(sys.argv)
