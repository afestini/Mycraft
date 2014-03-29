/*
 * types.h
 *
 *  Created on: 19.08.2009
 *      Author: afestini
 */

#ifndef TYPES_BY_SIZE_H_
#define TYPES_BY_SIZE_H_

template<bool condition, class A, class B> struct SelectT { typedef A Type; };
template<class A, class B> struct SelectT<false, A, B> { typedef B Type; };

struct NullType;

template<typename A = NullType, typename B = NullType, typename C = NullType,
		 typename D = NullType, typename E = NullType, typename F = NullType>
struct TypeList
{
	typedef A Head;
	typedef TypeList<B,C,D,E,F> Tail;
};

template<size_t size, typename list>
struct TypeOfSize
{
	typedef typename SelectT
	< sizeof(typename list::Head)==size,
	  typename list::Head,
	  typename TypeOfSize<size, typename list::Tail>::type
	>::Type type;
};

template<size_t size> struct TypeOfSize<size, TypeList<> > { typedef NullType type; };



typedef TypeList<char, short, int, long, long long> SignedType;
typedef TypeList<unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long> UnsignedType;
typedef TypeList<float, double, long double> FloatingType;

typedef TypeOfSize<1, SignedType>::type int8;
typedef TypeOfSize<2, SignedType>::type int16;
typedef TypeOfSize<4, SignedType>::type int32;
typedef TypeOfSize<8, SignedType>::type int64;

typedef TypeOfSize<1, UnsignedType>::type uint8;
typedef TypeOfSize<2, UnsignedType>::type uint16;
typedef TypeOfSize<4, UnsignedType>::type uint32;
typedef TypeOfSize<8, UnsignedType>::type uint64;

typedef TypeOfSize<1, FloatingType>::type float8;
typedef TypeOfSize<2, FloatingType>::type float16;
typedef TypeOfSize<4, FloatingType>::type float32;
typedef TypeOfSize<8, FloatingType>::type float64;


#endif /* TYPES_H_ */
