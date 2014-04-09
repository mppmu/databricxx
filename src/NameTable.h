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


#ifndef DBRX_NAMETABLE_H
#define DBRX_NAMETABLE_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>

#include "Name.h"


namespace dbrx {


class NameTable {
protected:
	using Mutex = std::mutex;
	using LockGuard = std::lock_guard<Mutex>;

	Mutex m_mutex;

	class ConstStringRef {
	protected:
		const std::string *m_ptr = nullptr;

	public:
		struct Hash {
		protected:
			std::hash<std::string> m_hash;
		public:
			size_t operator()(const ConstStringRef& sr) const { return m_hash(sr.value()); }
		};

		const std::string& value() const { return *m_ptr; }
		const std::string* ptr() const { return m_ptr; }

		bool operator==(ConstStringRef other) const { return value() == other.value(); }

		ConstStringRef() = default;
		ConstStringRef(const std::string *p): m_ptr(p) {}
	};


	std::vector< std::unique_ptr<std::string> > m_strings;
	std::unordered_map<ConstStringRef, Name, ConstStringRef::Hash> m_stringMap;

public:
	static NameTable& global();

	Name resolve(const std::string &s);

	using StringContent = std::string;

	virtual ~NameTable() {}
};


} // namespace dbrx

#endif // DBRX_NAMETABLE_H
