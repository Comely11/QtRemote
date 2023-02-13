#ifndef CCLIENTSSOCKET_H
#define CCLIENTSSOCKET_H



#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <winsock2.h>
#include <pthread.h>
#include <vector>
#include<iostream>
//#include <QObject>
#pragma once
#pragma pack(push)
#pragma pack(1)

//定义IP
#define SERVER_IP   "127.0.0.1"
// //定义端口
#define SERVER_PORT  8080

//包的数据解析
class CPacket
{
public:
    CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
    //打包
    CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
    {
        sHead = 0xFEFF;
        nLength = nSize + 4;
        sCmd = nCmd;
        if (nSize > 0)
        {
            strData.resize(nSize);
            memcpy((void*)strData.c_str(), pData, nSize);
        }
        else
        {
            strData.clear();
        }
        sSum = 0;
        for (size_t j = 0; j < strData.size(); j++)
        {
            sSum += BYTE(strData[j]) & 0xFF;
        }
    }
    CPacket(const CPacket& pack)
    {
        sHead = pack.sHead;
        nLength = pack.nLength;
        sCmd = pack.sCmd;
        strData = pack.strData;
        sSum = pack.sSum;
    }
    CPacket(const BYTE* pData, size_t& nSize)//方便解析数据
    {
        size_t i = 0;
        for (; i < nSize; i++)//寻找包头
        {
            if (*(WORD*)(pData + i) == 0xFEFF)
            {
                sHead = *(WORD*)(pData + i);
                i += 2;//如果包的长度为2FEFF 解析会发生错误，所以+2是为了防止解析错误不可控
                break;
            }
        }
        if (i + 4 + 2 + 2 > nSize)//包的数据不全，或者包头未能全部接收到
        {
            nSize = 0;
            return;
        }
        nLength = *(DWORD*)(pData + i); i += 4;
        if (nLength + i > nSize)//发送数据过大，包没有完全接收到，就返回解析失败
        {
            nSize = 0;
            return;
        }

        sCmd = *(WORD*)(pData + i); i += 2;
        if (nLength > 4)
        {
            strData.resize(nLength - 2 - 2);//-cmd-ssun;
            memcpy((void*)strData.c_str(), pData + i, nLength - 4);
          //  TRACE("%s\r\n",strData.c_str());
            i += nLength - 4;
        }
        sSum = *(WORD*)(pData + i); i += 2;

        //校验
        WORD sum = 0;

        for (size_t j = 0; j < strData.size(); j++)
        {
            sum += BYTE(strData[j]) & 0xFF;
        }
        if (sum == sSum)
        {
            nSize = i;//head2 length4 data...
            return;
        }
        nSize = 0;
    }
    ~CPacket() {};
    CPacket& operator=(const CPacket& pack)
    {
        if (this != &pack) {
            sHead = pack.sHead;
            nLength = pack.nLength;
            sCmd = pack.sCmd;
            strData = pack.strData;
            sSum = pack.sSum;
        }
        return *this;
    }//实现连=

    int Size()//获得包数据大小
    {
        return nLength + 6;
    }
    const char* Data()
    {
        strOut.resize(nLength + 6);
        BYTE* pData = (BYTE*)strOut.c_str();
        *(WORD*)pData = sHead; pData += 2;
        *(DWORD*)pData = nLength; pData += 4;
        *(WORD*)(pData) = sCmd; pData += 2;
        memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
        *(WORD*)pData = sSum;
        return strOut.c_str();
    }
public:
    WORD sHead;//包头FEFF
    DWORD nLength;//包长度（从控制命令开始到和校验结束）
    WORD sCmd;//控制命令
    std::string strData;//包数据
    WORD sSum;//和校验
    std::string strOut;
};
#pragma pack(pop)
//鼠标的结构体
typedef struct MouseEvent
{
    MouseEvent()
    {
        nAction = 0;
        nButton = -1;
        ptXY.x = 0;
        ptXY.y = 0;
    }
    //鼠标的动作
    WORD nAction;//点击,移动,双击
    WORD nButton;//左键，右键，中键
    POINT ptXY;//坐标

}MOUSEEV, * PMOUSEV;

typedef struct file_info
{
    file_info()
    {
        IsInvalid = 0;
        IsDirectory = -1;
        memset(szFileName, 0, sizeof(szFileName));
    }
    BOOL IsInvalid;//是否有效
    BOOL IsDirectory;//是否为目录 0否 1是
    BOOL HasNext;//是否还有后续 0没有 1有
    char szFileName[256];//文件名
}FILEINFO, * PFILEINFO;

