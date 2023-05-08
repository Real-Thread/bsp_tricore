import os

# toolchains options
ARCH='tricore'
CPU='tc36x'
CROSS_TOOL='tasking'

if  CROSS_TOOL == 'tasking':
	PLATFORM 	= 'tasking'
	EXEC_PATH 	= 'C:/Program Files (x86)/CodeSourcery/Sourcery G++ Lite/bin'

if os.getenv('RTT_EXEC_PATH'):
	EXEC_PATH = os.getenv('RTT_EXEC_PATH')

BUILD = 'debug'
EXEC_PATH = r"D:/software/AURIX-Studio-1.2.2/plugins/com.infineon.aurix.tools_1.2.2/build_system/tools/Compilers/Tasking_1.1r4/ctc/bin"

if PLATFORM == 'tasking':
    # toolchains
    PREFIX = 'cctc'
    CC = PREFIX + ''
    AS = PREFIX + ''
    AR = PREFIX + ''
    LINK = PREFIX + ''
    TARGET_EXT = 'elf'
    OBJDUMP = PREFIX + ''
    OBJCPY = EXEC_PATH + 'elfsize'

    DEVICE = ''
    CFLAGS = DEVICE
    AFLAGS =  DEVICE
    LFLAGS = DEVICE + ' - lrt - lfp_fpu - lcs_fpu - Wl - Oc - Wl - OL - Wl - Ot - Wl - Ox - Wl - Oy - Wl - -map - file = "TC364.map" - Wl - mc - Wl - mf - Wl - mi - Wl - mk - Wl - ml - Wl - mm - Wl - md - Wl - mr - Wl - mu - -no - warnings = -Wl - -error - limit = 42 - -strict - -nachronisms - -force - c + + -Ctc36x - o rtthread.elf - Wl - o rtthread.hex:IHEX - -lsl - core = vtc - -lsl - file =../ Lcf_Tasking_Tricore_Tc.lsl $(OBJS)'

    POST_ACTION = OBJCPY + 'rtthread.elf\n'

def dist_handle(BSP_ROOT, dist_dir):
    import shutil
    if CPU == 'tc26x':
        src = os.path.join(BSP_ROOT, "../libcpu/tc26x")
        dst = os.path.join(dist_dir, "libcpu")
    if CPU == 'tc36x':
        src = os.path.join(BSP_ROOT, "../libcpu/tc36x")
        dst = os.path.join(dist_dir, "libcpu")
    if CPU == 'tc37x':
        src = os.path.join(BSP_ROOT, "../libcpu/tc37x")
        dst = os.path.join(dist_dir, "libcpu")
        
    shutil.copytree(src, dst)
