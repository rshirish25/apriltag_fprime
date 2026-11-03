#ifndef APRILTAGTEST_TOPOLOGY_HPP
#define APRILTAGTEST_TOPOLOGY_HPP

#include "AprilTagTest/Top/AprilTagTestTopologyDefs.hpp"

namespace AprilTagTest {
void setupTopology(const TopologyState& state);
void startRateGroups();
void stopRateGroups();
void teardownTopology(const TopologyState& state);
}
#endif
