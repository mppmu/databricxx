// Copyright (C) 2015 Oliver Schulz <oschulz@mpp.mpg.de>

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


#include "ManagedStream.h"

#include <fstream>


using namespace std;


namespace dbrx {


bool ManagedStream::isStdStreamName(const std::string& fileName) const {
	return fileName == "-";
}


void ManagedStream::ownStdStream() const {
	const ManagedStream* noOwner = nullptr;
	if (! stdStreamOwner().compare_exchange_strong(noOwner, this))
		throw runtime_error("Can't take ownership of standard input/output stream, already belongs to someone else");
}


void ManagedStream::close() {
	m_owned_stream.reset();
}



std::atomic<const ManagedStream*> ManagedInputStream::m_owner;


void ManagedInputStream::open(const std::string& fileName) {
	close();

	if (isStdStreamName(fileName)) {
		ownStdStream();
		m_stream_ptr = stdStream();
	} else {
		m_stream_ptr = new ifstream(fileName.c_str());
		m_owned_stream.reset(m_stream_ptr);
	}
}


void ManagedInputStream::close() {
	m_stream_ptr = 0;
	ManagedStream::close();
}



std::atomic<const ManagedStream*> ManagedOutputStream::m_owner;


void ManagedOutputStream::open(const std::string& fileName) {
	close();

	if (isStdStreamName(fileName)) {
		ownStdStream();
		m_stream_ptr = stdStream();
	} else {
		m_stream_ptr = new ofstream(fileName.c_str());
		m_owned_stream.reset(m_stream_ptr);
	}
}


void ManagedOutputStream::close() {
	m_stream_ptr = 0;
	ManagedStream::close();
}


} // namespace dbrx
