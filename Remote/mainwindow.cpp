#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QString>



int MainWindow::SendCommandPacket(int nCmd, bool bAutoCloss, BYTE* pData, size_t nLength)
{

    CClinetSocket* pClient = CClinetSocket::getInstance();

    bool ret = pClient->InitSocket(0x7f000001,9527);//TODO:返回值的处理

    if (!ret)
    {
        qDebug()<<"Init Socket Error";
        return -1;
    }
   qDebug()<<"Connect Socket Successfully！";
    CPacket pack(nCmd, pData, nLength);
    ret = pClient->Send(pack);
    qDebug()<<"Send ret ="<< ret;
    for(int i=0;i<pack.Size();i++)
    {
        if(i%16==0)
        {
           printf("\n");
        }
           printf(" %02X ",(unsigned char)pack.Data()[i]);
    }

    int cmd = pClient->DealCommand();
    qDebug()<<"ack:"<<cmd;
    if (bAutoCloss)
    {
        pClient->CloseClient();
    }
    return cmd;
};

void MainWindow:: OnBnClickedButtonTest()
{
    // TODO: 在此添加控件通知处理程序代码
    char* Data="Hello World!";
    BYTE *TestData=(BYTE*)Data;
    SendCommandPacket(1981,false,TestData,strlen(Data));
}

void MainWindow::OnBnClickedDriversInfo()
{
    //发送命令1查询磁盘
    SendCommandPacket(1);
    std::string drives =CClinetSocket::getInstance()->GetPacket().strData.c_str();
    std::string dr;
        ui->treeWidget->clear();
       ui->treeWidget->setColumnCount(1);
           QList<QTreeWidgetItem *> items;
             QIcon icon;
    for (size_t i = 0; i < drives.size(); i++)
    {
        if(drives[i]==',' )
        {
            dr+=":";
                 items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(dr.data()))));
           dr.clear();
            continue;
        }
        dr += drives[i];
    }
        for (int i=0;i<items.size() ;i++ ) {
              QIcon icon;
            icon.addPixmap(QPixmap(":/image/drives.png"), QIcon::Selected);
             items.at(i)->setIcon(0,icon);

        }
     ui->treeWidget->insertTopLevelItems(0, items);

    printf("Drives:%s",dr.data());
}


QString treeItemToFullPath(QTreeWidgetItem* treeItem)
{
    QString fullPath= treeItem->text(0);

    while (treeItem->parent() != NULL)
    {
        fullPath= treeItem->parent()->text(0) + "/" + fullPath;
        treeItem = treeItem->parent();
    }
    if(fullPath.size()>0)
    {
        fullPath+='/';
    }
    return fullPath;
}
void MainWindow::LoadFileInfo(QTreeWidgetItem *item, int column)
{

    QString curItemRel = treeItemToFullPath(item);

    if (ui->treeWidget->columnCount()==0)return;

    qDebug()<<item->parent();

    int nCmd=SendCommandPacket(2, false, (BYTE*)curItemRel.toLocal8Bit().data(), curItemRel.length());
    PFILEINFO pInfo = (PFILEINFO)CClinetSocket::getInstance()->GetPacket().strData.c_str();
    CClinetSocket* pClient = CClinetSocket::getInstance();
      QList<QTreeWidgetItem *> hitems;
    int count = 0;
    while (pInfo->HasNext)
    {
        printf("[%s] ;is dir%d \r\n", pInfo->szFileName, pInfo->IsDirectory);

        if (pInfo->IsDirectory)
        {
            if ((QString(pInfo->szFileName) == ".") || (QString(pInfo->szFileName) == ".."))
            {

                int cmd = pClient->DealCommand();
                printf("ACK:%d\r\n", cmd);
                if (cmd < 0)break;
                pInfo = (PFILEINFO)CClinetSocket::getInstance()->GetPacket().strData.c_str();
                continue;
            }


           hitems.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString((QString(pInfo->szFileName))))));

        }
        else
        {

        }
        count++;
        int cmd = pClient->DealCommand();
    //	TRACE("ACK:%d\r\n", cmd);
        if (cmd < 0)break;
        pInfo = (PFILEINFO)CClinetSocket::getInstance()->GetPacket().strData.c_str();
    }

    for (int i=0;i<hitems.size() ;i++ ) {
          QIcon icon;
        icon.addPixmap(QPixmap(":/image/file.png"), QIcon::Selected);
         hitems.at(i)->setIcon(0,icon);

    }
    item->addChildren(hitems);

    pClient->CloseClient();
    //TRACE("Count=%d\r\n", count);
}
void MainWindow::LoadFileCurrent()
{
//    HTREEITEM hTree=m_Tree.GetSelectedItem();
//    std::string strPath=GetPath(hTree);

//    int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
//    PFILEINFO pInfo = (PFILEINFO)CClinetSocket::getInstance()->GetPacket().strData.c_str();
//    CClinetSocket* pClient = CClinetSocket::getInstance();

//    while (pInfo->HasNext)
//    {
//        TRACE("[%s] ;is dir%d \r\n", pInfo->szFileName, pInfo->IsDirectory);

//        if (!pInfo->IsDirectory)
//        {
//            m_List.InsertItem(0, pInfo->szFileName);
//            TRACE("filename:%s\r\n", pInfo->szFileName);
//        }

//        int cmd = pClient->DealCommand();
//        TRACE("ACK:%d\r\n", cmd);
//        if (cmd < 0)break;
//        pInfo = (PFILEINFO)CClinetSocket::getInstance()->GetPacket().strData.c_str();
//    }
    //    pClient->CloseClient();
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton_Test,&QPushButton::clicked,this,&MainWindow::OnBnClickedButtonTest);
      connect(ui->pushButton_fileManager,&QPushButton::clicked,this,&MainWindow::OnBnClickedDriversInfo);
        connect(ui->treeWidget,&QTreeWidget::itemPressed,this,&MainWindow::LoadFileInfo);
      //  connect(ui->treeWidget,&QTreeWidget::itemPressed,this,&configBtnNameDialog::onSelectedItem);
}

MainWindow::~MainWindow()
{
    delete ui;
}

