#ifndef CAMERAAPRILTAG_TOPOLOGYDEFS_HPP
#define CAMERAAPRILTAG_TOPOLOGYDEFS_HPP

#include "Svc/Subtopologies/CdhCore/PingEntries.hpp"
#include "Svc/Subtopologies/ComFprime/PingEntries.hpp"
#include "Svc/Subtopologies/CdhCore/SubtopologyTopologyDefs.hpp"
#include "Svc/Subtopologies/ComFprime/SubtopologyTopologyDefs.hpp"
#include "Svc/Subtopologies/ComFprime/Ports_ComPacketQueueEnumAc.hpp"
#include "Svc/Subtopologies/ComFprime/Ports_ComBufferQueueEnumAc.hpp"
#include "CameraAprilTag/Top/FppConstantsAc.hpp"

namespace PingEntries {
namespace CameraAprilTag_rateGroup { enum { WARN = 3, FATAL = 5 }; }
}

namespace CameraAprilTag {
struct TopologyState {
    const char* hostname;
    U16 port;
    CdhCore::SubtopologyState cdhCore;
    ComFprime::SubtopologyState comFprime;
};
namespace PingEntries = ::PingEntries;
}
#endif
