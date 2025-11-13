/**
 * @file
 * @brief     A Network Time Protocol client singleton
 * 
 * @author    Daniel Fernandez (Initial pascal implementation)
 * @author    Morgan Doel (Ported to c++)
 * @version   F4.x.x
 * @date      2012
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE. IMPORTANT PLEASE READ THE TERMS AND CONDITIONS OF THIS LICENSE AGREEMENT CAREFULLY BEFORE USING THIS SOFTWARE: 
 * @copyright Asirikuy's End-User License Agreement ("EULA") is a legal agreement between you (either an individual or a single entity) and Asirikuy for the use of the Asirikuy Framework in both source and binary forms. By installing, copying, or otherwise using the Asirikuy Framework, you agree to be bound by the terms of this EULA. This license agreement represents the entire agreement concerning the program between you and Asirikuy, (referred to as "licenser"), and it supersedes any prior proposal, representation, or understanding between the parties. If you do not agree to the terms of this EULA, do not install or use the Asirikuy Framework.
 * @copyright The Asirikuy Framework is protected by copyright laws and international copyright treaties, as well as other intellectual property laws and treaties. The Asirikuy Framework is licensed, not sold.
 * @copyright 1. GRANT OF LICENSE.
 * @copyright The Asirikuy Framework is licensed as follows:
 * @copyright (a) Installation and Use.
 * @copyright Asirikuy grants you the right to install and use copies of the Asirikuy Framework in both source and binary forms for personal and business use. You may also make modifications to the source code.
 * @copyright (b) Backup Copies.
 * @copyright You may make copies of the Asirikuy Framework as may be necessary for backup and archival purposes.
 * @copyright 2. DESCRIPTION OF OTHER RIGHTS AND LIMITATIONS.
 * @copyright (a) Maintenance of Copyright Notices.
 * @copyright You must not remove or alter any copyright notices on any and all copies of the Asirikuy Framework.
 * @copyright (b) Distribution.
 * @copyright You may not distribute copies of the Asirikuy Framework in binary or source forms to third parties outside of the Asirikuy community.
 * @copyright (c) Rental.
 * @copyright You may not rent, lease, or lend the Asirikuy Framework.
 * @copyright (d) Compliance with Applicable Laws.
 * @copyright You must comply with all applicable laws regarding use of the Asirikuy Framework.
 * @copyright 3. TERMINATION
 * @copyright Without prejudice to any other rights, Asirikuy may terminate this EULA if you fail to comply with the terms and conditions of this EULA. In such event, you must destroy all copies of the Asirikuy Framework in your possession.
 * @copyright 4. COPYRIGHT
 * @copyright All title, including but not limited to copyrights, in and to the Asirikuy Framework and any copies thereof are owned by Asirikuy or its suppliers. All title and intellectual property rights in and to the content which may be accessed through use of the Asirikuy Framework is the property of the respective content owner and may be protected by applicable copyright or other intellectual property laws and treaties. This EULA grants you no rights to use such content. All rights not expressly granted are reserved by Asirikuy.
 * @copyright 5. NO WARRANTIES
 * @copyright Asirikuy expressly disclaims any warranty for the Asirikuy Framework. The Asirikuy Framework is provided 'As Is' without any express or implied warranty of any kind, including but not limited to any warranties of merchantability, noninfringement, or fitness of a particular purpose. Asirikuy does not warrant or assume responsibility for the accuracy or completeness of any information, text, graphics, links or other items contained within the Asirikuy Framework. Asirikuy makes no warranties respecting any harm that may be caused by the transmission of a computer virus, worm, time bomb, logic bomb, or other such computer program. Asirikuy further expressly disclaims any warranty or representation to Authorized Users or to any third party.
 * @copyright 6. LIMITATION OF LIABILITY
 * @copyright In no event shall Asirikuy or any contributors to the Asirikuy Framework be liable for any damages (including, without limitation, lost profits, business interruption, or lost information) rising out of 'Authorized Users' use of or inability to use the Asirikuy Framework, even if Asirikuy has been advised of the possibility of such damages. In no event will Asirikuy or any contributors to the Asirikuy Framework be liable for loss of data or for indirect, special, incidental, consequential (including lost profit), or other damages based in contract, tort or otherwise. Asirikuy and contributors to the Asirikuy Framework shall have no liability with respect to the content of the Asirikuy Framework or any part thereof, including but not limited to errors or omissions contained therein, libel, infringements of rights of publicity, privacy, trademark rights, business interruption, personal injury, loss of privacy, moral rights or the disclosure of confidential information.
 */

