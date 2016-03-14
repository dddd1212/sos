import os

######### General configuration ########
RAW_DISK_FILE_PATH = os.path.join("output","sos.bin")
VMDK_DISK_FILE_PATH = os.path.join("output","sos.vmdk")


######### BOOT LOADER CONFIGURATION #################
COMPILE_BOOTLOADER = True
REAL_MODE_ASM_PATH = os.path.join("tools","nasm","nasm.exe")
REAL_MODE_FILE_PATH = os.path.join("bootloader", "real_mode2.asm")
REAL_MODE_OUTPUT = os.path.join("bootloader", "real_mode.bin")
#####################################################

######### Glue configuration ###########
GLUE_ALL = True
########################################

###### RAW to VMDK ###########
RAW_TO_VMDK = False
RAW_TO_VMDK_TOOL_PATH = os.path.join("tools","raw2vmdk","VBoxManage.exe")
##############################

##### Run in Bochs #####
BOCHS = True

def main():
    if COMPILE_BOOTLOADER:
        os.system(REAL_MODE_ASM_PATH + " " + REAL_MODE_FILE_PATH + " -o " + REAL_MODE_OUTPUT)
    
    if GLUE_ALL:
        open(RAW_DISK_FILE_PATH,"wb").write(open(REAL_MODE_OUTPUT,"rb").read().ljust(512*1024*2))
    
    
    if RAW_TO_VMDK:
        os.system("del %s"%VMDK_DISK_FILE_PATH)
        os.system(RAW_TO_VMDK_TOOL_PATH + " convertfromraw %s %s --format VMDK"%(RAW_DISK_FILE_PATH, VMDK_DISK_FILE_PATH))
    
    if BOCHS:
        raw_input("press to run in BOSCH...")
        os.system("bochs")
    
    
    
main()