Build Instructions

==[On Windows]==

1. Install Qt5.10 with MinGW.
2. Install Gpg4Win
3. Open your command prompt (CMD or PowerShell) and change current directory to E2EEIM/Temp-Server/MultiServer2/, the Qt project folder that contains [PROJECT_NAME].pro.
4. Run qmake command (You need to set qmake PATH to [QT_INSTALLED_DIRECTORY]/5.10.0/MinGW/bin/qmake.exe before run qmake command).
5. Run mingw32-make (You need to set PATH to MinGW before run mingw32-make command).

The executable file will be in release folder.




==[On macOS]==

1. Install Qt5.10
2. Install GPGTools
3. Open your terminal command prompt and change current directory to E2EEIM/Temp-Server/MultiServer2/, the Qt project folder that contains [PROJECT_NAME].pro.
4. Run qmake command. qmake will generate Makefile (You need to set qmake PATH to [QT_INSTALLED_DIRECTORY]/bin/qmake before run qmake command). If qmake show any error occur, please check your Qt version first (qmake --version).
5. Run make command.

The executable file will be in [PROJECT_NAME].app/Contents/MacOS folder.




==[On LINUX]==

1. Install Qt5.10
2. Install GnuPG
3. Install GPGME library
4. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains [PROJECT_NAME].pro.
5. Run qmake command. qmake will generate Makefile (You need to set PATH to qmake from [QT_INSTALLED_DIRECTORY]/bin/qmake before run qmake command). If qmake show any error occur, please check your Qt version first (qmake --version).
6. Run make command.

The executable file will be in current directory.
