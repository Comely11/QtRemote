#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <string>
#include <QMainWindow>
#include "direct.h"
#include"clientssocket.h"
#include "tchar.h"
#include <QTreeWidgetItem>
#include<iostream>
#include"QDebug"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    DWORD m_server_address;
    std::string m_nPort;
public:

    int SendCommandPacket(int nCmd,bool bAutoCloss=true,BYTE*pData=NULL,size_t nLength=0);

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
      void OnBnClickedButtonTest();
      void OnBnClickedDriversInfo();
      void LoadFileCurrent();
      void LoadFileInfo(QTreeWidgetItem *item, int column);
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
