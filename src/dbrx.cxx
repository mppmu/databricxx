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
#include <memory>

#include <unistd.h>

#include <TROOT.h>
#include <TSystem.h>
#include <TApplication.h>
#include <THttpServer.h>

#include "logging.h"
#include "Props.h"
#include "ApplicationBric.h"
#include "ApplicationConfig.h"


using namespace std;
using namespace dbrx;


ApplicationConfig g_config;
unique_ptr<TApplication> g_rootApplication;


void task_get_config_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " [OPTIONS] CONFIG.." << endl;
	cerr << "" << endl;
	cerr << "Options:" << endl;
	cerr << "-?              Show help" << endl;
	cerr << "-f FORMAT       Set output format (formats: [json], ...)" << endl;
	cerr << "-l LEVEL        Set logging level" << endl;
	cerr << "-V NAME=VALUE   Define variable value for configuration" << endl;
	cerr << "-s              Disable variable substitution in configuration" << endl;
	cerr << "-e              Do not use environment variables in configuration" << endl;
	cerr << "" << endl;
	cerr << "Combine and output given configurations in specified format (JSON by default)." << endl;
	cerr << "Supported output formats: \"json\" (more formats to come in future versions)." << endl;
}


int task_get_config(int argc, char *argv[], char *envp[]) {
	string outputFormat("json");
	ApplicationConfig config;

	int opt = 0;
	while ((opt = getopt(argc, argv, "?c:l:f:jV:se")) != -1) {
		switch (opt) {
			case '?': { task_get_config_printUsage(argv[0]); return 0; }
			case 'l': { g_config.applyLogLevelOverride(optarg); break; }
			case 'f': {
				dbrx_log_debug("Setting output format to %s", optarg);
				outputFormat = string(optarg);
				break;
			}
			case 'V': { config.addVar(optarg); break; }
			case 's': { config.substVars(false); break; }
			case 'e': { config.useEnvVars(false); break; }
			default: throw invalid_argument("Unkown command line option");
		}
	}

	while (optind < argc) {
		std::string from = argv[optind++];
		config.addConfigFromFile(from);
	}
	config.finalize();

	config.print(cout, outputFormat);

	return 0;
}


void task_run_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " [OPTIONS] CONFIG.." << endl;
	cerr << "" << endl;
	cerr << "Options:" << endl;
	cerr << "-?              Show help" << endl;
	cerr << "-c SETTINGS     Load configuration/settings" << endl;
	cerr << "-l LEVEL        Set logging level (default: \"info\")" << endl;
	cerr << "-w              Enable HTTP server" << endl;
	cerr << "-p PORT         HTTP server port (default: 8080)" << endl;
	cerr << "-k              Don't exit after processing (e.g. to keep HTTP server running)" << endl;
	cerr << "-V NAME=VALUE   Define variable value for configuration" << endl;
	cerr << "-s              Disable variable substitution in configuration" << endl;
	cerr << "-e              Do not use environment variables in configuration" << endl;
	cerr << "" << endl;
	cerr << "Run the given bric configuration. If multiple configuration are given, they" << endl;
	cerr << "are merged together (from left to right)." << endl;
}


int task_run(int argc, char *argv[], char *envp[]) {
	bool enableHTTP = false;
	uint16_t httpPort = 8080;
	bool keepRunning = false;

	int opt = 0;
	while ((opt = getopt(argc, argv, "?c:l:wp:kV:se")) != -1) {
		switch (opt) {
			case '?': { task_run_printUsage(argv[0]); return 0; }
			case 'l': { g_config.applyLogLevelOverride(optarg); break; }
			case 'w': { enableHTTP = true; break; }
			case 'p': { httpPort = atoi(optarg); break; }
			case 'k': { keepRunning = true; break; }
			case 'V': { g_config.addVar(optarg); break; }
			case 's': { g_config.substVars(false); break; }
			case 'e': { g_config.useEnvVars(false); break; }
			default: throw invalid_argument("Unkown command line option");
		}
	}

	if (! (optind < argc)) {
		task_run_printUsage(argv[0]);
		return 1;
	}

	while (optind < argc) {
		std::string from = argv[optind++];
		g_config.addConfigFromFile(from);
	}
	g_config.finalize();
	g_config.applyLoggingConfig();

	unique_ptr<THttpServer> httpServer;
	if (enableHTTP) {
		dbrx_log_info("Starting HTTP server on port %s", httpPort);
		httpServer = unique_ptr<THttpServer>(
			new THttpServer("civetweb:%s"_format(httpPort).c_str()));
	}

	ApplicationBric app("dbrx");
	app.applyConfig(g_config.config());
	app.run();

	if (keepRunning) {
		dbrx_log_info("Keeping program running");
		if (httpServer) dbrx_log_info("HTTP server active on port %s", httpPort);
		g_rootApplication->Run(true);
	}

	return 0;
}


void main_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " COMMAND ..." << endl << endl;
	cerr << "Commands: " << endl;
	cerr << "  get-config" << endl;
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
		// Disable ROOT on-screen graphics output (must be run before
		// TApplication constructor):
		gROOT->SetBatch(true);

		// Have to create an application to activate ROOT's on-demand class loader
		// (still true for ROOT-6?):
//		g_rootApplication = unique_ptr<TApplication>(new TApplication("dbrx", &nArgs, args));
		g_rootApplication = unique_ptr<TApplication>(new TApplication("dbrx", 0, 0));

		// Disable ROOT on-screen graphics output:
		gROOT->SetBatch(true);

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
		if (cmd == "get-config") return task_get_config(cmd_argc, cmd_argv, envp);
		else if (cmd == "run") return task_run(cmd_argc, cmd_argv, envp);
		else throw invalid_argument("Command \"%s\" not supported."_format(cmd));
	}
	catch(std::exception &e) {
		dbrx_log_error("%s (%s)",e.what(), typeid(e).name());
		return 1;
	}

	dbrx_log_info("Done.");
	return 0;
}
