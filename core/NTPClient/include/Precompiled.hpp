/**
 * @file Precompiled.hpp
 * @brief Precompiled header for NTPClient module
 * 
 * Includes common headers and platform-specific compatibility fixes
 */

#ifndef PRECOMPILED_HPP_
#define PRECOMPILED_HPP_

// Fix NOMINMAX conflicts on Windows
#if defined(_WIN32) || defined(_WIN64)
	#define NOMINMAX
#endif

// Boost ASIO and date/time includes
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/local_time/local_time.hpp>

// C++11 standard library includes
#include <mutex>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>

#endif // PRECOMPILED_HPP_
