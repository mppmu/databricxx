// Copyright (C) 2014 Oliver Schulz <oschulz@mpp.mpg.de>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include "logging.h"

#include <iostream>


using namespace std;


namespace dbrx {


LoggingFacility LoggingFacility::g_facility;


void LoggingFacility::applyConfig(const PropVal& config) {
	static const char *s_invalidConfErr = "Invalid configuration for LoggingFacility";
	try {
		PropVal logLevel = config.asProps().at(Names::level());
		if (logLevel.isName()) level(logLevel.asName());
		else if (logLevel.isString()) level(logLevel.asString());
		else throw invalid_argument(s_invalidConfErr);
	}
	catch (const std::bad_cast&) { throw invalid_argument(s_invalidConfErr); }
	catch (const std::out_of_range&) { throw invalid_argument(s_invalidConfErr); }
}


PropVal LoggingFacility::getConfig() const {
	return Props{{Names::level(), nameOf(level())}};
}


LoggingFacility::LoggingFacility(): m_output(&std::cerr) {}


} // namespace dbrx
