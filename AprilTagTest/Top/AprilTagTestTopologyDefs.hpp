#ifndef APRILTAGTEST_TOPOLOGYDEFS_HPP
#define APRILTAGTEST_TOPOLOGYDEFS_HPP

#include "Svc/Subtopologies/CdhCore/PingEntries.hpp"
#include "Svc/Subtopologies/ComFprime/PingEntries.hpp"
#include "Svc/Subtopologies/CdhCore/SubtopologyTopologyDefs.hpp"
#include "Svc/Subtopologies/ComFprime/SubtopologyTopologyDefs.hpp"
#include "Svc/Subtopologies/ComFprime/Ports_ComPacketQueueEnumAc.hpp"
#include "Svc/Subtopologies/ComFprime/Ports_ComBufferQueueEnumAc.hpp"
#include "AprilTagTest/Top/FppConstantsAc.hpp"

namespace PingEntries {
namespace AprilTagTest_rateGroup { enum { WARN = 3, FATAL = 5 }; }
}

namespace AprilTagTest {
struct TopologyState {
    const char* hostname;
    U16 port;
    CdhCore::SubtopologyState cdhCore;
    ComFprime::SubtopologyState comFprime;
};
namespace PingEntries = ::PingEntries;
}
#endif
