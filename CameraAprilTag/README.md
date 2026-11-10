# Camera → AprilTag deployment

This deployment connects `CameraAprilTag.camera.imageCaptured` to
`CameraAprilTag.aprilTag.processImage`. Camera captures a JPEG, waits for
`rpicam-still` to exit successfully, and then sends its full file path.
AprilTag loads that file on its own task, detects tags, estimates poses, and
appends them to `poses.csv` in the deployment process's working directory.

The current camera source is a temporary bench version for a single directly
connected camera (`cameraId = 0`). It does not run the CamArray HAT's I2C
selection command. This is a source change, not a startup setting; see
"Restore the HAT version" below when changing hardware.

## Pull, build, and run on a Raspberry Pi

Use 64-bit Raspberry Pi OS with Python 3.10 or newer
([F Prime tools requirement](https://pypi.org/project/fprime-tools/4.4.0/)).
Build natively on the Pi; the Mac executable cannot run there.

For an existing checkout, run:

```bash
cd ~/apriltag_fprime
git pull --ff-only origin main
git submodule update --init lib/fprime lib/apriltag lib/camera-components
```

For a fresh checkout instead, run:

```bash
git clone https://github.com/rshirish25/apriltag_fprime.git
cd apriltag_fprime
git submodule update --init lib/fprime lib/apriltag lib/camera-components
```

The explicit submodule list avoids installing the camera repository's unused
nested F Prime checkout. The parent project selects the required camera commit.

Install dependencies on the Pi:

```bash
sudo apt update
sudo apt install -y git build-essential python3-dev python3-venv libopencv-dev rpicam-apps
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

Ensure `rpicam-still --list-cameras` detects your directly connected camera
([Raspberry Pi camera documentation](https://www.raspberrypi.com/documentation/accessories/camera-software.html)).
Captures use the default `rpicam-still` preview
for testing with the Pi connected to a display.

Build and launch the GDS on the Pi:

Regenerate the build cache after pulling because this deployment adds a library.
The `-f` option replaces generated build files; source files are retained.

```bash
cd CameraAprilTag
fprime-util generate -f
fprime-util build -j 2
mkdir -p ../build-artifacts/Linux/CameraAprilTag/bin/images
fprime-gds --framing-selection fprime --communication-selection tcp-fast-server --tcp-fast-address 127.0.0.1
```

If using SSH, open a separate terminal on your laptop and forward the GDS UI:

```bash
ssh -L 5000:127.0.0.1:5000 YOUR_PI_USER@YOUR_PI_HOST
```

Visit `http://localhost:5000` on the laptop. In the GDS send
`CameraAprilTag.camera.TAKE_PHOTO` with `cameraId = 0` and `filePath = images`.
The GDS launches the deployment with its binary directory as the working
directory, so images appear in `build-artifacts/Linux/CameraAprilTag/bin/images`
and poses in `build-artifacts/Linux/CameraAprilTag/bin/poses.csv`.

## General build and run

Install the project's existing dependencies (including OpenCV C++ development
libraries), then run from the project root:

```bash
source .venv/bin/activate
cd CameraAprilTag
fprime-util generate
fprime-util build -j 4
fprime-gds --framing-selection fprime --communication-selection tcp-fast-server
```

On the Pi, create a writable image directory and send this command from the GDS:

```text
CameraAprilTag.camera.TAKE_PHOTO
  cameraId: 0
  filePath: /path/to/images
```

`filePath` is an existing directory, not a filename. This bench version accepts
only `cameraId = 0` and invokes `rpicam-still` directly. It does not need `sudo`
or `i2cset` to capture an image.

Use a short directory argument (at most 40 characters, for example `images`):
the framework's current `Fw::CmdStringArg` limit is 40, even though the upstream
camera model advertises 256. The image-path port supports 512 characters, so the
directory plus generated filename can be longer than a command argument.

Each successful capture automatically queues AprilTag processing; no second
command is needed. Camera command success acknowledges capture and notification,
not completion of asynchronous detection. Failed captures send no image path.
Images are retained on disk and should not be removed until processing finishes.

For this minimal deployment, captures are triggered by `TAKE_PHOTO`. Periodic
capture scheduling is not added. The original `AprilTagTest` deployment and
`PROCESS_IMAGE` command remain available.

## Test without a Pi

After building, run from the project root:

```bash
python3 CameraAprilTag/test/capture_smoke.py \
  build-artifacts/Darwin/CameraAprilTag/bin/CameraAprilTag
```

The test runs the actual deployment with temporary fake `sudo` and
`rpicam-still` executables that copy a saved test image. It verifies completed
captures reach AprilTag, incomplete/failed captures produce no CSV rows, and
the manual image command still works. Everything runs in a temporary directory;
the project's existing `poses.csv` is not used. This tests the handoff and
processing, not physical camera operation. No GDS server is needed.

## Restore the HAT version

The direct-camera source change is isolated in camera commit
`a62f728`. To restore the original camera selection code, run
these commands from the project root:

```bash
cd lib/camera-components
git switch main
git pull --ff-only origin main
git revert a62f728
git push origin main
cd ../..
git add lib/camera-components
git commit -m "Restore CamArray HAT camera selection"
git push origin main
```

Then update the submodule and rebuild the deployment on the Pi. The restored
HAT code uses `sudo -n i2cset` on bus 10, device/register `0x24`; verify these
settings match the HAT being tested and install `i2c-tools` for that hardware.

## Pose calibration

`Components/AprilTag/AprilTag.cpp` still contains the original 320×240 test-image
intrinsics and 0.096 m tag size. Replace these with calibration for the real
camera's 640×480 capture mode and the physical tag size before interpreting
camera poses. No-tag images produce no pose rows. CSV/image errors are printed
to the deployment log.

## Source changes

The camera output port, shared `ProjectZero.ImageFile` port, and library manifest
live in the camera submodule. Commit those changes in that repository before
recording its new revision in the parent repository. Its existing standalone
project can also discover the shared port through `ProjectZero/Components`.
