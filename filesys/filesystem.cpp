/*
 * filesystem.cpp
 *
 *  Created on: 29 Dec 2020
 *      Author: vitya
 */

#include "string.h"
#include <filesystem.h>
#include "traces.h"

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

	if (curop != FSOP_IDLE)
	{
		if (stra.errorcode)
		{
			stra.errorcode = 0;
			opresult = FSRESULT_IOERROR;
			curop = FSOP_IDLE;
		}
		else if (FSOP_DIR_READ == curop)
		{
			RunOpDirRead();
		}

		if (curop != FSOP_IDLE)
		{
			return;
		}
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
				return;
			}

			if (FSTRA_FILE_READ == curtra->ttype)
			{
				HandleFileRead();
			}
			else if (FSTRA_DIR_READ == curtra->ttype)
			{
				HandleDirRead();
			}
			else if (FSTRA_FILE_OPEN == curtra->ttype)
			{
				HandleFileOpen();
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

void TFileSystem::RunOpDirRead()
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

void TFileSystem::FileOpen(TFsTransFile * atra, const char * aname)
{
	strncpy(atra->path, aname, sizeof(atra->path));
	atra->opened = false;

	AddTransaction(&atra->fstra, FSTRA_FILE_OPEN);
}

void TFileSystem::FileRead(TFsTransFile * atra, void * dst, uint32_t len)
{
	atra->dataptr = (uint8_t *)dst;
	atra->remaining = len;
	atra->transferred = 0;

	if (!atra->opened)
	{
		// do not even add to the transactions
		atra->fstra.result = FSRESULT_FILE_NOT_OPEN;
		atra->fstra.completed = true;
		ExecCallback(&atra->fstra);
		return;
	}

	// check file end
	if (atra->filepos + atra->remaining > atra->fdata.size)
	{
		if (atra->filepos > atra->fdata.size)  // this should not happen !
		{
			atra->filepos = atra->fdata.size;
		}

		atra->remaining = (atra->fdata.size - atra->filepos);
	}

	AddTransaction(&atra->fstra, FSTRA_FILE_READ);
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

	trastate = 0;

	ExecCallback(ptra);
}

void TFileSystem::FinishCurOp(int aresult)
{
	opresult = aresult;
	curop = FSOP_IDLE;
}

bool TFileSystem::StartOpDirRead(uint64_t alocation, uint64_t acluster_end)
{
	op_location = alocation;
	op_cluster_end = acluster_end;

	curop = FSOP_DIR_READ;
	opstate = 0;

	RunOpDirRead();

	return (curop != FSOP_IDLE);
}

void TFileSystem::HandleDirRead()  // must be overridden
{
	// called only when curop == FSOP_IDLE

	TFsTransDir * tradir = (TFsTransDir *)curtra;

	if (0 == trastate)
	{
		trastate = 1;
		if (StartOpDirRead(tradir->curlocation, tradir->cluster_end))
		{
			return; // not finished yet
		}
	}
	else // process the result
	{
		tradir->fdata = fdata; // copy the local fdata
		tradir->curlocation = op_location;
		tradir->cluster_end = op_cluster_end;

		FinishCurTra(opresult);
	}
}

void TFileSystem::HandleFileOpen()
{
	// called only when curop == FSOP_IDLE

	TFsTransFile * traf = (TFsTransFile *)curtra;

	if (0 == trastate)  // start path resolution
	{
		strncpy(path, traf->path, sizeof(path));

		pseg_start = path;
		dir_location = rootdirstart;
		dir_cluster_end = dir_location + clusterbytes;
		trastate = 1;
	}

	if (1 == trastate) // search next path segment
	{
		if ((*pseg_start == '/') || (*pseg_start == '\\'))  ++pseg_start;

		pseg_end = pseg_start;
		while ((*pseg_end != 0) && (*pseg_end != '/') && (*pseg_end != '\\'))
		{
			++pseg_end;
			if (pseg_end - pseg_start > FS_FNAME_MAX_LEN - 1)
			{
				FinishCurTra(FSRESULT_INVALID_PATH);
				return;
			}
		}

		pseg_len = pseg_end - pseg_start;
		if (pseg_len <= 0)
		{
			FinishCurTra(FSRESULT_INVALID_PATH);
			return;
		}

		trastate = 2; // continue at process directory entry in fdata
		if (StartOpDirRead(dir_location, dir_cluster_end))
		{
			return; // not finished yet
		}
	}

	// process directory entry in fdata

	while (true)
	{
		if (opresult)
		{
			if (opresult == FSRESULT_EOF)  opresult = FSRESULT_FILE_NOT_FOUND;
			FinishCurTra(opresult);
			return;
		}

		if (strncasecmp(pseg_start, fdata.name, pseg_len) == 0)
		{
      #if 1
			  char segname[FS_FNAME_MAX_LEN];
			  strncpy(segname, pseg_start, pseg_len);
			  segname[pseg_len] = 0; // ensure zero termination !
			  TRACE("Segment \"%s\" found.\r\n", &segname[0]);
      #endif

			// segment found

			if (0 == *pseg_end) // this is the last path segment, this should be a file
			{
				if (fdata.attributes & FSATTR_NONFILE)
				{
					FinishCurTra(FSRESULT_FILE_NOT_FOUND);
					return;
				}

				traf->fdata = fdata;
				traf->opened = true;
				traf->filepos = 0;
				traf->curlocation = fdata.location;
				traf->cluster_end = fdata.location + clusterbytes;
				FinishCurTra(0);
				return;
			}

			if (0 == (fdata.attributes & FSATTR_DIR))  // this must be directory
			{
				FinishCurTra(FSRESULT_FILE_NOT_FOUND);
				return;
			}

			dir_location = fdata.location;
			dir_cluster_end = dir_location + clusterbytes;

			// resolve the next segment
			pseg_start = pseg_end;
			trastate = 1;
			HandleFileOpen();
			return;
		}

		// else: continue the search

		dir_location = op_location;
		dir_cluster_end = op_cluster_end;
		if (StartOpDirRead(dir_location, dir_cluster_end))
		{
			return; // operation is still running (HW block read is in progress)
		}
	}
}

void TFileSystem::HandleFileRead() // must be overridden
{
	FinishCurTra(FSRESULT_NOTIMPL);
}

