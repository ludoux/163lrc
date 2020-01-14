#include <iostream>
using namespace std;
using std::string;
#include <windows.h>
#include <list>
#include <QObject>
#include <QtConcurrent>
#include "SharedFramework.h"
#include "NeteaseMusic.h"
#include <QDebug>
using namespace QtConcurrent;
using namespace NeteaseMusic;
using namespace SharedFramework;
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tasks.cpp"
QTimer *myTimer;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->label_8->setStyleSheet("color:blue;");
    myTimer = new QTimer(this);
    ui->filenameBox->addItem("[title].lrc");
    ui->filenameBox->addItem("[track number]. [title].lrc");
    ui->filenameBox->addItem("[artist] - [title].lrc");
    ui->filenameBox->addItem("[title] - [artist].lrc");

    connect(myTimer, &QTimer::timeout, [=]() {
        if (cancelflag == 1 && ui->getButton->text() == "下载")//用户取消了下载
        {
            cancelflag = -1;
            ui->cancelButton->setEnabled(false);
            myTimer->setInterval(2000);
            ui->getButton->setText("冷却(4s)");//用定时器来延时下载按钮的恢复
            ui->statusinfolabel->setText("下载被用户取消！\n请查看 log.txt 获取更多内容");
            return;
        }
        else if (cancelflag == -1 && ui->getButton->text() == "冷却(4s)")
        {
            ui->getButton->setText("冷却(2s)");
            return;
        }
        else if (cancelflag == -1 && ui->getButton->text() == "冷却(2s)")
        {
            ui->getButton->setEnabled(true);
            ui->getButton->setText("下载");
            ui->cancelButton->setEnabled(true);
            myTimer->setInterval(600);
            myTimer->stop();
            return;
        }
        if (cancelflag == -2)
        {
            myTimer->stop();
            ui->getButton->setEnabled(true);
            ui->cancelButton->setEnabled(true);
            ui->getButton->setText("下载");
            return;
        }
        if (ui->alllabel->text() == ui->finishedlabel->text())//下载完成，下载按钮延时两秒
        {
            cancelflag = -2;
            ui->cancelButton->setEnabled(false);
            ui->getButton->setText("冷却(2s)");
            myTimer->setInterval(2000);
            ui->statusinfolabel->setText("下载完成！\n请查看 log.txt 获取更多内容");
            return;
        }
        ui->finishedlabel->setText(QString::number(finishedcount));
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_getButton_clicked()
{

    ui->alllabel->setText("0");
    ui->finishedlabel->setText("0");
    ui->statusinfolabel->setText("状态\n信息");
    cancelflag = 0;
    if (ui->musicButton->isChecked())
    {
        string lrcsts;
        Music m = Music(ui->idEdit->text().toLongLong());
        string errlog = downloadsinglemusic(m, ui->filenameBox->currentText().toStdString(), lrcsts, ui->styleBox->value(), ui->delayBox->value());
        ui->statusinfolabel->setText("执行完毕。\n<" + QString::fromStdString(lrcsts) + ">" + QString::fromStdString(errlog));
    }
    else if (ui->playlistButton->isChecked())
    {
        ui->getButton->setEnabled(false);
        Playlist p = Playlist(ui->idEdit->text().toLongLong());
        list<long long> l = p.GetSongidInPlaylist();//装载music
        ui->alllabel->setText(QString::number(p.getCount()));
        vector<Music> vm;
        for (list<long long>::iterator it = l.begin(); it != l.end(); it++)
        {
            vm.push_back(Music(*it));
        }
        createlogfile(p.GetName(), "Playlist", ui->idEdit->text().toLongLong(), p.GetName(), p.getCount());
        //写初始0
        finishedcount = 0;
        int ccc = p.getCount();
        string n = p.GetName(), ss = ui->filenameBox->currentText().toStdString();
        int c = p.getCount();
        int x = ui->styleBox->value();
        int y = ui->delayBox->value();

        QtConcurrent::run([=]() {
            vector<string> vs(ccc); //index从1开始，但是vector下标从0开始
            downloadmulmusic(vm, vs, n, c, ss, x, y);
        });

        myTimer->start(600);
    }
    else if (ui->albumButton->isChecked())
    {
        ui->getButton->setEnabled(false);
        Album a = Album(ui->idEdit->text().toLongLong());
        list<long long> l = a.GetSongidInAlbum();
        ui->alllabel->setText(QString::number(a.getCount()));
        vector<Music> vm;
        for (list<long long>::iterator it = l.begin(); it != l.end(); it++)
        {
            vm.push_back(Music(*it));
        }
        createlogfile(a.GetName(), "Album", ui->idEdit->text().toLongLong(), a.GetName(), a.getCount());
        //写初始0
        finishedcount = 0;
        int ccc = a.getCount();
        string n = a.GetName(), ss = ui->filenameBox->currentText().toStdString();
        int c = a.getCount();
        int x = ui->styleBox->value();
        int y = ui->delayBox->value();
        QtConcurrent::run([=]() {
            vector<string> vs(ccc); //index从1开始，但是vector下标从0开始
            downloadmulmusic(vm, vs, n, c, ss, x, y);
        });
        myTimer->start(600);
    }
}
void MainWindow::on_cancelButton_clicked()
{
    cancelflag = 1;
}
