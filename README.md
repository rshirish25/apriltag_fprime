# AprilTag F Prime deployment

Clone the project with its F Prime and AprilTag source dependencies:

```bash
git clone --recurse-submodules https://github.com/rshirish25/apriltag_fprime.git
cd apriltag_fprime
```

For an existing checkout, initialize the dependencies with:

```bash
git submodule update --init --recursive
```

The submodules pin F Prime to `v4.4.0` and AprilTag to `v3.4.5`.
After pulling project updates, run `git submodule update --init --recursive`
again to use the recorded dependency versions.

System dependencies such as OpenCV and the Python packages in
`requirements.txt` must still be installed separately.
