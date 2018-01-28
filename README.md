# Build Instructions
  On Windows
  1. install Qt5.10 with MinGW and MSVC.
  2. Open your command prompt (CMD or PowerShell) and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains [PROJECT_NAME].pro.
  3. run <b>qmake</B> command (You need to set <b>qmake</b> PATH to <b>[QT_INSTALLED_DIRECTORY]/5.10.0/[COMPILER]/bin/qmake.exe</b> before run qmake command).
  4. run <b>mingw32-make</b> (You need to set PATH to MinGW before run mingw32-make command).
  
The executable file will be in <b> release </b> folder.
<br><br>

On macOS
  1. install Qt5.10 with Clang 7.0.
  2. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains [PROJECT_NAME].pro.
  3. run <b>qmake</b> command. <b>qmake</b> will generate Makefile (You need to set <b>qmake</b> PATH to <b>[QT_INSTALLED_DIRECTORY]/[COMPILER]/bin/qmake</b>). If <b>qmake</b> show any occur error, please check your <b>Qt version</b> first (qmake --version).
  4. run <b>make</b> command.
  
The executable file will be in <b> [PROJECT_NAME].app/Contents/MacOS </b> folder.
<br><br>

On LINUX
  1. install Qt5.10.
  2. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains [PROJECT_NAME].pro.
  3. run <b>qmake</b> command. <b>qmake</b> will generate Makefile (You need to set PATH to <b>qmake</b> from <b>[QT_INSTALLED_DIRECTORY]/[COMPILER]/bin/qmake</b>). If <b>qmake</b> show any occur error, please check your <b>Qt version</b>  first (qmake --version).
  4. run <b>make</b> command.
  
The executable file will be in current directory.
