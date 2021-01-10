/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM Tests project: https://github.com/nvitya/nvcmtests
 * Copyright (c) 2020 Viktor Nagy, nvitya
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
 *  file:     filesystem.h
 *  brief:    File System Base Class
 *  version:  1.00
 *  date:     2020-12-29
 *  authors:  nvitya
*/

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "stdint.h"
#include "filesystem_types.h"
#include "stormanager.h"

#define FSRESULT_OK               0
#define FSRESULT_EOF              1  // end of file, end of find
#define FSRESULT_NOTIMPL          2
#define FSRESULT_IOERROR          3
#define FSRESULT_INVALID_PATH     4
#define FSRESULT_INVALID_NAME     5
#define FSRESULT_FILE_NOT_FOUND   6
#define FSRESULT_FILE_NOT_OPEN    7

#define FS_FNAME_MAX_LEN    64
#define FS_PATH_MAX_LEN    128
#define FS_MAX_TDATA       256

#define FS_INVALID_ADDR    0x0FFFFFFFFFFF0000ull

#define FSATTR_DIR        0x0001  // directory instead of normal file
#define FSATTR_VOLLABEL   0x0100  // volume label

#define FSATTR_NONFILE  (FSATTR_DIR | FSATTR_VOLLABEL)

struct TFileDateTime  // temporary, not fully specified
{
	uint32_t    fdate;
	uint32_t    ftime;
};

struct TFileDirData
{
	uint64_t        size;
	uint64_t        location;     // first data byte location
	uint64_t        dirlocation;  // directory entry location
	uint32_t        attributes;
	TFileDateTime   create_time;
	TFileDateTime   modif_time;
	char            name[FS_FNAME_MAX_LEN];
};

enum TFsOpType
{
	FSOP_IDLE = 0,
	FSOP_DIR_READ = 1,
};

enum TFsTraType
{
	FSTRA_DIR_READ,

	FSTRA_FILE_OPEN,
	FSTRA_FILE_READ,
	FSTRA_FILE_SEEK,

#if 0
	FSTRA_FOPEN,
	FSTRA_DELETE,

	FSTRA_FCLOSE,
	FSTRA_FREAD,
	FSTRA_FWRITE,
	FSTRA_FSEEK,
#endif
};

typedef void (* PFsCbFunc)(void * arg);

struct TFsTrans
{
	TFsTrans *        next;
	bool              completed;
	TFsTraType        ttype;
	uint8_t           state;
	uint8_t           phase;
	int               result;

	PFsCbFunc         callback = nullptr;
	void *            callbackarg = nullptr;
};

struct TFsTransDir
{
	TFsTrans         fstra;  // must be the first!
	TFileDirData     fdata;
	char             pattern[FS_FNAME_MAX_LEN];
	uint64_t         curlocation;
	uint64_t         cluster_end;
};

struct TFsTransFile
{
	TFsTrans         fstra;  // must be the first!
	char             path[FS_PATH_MAX_LEN];
	bool             opened;
	TFileDirData     fdata;

	uint8_t *        dataptr;
	uint32_t         remaining;
	uint32_t         transferred;

	uint64_t         filepos;
	uint64_t         curlocation;
	uint64_t         cluster_end;
};

class TFileSystem
{
public:
	bool             initialized = false;
	bool             fsok = false;

	int              initstate = 0;

	TFsOpType        curop = FSOP_IDLE;
	int              opstate = 0;
	int              opresult = 0;

	int              trastate = 0;

	uint8_t          clustersizeshift = 0;

	TStorManager *   pstorman = nullptr;
	uint64_t         firstaddr = 0;
	uint64_t         maxsize = 0;

	uint64_t         sector_base_mask = 0xFFFFFFFFFFFFFE00;  // gives the sector base address
	uint64_t         rootdirstart = 0;
	uint64_t         cluster_base_mask = 0xFFFFFFFFFFFFFE00;
	uint32_t         clusterbytes = 0;


	TFsTrans *       curtra = nullptr;
	TFsTrans *       lasttra = nullptr;

	uint64_t         op_location = 0;
	uint64_t         op_cluster_end = 0;

	TStorTrans       stra;
	TFileDirData     fdata;

	virtual ~TFileSystem() { }

	void             Init(TStorManager * astorman, uint64_t afirstaddr, uint64_t amaxsize);

	void             DirReadInit(TFsTransDir * atra, uint64_t adirstart, const char * apattern);  // completes instantly.
	void             DirReadExec(TFsTransDir * atra);

	void             FileOpen(TFsTransFile * atra, const char * aname);
	void             FileRead(TFsTransFile * atra, void * dst, uint32_t len);

	void             Run();
	void             HandleDirRead();
	void             HandleFileOpen();

public:
	virtual void     RunOpDirRead();
	virtual void     HandleInitState();
	virtual void     HandleFileRead();

protected:

	char             path[FS_PATH_MAX_LEN];
	char *           pseg_start;
	char *           pseg_end;
	int              pseg_len;
	uint64_t         dir_location;
	uint64_t         dir_cluster_end;

	uint32_t         chunksize;

	void             FinishCurOp(int aresult);
	bool             StartOpDirRead(uint64_t alocation, uint64_t acluster_end);  // returns true when not finished yet

	void             AddTransaction(TFsTrans * atra, TFsTraType atype);

	void             ExecCallback(TFsTrans * atra);

	void             FinishCurTra(int aresult);

};

#endif /* FILESYSTEM_H_ */
