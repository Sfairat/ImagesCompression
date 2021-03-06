#pragma once

#include <string>

#define METACLASS_NAME(CurrentName) CurrentName##MetaClass

#define START_METACLASS_DECLARATION(CurrentName, SuperName) \
public: \
class METACLASS_NAME(CurrentName) : public METACLASS_NAME(SuperName) \
{\
	static const std::wstring _className; \
public: \
	const virtual std::wstring* className() const { return &_className; }; 

#define DEFINE_METACLASS_DEFAULT_ALLOC(CurrentName) \
	virtual CurrentName* alloc() const { return new CurrentName; } 

#define DEFINE_METACLASS_ABSTRACT_ALLOC(CurrentName) \
	virtual CurrentName* alloc() const { return NULL; } 

#define END_METACLASS_DECLARATION(CurrentName, SuperName) \
}; \
static METACLASS_NAME(CurrentName) metaObject;\
friend class METACLASS_NAME(CurrentName); \
protected:\
CurrentName() : SuperName() {  };\
public:\
static CurrentName* alloc() { return static_cast<CurrentName*>(CurrentName::metaObject.alloc()); }; \
virtual const METACLASS_NAME(CurrentName)* metaClass() const { return static_cast<METACLASS_NAME(CurrentName)*>(&metaObject); };\
static const METACLASS_NAME(CurrentName)* metaClassStatic() { return static_cast<METACLASS_NAME(CurrentName)*>(&metaObject); };\
private:

#define SF_WSTRING_VALUE(arg) L#arg

#define DEFINE_OBJECT_IMPL(CurrentName) \
	const std::wstring CurrentName::METACLASS_NAME(CurrentName)::_className = SF_WSTRING_VALUE(CurrentName); \
CurrentName::METACLASS_NAME(CurrentName) CurrentName::metaObject; \

#define DEFAULT_METACLASS_DECLARATION(CurrentName, SuperName) \
	START_METACLASS_DECLARATION(CurrentName, SuperName) \
	DEFINE_METACLASS_DEFAULT_ALLOC(CurrentName) \
	END_METACLASS_DECLARATION(CurrentName, SuperName) \

#define ABSTRACT_METACLASS_DECLARATION(CurrentName, SuperName) \
	START_METACLASS_DECLARATION(CurrentName, SuperName) \
	END_METACLASS_DECLARATION(CurrentName, SuperName) \


#include <basetsd.h>

class SFObject
{
public:
	class METACLASS_NAME(SFObject) 
	{
		static const std::wstring _className;
	public:
		const virtual std::wstring* className() const { return &_className; }; 
		virtual SFObject* alloc() const { return new SFObject; } 
	};

	static METACLASS_NAME(SFObject) metaObject;
	friend class METACLASS_NAME(SFObject);

protected:
	SFObject() { _retainCount = 1; };

public:

	virtual const METACLASS_NAME(SFObject)* metaClass() const
	{ 
		return static_cast<METACLASS_NAME(SFObject)*>(&metaObject); 
	}

	static SFObject* alloc() 
	{ 
		return SFObject::metaObject.alloc();
	};

private:

	unsigned int _retainCount;

protected:

	virtual void free();
	virtual ~SFObject(void);

public:

	virtual SFObject* init();

	SFObject* retain();
	void release();

	virtual SFObject* copy();

	virtual int getRetainCount() const { return _retainCount; };
};

