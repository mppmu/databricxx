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


#include "textbrics.h"


using namespace std;


namespace dbrx {


void TextFileReader::processInput() {
	dbrx_log_trace("TextFileReader \"%s\", opening next input \"%s\""_format(absolutePath(), input.get()));
	try {
		m_inputStream.open(input);
	} catch (std::runtime_error &e) {
		throw runtime_error("Can't open \"%s\" for input in bric \"%s\": %s"_format(input.get(), absolutePath(), e.what()));
	}
}


bool TextFileReader::nextOutput() {
	if (getline(m_inputStream.stream(), output.get())) {
		return true;
	} else {
		m_inputStream.close();
		return false;
	}
}


} // namespace dbrx
