/**
 * @file
 * @brief Patch for Boost 1.49 Thread/ASIO compatibility with VS2022
 * 
 * Include this BEFORE any Boost headers to work around xtime issues
 */

#ifndef BOOST_THREAD_COMPAT_HPP_
#define BOOST_THREAD_COMPAT_HPP_

// Prevent Boost.Thread from using deprecated xtime
#define BOOST_THREAD_DONT_USE_CHRONO
#define BOOST_HAS_THREADS

#endif /* BOOST_THREAD_COMPAT_HPP_ */
