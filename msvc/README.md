Building Tennix on Windows
==========================

It's now possible (again) to build Tennix for Windows.


Dependencies
------------

 * Visual Studio 2026 Community
 * Development libraries and headers:
  * `SDL2-devel-2.32.10-VC.zip`
  * `SDL2_gfx-1.0.4.tar.gz`
  * `SDL2_image-devel-2.8.12-VC.zip`
  * `SDL2_mixer-devel-2.8.2-VC.zip`
  * `SDL2_net-devel-2.4.0-VC.zip`
  * `SDL2_ttf-devel-2.24.0-VC.zip`

Slightly older and newer versions of these libraries
and headers should also work, but these exact versions
are the ones that building has been tested with.

Unpack all dependencies into `msvc\dependencies\all`, so that
`msvc\dependencies\all\include` contains ALL header files,
and `msvc\dependencies\all\lib\x64` contains ALL .dll and .lib
files for linking. These paths are set in the project files.


Building
--------

Double-click msvc\Tennix.slnx to open the solution in Visual
Studio. Make sure the configuration is set to "Release / x64"
and choose "Build / Build solution" from the menu. After the
build is finished, this should create (among other files)
two executables:

	msvc\x64\Release\ArchiveTool.exe
	msvc\x64\Release\Tennix.exe

Next, create "tennix.tnx" by running the ArchiveTool.exe with
the provided "pack.bat", e.g. in PowerShell within msvc\:

    .\pack.bat

This should create the file:

    msvc\x64\Release\tennix.tnx

Next, copy runtime dependencies (in PowerShell within msvc\):

    copy .\dependencies\all\lib\x64\*.dll .\x64\Release\

After that, run msvc\x64\Release\Tennix.exe to play the game.

