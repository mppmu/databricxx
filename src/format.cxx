// Copyright (C) 2014 Oliver Schulz <oschulz@mpp.mpg.de>

// This is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include "format.h"

#include <iostream>
#include <cstdio>
#include <stdarg.h>


using namespace std;


namespace dbrx {




bool FormatString::isFormatFlag(char c) {
	switch(c) {
		case '-': case '+': case ' ': case '#': case '0': return true;
		default: return false;
	}
}


FormatString::ConstCharRange FormatString::findFmtElem(FormatString::ConstCharRange str) {
	static const char* invFormStrMsg = "Invalid format string";

	const char *c = str.begin();
	const char *until = str.end();
	while (c < until) {
		if (*c == '%') {
			const char *fmtBegin = c;
			++c;
			if (c == until) throw invalid_argument(invFormStrMsg);
			if (*c != '%') {
				// flags
				while (isFormatFlag(*c) && (c < until)) ++c;
				if (c == until) throw invalid_argument(invFormStrMsg);

				// width
				if (*c == '*') throw invalid_argument("String formatting argument width specification via \"*\" not supported.");
				else while (('0' <= *c) && (*c <= '9') && (c < until)) ++c;
				if (c == until) throw invalid_argument(invFormStrMsg);

				// precision
				if (*c == '.') {
					++c;
					if (c == until) throw invalid_argument(invFormStrMsg);
					while (('0' <= *c) && (*c <= '9') && (c < until)) ++c;
					if (c == until) throw invalid_argument(invFormStrMsg);
				}

				// length
				switch(*c) {
					case 'j': case 'z': case 't': case 'L':
						++c;
						break;
					case 'h': case 'l': {
						char last = *c;
						++c;
						if ((c < until) && (*c == last)) ++c;
						break;
					}
				}
				if (c == until) throw invalid_argument(invFormStrMsg);

				// specifier
				switch(*c) {
					case 'd': case 'i': case 'u': case 'o': case 'x': case 'X': case 'f': case 'F': case 'e':
					case 'E': case 'g': case 'G': case 'a': case 'A': case 'c': case 's': case 'p': case 'n':
						++c; break;
					default: throw invalid_argument(invFormStrMsg);
				}

				ConstCharRange result{fmtBegin, c};
				return {fmtBegin, c};
			} else ++c;
		} else ++c;
	}
	return {until, until};
}


void FormatString::printFormatted(std::ostream& os, ConstCharRange fmt, ...) {
	const size_t fmtMaxSize = 32;
	char fmtCStr[fmtMaxSize] = {};
	const size_t outMaxSize = 32;
	char outCStr[fmtMaxSize] = {};

	if (fmt.size() + 1 > fmtMaxSize) throw("String format element too long");
	for (size_t i = 0; i < fmt.size(); ++i) fmtCStr[i] = fmt[i];

	va_list argp;
	va_start(argp, fmt);
	int nOut = vsnprintf (outCStr, outMaxSize, fmtCStr, argp);
	va_end(argp);

	if (nOut < 0) throw runtime_error("Error during string formatting using vsnprintf");
	else for (int i = 0; i < nOut; ++i) os << outCStr[i];
}


} // namespace dbrx
