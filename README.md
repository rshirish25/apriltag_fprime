# Minimal AprilTag F´ project

One `PROCESS_IMAGE` command processes one saved image:

**OpenCV load as grayscale → AprilTag detection
→ AprilTag pose estimation → append poses to CSV.**

The detector is initialized once and reused. The tag family is `tag36h11`,
with one detector thread and `quad_decimate = 1.0`. Each visible tag gets its
own pose. Image capture and timestamps are not included.

## Files

There are **18 project files and 3 test images** in this download. Dependencies,
generated code, and build outputs are separate. There are no setup or launcher
scripts.

| File | Purpose |
| --- | --- |
| `README.md` | Setup, run instructions, and library references. |
| `CMakeLists.txt` | Registers the project and links its libraries. |
| `settings.ini` | F´ project and framework paths. |
| `requirements.txt` | Python-based F´ build tools and GDS. |
| `.gitignore` | Excludes dependencies and generated output. |
| `Components/AprilTag/AprilTag.fpp` | One command taking an image path and standard command ports. |
| `Components/AprilTag/AprilTag.hpp` | Component declaration, detector pointers, and frame counter. |
| `Components/AprilTag/AprilTag.cpp` | All image processing, CSV writing, and memory cleanup. |
| `Components/AprilTag/CMakeLists.txt` | Builds the component. |
| `AprilTagTest/Main.cpp` | Starts and stops the deployment. |
| `AprilTagTest/CMakeLists.txt` | Builds the executable. |
| `AprilTagTest/Top/instances.fpp` | Component and standard service instances. |
| `AprilTagTest/Top/topology.fpp` | Command, communications, and service connections. |
| `AprilTagTest/Top/system.fpp` | System used to generate the dictionary. |
| `AprilTagTest/Top/AprilTagTestTopologyDefs.hpp` | Startup state required by the topology. |
| `AprilTagTest/Top/AprilTagTestTopology.hpp` | Startup and shutdown declarations. |
| `AprilTagTest/Top/AprilTagTestTopology.cpp` | Initializes and runs the standard services. |
| `AprilTagTest/Top/CMakeLists.txt` | Builds topology wiring and the dictionary. |

## Setup: Ubuntu/WSL or 64-bit Raspberry Pi OS

On Windows, run these commands inside Ubuntu/WSL. On a Pi, use 64-bit
Raspberry Pi OS. No camera is needed. Start in the extracted
`apriltag_fprime_simple` folder.

```bash
sudo apt update
sudo apt install build-essential git python3-venv python3-pip libopencv-dev libssl-dev zlib1g-dev

mkdir -p lib
git clone --depth 1 --branch v4.4.0 https://github.com/nasa/fprime.git lib/fprime
git clone --depth 1 --branch v3.4.5 https://github.com/AprilRobotics/apriltag.git lib/apriltag

python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
```

OpenCV is a C++ system library; installing `opencv-python` with pip does not
replace `libopencv-dev`.

## Build and run

From the project folder, with the virtual environment active:

```bash
cd AprilTagTest
fprime-util generate
fprime-util build -j2
cp -r ../images ../build-artifacts/Linux/AprilTagTest/bin/
fprime-gds --framing-selection fprime
```

On the Pi, use `fprime-util build -j1` to reduce build memory usage. Build on
the machine that will run the executable; do not copy a desktop executable to
the Pi. The installation path for a native Raspberry Pi OS build is also
`build-artifacts/Linux/AprilTagTest/`.

