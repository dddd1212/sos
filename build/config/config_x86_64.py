import os

class Config(object):
    def __init__(self):
        self.config = {}
        self.config["PATH_FIX"] = 'windows_to_bash'
        self.config["CC"] = 'bash -c "/home/user/opt/cross/bin/x86_64-elf-gcc %s"'
        self.config["LD"] = 'bash -c "/home/user/opt/cross/bin/x86_64-elf-ld %s"'
        self.config["AS"] = 'bash -c "/home/user/opt/cross/bin/x86_64-elf-as %s"'
        self.config["OBJCOPY"] = 'bash -c "/home/user/opt/cross/bin/x86_64-elf-objcopy %s"'
        
        self.config["CFLAGS"] = '-g -Wall'
        self.config["LFLAGS"] = '-g'
        self.config["SFLAGS"] = '-g'
        self.config["OUT_EXT"] = "qkr"
        self.config["OUTPUT_BASE"] = '_output'
        self.config["BIN_BASE_SUBDIR"] = 'x86_64\\bin'
        self.config["OBJ_BASE_SUBDIR"] = 'x86_64\\obj'
        self.config["LOG"] = 2
        
        self.config["QEMU_PATH"] = r'"C:\Program Files\qemu\qemu-system-x86_64.exe"'
        self.config["QEMU_FLAGS"] = r'-device e1000e,netdev=net0 -netdev user,id=net0'