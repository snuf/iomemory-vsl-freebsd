//-----------------------------------------------------------------------------
// Copyright (c) 2006-2014, Fusion-io, Inc.(acquired by SanDisk Corp. 2014)
// Copyright (c) 2014 SanDisk Corp. and/or all its affiliates. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the SanDisk Corp. nor the names of its contributors
//   may be used to endorse or promote products derived from this software
//   without specific prior written permission.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------
#ifndef _FIO_PORT_ARCH_IA64_ATOMIC_H
#define _FIO_PORT_ARCH_IA64_ATOMIC_H

typedef struct fusion_atomic {
    volatile int value;
} fusion_atomic_t;

#if defined(__hpc__)
// Support using HP Ansi C compiler

#include <machine/sys/inline.h>

static inline void fusion_atomic_set(fusion_atomic_t *atomp, int32_t val)
{
    atomp->value = val;
}

static inline int32_t fusion_atomic_read(fusion_atomic_t *atomp)
{
    return atomp->value;
}

static inline void fusion_atomic_add(int32_t val, fusion_atomic_t *atomp)
{
    uint64_t old, ret;

    do
    {
        old = _Asm_zxt(_XSZ_4, atomp->value);
        _Asm_mov_to_ar(_AREG_CCV, old);

        ret = _Asm_cmpxchg(_SZ_W, _SEM_REL, &atomp->value, old+val, _LDHINT_NONE);
    } while (ret != old);
}

static inline void fusion_atomic_inc(fusion_atomic_t *atomp)
{
    _Asm_fetchadd(_FASZ_W, _SEM_ACQ, &atomp->value, 1, _LDHINT_NONE);
}

static inline int32_t fusion_atomic_incr(fusion_atomic_t *atomp)
{
    return (int32_t)_Asm_fetchadd(_FASZ_W, _SEM_ACQ, &atomp->value, 1, _LDHINT_NONE) + 1;
}

static inline int32_t fusion_atomic_add_return(int32_t val, fusion_atomic_t *atomp)
{
    uint64_t old, ret;

    do
    {
        old = _Asm_zxt(_XSZ_4, atomp->value);
        _Asm_mov_to_ar(_AREG_CCV, old);

        ret = _Asm_cmpxchg(_SZ_W, _SEM_REL, &atomp->value, old+val, _LDHINT_NONE);
    } while (ret != old);
    return (int32_t)old+val;
}

static inline void fusion_atomic_dec(fusion_atomic_t *atomp)
{
    _Asm_fetchadd(_FASZ_W, _SEM_ACQ, &atomp->value, -1, _LDHINT_NONE);
}

static inline int32_t fusion_atomic_decr(fusion_atomic_t *atomp)
{
    return (int32_t)_Asm_fetchadd(_FASZ_W, _SEM_ACQ, &atomp->value, -1, _LDHINT_NONE) - 1;
}

static inline void fusion_atomic_sub(int32_t val, fusion_atomic_t *atomp)
{
    uint64_t old, ret;

    do
    {
        old = _Asm_zxt(_XSZ_4, atomp->value);
        _Asm_mov_to_ar(_AREG_CCV, old);

        ret = _Asm_cmpxchg(_SZ_W, _SEM_REL, &atomp->value, old-val, _LDHINT_NONE);
    } while (ret != old);
}

static inline int32_t fusion_atomic_sub_return(int32_t val, fusion_atomic_t *atomp)
{
    uint64_t old, ret;

    do
    {
        old = _Asm_zxt(_XSZ_4, atomp->value);
        _Asm_mov_to_ar(_AREG_CCV, old);

        ret = _Asm_cmpxchg(_SZ_W, _SEM_REL, &atomp->value, old-val, _LDHINT_NONE);
    } while (ret != old);
    return (int32_t)old-val;
}

static inline int32_t fusion_atomic_exchange(fusion_atomic_t *atomp, int32_t val)
{
    // xchg has implied acquire semantics.  This isn't a full memory barrier; should we also perform a release?
    return _Asm_xchg(_SZ_W, &atomp->value, val, _LDHINT_NONE);
}

#else
// Use the sync builtins if our gcc version ( >= 4.0 ) supports it.  If not
// the following functions will be used instead.
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 2

static inline void fusion_atomic_set(fusion_atomic_t *atomp, int32_t val)
{
    atomp->value = val;
}

static inline int32_t fusion_atomic_read(fusion_atomic_t *atomp)
{
    return atomp->value;
}

static inline void fusion_atomic_add(int32_t val, fusion_atomic_t *atomp)
{
    (void)__sync_add_and_fetch(&atomp->value, val);
}

static inline void fusion_atomic_inc(fusion_atomic_t *atomp)
{
    (void)__sync_add_and_fetch(&atomp->value, 1);
}

static inline int32_t fusion_atomic_incr(fusion_atomic_t *atomp)
{
    return __sync_add_and_fetch(&atomp->value, 1);
}

static inline int32_t fusion_atomic_add_return(int32_t val, fusion_atomic_t *atomp)
{
    return __sync_add_and_fetch(&atomp->value, val);
}

static inline void fusion_atomic_dec(fusion_atomic_t *atomp)
{
    (void)__sync_sub_and_fetch(&atomp->value, 1);
}

static inline int32_t fusion_atomic_decr(fusion_atomic_t *atomp)
{
    return __sync_sub_and_fetch(&atomp->value, 1);
}

static inline void fusion_atomic_sub(int32_t val, fusion_atomic_t *atomp)
{
    (void)__sync_sub_and_fetch(&atomp->value, val);
}

static inline int32_t fusion_atomic_sub_return(int32_t val, fusion_atomic_t *atomp)
{
    return __sync_sub_and_fetch(&atomp->value, val);
}


/*
  Taken from http://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html

  This builtin (__sync_lock_test_and_set), as described by Intel, is not a
  traditional test-and-set operation, but rather an atomic exchange operation.
  It writes value into *ptr, and returns the previous contents of *ptr.

  Many targets have only minimal support for such locks, and do not support a full
  exchange operation. In this case, a target may support reduced functionality here
  by which the only valid value to store is the immediate constant 1. The exact value
  actually stored in *ptr is implementation defined.

  This builtin is not a full barrier, but rather an acquire barrier. This means that
  references after the builtin cannot move to (or be speculated to) before the builtin,
  but previous memory stores may not be globally visible yet, and previous memory loads
  may not yet be satisfied.
*/

static inline int32_t fusion_atomic_exchange(fusion_atomic_t *atomp, int32_t val)
{
    __sync_synchronize();
    return __sync_lock_test_and_set(&atomp->value, val);
}

#else // __GNUC__ >= 4 && __GNUC_MINOR__ >= 2

#error Unsupported GCC version for Itanium

#endif // __GNUC__ >= 4 && __GNUC_MINOR__ >= 2

#endif // __hpc__

#endif // _FIO_PORT_ARCH_IA64_ATOMIC_H

