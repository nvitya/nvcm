/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM project: https://github.com/nvitya/nvcm
 * Copyright (c) 2018 Viktor Nagy, nvitya.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     system.cpp
 *  brief:    Newlib C library requirements, _sbrk implementation
 *  version:  2.00
 *  date:     2018-02-10
 *  authors:  nvitya
 *  ------------------------------
 *    v2.00: new heap allocator using the linker script defined heap size
*/

#include <errno.h>

extern int errno;

extern "C"
{

extern unsigned  __end;       // end of the used ram RAM (where the heap begins), defined in the linker script
extern unsigned  __stack;     // end of the RAM (where the zero size stack begins), defined in the linker script
extern unsigned  _Heap_Limit; // end of the RAM available for heap (where the maximal size stack begins), defined in the linker script

unsigned         system_heap_size = 0;
char *           system_heap_end_ptr = nullptr;  // actual end of the RAM used as heap

__attribute__((weak))  unsigned _sbrk(int incr)
{
  char * sys_heap_limit = (char *)&_Heap_Limit;

  char * chunk_begin = system_heap_end_ptr;
  if (!chunk_begin)
  {
    chunk_begin = (char *)&__end;
  }

  char * chunk_end   = chunk_begin + incr;

  if (chunk_end > sys_heap_limit)
  {
    errno = ENOMEM;
    return (unsigned) -1;
  }

  system_heap_size += incr;
  system_heap_end_ptr = chunk_end;

  return (unsigned)chunk_begin;
}

// At some compiler settings these system functions might needed
// just providing empty functions

__attribute__((weak))  int _getpid(void)
{
	return 1;
}

__attribute__((weak))  int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

__attribute__((weak))  void _exit (int status)
{
	_kill(status, -1);
	while (1) {}		/* Make sure we hang here */
}

__attribute__((weak)) int _open(const char *name, int flags, int mode)
{
  return -1;
}

__attribute__((weak)) int _lseek(int file, int offset, int whence)
{
  return -1;
}

__attribute__((weak)) int _lseek_r(int file, int offset, int whence)
{
  return -1;
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
  return 0;
}
__attribute__((weak)) int _read_r(int file, char *ptr, int len)
{
  return 0;
}

__attribute__((weak)) int _write(int file, char *buf, int nbytes)
{
  return -1;
}

__attribute__((weak)) int _write_r(int file, char *buf, int nbytes)
{
  return -1;
}

__attribute__((weak)) int _close(void)
{
  return -1;
}

__attribute__((weak)) int _close_r(void)
{
  return -1;
}

__attribute__((weak)) int _fstat(int file, struct stat *st)
{
  return -1;
}
__attribute__((weak)) int _fstat_r(int file, struct stat *st)
{
  return -1;
}

__attribute__((weak)) int _link(char *oldname, char *newname)
{
  return -1;
}

__attribute__((weak)) int _unlink(char *name)
{
 return -1;
}

__attribute__((weak)) int _times(struct tms *buf)
{
  return -1;
}

__attribute__((weak)) int _wait(int *status)
{
  return -1;
}

__attribute__((weak)) int _isatty(int file)
{
  return -1;
}
__attribute__((weak)) int _isatty_r(int file)
{
  return -1;
}

} // extern "C"
