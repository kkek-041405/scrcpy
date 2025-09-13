# Copilot instructions for this repo

This repo builds scrcpy: a client/server tool to mirror and control Android devices. The client (C, desktop) pushes and runs a Java server (Android) and exchanges video, audio, and control streams over sockets via an adb tunnel.

## Architecture and data flow
- Two parts: client binary `scrcpy` (in `app/`) and Android server jar (in `server/`).
- Up to 3 sockets: video, audio, control. Each has a dedicated thread on both sides; control is bidirectional.
- Default tunnel uses adb reverse (device connects to the host). Forward can be forced with `--force-adb-forward`.
- The first socket sends device metadata (name). Video/audio send codec metadata then framed packets. Control uses a custom binary protocol.
- Ports: default client local port range 27183–27199 (see `app/meson.build`).

Key client modules (C, see `app/src/`):
- CLI/options: `cli.c`, `options.c`
- ADB & server bootstrap: `server.c`, `adb/*`
- Streams: `demuxer.c`, `decoder.c`, `display.c`, `audio_player.c`, `recorder.c`
- Control path: `input_manager.c` → queue → `controller.c` → `control_msg.c`
- Device messages from server: `device_msg.c`

Key server modules (Java, see `server/src/`):
- Entry & options: `Server.java`, `Options.java`
- Streaming: `ScreenEncoder.java`, `AudioCapture.java`, `AudioEncoder.java`
- Control: `Controller.java`, wrappers under `wrappers/` (hidden API via reflection)

## Build, run, test
- Build client and server from source (Android SDK/Java required):
  - Configure: `meson setup x --buildtype=release --strip -Db_lto=true`
  - Build: `ninja -Cx` (do not run as Administrator/root)
- Use prebuilt server (no Java/SDK):
  - Configure: `meson setup x --buildtype=release --strip -Db_lto=true -Dprebuilt_server=/path/to/scrcpy-server`
  - Build: `ninja -Cx`
- Run from build dir: `./run x [options]` (works on Windows/MSYS2, Linux, macOS)
- Install: `ninja -Cx install` (no sudo on Windows)
- Windows specifics: prefer MSYS2 toolchain; Java 17 is not in MSYS2, so either set `JAVA_HOME` in PATH or use `-Dprebuilt_server=...` (see `doc/windows.md`).
- Tests: only built in debug. Configure with `--buildtype=debug`, then `ninja -Cx`, run `meson test -Cx`. Notable tests live in `app/tests/` (e.g., `test_control_msg_serialize.c`, `test_device_msg_deserialize.c`).

## Conventions and patterns (important when changing behavior)
- Client/server versions must match; server is started like:
  `adb shell CLASSPATH=/data/local/tmp/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server <version> key=value ...` (see `doc/develop.md`).
- New CLI options: define in `options.c` and parse in `cli.c`. If the option affects the server, add a key in `server/src/.../Options.java` and pass it from client to server in `server.c`.
- New control message or device message: update both sides and serialization tests:
  - Client: `control_msg.[ch]` or `device_msg.[ch]`
  - Server: reader/writer in Java (`ControlMessageReaderTest`, `DeviceMessageWriterTest` reference behavior)
- Threading: avoid I/O on the main thread; use queues (see `controller.c`).
- Networking: default uses adb reverse; if forward is used, the server sends a dummy byte on the first socket to detect connection errors.

## Debugging the Android server
- Configure with `-Dserver_debugger=true`, rebuild, run.
- Forward JDWP and attach from Android Studio (see steps in `doc/develop.md`, "Debug the server").

## Where to look for examples
- CLI parsing: `app/src/cli.c` (short/long flags), `app/src/options.c`
- Protocol framing: `app/src/demuxer.c`, server `Streamer.java`
- Display/audio: `app/src/display.c`, `app/src/audio_player.c`
- ADB startup/tunnels: `app/src/server.c`, `app/src/adb/*`

Notes
- Upstream docs mention branches `master` (release) and `dev` (development) in `doc/build.md`. Confirm target branch for PRs in this fork before opening a change.
- Read `doc/develop.md` and `doc/build.md` for deeper details and OS-specific instructions.