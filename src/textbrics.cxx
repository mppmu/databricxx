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
	if (input == "stdin") m_inputStream = &std::cin;
	else {
		m_inputFile = unique_ptr<std::ifstream>(new ifstream(input));
		m_inputStream = m_inputFile.get();
	}
}


bool TextFileReader::nextOutput() {
	if (getline(*m_inputStream, output.get())) {
		return true;
	} else {
		m_inputStream = nullptr;
		m_inputFile = unique_ptr<std::ifstream>();
		return false;
	}
}



void TextFileWriter::newReduction() {
	dbrx_log_trace("TextFileWriter \"%s\", opening output \"%s\""_format(absolutePath(), target.get()));
	if (target == "stdout") m_outputStream = &std::cout;
	else if (target == "stderr") m_outputStream = &std::cerr;
	else {
		m_outputFile = unique_ptr<std::ofstream>(new ofstream(target));
		m_outputStream = m_outputFile.get();
	}
	if (! *m_outputStream) throw runtime_error("Can't open \"%s\" for output in bric \"%s\""_format(target.get(), absolutePath()));
}


void TextFileWriter::processInput() {
	*m_outputStream << input.get() << '\n' << flush;
	if (! *m_outputStream) throw runtime_error("Output to \"%s\" failed in bric \"%s\""_format(target.get(), absolutePath()));
}


void TextFileWriter::finalizeReduction() {
	m_outputStream = nullptr;
	m_outputFile = unique_ptr<std::ofstream>();
}


} // namespace dbrx
