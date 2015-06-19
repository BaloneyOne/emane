/*
 * Copyright (c) 2015 - Adjacent Link LLC, Bridgewater, New Jersey
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of Adjacent Link LLC nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef EMANEMODELSTDMARECEIVEMANAGER_HEADER_
#define EMANEMODELSTDMARECEIVEMANAGER_HEADER_

#include "emane/types.h"
#include "emane/upstreampacket.h"
#include "emane/downstreamtransport.h"
#include "emane/radioserviceprovider.h"
#include "emane/logserviceprovider.h"
#include "emane/frequencysegment.h"
#include "emane/models/tdma/scheduler.h"
#include "emane/utils/randomnumberdistribution.h"

#include "pormanager.h"
#include "basemodelmessage.h"
#include "emane/models/tdma/packetstatuspublisher.h"

#include <tuple>

namespace EMANE
{
  namespace Models
  {
    namespace TDMA
    {
      class ReceiveManager
      {
      public:
        ReceiveManager(NEMId id,
                       DownstreamTransport * pDownstreamTransport,
                       LogServiceProvider * pLogService,
                       RadioServiceProvider * pRadioService,
                       Scheduler * pScheduler,
                       PacketStatusPublisher * pPacketStatusPublisher);

        void setFragmentCheckThreshold(const std::chrono::seconds & threshold);

        void setFragmentTimeoutThreshold(const std::chrono::seconds & threshold);
        
        void setPromiscuousMode(bool bEnable);
        
        void loadCurves(const std::string & sPCRFileName);
        
        bool enqueue(BaseModelMessage && baseModelMessage,
                     const PacketInfo & pktInfo,
                     size_t length,
                     const TimePoint & startOfReception,
                     const FrequencySegments & frequencySegments,
                     const Microseconds & span,
                     const TimePoint & beginTime);

        void process(std::uint64_t u64AbsoluteSlotIndex);
        
      private:
        NEMId id_;
        DownstreamTransport * pDownstreamTransport_;
        LogServiceProvider * pLogService_;
        RadioServiceProvider * pRadioService_;
        Scheduler * pScheduler_;
        PacketStatusPublisher * pPacketStatusPublisher_;
        
        using PendingInfo = std::tuple<BaseModelMessage,
                                       PacketInfo,
                                       size_t,
                                       TimePoint, //sor
                                       FrequencySegments,
                                       Microseconds, // span
                                       TimePoint>; 
        PendingInfo pendingInfo_;
        std::uint64_t u64PendingAbsoluteSlotIndex_;
        PORManager porManager_;
        Utils::RandomNumberDistribution<std::mt19937, 
                                        std::uniform_real_distribution<float>> distribution_;

        bool bPromiscuousMode_;
        std::chrono::seconds fragmentCheckThreshold_;
        std::chrono::seconds fragmentTimeoutThreshold_;

        using FragmentKey = std::pair<NEMId,std::uint64_t>;
        using FragmentParts = std::map<size_t,std::vector<std::uint8_t>>;
        using FragmentInfo = std::tuple<std::set<size_t>,
                                        FragmentParts,
                                        TimePoint, // last fragment time
                                        NEMId, // destination
                                        Priority>;
        using FragmentStore = std::map<FragmentKey,FragmentInfo>;
        using FragmentTimeStore = std::map<TimePoint,FragmentKey>;
        
        FragmentStore fragmentStore_;
        FragmentTimeStore fragmentTimeStore_;
        TimePoint lastFragmentCheckTime_;
        
        ReceiveManager(const ReceiveManager &) = delete;

        ReceiveManager & operator=(const ReceiveManager &) = delete;
      };
    }
  }
}

#endif // EMANEMODELSTDMARECEIVEMANAGER_HEADER_
