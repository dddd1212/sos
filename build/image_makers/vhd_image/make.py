###### make.py template ##############



########## import make_class ###############
import os
import sys
this_file_dir = os.path.dirname(os.path.abspath(os.path.realpath(__file__)))
scripts_dir = os.path.join(this_file_dir,"..","..","scripts")
sys.path.insert(0,scripts_dir)
import make_class
sys.path.remove(scripts_dir)
############################################

sys.path.insert(0,this_file_dir)
import fat32tools
sys.path.remove(this_file_dir)
class VhdImageMake(make_class.Make):
    def __init__(self, project_path, **kargs):
        make_class.Make.__init__(self, project_path, obj_subdir = "system", # obj_subdir is used just to create the "system" subdir if it isn't exists
                                                     bin_subdir = "image", 
                                                     bin_filename = "qube.vhd",
                                                     **kargs)
        # When building an image, we consider the bin directory as the "obj" directory
        self.config["OBJ_BASE_SUBDIR"] = self.config["BIN_BASE_SUBDIR"]
    def build(self):
        self.make_dirs()
        bootloader_path = os.path.join(os.path.dirname(self._get_bin_file_path()), "..", "bootloader", "bootloader.bin")
        system_dir = os.path.abspath(os.path.join(os.path.dirname(self._get_bin_file_path()), "..", "system"))
        vhd_path = self._get_bin_file_path()
        
        if self._is_build_required([bootloader_path] + [os.path.join(system_dir, x) for x in os.listdir(system_dir)], [vhd_path]):
            self.log("Imaging: %s"%vhd_path, 1)
            boot_code = open(bootloader_path,"rb").read()[90:]
            files = fat32tools.make_vhd(system_dir, vhd_path)
            for orig, short in files:
                self.log("Copying to image: %s -> %s"%(os.path.join(system_dir,orig), short), 1)
            v = open(vhd_path,"r+b")
            v.seek(0x10000 + 90)
            v.write(boot_code)
            v.close()
        return 0
    def clean(self):
        return make_class.Make.clean(self)
    
def get_make_class():
    ret = VhdImageMake(project_path = this_file_dir)
    return ret

if __name__ == '__main__':
    get_make_class().run(sys.argv)
