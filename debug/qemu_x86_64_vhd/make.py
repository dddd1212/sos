###### make.py template ##############



########## import make_class ###############
import os
import sys
this_file_dir = os.path.dirname(os.path.abspath(os.path.realpath(__file__)))
scripts_dir = os.path.join(this_file_dir,"..","..","build","scripts")
sys.path.insert(0,scripts_dir)
import make_class
############################################


class QemuX86Vhd(make_class.Make):
    def __init__(self, project_path, **kargs):
        make_class.Make.__init__(self, project_path, **kargs)
                                                    
    def build(self):
        qube_vhd = os.path.abspath(os.path.join(self.output_base, self.config[make_class.BIN_BASE_SUBDIR], "image", "qube.vhd"))
        self.log("Executing with Qemu: %s"%qube_vhd)
        return self.execute("%s %s %s"%(self.config["QEMU_PATH"], self.config["QEMU_FLAGS"], qube_vhd))
    
    def clean(self):
        return 0
    

def get_make_class():
    ret = QemuX86Vhd(project_path = this_file_dir)
    return ret

if __name__ == '__main__':
    get_make_class().run(sys.argv)
