#ifndef PROPERTY_H_
#define PROPERTY_H_

template<typename T>
class Property
{
protected:
	T value;
public:
	explicit Property(const T& val) : value(val) {}

	inline const T& operator()() const { return value; }
	inline const T& operator()(const T& newVal) { return value=newVal; }
	inline Property<T>& operator=(const T& newVal) { return value = newVal, *this; }
};

#endif
