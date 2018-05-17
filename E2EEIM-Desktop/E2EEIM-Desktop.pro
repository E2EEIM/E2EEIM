#-------------------------------------------------
#
# Project created by QtCreator 2018-01-24T07:18:05
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = E2EEIM-Desktop
TEMPLATE = app


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#CONFIG+=sanitizer sanitize_address


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    addcontact.cpp \
    creategroup.cpp \
    signin.cpp \
    setting.cpp \
    connection.cpp \
    encryption.cpp

HEADERS += \
        mainwindow.h \
    addcontact.h \
    creategroup.h \
    signin.h \
    setting.h \
    connection.h \
    encryption.h

FORMS += \
        mainwindow.ui \
    addcontact.ui \
    creategroup.ui \
    signin.ui \
    setting.ui

RESOURCES += \
    res.qrc

win32{
    INCLUDEPATH += "C:\Program Files (x68)\Gpg4win\include"
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

extraclean.commands =  rm -f 2.payload 2payload.encrypted E2EEIM-Desktop.pro.user .qmake.stash servPubKey.key signUpResult.cipher signUpResult.txt userPublicKey.key signIn.payload signIn.epkg signInRan.cipher signInRan.txt signInResult.cipher signInResult.txt; rm -rf userData
distclean.depends = extraclean
QMAKE_EXTRA_TARGETS += distclean extraclean
