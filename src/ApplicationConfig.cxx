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


#include "ApplicationConfig.h"

#include <TSystem.h>

#include "logging.h"


using namespace std;


namespace dbrx {


void ApplicationConfig::addVar(const std::string varDef) {
	size_t eqPos = varDef.find('=');
	if (eqPos == varDef.npos) throw invalid_argument("Invalid variable specification \"%s\", must have format \"name=value\""_format(varDef));
	Name name = varDef.substr(0, eqPos);
	PropVal value = PropVal::fromString(varDef.substr(eqPos+1, varDef.npos));
	dbrx_log_trace("Adding configuration variable \"%s\" with value %s", name, value.toJSON());
	addVar(name, value);
}


void ApplicationConfig::addConfigFromFile(const std::string& fileName) {
	dbrx_log_debug("Reading configuration from \"%s\"", fileName);
	PropVal p = PropVal::fromFile(fileName);
	dbrx_log_debug("Done reading"); //!!
	if (!p.isProps()) throw invalid_argument("Invalid config in \"%s\", must contain an object, not a value or an array"_format(fileName));
	if (substVars()) {
		PropVal path(gSystem->DirName(fileName.c_str()));
		dbrx_log_trace("Substituting \"$_\" with config file path \"%s\"", path);
		Props subst{ {"_", path} };
		p.substVars(subst, false, true);
	}
	config().asProps() += p.asProps();		
}


void ApplicationConfig::finalize() {
	if (substVars()) {
		dbrx_log_debug("Applying variable substitions to config (%s environment variables)", useEnvVars() ? "including" : "without");
		config().substVars(varValues(), useEnvVars());
	}
}


void ApplicationConfig::applyLoggingConfig() {
	// m_overrideLogLevel overrides logging config in config()
	if (! m_overrideLogLevel.empty()) {
		config()["logLevel"] = m_overrideLogLevel;
	}

	if (config().contains("logLevel")) {
		auto level = LoggingFacility::levelOf(config().at("logLevel").asString());
		if (level != log_level()) {
			dbrx_log_debug("Changing logging level to %s", LoggingFacility::nameOf(level));
			log_level(level);
		}
	}
}


void ApplicationConfig::applyLogLevelOverride(const std::string &levelName) {
	if (!levelName.empty()) {
		Name normalizedLevelName = LoggingFacility::nameOf(LoggingFacility::levelOf(levelName));
		m_overrideLogLevel = normalizedLevelName;
		applyLoggingConfig();
	}
}


std::ostream& ApplicationConfig::print(std::ostream &out, const std::string &format) {
	if (format == "json") { config().toJSON(out); out << endl; }
	else throw invalid_argument("Unknown configuration output format");
	return out;
}


} // namespace dbrx
