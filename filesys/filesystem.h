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

#ifndef FILESYS_MAX_FSYS
  #define FILESYS_MAX_FSYS  4
#endif

#define FOPEN_CREATE              1
#define FOPEN_DIRECTORY           8

#define FSRESULT_OK               0
#define FSRESULT_EOF              1  // end of file, end of find
#define FSRESULT_NOTIMPL          2
#define FSRESULT_IOERROR          3
#define FSRESULT_INVALID_PATH     4
#define FSRESULT_INVALID_NAME     5
#define FSRESULT_FILE_NOT_FOUND   6
#define FSRESULT_FILE_NOT_OPEN    7
#define FSRESULT_DIR_NOT_FOUND    8
#define FSRESULT_INVALID_FDATABUF 9  // data buffer for fdata entry (directory read)

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
};


class TFsTransaction;
class TFileSystem;
class TFile;

typedef void (* PFsCbFunc)(TFile * afile, void * arg);

class TFsTransaction
{
	friend class TFileSystem;

public:
	bool              finished = true;
	int               result = 0;
	TFsTraType        tratype;

	uint8_t *         dataptr = nullptr;
	uint32_t          datalen = 0;         // read: dst buffer size, write: data size
	uint32_t          transferlen = 0;     // read: bytes written into the dst buffer

	PFsCbFunc         callback = nullptr;
	void *            callbackarg = nullptr;

protected:
	TFsTransaction *  nexttra = nullptr;
	TFileSystem *     filesys = nullptr;
};

class TFile : public TFsTransaction
{
public:
	bool             opened = false;
	bool             allocated_on_heap = false;
	bool             directory = false;  // false = normal file, true = direcotry mode
	uint32_t         open_flags = 0;

	char             path[FS_PATH_MAX_LEN];

public:
	TFileDirData     fdata;
	uint64_t         filepos = 0;
	uint64_t         curlocation = 0;
	uint64_t         cluster_end = 0;

	uint32_t         remaining = 0;

public:
	                 TFile(TFileSystem * afilesys);
	virtual          ~TFile() { }

	void             Open(const char * aname, uint32_t aflags);
	void             Read(void * dst, uint32_t len);

	int              WaitComplete(); // returns the result

public:
	void             FinishTra(int aresult);
};

class TFileSystem
{
	friend class TFile;

public:
	uint8_t          fsidx = 0;
	bool             initialized = false;
	bool             fsok = false;

	TStorManager *   pstorman = nullptr;
	uint64_t         firstaddr = 0;
	uint64_t         maxsize = 0;

public:
	uint32_t         clusterbytes = 0;
	uint64_t         rootdirstart = 0;
	uint8_t          clustersizeshift = 0;

public:
	virtual          ~TFileSystem() { }

	void             Init(TStorManager * astorman, uint64_t afirstaddr, uint64_t amaxsize);

	void             Run();

public:
	virtual TFile *  NewFileObj(void * astorage, unsigned astoragesize);
	virtual void     HandleInitState();
	virtual void     RunOpDirRead();
	virtual void     HandleFileRead();

protected:

	void             HandleFileOpen();
	void             HandleDirRead();

protected:
	int              initstate = 0;

	TFsOpType        curop = FSOP_IDLE;
	int              opstate = 0;
	int              opresult = 0;

	int              trastate = 0;


	uint64_t         sector_base_mask = 0xFFFFFFFFFFFFFE00;  // gives the sector base address
	uint64_t         cluster_base_mask = 0xFFFFFFFFFFFFFE00;


	TFile *          curtra = nullptr;
	TFile *          lasttra = nullptr;

	uint64_t         op_location = 0;
	uint64_t         op_cluster_end = 0;

	TStorTrans       stra;
	TFileDirData     fdata;

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

	void             AddTransaction(TFile * afile, TFsTraType atype);

	void             FinishCurTra(int aresult);
};

#endif /* FILESYSTEM_H_ */