#if defined _WIN32 || defined _WIN64
	#pragma warning(disable: 4996) /* Warning about not using some Microsoft secure versions of c functions */
	#define _CRT_RAND_S
	#include <stdlib.h>
	#define rand_r rand_s
#endif

#include "Precompiled.hpp"
#include "NTPClient.hpp"
#include "CriticalSection.h"
#include "EasyTradeCWrapper.hpp"
#include <chrono>

using boost::asio::ip::udp;
using boost::asio::steady_timer;
using namespace boost::gregorian;
using namespace boost::local_time;
using namespace boost::posix_time;

volatile NTPClient* NTPClient::instance_ = NULL;

NTPClient* NTPClient::getInstance()
{
  // Double-checked locking for lazy instantiation
  // Warning: This function may not be thread-safe if using a compiler prior to Visual C++ 2005.
  if(instance_ == NULL)
  {
    enterCriticalSection();
    if(instance_ == NULL)
    {
      instance_ = new NTPClient();
      fprintf(stderr, "[NOTICE] NTPClient has been instantiated\n");
    }
    leaveCriticalSection();
  }

  return const_cast<NTPClient*>(instance_);
}

time_t NTPClient::queryRandomNTPServer()
{
  boost::mutex::scoped_lock l(randomMutex_);

  time_t reference_times[MAX_REFERENCE_TIMES];

  // Get the local time
  time_t local_time = time(NULL);

  // Check for daylight savings or manual clock adjustment
  handleLocalClockAdjustment(local_time);

  // Limit the frequency of queries to the NTP servers. Use a local time offset between update intervals
  if(local_time < (updateTime_ + updateInterval_))
  {
    return local_time + localTimeOffset_;
  }

  // Get the time from several NTP servers
  queryMultipleServers(reference_times);

  // Validate reference times
  if(isMatchingHours(reference_times))
  {
    localTimeOffset_ = reference_times[0] - local_time;
    updateTime_      = local_time;
  }

  fprintf(stderr, "[DEBUG] NTPClient::queryRandomNTPServer() NTP time = %ld.\n", local_time + localTimeOffset_);
  return local_time + localTimeOffset_;
}

time_t NTPClient::queryNTPServer(const char* ntpServer)
{
  boost::mutex::scoped_lock l(queryMutex_);

  const int  NTP_PACKET_SIZE  = 48;
  const int  XMIT_TS_INDEX    = 40;
  const char LEAP_INDICATOR   = 0;
  const char VERSION_NUMBER   = 4;
  const char MODE             = 3;
  const char FIRST_BYTE       = (LEAP_INDICATOR << 6) + (VERSION_NUMBER << 3) + MODE;

  time_t ntp_time = 0;

  try
  {
    boost::system::error_code ec;
    auto results = resolver_.resolve(udp::v4(), ntpServer, "ntp", ec);
    boost::asio::detail::throw_error(ec, "resolve");
    udp::endpoint             receiver = *results.begin();
    udp::endpoint             sender;
    boost::uint8_t            data[NTP_PACKET_SIZE];
    
    // Initialize the buffer
    memset(data, 0, NTP_PACKET_SIZE);
    data[0] = FIRST_BYTE;

    // Make sure the socket is open
    if(!socket_.is_open())
    {
      socket_.open(udp::v4(), ec);
      boost::asio::detail::throw_error(ec, "open");
    }

    // Request the time from the NTP server
    socket_.send_to(boost::asio::buffer(data), receiver);

    // Set a timeout for the async_receive_from operation.
    deadline_.expires_after(std::chrono::milliseconds(ntpTimeout_));
    deadline_.async_wait(boost::bind(&NTPClient::handleTimeout, this));

    // Listen for a response from the server.
    socket_.async_receive_from(boost::asio::buffer(data), sender, boost::bind(&NTPClient::done, this));

    // Wait until a response is recieved or the timeout duration is reached.
    io_context_.restart();
    io_context_.run();

    // Use the first 32 bits of the transmit timestamp in host byte order.
    ntp_time = ntohl(*(uint32_t*)&data[XMIT_TS_INDEX]);

    // Change the epoch from NTP to UNIX(1900 to 1970)
    if(uint32_t(ntp_time) > 0)
    {
      ntp_time -= uint32_t(SECONDS_1900_TO_1970);
    }

    fprintf(stderr, "[DEBUG] NTPClient::queryNTPServer() ntp_time = %d.\n", ntp_time);
  }
  catch(std::exception exc)
  {
    fprintf(stderr, "[ERROR] NTPClient::queryNTPServer() Caught Exception: %s, Server: %s\n", exc.what(), ntpServer);
    done();
  }
  catch(...)
  {
    fprintf(stderr, "[ERROR] NTPClient::queryNTPServer() Caught Exception. Server: %s\n", ntpServer);
    done();
  }

  return ntp_time;
}

