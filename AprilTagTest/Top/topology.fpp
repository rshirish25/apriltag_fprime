module AprilTagTest {
  deployment topology AprilTagTest {
    import CdhCore.Subtopology
    import ComFprime.Subtopology
    instance aprilTag
    instance chronoTime
    instance rateGroup
    instance rateGroupDriver
    instance timer
    instance comDriver

    command connections instance CdhCore.cmdDisp
    event connections instance CdhCore.events
    telemetry connections instance CdhCore.tlmSend
    text event connections instance CdhCore.textLogger
    health connections instance CdhCore.$health
    time connections instance chronoTime

    connections Packets {
      CdhCore.events.PktSend -> ComFprime.comQueue.comPacketQueueIn[ComFprime.Ports_ComPacketQueue.EVENTS]
      CdhCore.tlmSend.PktSend -> ComFprime.comQueue.comPacketQueueIn[ComFprime.Ports_ComPacketQueue.TELEMETRY]
      ComFprime.fprimeRouter.commandOut -> CdhCore.cmdDisp.seqCmdBuff
      CdhCore.cmdDisp.seqCmdStatus -> ComFprime.fprimeRouter.cmdResponseIn
    }
    connections Communications {
      comDriver.allocate -> ComFprime.commsBufferManager.bufferGetCallee
      comDriver.deallocate -> ComFprime.commsBufferManager.bufferSendIn
      comDriver.$recv -> ComFprime.comStub.drvReceiveIn
      ComFprime.comStub.drvReceiveReturnOut -> comDriver.recvReturnIn
      ComFprime.comStub.drvSendOut -> comDriver.$send
      comDriver.ready -> ComFprime.comStub.drvConnected
    }
    connections Scheduling {
      timer.CycleOut -> rateGroupDriver.CycleIn
      rateGroupDriver.CycleOut[0] -> rateGroup.CycleIn
      rateGroup.RateGroupMemberOut[0] -> CdhCore.tlmSend.Run
      rateGroup.RateGroupMemberOut[1] -> ComFprime.comQueue.run
      rateGroup.RateGroupMemberOut[2] -> CdhCore.cmdDisp.run
      rateGroup.RateGroupMemberOut[3] -> CdhCore.$health.Run
      rateGroup.RateGroupMemberOut[4] -> ComFprime.commsBufferManager.schedIn
      rateGroup.RateGroupMemberOut[5] -> CdhCore.events.run
    }
  }
}
