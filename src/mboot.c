/* copyright (c) 2010 by markus duft <mduft@gentoo.org>
 * this file is part of the 'tachyon' operating system. */

#include "mboot.h"
#include "extp.h"

INSTALL_EXTENSION(EXTP_PMEM_REGION, mboot_pmem_extp, "multiboot")

static bool mboot_pmem_ext_cb(size_t idx, uintptr_t* start, size_t* len) {
    return false;
}

pmem_ext_t mboot_pmem_extp() {
    static pmem_ext_t ext = { 0, mboot_pmem_ext_cb };

    if(ext.reg_cnt == 0) {
        /* find regions */
        /* TODO: boot code must save initial context globally 
         * somewhere for us to access */
    }

    return ext;
}