NTPClient::NTPClient() : io_context_(),
  socket_(io_context_),
  resolver_(io_context_),
  deadline_(io_context_),
  randomMutex_(),
  queryMutex_(),
  updateTime_(0),
  localTimeOffset_(0),
  lastLocalTime_(0),
  updateInterval_(DEFAULT_UPDATE_INTERVAL),
  ntpTimeout_(DEFAULT_NTP_TIMEOUT),
  totalReferenceTimes_(DEFAULT_REFERENCE_TIMES)
{

}

char* NTPClient::generateServerName(char serverName[TIME_SERVER_NAME_LENGTH])
{
  const char country[2][4] = {".ca",".us"};
  const char* serverNameExt = ".pool.ntp.org";
  unsigned int number;

  srand (time(NULL));

  memset(serverName, '\0', TIME_SERVER_NAME_LENGTH);
  snprintf(serverName, sizeof(int), "%d", rand_r(&number) % 4);
  strcat(serverName, country[rand() % 2]);
  strcat(serverName, serverNameExt);

  return serverName;
}

void NTPClient::handleTimeout()
{
  fprintf(stderr, "[DEBUG] NTPClient::handleTimeout() A request to an NTP server timed out.\n");
  done();
}

void NTPClient::done()
{
  deadline_.cancel();
  deadline_.expires_at(std::chrono::steady_clock::time_point::max());
  io_context_.stop();
}

void NTPClient::handleLocalClockAdjustment(time_t localTime)
{
  if(abs(localTime - lastLocalTime_) > (SECONDS_PER_MINUTE * 30))
  {
    localTimeOffset_ += lastLocalTime_ - localTime;
    updateTime_      += lastLocalTime_ - localTime;
  }

  lastLocalTime_ = localTime;
}

bool NTPClient::isMatchingHours(time_t ntpTimes[MAX_REFERENCE_TIMES])
{
  // Return true if the times are all the same hour (minutes and seconds are ignored).

  for(int i = 0; i < totalReferenceTimes_; i++)
  {
    for(int j = 0; j < totalReferenceTimes_; j++)
    {
      if(  (ntpTimes[i] <= 0) 
        || (ntpTimes[j] <= 0) 
        || (((ntpTimes[i] % SECONDS_PER_DAY) / SECONDS_PER_HOUR) != ((ntpTimes[j] % SECONDS_PER_DAY) / SECONDS_PER_HOUR)))
      {
        return false;
      }
    }
  }

  return true;
}

void NTPClient::queryMultipleServers(time_t ntpTimes[MAX_REFERENCE_TIMES])
{
  char serverName[TIME_SERVER_NAME_LENGTH];

  for(int i = 0; i < totalReferenceTimes_; i++)
  {
    ntpTimes[i] = 0;
    // Keep trying random NTP servers until one responds with a valid time.
    while(ntpTimes[i] <= 0)
    {
      ntpTimes[i] = queryNTPServer(generateServerName(serverName));
    }
  }

}
