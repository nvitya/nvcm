/*
 * fileman.cpp
 *
 *  Created on: 16 Jan 2021
 *      Author: vitya
 */

#include <fileman.h>

TFileManager fileman;

void TFileManager::Init()
{
	fscount = 0;
}

bool TFileManager::AddFileSystem(TFileSystem * afilesystem)
{
	if (fscount >= FILESYS_MAX_FSYS)
	{
		return false;
	}

	fslist[fscount] = afilesystem;
	afilesystem->fsidx = fscount;

	++fscount;

	return true;
}

void TFileManager::Run()
{
	for (unsigned fsidx = 0; fsidx < fscount; ++fsidx)
	{
		fslist[fsidx]->Run();
	}
}

TFile * TFileManager::NewFileObj(const char * adrivepath, void * astorage, unsigned astoragesize)
{
	TFileSystem * fsys = FileSystemFromPath(adrivepath);
	if (!fsys)
	{
		return nullptr;
	}

	return fsys->NewFileObj(astorage, astoragesize);
}

void TFileManager::ReleaseFileObj(TFile * afile)
{
	if (afile)
	{
		if (afile->allocated_on_heap)
		{
			afile->allocated_on_heap = false; // prevents also double free
			delete afile;
		}
	}
}

TFileSystem * TFileManager::FileSystemFromPath(const char * adrivepath)
{
	if (adrivepath)
	{
		char c = *adrivepath;
		if ((0 != c) && ('/' != c))  // drive letter present ?
		{
			if ((c >= 'a') && (c <= 'z'))  c = c - ('a'-'A');  // convert to uppercase
			if ((c >= 'C') && (c <= 'Z'))
			{
				unsigned didx = c - 'C';
				if (didx < FILESYS_MAX_FSYS)
				{
					return fslist[didx];
				}
			}

			return nullptr;  // invalid file system requested
		}
	}

	return fslist[0];  // return the first filesystem
}
