#ifndef SLMATH_VECTOR_SSE_H
#define SLMATH_VECTOR_SSE_H

#include <slm/slmath_pp.h>
#include <stdlib.h> // for malloc and free
#include <string.h> // for memcpy

SLMATH_BEGIN()

/**
 * Very minimal but very efficient std::vector clone for plain data SSE contents like vec4.
 * This is useful for using SSE support on 32-bit Visual Studio builds,
 * which suffer from the memory allocation alignment problem (std::vector memory not aligned to vec4).
 * NOTE: vector_sse does NOT call constructors/destructors of the contained elements correctly,
 * so it is NOT suitable to store anything else than "plain old datastructures" to these.
 * If more correct semantics of constructors/destructors are needed then std::vector should be used.
 * For documentation of the methods, please see std::vector.
 *
 * @ingroup vec_util
 */
template <class T> class vector_sse
{
public:
	/** Constructs empty vector. */
	vector_sse()								: m_buf(0), m_actualBuf(0), m_cap(0), m_size(0) {reallocate(8);}

	~vector_sse()								{resize(0); free(m_actualBuf);}

	// Modifiers

	/** Returns ith item */
	T&			operator[]( size_t i )			{assert(i<m_size); return m_buf[i];}
	/** Returns the first item */
	T&			front()							{assert(m_size); return m_buf[0];}
	/** Returns the last item */
	T&			back()							{assert(m_size); return m_buf[m_size-1];}
	/** Returns pointer to the first item */
	T*			begin()							{return m_buf;}
	/** Returns pointer to one beyond the last item */
	T*			end()							{return m_buf+m_size;}
	/** Resizes vector +1 and sets the last element to specified value */
	void		push_back( const T& e )			{if(m_size==m_cap) reallocate(m_cap*2); m_buf[m_size++] = e;}
	/** Resizes vector -1 */
	void		pop_back()						{assert(m_size); --m_size;}
	/** Resizes vector to n elements */
	void		resize( size_t n )				{if (n>m_cap) {if (n<m_cap*2) n=m_cap*2; reallocate(n);} m_size=n;}

	// Inspectors

	/** Returns ith item */
	const T&	operator[]( size_t i ) const	{assert(i<m_size); return m_buf[i];}
	/** Returns the first item */
	const T&	front() const					{assert(m_size); return m_buf[0];}
	/** Returns the last item */
	const T&	back() const					{assert(m_size); return m_buf[m_size-1];}
	/** Returns pointer to the first item */
	const T*	begin() const					{return m_buf;}
	/** Returns pointer to one beyond the last item */
	const T*	end() const						{return m_buf+m_size;}
	/** Returns true if vector size is 0 */
	bool		empty() const					{return m_size==0;}
	/** Returns size */
	size_t		size() const					{return m_size;}

private:
	T*			m_buf;
	char*		m_actualBuf;
	size_t		m_cap;
	size_t		m_size;

	void reallocate( size_t n )
	{
		assert(n>m_size);
		T* obuf = m_buf;
		char* oactual = m_actualBuf;
		m_actualBuf = (char*)malloc(sizeof(T)*(m_cap = n) + 8);
		// this is hardcoded to the assumption that the base allocator returns 8 byte aligned memory, and makes it 16 byte aligned
		// this is more efficient than the generic aligned allocators which have to store additional information in the block etc
		assert( m_actualBuf && ((size_t)m_actualBuf & 0x7) == 0 );
		m_buf = (T*)((size_t)m_actualBuf & 8 ? m_actualBuf + 8 : m_actualBuf);
		if (m_size) memcpy( m_buf, obuf, m_size*sizeof(T) );
		if (oactual) free(oactual);
	}
};

SLMATH_END()

#endif // SLMATH_VECTOR_SSE_H
