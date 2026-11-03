#ifndef VISION_APRILTAG_HPP
#define VISION_APRILTAG_HPP

#include "Components/AprilTag/AprilTagComponentAc.hpp"
#include "apriltag.h"

namespace Vision {

class AprilTag final : public AprilTagComponentBase {
  public:
    explicit AprilTag(const char* name);
    ~AprilTag() override;

  private:
    void PROCESS_IMAGE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq,
                                  const Fw::CmdStringArg& imagePath) override;

    apriltag_family_t* m_family;
    apriltag_detector_t* m_detector;
    U64 m_frame = 0;
};

}
#endif
