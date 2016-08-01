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


#ifndef DBRX_MANAGEDSTREAM_H
#define DBRX_MANAGEDSTREAM_H

#include <memory>
#include <atomic>
#include <iostream>


namespace dbrx {


class ManagedStream {
protected:
	std::unique_ptr<std::ios> m_owned_stream;

	virtual std::ios* stdStream() const = 0;
	virtual std::atomic<const ManagedStream*>& stdStreamOwner() const = 0;

	virtual bool isStdStreamName(const std::string& fileName) const;
	virtual void ownStdStream() const;
	virtual void releaseStdStream() const;

public:
	virtual std::ios& stream() = 0;

	virtual void open(const std::string& fileName) = 0;
	virtual void close();

	virtual ~ManagedStream() {}
};



class ManagedInputStream: public ManagedStream {
protected:
	static std::atomic<const ManagedStream*> m_owner;
	std::istream* m_stream_ptr = nullptr;

	std::istream* stdStream() const final override { return &std::cin; }
	std::atomic<const ManagedStream*>& stdStreamOwner() const final override { return m_owner; }

public:
	std::istream& stream() final override { return *m_stream_ptr; }

	void open(const std::string& fileName) override;
	void close() override;

	ManagedInputStream() = default;
	ManagedInputStream(const std::string& fileName) { open(fileName); }
};



class ManagedOutputStream: public ManagedStream {
protected:
	static std::atomic<const ManagedStream*> m_owner;
	std::ostream* m_stream_ptr = nullptr;

	std::ostream* stdStream() const final override { return &std::cout; }
	std::atomic<const ManagedStream*>& stdStreamOwner() const final override { return m_owner; }

public:
	std::ostream& stream() final override { return *m_stream_ptr; }

	void open(const std::string& fileName) override;
	void close() override;

	ManagedOutputStream() = default;
	ManagedOutputStream(const std::string& fileName) { open(fileName); }
};


} // namespace dbrx

#endif // DBRX_MANAGEDSTREAM_H
