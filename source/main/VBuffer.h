#ifndef VBUFFER
#define VBUFFER

#include "JSFramework.h"
#include "Console.h"

enum BUFFER_TYPE {VERTEX_BUFFER=0, INDEX_BUFFER};

class Buffer {
private:
	unsigned ID;
	int GLType;
	int Type;

	static const Buffer* Bound[2];

public:
	inline bool Initialized() const { return ID != 0; }
	inline void Bind() const
	{
		glBindBufferARB(GLType, (Bound[Type]=this)->ID);
	}
	void* Map() 
	{
		Bind();		
		return glMapBufferARB(GLType, GL_WRITE_ONLY_ARB);
	}
	void Unmap() 
	{
		Bind();
		glUnmapBufferARB(GLType);
	}
	static void Unbind(BUFFER_TYPE buf)
	{
		glBindBufferARB(0x8892+buf, 0);
	}
	
	void* Address(unsigned Offset) 
	{
		Bind();
		return (void*)Offset;
	}

	void init()
	{
		glGenBuffersARB(1, &ID);		
	}

	void setData(void* data, size_t size)
	{
		Bind();
		glBufferDataARB(GLType, 0, 0, GL_STATIC_DRAW_ARB);
		glBufferDataARB(GLType, size, data, GL_STATIC_DRAW_ARB);
	}

	Buffer() : ID(0), GLType(0x8892+VERTEX_BUFFER), Type(VERTEX_BUFFER) 
	{
	}

	Buffer(BUFFER_TYPE type, unsigned) : ID(0), GLType(0x8892+type), Type(type) 
	{
		init();
		setData(0, 0);
	}

	~Buffer() 
	{
		if (ID) glDeleteBuffersARB(1, &ID);
	}
};

#endif
