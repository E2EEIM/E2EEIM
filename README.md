# Build Instructions #
 ## On Windows ##
  1. Install Qt5.10 with MinGW.
  2. Install Gpg4Win
  3. Open your command prompt (CMD or PowerShell) and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains [PROJECT_NAME].pro.
  4. Run **qmake** command (You need to set **qmake** PATH to **[QT_INSTALLED_DIRECTORY]/5.10.0/MinGW/bin/qmake.exe** before run qmake command).
  5. Run **mingw32-make** (You need to set PATH to MinGW before run mingw32-make command).
  
The executable file will be in **release** folder.




## On macOS ##
  1. Install Qt5.10
  2. Install GPGTools
  3. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains [PROJECT_NAME].pro.
  4. Run **qmake** command. **qmake** will generate Makefile (You need to set **qmake** PATH to **[QT_INSTALLED_DIRECTORY]/bin/qmake** before run qmake command). If **qmake** show any error occur, please check your **Qt version** first (qmake --version).
  5. Run **make** command.
  
The executable file will be in **[PROJECT_NAME].app/Contents/MacOS** folder.




## On LINUX ##
  1. Install Qt5.10.
  2. Install GnuPG and GPGME
  3. Open your terminal command prompt and change current directory to E2EEIM/E2EEIM-Desktop, the Qt project folder that contains [PROJECT_NAME].pro.
  4. Run **qmake** command. **qmake** will generate Makefile (You need to set PATH to **qmake** from **[QT_INSTALLED_DIRECTORY]/bin/qmake** before run qmake command). If **qmake** show any error occur, please check your **Qt version**  first (qmake --version).
  5. Run **make** command.
  
The executable file will be in current directory.


# Note #
1. The application will create **userData** folder in the same directory that store the application excutable file. The application will create more files and folder inside **userData** folder are as follows.
     - When user sign in for the first time, the application will create folder, set folder name as **[USER_NAME]** inside **userData** directory and create **conversation** folder inside **userData/[USER_NAME]** folder.
     - First time, user click contact menu or group menu (icon in dark color bar), The application will create contactList.txt and groupList.txt file in **userData/[USER_NAME]** folder.
     - When user click any item on contact list, the application will create file for each contact and set file name as **[CONTACT_ID]** and stored in **userData/[USER_NAME]/conversation/**.
     
**userData will remove by make distclean**
    
2. **Sign in dialog only require USER_ID, server will send encrypted message to user to decrypt for vertify identidy when user sign in(when encryption part implemented).**

