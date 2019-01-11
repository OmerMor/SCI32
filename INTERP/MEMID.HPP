// memid.hpp

#ifndef MEMID_HPP
#define MEMID_HPP

#include <string.h>

#ifndef MEMMGR_HPP
#include "memmgr.hpp"
#endif

struct MemID {
	MemID() : handle(0)
	{
	}

	MemID(const MemID& id) : handle(id.handle)
	{
	}
	
	MemID(SOL_Handle h) : handle(h)
	{
	}

	MemAttrs	Attrs() const;
	void		Clear() const;
	void		Critical(Bool = True) const;
	void		Fill(uchar) const;

	void Free()
	{
		if (handle) {
			memMgr->Free(handle);
			handle = 0;
		}
	}

	void Get(MemType type, size_t size, MemAttrs a = MOVEABLE,
				unsigned num = 0, MemClient* client = 0, char initChar = 0,
				ReservedHandle h = IllegalHandle)
	{
		handle = memMgr->Alloc(type | a, size, num, client, initChar, h);
	}

	void Get(MemType type, size_t size, ReservedHandle h, MemAttrs a = 0)
	{
		Get(type, size, a | PERSIST, 0, 0, 0, h);
	}

	void Get(MemType type, MemID id, size_t ofs, size_t size,
				MemAttrs a = MOVEABLE)
	{
		Get(type, size, a);
		memcpy(**this, &id[ofs], size);
	}
	
	MemType			GetMemType() const;
	const char*		GetMemTypeStr() const;

	Bool IsValid() const
	{
		return memMgr->IsValid(handle);
	}
	
	void*		Lock() const;
	void*		Unlock() const;
	void		SetDiscardable() const;
	void		SetNotDiscardable() const;
	void*		Realloc(size_t) const;
	int		Read(int fd) const;
	int		Read(int fd, size_t ofs, size_t size) const;
	int		Load(MemType, char* fileName, MemAttrs = MOVEABLE, ReservedHandle
					h = IllegalHandle);
	size_t	Size() const;
#ifdef DEBUG
	void		SetChecksum(Bool = True) const;
#endif

	operator SOL_Handle() const
	{
		return handle;
	}

	void* operator*() const
	{
		//return memMgr->GetAddr(handle);
		return memMgr->desc[handle].data;
	}

	char& operator[](size_t s) const
	{
		//return *((char*) memMgr->GetAddr(handle) + s);
		return *((char*) memMgr->desc[handle].data + s);
	}
		
	Bool operator==(const MemID& id) const
	{
		return handle == id.handle;
	}

	Bool operator !=(const MemID& id) const
	{
		return !(*this == id);
	}

	Bool operator !() const
	{
		return handle == 0;
	}

protected:
	SOL_Handle	handle;
};

template <class T, MemType type>
struct ID : MemID {
	ID() {}
	ID(const MemID& id) : MemID(id) {}
	ID(SOL_Handle h) : MemID(h) {}

	void
	Get()
	{
		MemID::Get(type, sizeof(T));
		new(handle) T;
	}

	void
	Get(size_t size, MemAttrs attrs = MOVEABLE, unsigned num = 0,
			MemClient* client = 0, char initChar = 0,
			ReservedHandle h = IllegalHandle)
	{
		MemID::Get(type, size, attrs, num, client, initChar, h);
	}

	void
	Get(size_t size, ReservedHandle h, MemAttrs attrs = 0)
	{
		MemID::Get(type, size, attrs | PERSIST, 0, 0, 0, h);
	}

	void
	Get(MemID id, size_t ofs, size_t size)
	{
		MemID::Get(type, id, ofs, size);
	}

	T*
	Lock() const
	{
		return (T*) MemID::Lock();
	}

	T*
	Unlock() const
	{
		return (T*) MemID::Unlock();
	}

	void
	Free()
	{
		if (handle) {
#if 0
			//	WATCOM 9.5 and 9.5a won't allow this:  gives error 252
			((T*) memMgr->GetAddr(handle))->~T();
#endif
			MemID::Free();
		}
	}
	
	T*
	Realloc(size_t size) const
	{
		return (T*) MemID::Realloc(size);
	}

	T* operator *() const	{ return (T*) MemID::operator*(); }
	T*	operator->() const	{ return (T*) MemID::operator*(); }
};

inline void*
operator new(size_t, SOL_Handle handle)
{
	//return memMgr->GetAddr(handle);
	return (char*) memMgr->desc[handle].data;
}

#endif

