#include <QTime>
#include <QDateTime>
#include <regex>
#include <direct.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <windows.h>
#include <omp.h>
using namespace std;
using std::string;
#include "SharedFramework.h"
#include "NeteaseMusic.h"
using namespace NeteaseMusic;
using namespace SharedFramework;

volatile int cancelflag = 0;
volatile int finishedcount = 0;
string formatfilename(string raw)
{
    //因为有一些标点符号不能作为文件名
    raw = regex_replace(raw, regex("\""), "");
    raw = regex_replace(raw, regex("[<]"), "");
    raw = regex_replace(raw, regex("[>]"), "");
    raw = regex_replace(raw, regex("[|]"), "");
    raw = regex_replace(raw, regex("[:]"), "");
    raw = regex_replace(raw, regex("[*]"), "");
    raw = regex_replace(raw, regex("[?]"), "");
    raw = regex_replace(raw, regex("[/]"), "");
    raw = regex_replace(raw, regex("\\\\"), "");
    return raw;
}
string downloadsinglemusic(Music m, string filenamepattern, string &lrcsts, int mode = 0, int delayMsec = 100, int index = 1, string unformattedfoldername = "", bool nineninenine = false)
{
    string sindex;
    //分别考虑最大为999和99个文件的情况
    if (nineninenine == false)
    {
        if (index < 10)
        {
            sindex = "0" + to_string(index);
        }
        else
        {
            sindex = to_string(index);
        }
    }
    else
    {
        if (index < 10)
        {
            sindex = "00" + to_string(index);
        }
        else if (index < 100)
        {
            sindex = "0" + to_string(index);
        }
        else
        {
            sindex = to_string(index);
        }
    }
    ExtendedLyrics e = ExtendedLyrics(m.GetID());
    e.FetchOnlineLyrics();
    lrcsts = e.GetStatus();
    filenamepattern = regex_replace(filenamepattern, regex("\\[title]"), m.GetTitle());
    filenamepattern = regex_replace(filenamepattern, regex("\\[artist]"), m.GetAartist());
    filenamepattern = regex_replace(filenamepattern, regex("\\[track number]"), sindex);
    filenamepattern = formatfilename(filenamepattern);//出来的就是最终的文件名
    string c = e.GetCustomLyric(mode, delayMsec);
    if (c != "")
    {
        if (unformattedfoldername == "")
        {                                           //单曲
            std::ofstream OsWrite(filenamepattern); //覆盖文件
            OsWrite << c << std::endl;
            OsWrite.close();
        }
        else
        {                                                                                          //多曲，写到文件夹里面
            std::ofstream OsWrite(formatfilename(unformattedfoldername) + "\\" + filenamepattern); //覆盖文件
            OsWrite << c << std::endl;
            OsWrite.close();
        }
    }
    return e.GetErrorLog();

}
void downloadmulmusic(vector<Music> vm, vector<string> &vs, string unformattedfoldername, int count, string filenamepattern, int mode = 0, int delayMsec = 100)
{
    QTime time;
    time.start();
#pragma omp parallel for num_threads(8)
    for (int i = 0; i < count; i++)
    {
        if (cancelflag != 0)
        {
            //被取消了，不能break只能continue
            continue;
        }

        string lrcsts;
        string errinfo;
        if (count <= 99)
            errinfo = downloadsinglemusic(vm[i], filenamepattern, lrcsts, mode, delayMsec, i + 1, unformattedfoldername);
        else
            errinfo = downloadsinglemusic(vm[i], filenamepattern, lrcsts, mode, delayMsec, i + 1, unformattedfoldername, true);

#pragma omp critical
        {
            finishedcount++;
            stringstream ss;
            ss << std::left << setw(7) << i + 1 << "|" << std::left << setw(12) << vm[i].GetID() << "|" << std::left << setw(50) << vm[i].GetTitle() << "|" << std::left << setw(40) << vm[i].GetAlbum() << "|" << std::left << setw(30) << vm[i].GetAartist() << "|" << std::left << setw(15) << lrcsts << "|" << errinfo;
            if (omp_in_parallel() == 0)
                ss << "<PARALLEL FAIL!!!>";
            else
                ss << "<thread_no." << omp_get_thread_num() << ">";
            vs[i] = ss.str();
        }
    }
    std::ofstream OsWrite(formatfilename(unformattedfoldername) + "\\log.txt", ios::app); //追加文件
    OsWrite << "Settings:[mode:" << mode << ",delayMsec:" << delayMsec << ",filenamepattern:\"" << filenamepattern << "\"]" << std::endl;
    if (cancelflag == 1)
        OsWrite << "TaskStatus:<Canceled by user>" << std::endl;
    else
        OsWrite << "TaskStatus:<Finished>" << std::endl;
    OsWrite << "Finished Datetime:" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz").toStdString() << std::endl;
    OsWrite << "Used Time:" << time.elapsed() / 1000.0 << "sec" << std::endl
            << std::endl
            << std::endl;
    OsWrite << "SongNum|SongID      |SongName                                          |SongAlbum                               |SongArtist                    |LrcSts         |ErrorInfo" << std::endl;

    for (int i = 0; i < count; i++)
    {
        OsWrite << vs[i] << std::endl;
    }
    OsWrite << std::endl
            << std::endl
            << "Enjoy music with lyrics now!(*^_^*)";
    OsWrite.close();
}
void createlogfile(string unformattedfoldername, string type, long long id, string name, int count)
{
    string folderPath = ".\\" + formatfilename(unformattedfoldername);
    if (0 != access(folderPath.c_str(), 0))
        mkdir(folderPath.c_str());
    std::ofstream OsWrite(formatfilename(unformattedfoldername) + "\\log.txt"); //覆盖文件
    OsWrite << type << "ID:" << to_string(id) << std::endl;
    OsWrite << type << "Name:" << name << std::endl;
    OsWrite << "Count:" << count << std::endl;
    OsWrite.close();
}
