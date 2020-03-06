import os
import sys
  
# The Make class builds a project.
# There is a default behaviour and you can inherit from this class
# in order to change the default behaviour. Common changes can be 
# done by the __init__ arguments.
#
# Make object has 3 main functions for the build process:
# 1. build
# 2. clean
# 3. rebuild
#
# By default the Make class builds a project that located at the 'project_path'
# by compile every .c file in the project path, assembly every .asm and .S file in the project path,
# and link to .o products of the compilation and the assembly.
#
# All of the environment of the compilation (toolchain, output pathes, etc) is taken from
# the build configuration (we will explain later) and can be tweeked by arguments to the __init__ function.
# 
# The configuration is an another python script that contains a lot of info about the current wanted build.
# Everything that is needed in order to make a full firmware is there. The toolchain path, specific flags for 
# the toolchain, output directories and a lot more.
#
# The configurations are normally located at Qube\config subdir.
# By default, the script reads the file at Qube\config\default_config.txt in order to choose the default configuration .py file.
# If the environment variable QUBE_CONFIG exists, the class will use the filename in it as the default configuration.
#
# The output base directory is taken from the configuration.

# The __init__ function arguments:
#
# Mandatory:
# project_path - full path the to project base dir.
# 
# Optional:
# config_name - Can be used in order to peek specific config .py file.
#               If just a name is given, the script will search for the configuration in the Qube\config directory.
#               If fullpath is given, the script will use it as the configuration.
# obj_subdir - can be use in order to change the default intermediate products (like .o files) subdirectory.
#              by default its just in the base output directory.
# bin_subdir - can be use in order to change the output subdirectory.
#              by default its just in the base output directory.
# bin_filename - Can be use in order to choose the output filename. The default name is the project dirname.
# bin_fileext - Can be use (if bin_filename wasn't used) in order to choose the output filename extention.
#               The default is taken from the configuration ("OUT_EXT").
# additional_cflags - flags to add to the compile commandline.
# additional_lflags - flags to add to the link commandline.
# config - dictionary that can be used in order to override the configuration dictionary.


# environment variables:
QUBE_CONFIG = "QUBE_CONFIG"

