/*
 * fileman.h
 *
 *  Created on: 16 Jan 2021
 *      Author: vitya
 */

#ifndef FILEMAN_H_
#define FILEMAN_H_

#include "filesystem.h"

class TFileManager
{
public:
	virtual          ~TFileManager() { }
	void             Init();

	bool             AddFileSystem(TFileSystem * afilesystem);

	TFile *          NewFileObj(const char * adrivepath, void * astorage, unsigned astoragesize);
	void             ReleaseFileObj(TFile * afile);

	TFileSystem *    FileSystemFromPath(const char * adrivepath);  // understands C:, d: etc.

	void             Run();

protected:
	unsigned         fscount = 0;
	TFileSystem *    fslist[FILESYS_MAX_FSYS];
};

extern TFileManager fileman;

#endif /* FILEMAN_H_ */
