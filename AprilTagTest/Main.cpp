#include "AprilTagTest/Top/AprilTagTestTopology.hpp"
#include <Os/Os.hpp>
#include <Fw/Logger/Logger.hpp>
#include <signal.h>
#include <getopt.h>
#include <cstdlib>

static void signalHandler(int) {
    AprilTagTest::stopRateGroups();
}

int main(int argc, char* argv[]) {
    Os::init();
    AprilTagTest::TopologyState state = {};

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

    AprilTagTest::setupTopology(state);
    AprilTagTest::startRateGroups();
    AprilTagTest::teardownTopology(state);
    return 0;
}
