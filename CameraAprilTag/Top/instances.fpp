module CameraAprilTag {
  module Default {
    constant QUEUE_SIZE = 10
    constant STACK_SIZE = 64 * 1024
  }
  instance aprilTag: Vision.AprilTag base id 0x10020000 \
    queue size 8 \
    stack size 256 * 1024 \
    priority 30
  instance camera: ProjectZero.CameraComponent base id 0x10030000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 30
  instance rateGroup: Svc.ActiveRateGroup base id 0x10001000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 35
  instance chronoTime: Svc.ChronoTime base id 0x10010000
  instance rateGroupDriver: Svc.RateGroupDriver base id 0x10011000
  instance timer: Svc.LinuxTimer base id 0x10013000
  instance comDriver: Drv.TcpClient base id 0x10014000
}
