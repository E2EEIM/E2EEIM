Build Instructions

==[On Windows]==

1. Install Qt5.10 with MinGW.
2. Install Gpg4Win
3. Open your command prompt (CMD or PowerShell) and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains [PROJECT_NAME].pro.
4. Run qmake command (You need to set qmake PATH to [QT_INSTALLED_DIRECTORY]/[QT_VERSION]/MinGW/bin/qmake.exe before run qmake command).
5. Run mingw32-make (You need to set PATH to MinGW before run mingw32-make command).

The executable file will be in release folder.

[Run E2EEIM-Desktop on Window]
1. Open you command prompt (CMD or PowerShell) and change current directory to directory that contain executable file "E2EEIM/E2EEIM-Desktop/release"
2. execute executable file (E2EEIM-Desktop.exe)





==[On macOS]==

1. Install Qt5.10
2. Install GPGTools
3. Install Homebrew (https://www.brew.sh)
4. Install GPGME (brew install gpgme)
5. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains [PROJECT_NAME].pro.
6. Run qmake command. qmake will generate Makefile (You need to set qmake PATH to [QT5_INSTALLED_DIRECTORY]/bin/ before run qmake command). If qmake show any error occur, please check your Qt version first (qmake --version).
7. Run make command.

The executable file will be in [PROJECT_NAME].app/Contents/MacOS folder.

[Run E2EEIM-Desktop on macOS]
1. Open you command prompt(Terminal) and change current directory to directory that contain executable file "E2EEIM/E2EEIM-Desktop/E2EEIM-Desktop.app/Contents/MacOS"
2. execute executable file (./E2EEIM-Desktop)





==[On LINUX]==

1. Install Qt5.10
2. Install GnuPG
3. Install GPGME library
4. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains [PROJECT_NAME].pro.
5. Run qmake command. qmake will generate Makefile (You need to set PATH to qmake from [QT5_INSTALLED_DIRECTORY]/bin/ before run qmake command). If qmake show any error occur, please check your Qt version first (qmake --version).
6. Run make command.

The executable file will be in current directory.

[Run E2EEIM-Desktop on LINUX]
1. Open you command prompt(Terminal) and change current directory to directory that contain executable file "E2EEIM/E2EEIM-Desktop/"
2. execute executable file (./E2EEIM-Desktop)
