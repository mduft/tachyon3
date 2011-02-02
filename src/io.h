/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

static inline uint8_t inb (uint16_t p) {
  uint8_t v;
  asm volatile("inb %w1,%0" : "=a" (v) : "Nd" (p));
  return v;
}

static inline uint16_t inw (uint16_t p) {
  uint16_t v;
  asm volatile("inw %w1,%0" : "=a" (v) : "Nd" (p));
  return v;
}

static inline uint32_t inl (uint16_t p) {
  uint32_t v;
  asm volatile("inl %w1,%0" : "=a" (v) : "Nd" (p));
  return v;
}

static inline void outb (uint8_t v, uint16_t p) {
  asm volatile("outb %b0,%w1": :"a" (v), "Nd" (p));
}

static inline void outw (uint16_t v, uint16_t p) {
  asm volatile("outw %w0,%w1": :"a" (v), "Nd" (p));
}

static inline void outl (uint32_t v, uint16_t p) {
  asm volatile("outl %0,%w1": :"a" (v), "Nd" (p));
}
