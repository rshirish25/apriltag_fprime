#include "CameraAprilTag/Top/CameraAprilTagTopology.hpp"
#include <Os/Os.hpp>
#include <Fw/Logger/Logger.hpp>
#include <signal.h>
#include <getopt.h>
#include <cstdlib>

static void signalHandler(int) {
    CameraAprilTag::stopRateGroups();
}

int main(int argc, char* argv[]) {
    Os::init();
    CameraAprilTag::TopologyState state = {};

    int option;
    while ((option = getopt(argc, argv, "a:p:")) != -1) {
        switch (option) {
            case 'a': state.hostname = optarg; break;
            case 'p': state.port = static_cast<U16>(std::atoi(optarg)); break;
            default: return 1;
        }
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    Fw::Logger::log("Hit Ctrl-C to quit\n");

    CameraAprilTag::setupTopology(state);
    CameraAprilTag::startRateGroups();
    CameraAprilTag::teardownTopology(state);
    return 0;
}
