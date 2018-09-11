Build Instructions

==[On Windows]==

1. Install Qt5 with MinGW.
2. Install Gpg4Win
3. Open your command prompt (CMD or PowerShell) and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains E2EEIM-Desktop.pro file.
4. Set gpgme.h INCLUDEPATH and libgpgme LIBS path in E2EEIM-Desktop.pro file.
5. Run qmake command (You need to set qmake PATH to [QT_INSTALLED_DIRECTORY]/[Qt5_version]/MinGW/bin/ before run qmake command).
6. Run mingw32-make (You need to set PATH to MinGW before run mingw32-make command).

The executable file will be in release folder.




==[On macOS]==

1. Install Qt5
2. Install GPGTools
3. Install Homebrew(https://www.brew.sh).
4. Install GPGME library (brew install gpgme).
6. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains E2EEIM-Desktop.pro file.
5. Set gpgme.h INCLUDEPATH and libgpgme LIBS path in E2EEIM-Desktop.pro file.
7. Run qmake command. qmake will generate Makefile (You need to set qmake PATH to [QT_INSTALLED_DIRECTORY]/bin/ before run qmake command). If qmake show any error occur, please check your Qt version first (qmake --version).
8. Run make command.

The executable file will be in E2EEIM-Desktop.app/Contents/MacOS folder.




==[On LINUX]==

1. Install Qt5
2. Install GnuPG
3. Install GPGME library
5. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains E2EEIM-Desktop.pro file.
4. Set gpgme.h INCLUDEPATH and libgpgme LIBS path in E2EEIM-Desktop.pro file.
6. Run qmake command. qmake will generate Makefile (You need to set PATH to qmake from [QT_INSTALLED_DIRECTORY]/bin/ before run qmake command). If qmake show any error occur, please check your Qt version first (qmake --version).
7. Run make command.

The executable file will be in current directory.


HOW TO RUN
1. Set GUI pinentry for gpg-agent
    For macOS(GPGTools Pinentry mac)
    For Linux(pinentry-gtk2)
2. Set gpg-agent to remember passphrase as long as passible.
3. Find executable file.
4. Execute excecutable file.
