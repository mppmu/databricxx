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


#ifndef DBRX_APPLICATIONCONFIG_H
#define DBRX_APPLICATIONCONFIG_H

#include <iostream>

#include "Props.h"


namespace dbrx {


class ApplicationConfig {
protected:
	PropVal m_config = Props();
	Props m_varValues;
	bool m_substVars = true;
	bool m_useEnvVars = true;
	Name m_overrideLogLevel;

public:
	virtual const PropVal& config() const { return m_config; }
	virtual PropVal& config() { return m_config; }

	virtual const Props& varValues() const { return m_varValues; }
	virtual Props& varValues() { return m_varValues; }

	virtual void addVar(PropKey key, PropVal value) { varValues()[key] = std::move(value); }
	virtual void addVar(const std::string varDef);

	virtual void addConfigFromFile(const std::string& fileName);

	virtual bool substVars() { return m_substVars; }
	virtual void substVars(bool enabled) { m_substVars = enabled; }

	virtual bool useEnvVars() { return m_useEnvVars; }
	virtual void useEnvVars(bool enabled) { m_useEnvVars = enabled; }

	virtual void finalize();

	virtual void applyLoggingConfig();
	virtual void applyLogLevelOverride(const std::string &levelName);

	virtual std::ostream& print(std::ostream &out, const std::string& format = "json");

	friend const std::string to_string(ApplicationConfig appConfig)
		{ return to_string(appConfig.config()); }
};


inline std::ostream& operator<<(std::ostream &os, ApplicationConfig appConfig)
	{ return os << appConfig.config(); }


} // namespace dbrx

#endif // DBRX_APPLICATIONCONFIG_H