Open [http://127.0.0.1:5000](http://127.0.0.1:5000). GDS starts the executable
for you. The framing flag selects the protocol used by this deployment.
If using a Pi over SSH, forward the browser port from your computer:

```bash
ssh -L 5000:127.0.0.1:5000 YOUR_USER@PI_ADDRESS
```

Then open the same browser address on your computer. Stop the deployment
and GDS with Ctrl-C in their terminal.

### Process an image

In the GDS command page, select `AprilTagTest.aprilTag.PROCESS_IMAGE` and enter:

| Argument | Value |
| --- | --- |
| `imagePath` | `images/four_320x240.png` |

The output path, camera calibration, and tag size are hardcoded near the top of
`Components/AprilTag/AprilTag.cpp`: `CSV_PATH = "poses.csv"`, `FX = FY = 288`,
`CX = 160`, `CY = 120`, and `TAG_SIZE_METERS = 0.096`. These values match the
supplied 320x240 test images. Edit these constants and rebuild when changing
the output path, camera calibration, image resolution, or tag size. Restart GDS
and the deployment after rebuilding so both use the updated command interface.

Send the command once. It should append four pose rows for tag IDs 0–3.
Sending it again appends four more rows with the next frame number. GDS will
show the standard command dispatch/completion events; this component has no
custom events or telemetry.

GDS runs the executable from its installed `bin` directory. The image-copy
command above places the test images there so the short relative paths work.
The resulting file is:

```text
build-artifacts/Linux/AprilTagTest/bin/poses.csv
```

From another terminal in the project root, inspect it with:

```bash
head -n 5 build-artifacts/Linux/AprilTagTest/bin/poses.csv
```

For your own saved images, copy them into that `bin/images` directory and use
their short relative paths. The command's image path is limited to 40 characters;
the hardcoded CSV path is not subject to that command-string limit.

## Test images and calibration

| File | Expected IDs | fx | fy | cx | cy |
| --- | --- | --- | --- | --- | --- |
| `single_320x240.png` | 0 | 288 | 288 | 160 | 120 |
| `four_320x240.png` | 0, 1, 2, 3 | 288 | 288 | 160 | 120 |
| `four_640x480.png` | 0, 1, 2, 3 | 576 | 576 | 320 | 240 |

Use `TAG_SIZE_METERS = 0.096` for these tests. For the 640x480 image, change
`FX = FY = 576`, `CX = 320`, and `CY = 240` in `AprilTag.cpp` and rebuild.
These are valid rendered tags
with synthetic calibration values for testing the pipeline, not a physical
accuracy validation. For real images, use the camera intrinsics at the image's
resolution and the measured side of the tag's black border, excluding the
outer white margin. AprilTag's pose estimator does not accept distortion
coefficients; provide already-undistorted images and their corresponding
intrinsics when distortion matters. Undistortion is not included here.

## CSV

```text
frame,tag_id,tx,ty,tz,r11,r12,r13,r21,r22,r23,r31,r32,r33
```

Translation is in metres. The matrix is written row by row and maps tag
coordinates into camera coordinates:

```text
p_camera = R * p_tag + t
```

Camera axes are x right, y down, and z forward through the lens. The tag
frame follows AprilTag's convention. Four tags produce four independent
tag poses; this does not combine them into a single Nodesat pose.

The file is opened in append mode and receives a header only when empty.
Frame numbering starts at zero and resets when the deployment restarts; use
a new CSV filename for each run if frame numbers need to be unique. An image
with no detections writes no pose rows. No timestamp is added.

## Scope

As requested, the component assumes valid inputs and a writable CSV path.
There is no custom error handling, input validation, failure enum, or extra
telemetry. A bad input can stop processing or the application; CSV write
failures are not reported. Use an output path different from the input image
and send a command after the image is fully saved. Memory cleanup remains
so repeated commands release their detection and pose results.

The standard F´ service topology retains its own normal diagnostics and
error handling. Only the custom AprilTag component has been reduced.

This project does not measure RAM. The earlier desktop benchmark uses a
different image loader and conversion implementation; switching this version
to OpenCV changes library overhead and means those total-RAM figures are not
an exact match.

For flight integration later, reuse `Components/AprilTag/`, register and link
the libraries, add an instance and command connections to the flight topology,
and trigger the command after a saved image is ready. This standalone deployment
does not add automatic capture, repeated processing, or start/stop commands.

## Open-source functions and versions

The algorithms are called directly from the upstream libraries. The custom
code supplies the F´ command handler, connects the pixel buffer, and writes CSV;
it does not implement a detector, grayscale algorithm, or pose solver.

- [F´ v4.4.0](https://github.com/nasa/fprime/tree/v4.4.0), commit
  `1a83bb878be673f405d5de3c0cc5ed37b5fc3596`. Deployment setup is reduced from
  the standard F´ scaffolding. [F´ license](https://github.com/nasa/fprime/blob/v4.4.0/LICENSE.txt).
- [AprilTag v3.4.5](https://github.com/AprilRobotics/apriltag/tree/v3.4.5), commit
  `94be783968e5091bcc9972c72c84fd63efce2935`. Detector setup and the grayscale
  buffer connection follow the [OpenCV example](https://github.com/AprilRobotics/apriltag/blob/v3.4.5/example/opencv_demo.cc).
  Pose estimation uses the [documented `estimate_tag_pose` call](https://github.com/AprilRobotics/apriltag/tree/v3.4.5#pose-estimation).
  [AprilTag license](https://github.com/AprilRobotics/apriltag/blob/v3.4.5/LICENSE.md).
- [OpenCV](https://github.com/opencv/opencv): `cv::imread` with `cv::IMREAD_GRAYSCALE`.
  OpenCV is installed separately through the system
  package manager. [OpenCV license](https://github.com/opencv/opencv/blob/4.x/LICENSE).

The dependencies are downloaded separately and keep their upstream notices.

## Validation

The one-argument command and hardcoded settings were built and tested under
Ubuntu/WSL on x86-64 through headless GDS and `fprime-cli`. A single-tag image
followed by two four-tag images produced nine pose rows, frame numbers 0–2,
the expected tag IDs, and one CSV header. Poses were finite with positive
depth and orthogonal rotation matrices. Test output was written to a temporary
directory, leaving the existing `poses.csv` untouched.

The original command interface was built and tested on Linux x86-64 using
F´ v4.4.0, AprilTag v3.4.5, and OpenCV
4.6.0. Commands sent through the official headless GDS and `fprime-cli`
processed all three images and a repeated four-tag image. The four commands
produced 13 pose rows, the expected IDs, frame numbers 0–3, and one CSV header.
Translations were finite with positive depth; rotation matrices were orthogonal
with determinant +1. The test used GDS's `--no-zmq` TCP option. The browser GUI
and Raspberry Pi hardware have not been tested here.
