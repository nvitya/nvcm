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
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include <errno.h>

extern int errno;
register char * stack_ptr asm("sp");

extern "C" unsigned _sbrk(int incr)
{
	extern char end asm("end");
	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0)  heap_end = &end;

	prev_heap_end = heap_end;
	if (heap_end + incr > stack_ptr)
	{
		errno = ENOMEM;
		return (unsigned) -1;
	}

	heap_end += incr;

	return (unsigned)prev_heap_end;
}

/* nvitya's note:
  some GNU libraries require these (_getpid, _kill, _exit) functions.
  but they are not required if you set the following options at the C++ optimization (In Eclipse CDT + GNU ARM Eclipse)
   - do not use exceptions (-fno-exceptions)
   - do not use RTTI
   - do not use _cxa_atexit()
   - do not use thread-safe statistics
 and the code will be significantly smaller too!
*/

#if 1

extern "C" int _getpid(void)
{
	return 1;
}

extern "C" int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

extern "C" void _exit (int status)
{
	_kill(status, -1);
	while (1) {}		/* Make sure we hang here */
}
#endif
