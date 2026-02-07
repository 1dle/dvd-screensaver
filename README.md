# DVD Screensaver
A classic Windows screensaver where a logo bounces around the screen — just like the old DVD players.
Supports any PNG image, as long as it’s embedded into the executable at build time.

> “It bounces around all day and sometimes it looks like it’s heading right into the corner of the screen and at the last minute hits the wall and bounces away.”  
> — Jim Halpert

![](docs/screenshot.png)

## Technologies
- Win32 API — screensaver integration and window management
- OpenGL — rendering
- stb_image — PNG loading
- Windows resource system — image embedded at compile time

Uses only libraries available on Windows.

## Features
- Smooth, time-based animation
- Multi-monitor support
- Old-school DVD screensaver vibes

## Future development ideas
- Configuration dialog (choose image, speed, colors, etc.)

## Build
You should be able to build and debug the project in Visual Studio after cloning the repository.

To build the project as a Windows screensaver, set the following option:
```text
Configuration Properties > Advanced > Target File Extension : .scr
```
To debug directly from Visual Studio in fullscreen mode:
```text
Configuration Properties > Advanced > Debugging > Command Arguments : -s
```

## Install

1. Download the latest `.scr` file from the release page.
3. Right-click and **Install**.

The screensaver will now appear in the Windows Screensaver Settings.
Select **DVD Screensaver** and apply.

### Windows SmartScreen

Windows may show a SmartScreen warning when running the screensaver
for the first time. This is expected for unsigned executables.

Click **More info** → **Run anyway** to continue.

## License

This project is licensed under the MIT License.
See the [LICENSE](LICENSE) file for details.
