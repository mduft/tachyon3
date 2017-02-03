/* copyright (c) 2010 by markus duft <markus.duft@ssi-schaefer.com>
 * this file is part of the 'tachyon' operating system. */

#include "mboot.h"
#include "extp.h"
#include "vmem.h"
#include "spc.h"
#include "log.h"
#include "kheap.h"
#include "vmem_mgmt.h"

#include <paging.h>

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

#define MBOOT_VTEMP_REG_START   0xA0000000
#define MBOOT_VTEMP_REG_END     0xA00F0000

static void mboot_pmem_init();
static void mboot_pmem_protect();

INSTALL_EXTENSION(EXTP_PMEM_REGION, mboot_pmem_init, "multiboot")
INSTALL_EXTENSION(EXTP_PMEM_RESERVE, mboot_pmem_protect, "multiboot")

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

typedef struct {
    uint32_t    start;
    uint32_t    end;
    uint32_t    cmdline;
    uint32_t    resvd;
} PACKED mboot_mod_t;

static void* mboot_find_free_page() {
    /* this is some arbitrary address to search for free room
     * for temporary mappings */
    static void* mboot_pg_temp = (void*)MBOOT_VTEMP_REG_START;

    while(vmem_resolve(spc_current(), mboot_pg_temp) != 0 && 
            (uintptr_t)mboot_pg_temp < MBOOT_VTEMP_REG_END) 
    {
        mboot_pg_temp = (void*)((uintptr_t)mboot_pg_temp + PAGE_SIZE_4K);
    }

    if((uintptr_t)mboot_pg_temp >= MBOOT_VTEMP_REG_END) {
        warn("out of temporary virtual space to map multiboot structures\n");
        return NULL;
    }

    return mboot_pg_temp;
}

static void* mboot_map(uint32_t mbi_struct) {
    void* vaddr = mboot_find_free_page();

    uintptr_t page = ALIGN_DN(mbi_struct, PAGE_SIZE_4K);
    uintptr_t off = ALIGN_RST(mbi_struct, PAGE_SIZE_4K);

    if(!vaddr || !vmem_map(spc_current(), page, vaddr, 0)) {
        error("cannot temporary map multiboot structure!\n")
        return NULL;
    }

    return (void*)((uintptr_t)vaddr + off);
}

static void mboot_unmap(void* mapped) {
    vmem_unmap(spc_current(), mapped);
}

static void mboot_pmem_protect() {
    if(boot_state.ax != MBOOT_MAGIC) {
        return;
    }

    pmem_reserve(ALIGN_DN(boot_state.bx, PMEM_PAGESIZE), sizeof(mboot_info_t));
    register mboot_info_t* mbi = (mboot_info_t*)mboot_map(boot_state.bx);

    if(mbi->flags & MBOOT_FL_MEMMAP) {
        pmem_reserve(ALIGN_DN(mbi->mmap_addr, PMEM_PAGESIZE), mbi->mmap_len);
    }

    if(mbi->flags & MBOOT_FL_MODS) {
        if(mbi->mods_cnt > 0) {
            pmem_reserve(ALIGN_DN(mbi->mods_addr, PMEM_PAGESIZE), mbi->mods_cnt * sizeof(mboot_mod_t));
            register unsigned int i;
            register mboot_mod_t* mod = (mboot_mod_t*)mboot_map(mbi->mods_addr);
            for(i = 0; i < mbi->mods_cnt; ++i, ++mod) {
                pmem_reserve(ALIGN_DN(mod->start, PMEM_PAGESIZE), mod->end - mod->start);
            }
            mboot_unmap(mod);
        }
    }

    mboot_unmap(mbi);
}

static void mboot_pmem_init() {
    if(boot_state.ax != MBOOT_MAGIC) {
        return;
    }

    register mboot_info_t* mbi = (mboot_info_t*)mboot_map(boot_state.bx);

    if(mbi->flags & MBOOT_FL_MEMMAP) {
        mboot_mmap_t* mm = (mboot_mmap_t*)mboot_map(mbi->mmap_addr);

        for(mboot_mmap_t* mmap = mm; (uintptr_t)mmap < ((uintptr_t)mm + mbi->mmap_len);
            mmap = (mboot_mmap_t*)((uintptr_t)mmap + mmap->size + sizeof(mmap->size))) 
        {
            if(mmap->type == MBOOT_MM_AVAILABLE) {
                pmem_add(mmap->addr, mmap->len);
            }
        }

        mboot_unmap(mm);
    }

    mboot_unmap(mbi);
}

rd_header_t* mboot_find_rd() {
    if(boot_state.ax != MBOOT_MAGIC) {
        return NULL;
    }

    register mboot_info_t* mbi = (mboot_info_t*)mboot_map(boot_state.bx);

    rd_header_t* result = NULL;
    if(mbi->flags & MBOOT_FL_MODS) {
        if(mbi->mods_cnt > 0) {
            uint32_t i;
            trace("mods_addr: %p\n", mbi->mods_addr);
            register mboot_mod_t* mod = (mboot_mod_t*)mboot_map(mbi->mods_addr);
            for(i = 0; i < mbi->mods_cnt; ++i, ++mod) {
                void* mapped = mboot_map(mod->start);
                if(((rd_header_t*)mapped)->magic == RD_MAGIC) {
                    size_t sz = mod->end - mod->start;
                    trace("found rd at %p, nfiles=%d, size=%d bytes\n", 
                        mod->start, ((rd_header_t*)mapped)->num_files, sz);
                    if(sz > (RD_VSZ - PAGE_SIZE_4K)) {
                        fatal("rd too large to be mapped: %d bytes!\n", sz);
                    }

                    // continously map until all is done
                    phys_addr_t first_phys = ALIGN_DN(mod->start, PMEM_PAGESIZE);
                    phys_addr_t last_phys = ALIGN_UP(mod->end, PMEM_PAGESIZE);
                    
                    trace("mapping rd from %p to %p (%d bytes)\n", first_phys, last_phys, sz);

                    uintptr_t virt = RD_VIRTUAL;
                    while(first_phys < last_phys) {
                        if(!vmem_map(spc_current(), first_phys, (void*)virt, PG_GLOBAL)) {
                            fatal("cannot map %p to %p for rd\n", first_phys, virt);
                        }

                        vmem_mgmt_add_global_mapping(first_phys, (void*)virt, PG_GLOBAL);
                        
                        first_phys += PAGE_SIZE_4K;
                        virt += PAGE_SIZE_4K;
                    }

                    return (rd_header_t*)(RD_VIRTUAL + ALIGN_RST(mod->start, PAGE_SIZE_4K));
                }
                mboot_unmap(mapped);
            }
            mboot_unmap(mod);
        }
    }

    mboot_unmap(mbi);
    return result;
}
