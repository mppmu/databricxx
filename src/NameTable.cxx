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


#include "NameTable.h"

using namespace std;


namespace dbrx {


NameTable& NameTable::global() {
	static NameTable globalTable;
	return globalTable;
}


Name NameTable::resolve(const std::string &s) {
	if (s.empty()) return Name();
	else {
		LockGuard lock(m_mutex);
		auto found = m_stringMap.find(ConstStringRef(&s));
		if (found != m_stringMap.end()) return found->second;
		else {
			std::unique_ptr<std::string> stringPtr(new std::string(s));
			ConstStringRef sr(&*stringPtr);
			Name name(&*stringPtr);

			if (m_strings.size() == m_strings.capacity())
				m_strings.reserve(std::max(size_t(16), m_strings.size() * 2));
			m_strings.push_back(std::move(stringPtr));

			return m_stringMap[sr] = name;
		}
	}
}


} // namespace dbrx
