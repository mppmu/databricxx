// Copyright (C) 2011-2014 Oliver Schulz <oschulz@mpp.mpg.de>

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

#include <stdexcept>
#include <cstring>
#include <cstdio>


namespace dbrx {


LogLevel g_logLevel = LL_INFO;


void log_level(LogLevel level)
	{ g_logLevel = level; }

void log_level(const char *level)
	{ log_level(string2LogLevel(level)); }

void log_level(const std::string &level)
	{ log_level(level.c_str()); }


LogLevel string2LogLevel(const char *level) {
	if      (strncasecmp(level, "off",   6) == 0) return LL_OFF;
	else if (strncasecmp(level, "trace", 6) == 0) return LL_TRACE;
	else if (strncasecmp(level, "debug", 6) == 0) return LL_DEBUG;
	else if (strncasecmp(level, "info",  6) == 0) return LL_INFO;
	else if (strncasecmp(level, "warn",  6) == 0) return LL_WARN;
	else if (strncasecmp(level, "error", 6) == 0) return LL_ERROR;
	else if (strncasecmp(level, "all",   6) == 0) return LL_ALL;
	else throw std::invalid_argument(std::string("Invalid logging level name \"") + level + "\"");
}


const char* logLevel2String(LogLevel level) {
	if (level > LL_ERROR  ) return "ALL";
	else if (level > LL_WARN  ) return "ERROR";
	else if (level > LL_INFO  ) return "WARN";
	else if (level > LL_DEBUG  ) return "INFO";
	else if (level > LL_TRACE  ) return "DEBUG";
	else if (level > LL_OFF  ) return "TRACE";
	else return "OFF";
}


void v_log_generic_impl(const char* tag, const char *fmt, va_list argp) {
	vfprintf(stderr, (std::string(tag) + fmt + "\n").c_str(), argp);
}

void log_trace_impl(const char *fmt, ...) { va_list argp; va_start(argp, fmt); v_log_generic_impl("!TRACE: ", fmt, argp);	va_end(argp); }

void log_debug_impl(const char *fmt, ...) { va_list argp; va_start(argp, fmt); v_log_generic_impl("!DEBUG: ", fmt, argp);	va_end(argp); }

void log_info_impl(const char *fmt, ...) { va_list argp; va_start(argp, fmt); v_log_generic_impl("!INFO:  ", fmt, argp);	va_end(argp); }

void log_warn_impl(const char *fmt, ...) { va_list argp; va_start(argp, fmt); v_log_generic_impl("!WARN: ", fmt, argp);	va_end(argp); }

void log_error_impl(const char *fmt, ...) { va_list argp; va_start(argp, fmt); v_log_generic_impl("!ERROR: ", fmt, argp);	va_end(argp); }


} // namespace dbrx