//socket配置


std::string GetErrInfo(int wsaErrCode);
void Dump(BYTE* pData, size_t nSize);
class CClinetSocket
{
public:
    static CClinetSocket* getInstance()
    {//静态函数没有this指针，所以无法直接访问成员变量
        if (m_instance == NULL)
        {
            m_instance = new CClinetSocket();
        }
        return m_instance;
    }
    bool InitSocket(int nIP,int nPort)
    {
        //**
        if (m_sock != INVALID_SOCKET)
        {
            closesocket(m_sock);
        }
        m_sock = socket(AF_INET, SOCK_STREAM, 0);//创建sock

        //TODO:校验
        if (m_sock == -1)
        {
            return false;
        }
        sockaddr_in serv_adr;
        memset(&serv_adr, 0, sizeof(serv_adr));
        serv_adr.sin_family = AF_INET;
       printf("addr%08X nIP %08X %d\r\n",inet_addr("127.0.0.1"), htonl(9527),__LINE__);

        serv_adr.sin_addr.s_addr =htonl( nIP);
        serv_adr.sin_port = htons(nPort);
        //绑定
        if (serv_adr.sin_addr.s_addr == INADDR_NONE)
        {
        printf("指定的IP地址，不存在！");
            return false;

        }
        int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
        if (ret == -1) {
        printf("connect socket failure\r\n");
            return false;
        }
        printf("socket init done!\r\n");

        return true;
    }


#define BUFFER_SIZE 2048000

    int DealCommand()
    {
        if (m_sock == -1)
        {
            return -1;
        }

        char* buffer = m_buffer.data();//解析
        static size_t index = 0;
        while (true)
        {
            size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);
            if ((len <=  0)&&(index<=0))
            {
                return -1;
            }
            //TODO:处理命令
            //Dump((BYTE*)buffer,index);
            index += len;
            len = index;
            m_packet = CPacket((BYTE*)buffer, len);
            if (len > 0)
            {
                memmove(buffer, buffer + len, index - len);
                index -= len;
                return m_packet.sCmd;//1111111111111111111111111
            }
        }
        return -1;
    }

    bool Send(const char* pData, int nSize)
    {
        if (m_sock == -1)
        {
            return false;

        }
        return send(m_sock, pData, nSize, 0) > 0;
    }
    bool Send(CPacket& pack)
    {
         printf("m_sock=%d\r\n",m_sock);
        if (m_sock == -1)
        {
            return false;

        }
        return send(m_sock, pack.Data(), pack.Size(), 0) > 0;
    }

    bool GetFilePath(std::string& strPath)
    {//当前的命令是2才会获取
        if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4))
        {
            strPath = m_packet.strData;
            return true;
        }
        return false;
    }
    bool GetMouseEvent(MOUSEEV& mouse)
    {
        if (m_packet.sCmd == 5)
        {
            memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
            return true;
        }
        return false;
    }
    CPacket& GetPacket()
    {
        return m_packet;
    }
    void CloseClient()
    {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
    }
private:
    std::vector<char> m_buffer;
    SOCKET m_sock;
    CPacket m_packet;
    CClinetSocket& operator=(const CClinetSocket& ss) {}
    CClinetSocket(const CClinetSocket& ss)
    {
        m_sock = ss.m_sock;
    }
    CClinetSocket()
    {
        if (InitSockEnv() == FALSE)
        {
             printf("无法初始化套接字环境,请检查网络环境");

            exit(0);
        }
        m_buffer.resize(BUFFER_SIZE);
        memset(m_buffer.data(), 0, BUFFER_SIZE);
    };
    ~CClinetSocket()
    {
        closesocket(m_sock);
        WSACleanup();
    }

    BOOL InitSockEnv()
    {
        WSADATA data;
        //server;
        if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
        {
            return FALSE;
        };//TODO：返回值处理
    //	SOCKET serv_sock = socket(PF_INET, SOCK_STREAM, 0);//创建sock
        return TRUE;
    }
    static	void releaseInstance()
    {
        if (m_instance != NULL)
        {
            CClinetSocket* tmp = m_instance;
            m_instance = NULL;
            delete tmp;
        }
    }
    static	CClinetSocket* m_instance;
    class CHelper
    {
    public:
        CHelper()
        {
            CClinetSocket::getInstance();
        }
        ~CHelper()
        {
            CClinetSocket::releaseInstance();
        };

    };
    static CHelper m_helper;

};



#endif // CCLIENTSSOCKET_H
