#pragma once

#include <wrl.h>
namespace wrl = Microsoft::WRL::Wrappers;

class fileview
{
	BYTE const * m_view;
	LARGE_INTEGER m_size;
	typedef wrl::HandleT<wrl::HandleTraits::HANDLENullTraits> MapHandle;

	fileview( fileview const & );
	fileview & operator=(fileview const &);
public:

	fileview( char const * name ) throw() :
		m_view(),
		m_size()
	{
		wrl::FileHandle const file( CreateFileA( name,
			GENERIC_READ,
			FILE_SHARE_READ,
			nullptr, // default security
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr ) ); // no template

		if ( !file.IsValid() ) return;
		GetFileSizeEx( file.Get(), &m_size );
		if ( m_size.QuadPart == 0 ) return;

		MapHandle const map( CreateFileMapping( file.Get(),
			nullptr, // default security
			PAGE_READONLY,
			0, 0, // match file size
			nullptr ) ); // no name

		m_view = static_cast<BYTE const *>(MapViewOfFile( map.Get(),
			FILE_MAP_READ,
			0, 0, // offset
			0 )); // match file size
	}

	~fileview() throw()
	{
		if ( valid() )
		{
			UnmapViewOfFile( m_view );
		}
	}

	bool valid() const throw() // If !valid() call GetLastError for reason
	{
		return nullptr != m_view;
	}

	BYTE const * begin() const throw()
	{
		return m_view;
	}

	BYTE const * end() const throw()
	{
		return begin() + m_size.LowPart;
	}

	size_t size() const throw()
	{
		return m_size.LowPart;
	}
};
