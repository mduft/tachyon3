/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#include <tachyon.h>
#include <uapi.h>
#include <thread.h>

uapi_desc_t* udesc;

extern int main(int argc, char** argv);

void umain(uapi_desc_t* uapi) {
    udesc = uapi;

    // TODO
    main(0, NULL);
}

uapi_desc_t* udesc_get() {
    return udesc;
}
