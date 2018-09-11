Build Instructions

==[On Windows]==

1. Install Qt5 with MinGW.
2. Install Gpg4Win
3. Open your command prompt (CMD or PowerShell) and change current directory to E2EEIM/E2EEIM-Server, the Qt project folder that contains E2EEIM-Server.pro file.
4. Set gpgme.h INCLUDEPATH and libgpgme LIBS path in E2EEIM-Server.pro file.
5. Run qmake command (You need to set qmake PATH to [QT_INSTALLED_DIRECTORY]/[Qt5_version]/MinGW/bin/ before run qmake command).
6. Run mingw32-make (You need to set PATH to MinGW before run mingw32-make command).

The executable file will be in release folder.




==[On macOS]==

1. Install Qt5
2. Install GPGTools
3. Install Homebrew(https://www.brew.sh).
4. Install GPGME library (brew install gpgme).
6. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains E2EEIM-Server.pro file.
5. Set gpgme.h INCLUDEPATH and libgpgme LIBS path in E2EEIM-Server.pro file.
7. Run qmake command. qmake will generate Makefile (You need to set qmake PATH to [QT_INSTALLED_DIRECTORY]/bin/ before run qmake command). If qmake show any error occur, please check your Qt version first (qmake --version).
8. Run make command.

The executable file will be in E2EEIM-Server.app/Contents/MacOS folder.




==[On LINUX]==

1. Install Qt5
2. Install GnuPG
3. Install GPGME library
4. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains E2EEIM-Server.pro file.
4. Set gpgme.h INCLUDEPATH and libgpgme LIBS path in E2EEIM-Server.pro file.
5. Run qmake command. qmake will generate Makefile (You need to set PATH to qmake from [QT_INSTALLED_DIRECTORY]/bin/ before run qmake command). If qmake show any error occur, please check your Qt version first (qmake --version).
6. Run make command.

The executable file will be in current directory.


HOW TO RUN
1. Set firewall to allow port for E2EEIM-Server.
1. Set gpg-agent to remember passphrase as long as passible.
2. Find executable file.
3. Execute excecutable file.
