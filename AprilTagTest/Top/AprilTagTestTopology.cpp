#include "AprilTagTest/Top/AprilTagTestTopologyAc.hpp"

namespace AprilTagTest {

Svc::RateGroupDriver::DividerSet divisors{{{1, 0}}};
Svc::ActiveRateGroup::ContextArray context(0);

void setupTopology(const TopologyState& state) {
    initComponents(state);
    setBaseIds();
    connectComponents();
    regCommands();
    configComponents(state);
    rateGroupDriver.configure(divisors);
    rateGroup.configure(context);
    if (state.hostname != nullptr && state.port != 0) {
        comDriver.configure(state.hostname, state.port);
    }
    readParameters();
    loadParameters();
    startTasks(state);
    if (state.hostname != nullptr && state.port != 0) {
        Os::TaskString name("TcpReceive");
        comDriver.start(name, 34, Default::STACK_SIZE);
    }
}

void startRateGroups() {
    timer.startTimer(Fw::TimeInterval(1, 0));
}

void stopRateGroups() {
    timer.quit();
}

void teardownTopology(const TopologyState& state) {
    stopTasks(state);
    freeThreads(state);
    comDriver.stop();
    (void)comDriver.join();
    tearDownComponents(state);
    deinitComponents(state);
}

}
