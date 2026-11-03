QT += widgets network sql multimedia multimediawidgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    adduseritem.cpp \
    applyfrienddialog.cpp \
    applyfrienditem.cpp \
    applyfriendlist.cpp \
    applyfriendpage.cpp \
    bubbleframe.cpp \
    chatdialog.cpp \
    chatitembase.cpp \
    chatpage.cpp \
    chatuseritem.cpp \
    chatuserlist.cpp \
    chatview.cpp \
    clickbutton.cpp \
    clicklabel.cpp \
    clickoncelabel.cpp \
    contactuseritem.cpp \
    contactuserlist.cpp \
    customizeedit.cpp \
    customizefilelabel.cpp \
    filetcpmgr.cpp \
    findresultdialog.cpp \
    friendlabel.cpp \
    global.cpp \
    grouptipitem.cpp \
    httpmgr.cpp \
    imagecropperlabel.cpp \
    listitembase.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
    messagedelegate.cpp \
    messagemodel.cpp \
    messagetextedit.cpp \
    picturebubble.cpp \
    redpointlabel.cpp \
    registerdialog.cpp \
    resetdialog.cpp \
    searchlist.cpp \
    sqlmgr.cpp \
    tcpmgr.cpp \
    textbubble.cpp \
    timerbtn.cpp \
    toast.cpp \
    userdata.cpp \
    usermgr.cpp \
    videoplayerdialog.cpp

HEADERS += \
    Singleton.h \
    adduseritem.h \
    applyfrienddialog.h \
    applyfrienditem.h \
    applyfriendlist.h \
    applyfriendpage.h \
    bubbleframe.h \
    chatdialog.h \
    chatitembase.h \
    chatpage.h \
    chatuseritem.h \
    chatuserlist.h \
    chatview.h \
    clickbutton.h \
    clicklabel.h \
    clickoncelabel.h \
    contactuseritem.h \
    contactuserlist.h \
    customizeedit.h \
    customizefilelabel.h \
    filetcpmgr.h \
    findresultdialog.h \
    friendlabel.h \
    global.h \
    grouptipitem.h \
    httpmgr.h \
    imagecropperdialog.h \
    imagecropperlabel.h \
    listitembase.h \
    logindialog.h \
    mainwindow.h \
    messagedelegate.h \
    messagemodel.h \
    messagetextedit.h \
    picturebubble.h \
    redpointlabel.h \
    registerdialog.h \
    resetdialog.h \
    searchlist.h \
    sqlmgr.h \
    tcpmgr.h \
    textbubble.h \
    timerbtn.h \
    toast.h \
    userdata.h \
    usermgr.h \
    videoplayerdialog.h

FORMS += \
    adduseritem.ui \
    applyfrienddialog.ui \
    applyfrienditem.ui \
    chatdialog.ui \
    chatpage.ui \
    chatuseritem.ui \
    contactuseritem.ui \
    findresultdialog.ui \
    friendlabel.ui \
    grouptipitem.ui \
    logindialog.ui \
    mainwindow.ui \
    registerdialog.ui \
    resetdialog.ui \
    videoplayerdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    config.ini

CONFIG(debug, debug | release) {
    #指定要拷贝的文件目录为工程目录下release目录下的所有dll、lib文件，例如工程目录在D:\QT\Test
    #PWD就为D:/QT/Test，DllFile = D:/QT/Test/release/*.dll
    TargetConfig = $${PWD}/config.ini
    #将输入目录中的"/"替换为"\"
    TargetConfig = $$replace(TargetConfig, /, \\)
    #将输出目录中的"/"替换为"\"
    OutputDir =  $${OUT_PWD}/$${DESTDIR}
    OutputDir = $$replace(OutputDir, /, \\)
    //执行copy命令
    QMAKE_POST_LINK += copy /Y \"$$TargetConfig\" \"$$OutputDir\" &
    # 首先，定义static文件夹的路径
    StaticDir = $${PWD}/static
    # 将路径中的"/"替换为"\"
    StaticDir = $$replace(StaticDir, /, \\)
    #message($${StaticDir})
    # 使用xcopy命令拷贝文件夹，/E表示拷贝子目录及其内容，包括空目录。/I表示如果目标不存在则创建目录。/Y表示覆盖现有文件而不提示。
    QMAKE_POST_LINK += xcopy /Y /E /I \"$$StaticDir\" \"$$OutputDir\\static\\\"
}else{
      #release
    message("release mode")
    #指定要拷贝的文件目录为工程目录下release目录下的所有dll、lib文件，例如工程目录在D:\QT\Test
    #PWD就为D:/QT/Test，DllFile = D:/QT/Test/release/*.dll
    TargetConfig = $${PWD}/config.ini
    #将输入目录中的"/"替换为"\"
    TargetConfig = $$replace(TargetConfig, /, \\)
    #将输出目录中的"/"替换为"\"
    OutputDir =  $${OUT_PWD}/$${DESTDIR}
    OutputDir = $$replace(OutputDir, /, \\)
    //执行copy命令
    QMAKE_POST_LINK += copy /Y \"$$TargetConfig\" \"$$OutputDir\"
    # 首先，定义static文件夹的路径
    StaticDir = $${PWD}/static
    # 将路径中的"/"替换为"\"
    StaticDir = $$replace(StaticDir, /, \\)
    #message($${StaticDir})
    # 使用xcopy命令拷贝文件夹，/E表示拷贝子目录及其内容，包括空目录。/I表示如果目标不存在则创建目录。/Y表示覆盖现有文件而不提示。
     QMAKE_POST_LINK += xcopy /Y /E /I \"$$StaticDir\" \"$$OutputDir\\static\\\"
}
win32-msvc*:QMAKE_CXXFLAGS += /wd"4819" /utf-8