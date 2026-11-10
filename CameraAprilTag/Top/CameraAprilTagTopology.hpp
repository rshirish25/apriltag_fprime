#ifndef CAMERAAPRILTAG_TOPOLOGY_HPP
#define CAMERAAPRILTAG_TOPOLOGY_HPP

#include "CameraAprilTag/Top/CameraAprilTagTopologyDefs.hpp"

namespace CameraAprilTag {
void setupTopology(const TopologyState& state);
void startRateGroups();
void stopRateGroups();
void teardownTopology(const TopologyState& state);
}
#endif
