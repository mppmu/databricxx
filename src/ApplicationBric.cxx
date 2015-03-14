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


#include "ApplicationBric.h"

#include <iostream>

#include <TROOT.h>
#include <TSystem.h>


using namespace std;


namespace dbrx {


bool ApplicationBric::nextExecStepImpl() {
	while (! brics.execFinished()) { brics.nextExecStep(); }
	setExecFinished();
}


void ApplicationBric::postConfig() {
	auto level = LoggingFacility::levelOf(Name(logLevel));
	auto normalizedName = LoggingFacility::nameOf(level);
	logLevel = normalizedName;
	if (level != log_level()) {
		dbrx_log_debug("Changing logging level to %s", normalizedName);
		log_level(level);
	}
}


bool ApplicationBric::AppBricGroup::nextExecStepImpl() {
	Bric &mainBric = *m_brics.at("main");

	dbrx_log_info("Running bric \"%s\"", mainBric.absolutePath());
	while (! mainBric.execFinished()) mainBric.nextExecStep();
	setExecFinished();
	dbrx_log_info("Finished running bric \"%s\"", mainBric.absolutePath());
}


void ApplicationBric::applyConfig(const PropVal& config) {
	// Requirements have to be loaded before applying the actual config

	const Props &props = config.asProps();
	auto foundReq = props.find(requires.name());
	if (foundReq != props.end()) {
		requires.applyConfig(foundReq->second);

		for (const string &dep: requires.get()) {
			dbrx_log_info("Processing requirement \"%s\"", dep);
			if (dep.find('(') != dep.npos) {
				// Looks like a command to run, not a script or a library to load
				string cmd = dep + string(";");
				dbrx_log_debug("Running gROOT->ProcessLine(\"%s\")", cmd);
				gROOT->ProcessLineSync(cmd.c_str());
			} else if ((dep.find(".c") != dep.npos) || (dep.find(".C") != dep.npos)) {
				string cmd = string(".L ") + dep;
				dbrx_log_debug("Running gROOT->ProcessLine(\"%s\")", cmd);
				gROOT->ProcessLineSync(cmd.c_str());
			} else {
				dbrx_log_debug("Running gSystem->Load(\"%s\")", dep);
				if (gSystem->Load(dep.c_str()) < 0)
					throw runtime_error("Couldn't load \"%s\""_format(dep));
			}
		}
	}	
	
	Bric::applyConfig(config);
}


void ApplicationBric::run() {
	if (hasParent()) throw invalid_argument("Can't call run on bric \"%s\", not a top bric"_format(absolutePath()));

	initBricHierarchy();

	assert(! execFinished());
	while (!execFinished()) nextExecStep();
}


} // namespace dbrx
