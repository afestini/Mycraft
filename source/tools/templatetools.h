/*
 * tools.h
 *
 *  Created on: 28.10.2008
 *      Author: festini
 */

#ifndef TEMPLATE_TOOLS_H_
#define TEMPLATE_TOOLS_H_

#include <sstream>
/*
 * Cast anything into everything via string(stream)
 * Intended for casts to and from string
 */
template <typename T, typename S>
inline T lex_cast(const S& x)
{
	std::stringstream tmp;
	T res;
	return tmp << x, tmp >> res, res;
}

/*
 * Replace all occurrences of a substring with a new string
 */
inline std::string& replace( std::string& org, const std::string& pattern, const std::string& txt )
{
	size_t l = txt.length();
	for ( size_t pos = 0; ( pos = org.find(pattern, pos) ) != std::string::npos; pos += l )
		org.replace(pos, l, txt);
	return org;
}

/*
 * Clamps a value to a specified range
 */
#undef min
#undef max

/*
 * Remap value from one range to another
 */
template<typename T>
inline T mapToRange( T val, T oldMin, T oldMax, T newMin, T newMax )
{
	return newMin + ((val - oldMin) * (newMax - newMin)) / (oldMax - oldMin);
}

#endif /* TEMPLATE_TOOLS_H_ */
