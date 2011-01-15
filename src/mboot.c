/* copyright (c) 2010 by markus duft <mduft@gentoo.org>
 * this file is part of the 'tachyon' operating system. */

#include "mboot.h"
#include "extp.h"

#define MAX_MBOOT_MEM_REGIONS   16
#define MBOOT_MAGIC             0x2BADB002

#define MBOOT_FL_MEM            0x001
#define MBOOT_FL_BOOTDEV        0x002
#define MBOOT_FL_CMDLINE        0x004
#define MBOOT_FL_MODS           0x008
#define MBOOT_FL_ELFSYM         0x020
#define MBOOT_FL_MEMMAP         0x040
#define MBOOT_FL_DRIVES         0x080
#define MBOOT_FL_CONFIG_TABLE   0x100
#define MBOOT_FL_LOADER_NAME    0x200
#define MBOOT_FL_APM_TABLE      0x400
#define MBOOT_FL_VIDEO_INFO     0x800

#define MBOOT_MM_AVAILABLE      1
#define MBOOT_MM_RESERVED       2

INSTALL_EXTENSION(EXTP_PMEM_REGION, mboot_pmem_init, "multiboot")

typedef struct {
    uint32_t    flags;
    uint32_t    mem_lower;
    uint32_t    mem_upper;
    uint32_t    boot_dev;
    uint32_t    cmdline;
    uint32_t    mods_cnt;
    uint32_t    mods_addr;
    uint32_t    elf_sh_num;
    uint32_t    elf_sh_size;
    uint32_t    elf_sh_addr;
    uint32_t    elf_sh_shndx;
    uint32_t    mmap_len;
    uint32_t    mmap_addr;
    uint32_t    drives_len;
    uint32_t    drives_addr;
    uint32_t    config_table;
    uint32_t    loader_name;
    uint32_t    apm_tab;
    uint32_t    vbe_control_info;
    uint32_t    vbe_mode_info;
    uint32_t    vbe_mode;
    uint32_t    vbe_iface_seg;
    uint32_t    vbe_iface_off;
    uint32_t    vbe_iface_len;
} PACKED mboot_info_t;

typedef struct {
    uint32_t    size;
    uint64_t    addr;
    uint64_t    len;
    uint32_t    type;
} PACKED mboot_mmap_t;

void mboot_pmem_init() {
    if(boot_state.ax != MBOOT_MAGIC) {
        return;
    }

    register mboot_info_t* mbi = (mboot_info_t*)(uintptr_t)boot_state.bx;

    if(mbi->flags & MBOOT_FL_MEMMAP) {
        for(mboot_mmap_t* mmap = (mboot_mmap_t*)(uintptr_t)mbi->mmap_addr;
            (uintptr_t)mmap < (mbi->mmap_addr + mbi->mmap_len);
            mmap = (mboot_mmap_t*)((uintptr_t)mmap + 
                mmap->size + sizeof(mmap->size))) 
        {
            if(mmap->type == MBOOT_MM_AVAILABLE) {
                pmem_add(mmap->addr, mmap->len);
            }
        }
    }
}

