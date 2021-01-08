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

	if (!stra.completed)
	{
		return;
	}

	if (fsok)
	{
		// handle transactions
		if (curtra)
		{
			if (stra.errorcode)
			{
				stra.errorcode = 0;
				FinishCurTra(FSRESULT_IOERROR);
			}
			else if (FSTRA_DIR_READ == curtra->ttype)
			{
				HandleDirRead();
			}
			else
			{
				// unknown transaction
				FinishCurTra(FSRESULT_NOTIMPL);
			}
		}
	}
	else
	{
		if (!initialized)
		{
			HandleInitState();
		}
	}
}

void TFileSystem::HandleInitState()
{
	// should be overridden
}

void TFileSystem::DirReadInit(TFsTransDir * atra, uint64_t adirstart, const char * apattern)
{
	atra->curlocation = adirstart;
	atra->cluster_end = (adirstart & cluster_base_mask) + clusterbytes;

	strncpy(&atra->pattern[0], apattern, sizeof(atra->pattern));
	atra->fstra.completed = true;
	atra->fstra.state = 0;
	atra->fstra.phase = 0;
}

void TFileSystem::DirReadExec(TFsTransDir * atra)
{
	AddTransaction(&atra->fstra, FSTRA_DIR_READ);
}

void TFileSystem::AddTransaction(TFsTrans * atra, TFsTraType atype)
{
	atra->next = nullptr;
	atra->completed = false;
	atra->result = 0;
	atra->ttype = atype;

	if (lasttra)
	{
		lasttra->next = atra;
		lasttra = atra;
	}
	else
	{
		// set as first
		curtra = atra;
		lasttra = atra;
	}
}

void TFileSystem::ExecCallback(TFsTrans * atra)
{
	if (atra->callback)
	{
		(* (atra->callback))(atra->callbackarg);
	}
}

void TFileSystem::FinishCurTra(int aresult)
{
	// request completed
	// the callback function might add the same transaction object as new
	// therefore we have to remove the transaction from the chain before we call the callback

	TFsTrans * ptra = curtra;

	curtra->result = aresult;
	curtra->completed = true;
	curtra = curtra->next; // advance to the next transaction
	if (!curtra)  lasttra = nullptr;

	state = 0;

	ExecCallback(ptra);
}

void TFileSystem::HandleDirRead()  // must be overridden
{
	FinishCurTra(FSRESULT_NOTIMPL);
}
