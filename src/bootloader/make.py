###### make.py template ##############



########## import make_class ###############
import os
import sys
this_file_dir = os.path.dirname(os.path.abspath(os.path.realpath(__file__)))
scripts_dir = os.path.join(this_file_dir,"..","..","build","scripts")
sys.path.insert(0,scripts_dir)
import make_class
sys.path.remove(scripts_dir)
############################################


class BootloaderMake(make_class.Make):
    def __init__(self, project_path, **kargs):
        make_class.Make.__init__(self, project_path, obj_subdir = "bootloader", 
                                                     bin_subdir = "bootloader",
                                                     additional_cflags = '-ffreestanding -nostdlib -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables',
                                                     config = {make_class.OUT_EXT:"elf"},
                                                     **kargs)
        
        # We have to do this after the __init__ because we use the output_base and config:
        linker_ld = self._fix_path(os.path.join(this_file_dir,"linker.ld"))
        real_mode_o = self._fix_path(os.path.dirname(self._get_obj_file_path("real_mode.asm")))
        self.additional_lflags = "-T %s -L %s"%(linker_ld, real_mode_o)
        
    def _get_flat_bin_file_path(self):
        bin = self._get_bin_file_path()
        
        return  bin[:bin.rfind(".")]+ ".bin"
    
    def build(self):
        ret = make_class.Make.build(self)
        if (ret != 0):
            return ret
        bin_path = self._get_bin_file_path()
        flat_bin_path = self._get_flat_bin_file_path()
        command = self.config[make_class.OBJCOPY]%("-O binary " + self._fix_path(bin_path) + " " + self._fix_path(flat_bin_path))
        if self._is_build_required([bin_path], [flat_bin_path]):
            self.log("Objdumping: %s"%flat_bin_path, 1)
            return self.execute(command)
        else:
            return 0
    
    def clean(self):
        self._clean_file(self._get_flat_bin_file_path())
        return make_class.Make.clean(self)
    
    def _link(self, in_files):
        in_files = [x for x in in_files if not x.endswith("real_mode.o")]
        return make_class.Make._link(self, in_files)
               
    
def get_make_class():
    ret = BootloaderMake(project_path = this_file_dir)
    return ret


def main():
    if len(sys.argv) < 2:
        print("Usage: make.py [build/rebuild/cleen]\n")
        sys.exit(1)
    make = get_make_class()
    if sys.argv[1] == "build":
        return make.build()
    if sys.argv[1] == 'rebuild':
        return make.rebuild()
    if sys.argv[1] == 'clean':
        return make.clean()

if __name__ == '__main__':
    main()