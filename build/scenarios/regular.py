import os
import sys
bootloader_projects = ['_static_files', 'bootloader']
kernel_projects = ['qkr_libc', 'qkr_memory_manager', 'qkr_screen', 'qkr_acpi', 'qkr_interrupts', 'qkr_scheduler',
                   'qkr_keyboard','qkr_qbject_manager','qkr_disk_manager','qkr_fat32','qkr_device_manager',
                   'qkr_sync','qkr_network_manager','qkr_intel82574','qkr_kernel']
image_projects = ['vhd_image']
debug_projects = ['qemu_x86_64_vhd']

this_file_dir = os.path.dirname(os.path.abspath(os.path.realpath(__file__)))
qube_base_dir = os.path.join(this_file_dir,"..","..")
projects_base_dir = os.path.join(qube_base_dir, "src")
image_makers_base_dir = os.path.join(qube_base_dir,"build","image_makers")
debug_base_dir = os.path.join(qube_base_dir,"debug")
build_scripts_base_dir = os.path.join(qube_base_dir,"build","scripts")

def build():
    print("\n\n**************************** BUILD ******************************")
    do_projects("build", bootloader_projects, projects_base_dir, "bootloader")
    do_projects("build", kernel_projects, projects_base_dir, "kernel")
    do_projects("build", image_projects, image_makers_base_dir, "image_makers")
    
def clean():
    print("\n\n**************************** CLEAN ******************************")
    do_projects("clean", image_projects[::-1], image_makers_base_dir, "image_makers")
    do_projects("clean", kernel_projects[::-1], projects_base_dir, "kernel")
    do_projects("clean", bootloader_projects[::-1], projects_base_dir, "bootloader")
    
def rebuild():
    clean()
    build()

def do_projects(cmd, projects, base_dir, type):
    global g_project
    for project in projects:
        if g_project != None:
            if project != g_project:
                continue
        print("\n------- %s -------" % project)
        make_path = os.path.join(base_dir, project, "make.py")
        glob = {"__file__":make_path}
        if not os.path.exists(make_path):
            sys.path.insert(0,build_scripts_base_dir)
            import make_class
            sys.path.remove(build_scripts_base_dir)
            glob["make_class"] = make_class
            if type == 'kernel': # use the kernel make:
                make_path = os.path.join(build_scripts_base_dir, "kernel_default_make.py")
            else:
                make_path = os.path.join(build_scripts_base_dir, "default_make.py")
        make_path = os.path.abspath(make_path)
        try:
            exec(open(make_path).read(), glob)
            ret = glob["get_make_class"]().run([make_path, cmd])
        except:
            print("Error in %s"%make_path)
            raise
        if (ret != 0):
            print("Error in %s %s. exiting."%(cmd, make_path))
            sys.exit(1)

def debug():
    do_projects("build", debug_projects, debug_base_dir, "debug")

def main():
    global g_project
    if len(sys.argv) < 2:
        print("Usage: regular.py [build/rebuild/cleen]\n")
        sys.exit(1)
    g_project = None
    if sys.argv[-1] not in ("build","rebuild","clean","debug"):
        g_project = sys.argv[-1]
        sys.argv = sys.argv[:-1]
    for arg in sys.argv[1:]:
        if arg == "build":
            build()
        if arg == 'rebuild':
            rebuild()
        if arg == 'clean':
            clean()
        if arg == 'debug':
            debug()
    return 0
if __name__ == '__main__':
    main()