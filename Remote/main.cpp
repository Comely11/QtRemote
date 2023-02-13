#include "mainwindow.h"
#include <QApplication>

/*
int DeleteLocalFile()
{
    //TODO:
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);//拿到路径
    TCHAR sPath[MAX_PATH] = _T("");
    //mbstowcs(sPath, strPath.c_str(), strPath.size());//多字节转换成宽字节（容易乱码）
    MultiByteToWideChar(CP_ACP,0,strPath.c_str(),strPath.size(),sPath,sizeof(strPath)/sizeof(TCHAR));//处理字符
    DeleteFileA(strPath.c_str());
    CPacket pack(9, NULL, 0);
    bool ret = CServerSocket::getInstance()->Send(pack);
    TRACE("Send ret=%d\r\n", ret);
    return 0;
}*/
int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);
    fflush(stdout);
    QApplication a(argc, argv);


    MainWindow w;
     w.show();
    return a.exec();
}
