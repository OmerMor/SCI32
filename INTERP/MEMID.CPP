// memid.cpp

#include "sol.hpp"

#include "dos.hpp"
#include "memid.hpp"

MemAttrs
MemID::Attrs() const
{
	return memMgr->GetMemAttrs(*this);
}

MemType
MemID::GetMemType() const
{
	return memMgr->GetMemType(*this);
}

const char*
MemID::GetMemTypeStr() const
{
	return memMgr->GetMemTypeStr(*this);
}

void
MemID::Clear() const
{
	Fill(0);
}

void
MemID::Critical(Bool on) const
{
	memMgr->Critical(handle, on);
}

void
MemID::Fill(uchar c) const
{
	memset(**this, c, (size_t) memMgr->GetSize(*this));
}

void*
MemID::Lock() const
{
	return memMgr->Lock(*this);
}

void*
MemID::Unlock() const
{
	return memMgr->Unlock(*this);
}

void
MemID::SetDiscardable() const
{
	memMgr->SetDiscardable(*this);
}

void
MemID::SetNotDiscardable() const
{
	memMgr->SetNotDiscardable(*this);
}

void*
MemID::Realloc(size_t size) const
{
	memMgr->Realloc(*this, size);
	return **this;
}

int
MemID::Read(int fd) const
{
	return ReadMemID(fd, *this, Size());
}

int
MemID::Read(int fd, size_t ofs, size_t size) const
{
	return ::Read(fd, &(*this)[ofs], size);
}

int
MemID::Load(MemType type, char* fileName, MemAttrs attrs, ReservedHandle h)
{
	int fd;

	if ((fd = Open(fileName, O_RDONLY | O_BINARY)) == -1)
		return 0;

	int size = FileLength(fd);
	Get(type, size, h, attrs);

	int rc = Read(fd);
	if (rc != size) {
		Free();
		rc = 0;
	}
	
	Close(fd);
	
	return rc;
}

size_t
MemID::Size() const
{
	return handle ? (size_t) memMgr->GetSize(*this) : 0;
}

#ifdef DEBUG
void
MemID::SetChecksum(Bool onoff) const
{
	memMgr->SetChecksum(*this, (int)onoff);
}
#endif



