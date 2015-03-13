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


#ifndef DBRX_APPLICATIONBRIC_H
#define DBRX_APPLICATIONBRIC_H

#include "Bric.h"


namespace dbrx {


class ApplicationBric: public virtual Bric, public BricImpl {
protected:
	bool nextExecStepImpl() override;

	void postConfig() override;

public:
	class AppBricGroup: public virtual Bric, public BricImpl {
	protected:
		ApplicationBric *m_app;

		bool canHaveDynBrics() const override { return true; }

		bool nextExecStepImpl() override;

	public:
		AppBricGroup() {}

		AppBricGroup(ApplicationBric *app, PropKey bricName)
			: BricImpl(app, bricName), m_app(app) {}

		using BricImpl::BricImpl;
	};

	AppBricGroup brics{this, "brics"};

	Param<std::vector<std::string>> requires{this, "requires", "Requirements to load before execution (e.g. libraries or scripts)"};
	Param<std::string> logLevel{this, "logLevel", "Logging level", "info"};

	void applyConfig(const PropVal& config) override;

	void run();

	ApplicationBric() {}
	ApplicationBric(PropKey bricName): BricImpl(bricName) {}
};


} // namespace dbrx

#endif // DBRX_APPLICATIONBRIC_H
