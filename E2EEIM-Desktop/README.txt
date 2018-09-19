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
4. Open your terminal command prompt and change current directory to
   E2EEIM/E2EEIM-Desktop, the Qt project folder that contains the
   E2EEIM-Desktop.pro file.
5. Set gpgme.h INCLUDEPATH and libgpgme LIBS path in E2EEIM-Desktop.pro file in
   the section for 'unix:!osx'.  Note, however, that if your system headers
   are installed under /usr/include, then you should just remove the
   line in that section for INCLUDEPATH altogether.
6. Run the 'qmake' command.  The qmake tool will generate a Makefile for
   you.  You may need to set your PATH to find qmake if it is installed in a 
   non-standard location before you attempt to run the qmake command.
   If qmake shows any error messages, please verify you have installed
   Qt correctly.
7. Run the 'make' command to generate the executable, which will be
   named 'E2EEIM-Desktop'.

The executable file will be in current directory.


HOW TO RUN
1. Configure GUI pinentry for gpg-agent
    For macOS(GPGTools Pinentry mac)
    For Linux(pinentry-gtk2)
2. Configure gpg-agent to remember passphrases for as long as passible.
3. Find the name of the executable file.
4. Execute that excecutable file.
   On Linux, you would type:
./E2EEIM-Desktop
