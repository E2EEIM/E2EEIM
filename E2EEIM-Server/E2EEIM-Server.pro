QT -= gui
QT += network

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG+=debug
CONFIG+=sanitizer sanitize_address

SOURCES += \
        main.cpp \
    server.cpp \
    clienttask.cpp \
    clientthread.cpp

HEADERS += \
    server.h \
    clienttask.h \
    clientthread.h \
    encryption.h

win32{
    INCLUDEPATH += "C:\Program Files (x86)\Gpg4win\include"
    LIBS += "C:\Program Files (x86)\Gpg4win\lib\libgpgme.imp"
}

unix:!osx {
    INCLUDEPATH += /usr/local/include
    LIBS += -lgpgme
}

osx{
    INCLUDEPATH += /usr/local/include
    LIBS += /usr/local/lib/libgpgme.11.dylib
}

extraclean.commands =  rm -f serverPubKey.key temp.cipher signUp.pgp signUpResult.ciper \
 temp.data temp.cipher signInVerify.pgp signInResult.cipher signIn.pgp rannum.cipher \
addFriendCon.cipher searchUser.cipher username.keyword .qmake.stash temp.key key.cipher; #gpg --delete-secret-keys server@e2eeim.chat; gpg --delete-keys server@e2eeim.chat;
distclean.depends = extraclean
QMAKE_EXTRA_TARGETS += distclean extraclean
