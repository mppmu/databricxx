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


#include "Props.h"

#include <cassert>
#include <iostream>
#include <sstream>

using namespace std;


namespace dbrx {


void PropVal::destructorImpl() {
	switch (m_type) {
		case Type::STRING: m_content.s.~String(); break;
		case Type::ARRAY: m_content.a.~Array(); break;
		case Type::INDEXED: m_content.m.~Indexed(); break;
		case Type::STRUC: m_content.o.~Struc(); break;
		default: assert(false);
	}
}


bool PropVal::comparisonImpl(const PropVal &other) const {
	switch (m_type) {
		case Type::NAME:
			switch (other.m_type) {
				case Type::NAME: return m_content.n == other.m_content.n;
				case Type::STRING: return m_content.n.str() == other.m_content.s;
				default: return false;
			}
		case Type::STRING:
			switch (other.m_type) {
				case Type::STRING: return m_content.s == other.m_content.s;
				case Type::NAME: return m_content.s == other.m_content.n.str();
				default: return false;
			}
		case Type::ARRAY: return (other.m_type == Type::ARRAY) && (m_content.a == other.m_content.a);
		case Type::INDEXED: return (other.m_type == Type::INDEXED) && (m_content.m == other.m_content.m);
		case Type::STRUC: return (other.m_type == Type::STRUC) && (m_content.o == other.m_content.o);
		default: assert(false);
	}
}


std::ostream& PropVal::print(std::ostream &os, Real x) {
	os.precision(17);
	return cout << x;
}


std::ostream& PropVal::print(std::ostream &os, Name x) {
	return print(os, x.str());
}


std::ostream& PropVal::print(std::ostream &os, const String &x) {
	os << "\"";
    for (char c: x) {
        switch (c) {
            case '\\': os << "\\\\"; break;
            case '\f': os << "\\f"; break;
            case '\n': os << "\\n"; break;
            case '\r': os << "\\r"; break;
            case '"': os << "\\\""; break;
            default: os << c; break;
        }
    }
	os << "\"";
	return os;
}


std::ostream& PropVal::print(std::ostream &os, const Array &x) {
	os << "[";
	bool first = true;
	for (auto const &v: x) {
		if (!first) os << ", ";
		v.print(os);
		first = false;
	}
	os << "]";
	return os;
}


std::ostream& PropVal::print(std::ostream &os, const Indexed &x) {
	os << "{";
	bool first = true;
	for (auto const &e: x) {
		if (!first) os << ", ";
		os << "\"" << e.first << "\": ";
		e.second.print(os);
		first = false;
	}
	os << "}";
	return os;
}


std::ostream& PropVal::print(std::ostream &os, const Struc &x) {
	using CPRef = std::pair<Name, const PropVal*>;
	vector<CPRef> props;
	props.reserve(x.size());

	for (auto const &e: x) props.push_back({e.first, &e.second});
	auto compare = [](const CPRef &a, const CPRef &b) { return a.first < b.first; };
	sort(props.begin(), props.end(), compare);

	os << "{";
	bool first = true;
	for (auto const &e: props) {
		if (!first) os << ", ";
		os << "\"" << e.first << "\": ";
		e.second->print(os);
		first = false;
	}
	os << "}";
	return os;
}


std::ostream& PropVal::print(std::ostream &os) const {
	switch (m_type) {
		case Type::NONE: print(os, m_content.e); break;
		case Type::BOOL: print(os, m_content.b); break;
		case Type::INTEGER: print(os, m_content.i); break;
		case Type::REAL: print(os, m_content.r); break;
		case Type::NAME: print(os, m_content.n); break;
		case Type::STRING: print(os, m_content.s); break;
		case Type::ARRAY: print(os, m_content.a); break;
		case Type::INDEXED: print(os, m_content.m); break;
		case Type::STRUC: print(os, m_content.o); break;
		default: assert(false);
	}
	return os;
}


std::string PropVal::toString() const {
	stringstream out;
	print(out);
	return out.str();
}


} // namespace dbrxq