# config variables:
OUT_EXT = "OUT_EXT"
PATH_FIX = "PATH_FIX"
OUTPUT_BASE = "OUTPUT_BASE"
BIN_BASE_SUBDIR = "BIN_BASE_SUBDIR"
OBJ_BASE_SUBDIR = "OBJ_BASE_SUBDIR"
CC = 'CC'
LD = 'LD'
AS = 'AS'
OBJCOPY = 'OBJCOPY'
CFLAGS = 'CFLAGS'
LFLAGS = 'LFLAGS'
SFLAGS = 'SFLAGS'
LOG = 'LOG'
class Make(object):
    def __init__(self, project_path,
                       config_name = None,
                       obj_subdir = "", 
                       bin_subdir = "",
                       bin_filename = None,
                       bin_fileext = None,
                       additional_cflags = None,
                       additional_sflags = None,
                       additional_lflags = None,
                       config = {}):
        self.project_path = project_path
        self.log_severity = 2
        self.config = self._get_config(config_name).config
        self.log_severity = self.config["LOG"]
        # override config values:
        for i in config:
            self.config[i] = config[i]
        
        self.obj_subdir = obj_subdir
        self.bin_subdir = bin_subdir
        self.additional_cflags = additional_cflags
        self.additional_sflags = additional_sflags
        self.additional_lflags = additional_lflags
        if bin_filename == None:
            bin_filename = os.path.basename(project_path)
            if bin_fileext == None:
                bin_fileext = self.config[OUT_EXT]
            self.bin_filename = ".".join([bin_filename, bin_fileext])
        else:
            self.bin_filename = bin_filename
        rel_out_dir = self.config[OUTPUT_BASE]
        cur_dir_name = os.path.dirname(os.path.abspath(os.path.realpath(__file__)))
        qube_base_dir = os.path.abspath(os.path.join(cur_dir_name,"..",".."))
        self.output_base = os.path.join(qube_base_dir, rel_out_dir)
    
    def _compile(self, file):
        flags = self.config[CFLAGS]
        if self.additional_cflags:
            flags = " ".join([flags, self.additional_cflags])
        obj_file_path = self._get_obj_file_path(file)
        command = self.config[CC]%" ".join([flags, self._fix_path(file), "-c -o %s"%self._fix_path(obj_file_path)])
        if self._is_build_required([file], [obj_file_path]):
            self.log("Compiling: %s"%file, 1)
            return self.execute(command)
        else:
            return 0
    
    def _assemble(self, file):
        flags = self.config[SFLAGS]
        if self.additional_sflags:
            flags = " ".join([flags, self.additional_sflags])
        obj_file_path = self._get_obj_file_path(file)
        command = self.config[AS]%" ".join([flags, self._fix_path(file), "-o %s"%self._fix_path(obj_file_path)])
        if self._is_build_required([file], [obj_file_path]):
            self.log("Assembling: %s"%file, 1)
            return self.execute(command)
        else:
            return 0
    
    def _link(self, in_files):
        flags = self.config[LFLAGS]
        if self.additional_lflags:
            flags = " ".join([flags, self.additional_lflags])
        bin_file_path = self._get_bin_file_path()
        command = self.config[LD]%" ".join([flags] + [self._fix_path(file) for file in in_files] + ["-o %s"%self._fix_path(bin_file_path)])
        if self._is_build_required(in_files, [bin_file_path]):
            self.log("Linking: %s"%bin_file_path, 1)
            return self.execute(command)
        else:
            return 0
        
    def _get_bin_file_path(self):
        return os.path.abspath(os.path.join(self.output_base, self.config[BIN_BASE_SUBDIR], self.bin_subdir, self.bin_filename))
        
        
    def _get_obj_file_path(self, file_path):
        n = os.path.basename(file_path)
        ext = n[n.rfind("."):]
        if '.' in n:
            out_file_name = n[:-len(ext)] + ".o"
        else:
            out_file_name = n + ".o"
        return os.path.abspath(os.path.join(self.output_base, self.config[OBJ_BASE_SUBDIR], self.obj_subdir, out_file_name))
        
    def _is_build_required(self, in_files, out_files):
        if len(in_files) == 1:
            if in_files[0].endswith(".c"): # if it c file, and h file in the current directory has been change, we will build it again:
                in_files += [os.path.join(os.path.dirname(in_files[0]), x) for x in os.listdir(os.path.dirname(in_files[0])) if x.endswith(".h")]
        out = [os.path.getmtime(f) for f in out_files if os.path.exists(f)]
        if len(out) == 0:
            return True
        return max([os.path.getmtime(f) for f in in_files  if os.path.exists(f)]) > \
               min(out)
        
    def _fix_path(self, filepath):
        if PATH_FIX in self.config:
            if self.config[PATH_FIX] == 'windows_to_bash':
                filepath = filepath.replace("C:","/mnt/c").replace("\\","/")
                
        return filepath
    def execute(self, cmd):
        self.log("execute: %s"%cmd)
        return os.system(cmd)
        
    def _build(self):
        for f in self.get_files_to_compile():
            ret = self._compile(f)
            if ret != 0:
                return ret
        for f in self.get_files_to_assembly():
            ret = self._assemble(f)
            if ret != 0:
                return ret
        return self._link(in_files = self.get_files_to_link())
                
    def _clean(self):
        self._clean_file(self._get_bin_file_path())
        for obj in self.get_files_to_link():
            self._clean_file(obj)
        return 0
        
    def _clean_file(self, file_path):
        if os.path.exists(file_path):
            self.log("Detete file: %s"%file_path, 1)
            os.unlink(file_path)

    def _get_config(self, config_name = None):
        cur_dir_name = os.path.dirname(os.path.abspath(os.path.realpath(__file__)))
        config_dir = os.path.abspath(os.path.join(cur_dir_name,"..","config"))
        if config_name != None:
            pass
        elif QUBE_CONFIG in os.environ:
            config_name = os.environ[QUBE_CONFIG]
        else:
            f = os.path.join(config_dir, "default_config.txt")
            if not os.path.exists(f):
                print("No QUBE_CONFIG env variable, and no default_config.txt file!")
                sys.exit(1)
            config_name = open(f).read()
        
        config_path = os.path.join(config_dir, config_name)
        if not os.path.exists(config_path):
            print("Cannot find %s!"%config_path)
            print("Try to use QUBE_CONFIG env variable to indicate the configruation file name")
            sys.exit(1)
        self.log("Start working using the configuration file: %s"%config_path, 3)
        ret = {}
        exec(open(config_path).read(), ret)
        return ret["Config"]()
    
    def get_files_to_compile(self):
        ret = []
        cur_path = os.path.abspath(self.project_path)
        for i in os.listdir(cur_path):
            if i.endswith(".c"):
                ret.append(os.path.join(cur_path, i))
        return ret
    
    def get_files_to_assembly(self):
        ret = []
        cur_path = os.path.abspath(self.project_path)
        for i in os.listdir(cur_path):
            if i.endswith(".asm") or i.endswith(".S"):
                ret.append(os.path.join(cur_path, i))
        return ret
    
    
    def get_files_to_link(self):
        ret = []
        for f in self.get_files_to_compile():
            ret.append(self._get_obj_file_path(f))
        for f in self.get_files_to_assembly():
            ret.append(self._get_obj_file_path(f))
            
        return ret
    
    def make_dirs(self):
        os.makedirs(self.output_base, exist_ok = True) 
        
        if BIN_BASE_SUBDIR in self.config:
            os.makedirs(os.path.join(self.output_base, self.config[BIN_BASE_SUBDIR], self.bin_subdir), exist_ok = True) 
        if OBJ_BASE_SUBDIR in self.config:
            os.makedirs(os.path.join(self.output_base, self.config[OBJ_BASE_SUBDIR], self.obj_subdir), exist_ok = True) 
        
    def build(self):
        self.make_dirs()
        return self._build()
        
    def rebuild(self):
        ret = self.clean()
        if ret != 0:
            return ret
        return self.build()
    
    def clean(self):
        return self._clean()
    
    def log(self, s, severity = 2):
        if self.log_severity >= severity:
            print(s)
    
    def run(self, argv):
        if len(argv) < 2:
            print("Usage: make.py [build/rebuild/cleen]\n")
            sys.exit(1)
        if argv[1] == "build":
            return self.build()
        if argv[1] == 'rebuild':
            return self.rebuild()
        if argv[1] == 'clean':
            return self.clean()

