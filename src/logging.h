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


#ifndef DBRX_LOGGING_H
#define DBRX_LOGGING_H

#include <string>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <cstdint>

#include "format.h"
#include "Props.h"


namespace dbrx {


enum class LogLevel: int32_t {
	ALL   = 0,
	TRACE = 10,
	DEBUG = 20,
	INFO  = 30,
	WARN  = 40,
	ERROR = 50,
	OFF   = std::numeric_limits<int32_t>::max()
};



class LoggingFacility: public Configurable {
protected:
	// Static value is safe here, as C++ guarantees static values to be zero
	// initialized before any other form of initialization. Logging will just
	// remain disabled during construction of other static values until
	// constructor of g_facility is reached.
	static LoggingFacility g_facility;

	LogLevel m_level{LogLevel::INFO};
	std::ostream* m_output{nullptr};

	const char* tag(LogLevel logLevel) const {
		if (logLevel == LogLevel::ERROR  ) return "!ERROR: ";
		else if (logLevel == LogLevel::WARN  ) return "!WARN: ";
		else if (logLevel == LogLevel::INFO  ) return "!INFO: ";
		else if (logLevel == LogLevel::DEBUG  ) return "!DEBUG: ";
		else if (logLevel == LogLevel::TRACE  ) return "!TRACE: ";
		else return "!LOG: ";
	}

public:
	struct Names {
		static Name level() { static Name n("level"); return n; }
		static Name all() { static Name n("all"); return n; }
		static Name trace() { static Name n("trace"); return n; }
		static Name debug() { static Name n("debug"); return n; }
		static Name info() { static Name n("info"); return n; }
		static Name warn() { static Name n("warn"); return n; }
		static Name error() { static Name n("error"); return n; }
		static Name off() { static Name n("off"); return n; }
	};


	static LogLevel levelOf(Name name) {
		if      (name == Names::off()) return LogLevel::OFF;
		else if (name == Names::trace()) return LogLevel::TRACE;
		else if (name == Names::debug()) return LogLevel::DEBUG;
		else if (name == Names::info()) return LogLevel::INFO;
		else if (name == Names::warn()) return LogLevel::WARN;
		else if (name == Names::error()) return LogLevel::ERROR;
		else if (name == Names::all()) return LogLevel::ALL;
		else throw std::invalid_argument("Invalid logging level name \"%s\""_format(name));
	}


	static Name nameOf(LogLevel logLevel) {
		if (logLevel == LogLevel::OFF  ) return Names::off();
		else if (logLevel >= LogLevel::ERROR  ) return Names::error();
		else if (logLevel >= LogLevel::WARN  ) return Names::warn();
		else if (logLevel >= LogLevel::INFO  ) return Names::info();
		else if (logLevel >= LogLevel::DEBUG  ) return Names::debug();
		else if (logLevel >= LogLevel::TRACE  ) return Names::trace();
		else return Names::all();
	}

	static LoggingFacility& global() { return g_facility; }

	LogLevel level() const { return m_level; }
	void level(LogLevel logLevel) { m_level = logLevel; }
	void level(Name logLevel) { level(levelOf(logLevel)); }

	std::ostream* output() const { return m_output; }
	void output(std::ostream* os) { m_output = os; }

	bool logEnabled(LogLevel logLevel) { return logLevel >= m_level; }


	template <typename ...Args> void log(LogLevel logLevel, std::string fmt, Args&&... args) const {
		// Build intermediate stringstream to make sure output is atomic.
		// Assumes output stream is thread-safe (default for standard streams).
		using namespace std;
		if (output() == nullptr) return;
		stringstream tmp;
		tmp << tag(logLevel);
		FormatString(std::move(fmt)).print(tmp, std::forward<Args>(args)...);
		tmp << endl;
		*output() << tmp.str() << flush;
	}

	void log(LogLevel logLevel, const std::string& msg) const {
		// Build intermediate stringstream to make sure output is atomic.
		// Assumes output stream is thread-safe (default for standard streams).
		using namespace std;
		if (output() == nullptr) return;
		stringstream tmp;
		tmp << tag(logLevel);
		tmp << msg;
		tmp << endl;
		*output() << tmp.str() << flush;
	}

	template <typename ...Args> void logTrace(Args&&... args) const
		{ log(LogLevel::TRACE, std::forward<Args>(args)...); }

	template <typename ...Args> void logDebug(Args&&... args) const
		{ log(LogLevel::DEBUG, std::forward<Args>(args)...); }

	template <typename ...Args> void logInfo(Args&&... args) const
		{ log(LogLevel::INFO, std::forward<Args>(args)...); }

	template <typename ...Args> void logWarn(Args&&... args) const
		{ log(LogLevel::WARN, std::forward<Args>(args)...); }

	template <typename ...Args> void logError(Args&&... args) const
		{ log(LogLevel::ERROR, std::forward<Args>(args)...); }

	void logNothing() {}

	void applyConfig(const PropVal& config) override;
	PropVal getConfig() const override;

	LoggingFacility();
	virtual ~LoggingFacility() {}
};


inline Name to_name(LogLevel logLevel) { return LoggingFacility::nameOf(logLevel); }
inline const std::string& to_string(LogLevel logLevel) { return to_name(logLevel).str(); }


inline LoggingFacility& log_facility() { return LoggingFacility::global(); }

inline LogLevel log_level() { return log_facility().level(); }
inline void log_level(LogLevel logLevel) { log_facility().level(logLevel); }
inline void log_level(Name logLevel) { log_facility().level(logLevel); }

inline bool log_enabled(LogLevel logLevel) { return log_facility().logEnabled(logLevel); }


} // namespace dbrx


#define dbrx_log_trace(...) (dbrx::log_facility().logEnabled(dbrx::LogLevel::TRACE) ? dbrx::log_facility().logTrace(__VA_ARGS__) : dbrx::log_facility().logNothing())
#define dbrx_log_debug(...) (dbrx::log_facility().logEnabled(dbrx::LogLevel::DEBUG) ? dbrx::log_facility().logDebug(__VA_ARGS__) : dbrx::log_facility().logNothing())
#define dbrx_log_info(...) (dbrx::log_facility().logEnabled(dbrx::LogLevel::INFO) ? dbrx::log_facility().logInfo(__VA_ARGS__) : dbrx::log_facility().logNothing())
#define dbrx_log_warn(...) (dbrx::log_facility().logEnabled(dbrx::LogLevel::WARN) ? dbrx::log_facility().logWarn(__VA_ARGS__) : dbrx::log_facility().logNothing())
#define dbrx_log_error(...) (dbrx::log_facility().logEnabled(dbrx::LogLevel::ERROR) ? dbrx::log_facility().logError(__VA_ARGS__) : dbrx::log_facility().logNothing())


#endif // DBRX_LOGGING_H
