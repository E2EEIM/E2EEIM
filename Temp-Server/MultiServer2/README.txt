Build Instructions

==[On Windows]==

1. Install Qt5.10 with MinGW.
2. Install Gpg4Win
3. Open your command prompt (CMD or PowerShell) and change current directory to E2EEIM/Temp-Server/MultiServer2/, the Qt project folder that contains [PROJECT_NAME].pro.
4. Run qmake command (You need to set qmake PATH to [QT5_INSTALLED_DIRECTORY]/[Qt_version]/MinGW/bin/ before run qmake command).
5. Run mingw32-make (You need to set PATH to MinGW before run mingw32-make command).

The executable file will be in release folder.

[Run E2EEIM-Server on Window]
1. Open you command prompt (CMD or PowerShell) and change current directory to directory that contain executable file "E2EEIM/Temp-Server/MultiServer2/release"
2. execute executable file (MultiServer2.exe)




==[On macOS]==

1. Install Qt5.10
2. Install GPGTools
3. Install Homebrew (https://www.brew.sh)
4. Install GPGME (brew install gpgme)
5. Open your terminal command prompt and change current directory to E2EEIM/Temp-Server/MultiServer2/, the Qt project folder that contains [PROJECT_NAME].pro.
6. Run qmake command. qmake will generate Makefile (You need to set qmake PATH to [QT5_INSTALLED_DIRECTORY]/bin/ before run qmake command). If qmake show any error occur, please check your Qt version first (qmake --version).
7. Run make command.

The executable file will be in [PROJECT_NAME].app/Contents/MacOS folder.

[Run E2EEIM-Server on macOS]
1. Open you command prompt (terminal) and change current directory to directory that contain executable file "E2EEIM/Temp-Server/MultiServer2/MultiServer2.app/Contents/MacOS"
2. execute executable file (./MultiServer2)




==[On LINUX]==

1. Install Qt5.10
2. Install GnuPG
3. Install GPGME library
4. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains [PROJECT_NAME].pro.
5. Run qmake command. qmake will generate Makefile (You need to set PATH to qmake from [QT5_INSTALLED_DIRECTORY]/bin/ before run qmake command). If qmake show any error occur, please check your Qt version first (qmake --version).
6. Run make command.

The executable file will be in current directory.


[Run E2EEIM-Server on LINUX]
1. Open you command prompt(terminal) and change current directory to directory that contain executable file "E2EEIM/Temp-Server/MultiServer2/"
2. execute executable file (./MultiServer2)



***NOTE***
1. In current progress server application will not save any data, if server application stop running every users data are gone.
2. Server application listen on fixed port 2222
