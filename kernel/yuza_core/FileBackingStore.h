#pragma once
#include <BackingStore.h>

class FileBackingStore : public BackingStore
{
public:
	FileBackingStore(HANDLE fileDesc);
	virtual ~FileBackingStore();

	virtual bool HasPage(off_t) override;
	virtual int Read(off_t, void*, int) override;
	virtual int Write(off_t, const void*) override;
	virtual off_t Commit(off_t size) override;

private:
	HANDLE m_fileDesc;
};

