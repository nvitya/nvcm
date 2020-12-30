/*
 * filesystem.cpp
 *
 *  Created on: 29 Dec 2020
 *      Author: vitya
 */

#include "string.h"
#include <filesystem.h>

void TFileSystem::Init(TStorManager * astorman, uint64_t afirstaddr, uint64_t amaxsize)
{
	pstorman = astorman;
	firstaddr = afirstaddr;
	maxsize = amaxsize;

	initialized = false;
	initstate = 0;
	fsok = false;

	memset(&stra, 0, sizeof(stra));
	stra.completed = true;
}

void TFileSystem::Run()
{
	if (!pstorman)
	{
		return;
	}

	pstorman->Run();

	if (initialized)
	{
		if (fsok)
		{
			HandleTransactions();
		}
	}
	else
	{
		HandleInitState();
	}
}

void TFileSystem::HandleTransactions()
{
	// should be overridden
}

void TFileSystem::HandleInitState()
{
	// should be overridden
}
