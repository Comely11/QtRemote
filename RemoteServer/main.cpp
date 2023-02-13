#include <QCoreApplication>
#include"serversocket.h"


#include <stdio.h>
#include<io.h>
#include <list>
#include "direct.h"


void Dump(BYTE* pData, size_t nSize)
{
    std::string strOut;
    printf("SendData:");
    for (size_t i = 0; i < nSize; i++)
    {
        char buf[8] = "";
        if (i > 0 && (i % 16 == 0))strOut += "\n";
        snprintf(buf, sizeof(buf), " %02X ", pData[i] & 0xFF);
        strOut += buf;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
}
//要创建一个磁盘分区的信息
int  MakeDriverInfo()
{
   std::string result;
    //driver是从1开始的值 1A 2B 3C 4D 代表这硬盘的盘符
    //改变当前的驱动 如果能改变成功说明这个驱动是存在的，否则不存在。
   for (int i = 1; i <= 26; i++)
   {
       if (_chdrive(i) == 0)//返回值为0说明切换是成功的，证明磁盘存在
       {
           if (result.size() > 0)
           {
               result += ',';
           }
           result += 'A' + i - 1;
       };

   }
   result += ',';
    CPacket pack(1,(BYTE*)result.c_str(), result.size()); //打包使用
    Dump((BYTE*)pack.Data(),pack.Size());
   CServerSocket::getInstance()->Send(pack);
    return 0;
}
int MakeDirectoryInfo()
{
    std::list<FILEINFO>ListFileInfos;
    std::string strPath;
    if(CServerSocket::getInstance()->GetFilePath(strPath)==false)
    {
        qDebug()<<"当前的命令，不是获取文件列表，命令解析错误";
        return -1;
    }
    if (_chdir(strPath.c_str())!=0)
    {
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
        //ListFileInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
         qDebug()<<"没有权限,访问目录!";
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if((hfind =_findfirst("*",&fdata))==-1)
    {
          qDebug()<<"没有找到文件";
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        return -3;
    };
    int count = 0;
    do
    {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib&_A_SUBDIR)!=0;//判断是否为文件夹
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
       // ListFileInfos.push_back(finfo);
        qDebug()<<finfo.szFileName;
        CPacket pack(2,(BYTE*)&finfo,sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        count++;
    }
    while (!_findnext(hfind, &fdata));
   qDebug()<<"Server Count="<<count;
    //发送信息到控制端
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        return 0;



}

int ExcutCommand(int nCmd)
{
    int ret=0;

    switch (nCmd)
    {
    case 1://查看磁盘分区
       ret= MakeDriverInfo();
        break;
    case 2://查看指定目录下的文件
        ret =2;// MakeDirectoryInfo();
        break;
    case 3://打开文件
        ret =3;// RunFile();
        break;
    case 4://下载文件
        ret =4;// DownLoadFile();
        break;
    case 5://鼠标的操作
        ret =5;// MouseEvent();
        break;
    case 6://远程监控(发送屏幕内容（截图）)
        ret =6;// SendScreen();
        break;
//    case 7: //锁机
//        ret = LockMachine();
//        break;
//    case 8: //解锁
//        ret = UnLockMachine();
//        break;
//    case 9: //删除文件
//        ret = DeleteLocalFile();
//        break;
    case 1981://测试
      //  ret = TestConnect();
    default:
        break;
    }
    return ret;
}

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);
    fflush(stdout);
    QCoreApplication a(argc, argv);
    CServerSocket* pserver = CServerSocket::getInstance();
               int count = 0;
               if (pserver->InitSocket() == false)
               {
                   qDebug()<<"络初始化异常，未能成功初始化，请检查网络状态";

                   exit(0);
               }
               while(CServerSocket::getInstance()!=NULL)
               {
                  if (pserver->AcceptClient() == false)
                  {
                      if (count >= 3)
                      {  qDebug()<<"多次无法正常接入用户，结束程序";

                          exit(0);
                      }
                        qDebug()<<"接入用户失败，自动重试";

                      count++;
                  }
                qDebug()<<"AccptClient return true";
                  int ret = pserver->DealCommand();

                  if (ret > 0)
                  {
                       ret=  ExcutCommand(ret);
                       if (ret != 0)
                       {

                            qDebug()<<"执行命令失败:"<<pserver->GetPacket().sCmd<<"ret="<<ret;

                       }
                       //使用短链接
                       pserver->CloseClient();
                        qDebug()<<"Commnad has done";
                  }
                }
    return a.exec();
}
