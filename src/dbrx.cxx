// Copyright (C) 2011 Oliver Schulz <oliver.schulz@tu-dortmund.de>

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


#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>

#include <unistd.h>

#include <TROOT.h>
#include <TSystem.h>
#include <TApplication.h>

#include "logging.h"
#include "Props.h"
#include "ApplicationBric.h"


using namespace std;
using namespace dbrx;


PropVal g_config = Props();
Name g_logLevelOverride;
unique_ptr<TApplication> g_rootApplication;

void configureLogging() {
	// g_logLevel overrides logging config in g_config
	if (! g_logLevelOverride.empty()) {
		g_config["logLevel"] = g_logLevelOverride;
	}

	if (g_config.contains("logLevel")) {
		auto level = LoggingFacility::levelOf(g_config.at("logLevel").asString());
		if (level != log_level()) {
			dbrx_log_debug("Changing logging level to %s", LoggingFacility::nameOf(level));
			log_level(level);
		}
	}
}


void configureLogging(const std::string &levelName) {
	if (!levelName.empty()) {
		Name normalizedLevelName = LoggingFacility::nameOf(LoggingFacility::levelOf(levelName));
		g_logLevelOverride = normalizedLevelName;
		configureLogging();
	}
}


void addConfigFrom(PropVal &config, const std::string &from) {
	dbrx_log_debug("Reading \"%s\"", from);
	PropVal p = PropVal::fromFile(from);
	dbrx_log_debug("Done reading");
	if (!p.isProps()) throw invalid_argument("Invalid config in \"%s\", but contain an object, not a value or an array"_format(from));
	config.asProps() += p.asProps();
}


void addGlobalConfigFrom(const std::string &from) {
	dbrx_log_debug("Adding \"%s\" to config", from);
	addConfigFrom(g_config, from);
}


void printConfig(std::ostream &out, const PropVal &config, const std::string &format) {
	if (format == "json") { config.toJSON(out); out << endl; }
	else throw invalid_argument("Unknown configuration output format");
}


void task_config_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " [OPTIONS] CONFIG.." << endl;
	cerr << "" << endl;
	cerr << "Options:" << endl;
	cerr << "-?          Show help" << endl;
	cerr << "-f FORMAT   Set output format (formats: [json], ...)" << endl;
	cerr << "-l LEVEL    Set logging level" << endl;
	cerr << "" << endl;
	cerr << "Combine and output given configurations in specified format (JSON by default)." << endl;
	cerr << "Supported output formats: \"json\" (more formats to come in future versions)." << endl;
}


int task_config(int argc, char *argv[], char *envp[]) {
	string outputFormat("json");

	int opt = 0;
	while ((opt = getopt(argc, argv, "?c:l:f:j")) != -1) {
		switch (opt) {
			case '?': { task_config_printUsage(argv[0]); return 0; }
			case 'l': { configureLogging(optarg); break; }
			case 'f': {
				dbrx_log_debug("Setting output format to %s", optarg);
				outputFormat = string(optarg);
				break;
			}
			default: throw invalid_argument("Unkown command line option");
		}
	}

	PropVal config = Props();
	while (optind < argc) {
		std::string from = argv[optind++];
		dbrx_log_debug("Reading config from \"%s\"", from);
		addConfigFrom(config, from);
	}

	printConfig(cout, config, outputFormat);
}


void task_run_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " [OPTIONS] CONFIG.." << endl;
	cerr << "" << endl;
	cerr << "Options:" << endl;
	cerr << "-?          Show help" << endl;
	cerr << "-c SETTINGS Load configuration/settings" << endl;
	cerr << "-l LEVEL    Set logging level (default: \"info\")" << endl;
	cerr << "" << endl;
	cerr << "Run the given bric configuration. If multiple configuration are given, they" << endl;
	cerr << "are merged together (from left to right)." << endl;
}


int task_run(int argc, char *argv[], char *envp[]) {
	int opt = 0;

	while ((opt = getopt(argc, argv, "?c:l:")) != -1) {
		switch (opt) {
			case '?': { task_run_printUsage(argv[0]); return 0; }
			case 'l': { configureLogging(optarg); break; }
			default: throw invalid_argument("Unkown command line option");
		}
	}

	if (! (optind < argc)) {
		task_run_printUsage(argv[0]);
		return 1;
	}

	while (optind < argc) {
		std::string from = argv[optind++];
		dbrx_log_debug("Reading config from \"%s\"", from);
		addGlobalConfigFrom(from);
	}
	configureLogging();

	ApplicationBric app("app");
	app.applyConfig(g_config);
	app.run();

	return 0;
}


void main_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " COMMAND ..." << endl << endl;
	cerr << "Commands: " << endl;
	cerr << "  config" << endl;
	cerr << "  run" << endl;
	cerr << "" << endl;
	cerr << "Use" << endl;
	cerr << "" << endl;
	cerr << "    " << progName << " COMMAND -?" << endl;
	cerr << "" << endl;
	cerr << "to get help for the individual commands." << endl;
}


int main(int argc, char *argv[], char *envp[]) {
	try {
		// Have to create an application to activate ROOT's on-demand class loader
		// (still true for ROOT-6?):
		g_rootApplication = unique_ptr<TApplication>(new TApplication("dbrx", 0, 0));

		// Set ROOT program name (necessary / useful ?):
		gSystem->SetProgname("dbrx");

		// Have to tell ROOT to load vector dlls, otherwise ROOT will produce
		// "is not of a class known to ROOT" errors on creation of STL vector
		// branches (still true for ROOT-6?):
		gROOT->ProcessLineSync("#include <vector>");

		// Make sure Cling has all databricxx headers loaded. This currently
		// seems to be the only way, gSystem->Load("libdatabricxx.so") is not
		// sufficient for some reason:
		gROOT->ProcessLineSync("dbrx::PropVal();");


		string progName(argv[0]);

		if (argc < 2) { main_printUsage(argv[0]); return 1; }

		string cmd(argv[1]);


		int cmd_argc = argc - 1;
		char **cmd_argv = argv + 1;


		if (cmd == "-?") { main_printUsage(argv[0]); return 0; }
		if (cmd == "config") return task_config(cmd_argc, cmd_argv, envp);
		else if (cmd == "run") return task_run(cmd_argc, cmd_argv, envp);
		else throw invalid_argument("Command \"%s\" not supported."_format(cmd));
	}
	catch(std::exception &e) {
		dbrx_log_error("%s (%s)",e.what(), typeid(e).name());
		return 1;
	}

	dbrx_log_info("Done.");
}
