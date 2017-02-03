/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#include "path.h"
#include "log.h"
#include "string.h"

path_t* path_create(char const* p, heap_t* heap) {
    size_t sep_len = strlen(PATH_SEPERATOR);
    if(p == NULL || strncmp(p, PATH_SEPERATOR, sep_len) != 0) {
        warn("invalid path given, either NULL or not absolute: %s\n", p);
        return NULL;
    }

    path_t* path = (path_t*)heap_alloc(heap, sizeof(path_t));
    path->heap = heap;

    size_t count = 0;
    char const* comp = p;
    while((comp = strstr(comp, PATH_SEPERATOR)) != NULL) { ++count; comp += sep_len; }

    path->count = count;
    path->components = (char**)heap_alloc(heap, sizeof(char*) * count);

    comp = p;
    count = 0;
    while((comp = strstr(comp, PATH_SEPERATOR)) != NULL) {
        comp += sep_len; // skip separator.

        char const* next = strstr(comp, PATH_SEPERATOR);
        size_t comp_len;
        if(next != NULL) {
            comp_len = ((uintptr_t)next - (uintptr_t)comp);
        } else {
            comp_len = strlen(comp);
        }

        char* buf = (char*)heap_alloc(heap, comp_len + 1);
        strncpy(buf, comp, comp_len);
        buf[comp_len] = 0;

        path->components[count++] = buf;
    }

    return path;
}

char* path_string(path_t* path) {
    size_t i, len = 0;
    size_t sep_len = strlen(PATH_SEPERATOR);
    size_t pos = 0;

    if(path == NULL || (path->count > 0 && path->components == NULL)) {
        warn("given path is invalid: %p\n", path);
        return NULL;
    }

    for(i = 0; i < path->count; ++i) {
        len += strlen(path->components[i]);
    }

    char * result = heap_alloc(path->heap, len);
    len += (path->count * sep_len) + 1;
    
    for(i = 0; i < path->count; ++i) {
        // insert path seperator - path_t is always absolute
        strncpy(&result[pos], PATH_SEPERATOR, sep_len);
        pos += sep_len;

        // insert the actual component of the path
        size_t comp_len = strlen(path->components[i]);
        strncpy(&result[pos], path->components[i], comp_len);
        pos += comp_len;
    }

    result[pos] = 0;
    return result;
}
