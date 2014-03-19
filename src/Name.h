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


#ifndef DBRX_NAME_H
#define DBRX_NAME_H

#include <string>
#include <iosfwd>

#include <TString.h>


namespace dbrx {


class Name {
protected:
	// Temporary implementation, use string interning later
	std::string m_value;

public:
	operator const std::string& () const { return m_value; }
	const std::string& str() const { return m_value; }
	const char* c_str() const { return m_value.c_str(); }

	operator TString () const { return m_value; }
	operator const char* () const { return m_value.c_str(); }

	Name(const std::string &s) : m_value(s) { }
	Name(const TString &s) : m_value(s) { }
	Name(const char *s) : m_value(s) { }

	Name& operator=(const Name &other) = default;
	Name& operator=(Name &&other) noexcept { m_value = std::move(other.m_value); return *this; }

	Name(const Name &other) = default;
	Name(Name &&other) noexcept = default;
	Name() = default; // Why is this necessary for classes derived from Name like Input?

	// virtual ~Name() {}
};


template<typename T> std::ostream & operator<<(std::ostream &os, const Name &n) {
	return os << n.str();
}



class HasName {
public:
	virtual const Name& name() const = 0;
	virtual void name(Name n) = 0;

	virtual ~HasName() {}
};



class HasNameImpl: public virtual HasName {
protected:
	Name m_name;

public:
	const Name& name() const { return m_name; }
	void name(Name n) { m_name = std::move(n); }

	HasNameImpl() = default;
	HasNameImpl(const Name &n): m_name(n) {}
};


} // namespace dbrx

#endif // DBRX_NAME_H
