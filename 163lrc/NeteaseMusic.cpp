#include <iostream>
#include <iomanip>
#include <regex>
#include <math.h>
#include <windows.h>
#include <list>
using namespace std;
using std::string;
#include "SharedFramework.h"
#include "NeteaseMusic.h"
using namespace SharedFramework;
using namespace NeteaseMusic;

void ExtendedLyrics::SetStatus(LyricsStatus ls)
{
    if (_status == UNSURED || ls == ERRORx)
    {
        _status = ls;
    }
}
string ExtendedLyrics::GetStatus()
{
    if (_status == UNMATCHED)
        return "UNMATCHED";
    else if (_status == ERRORx)
        return "ERROR";
    else if (_status == NOTSUPPLIED)
        return "NOTSUPPLIED";
    else if (_status == EXISTED)
        return "EXISTED";
    else if (_status == NOLYRICS)
        return "NOLYRICS";
    else if (_status == UNSURED)
        return "UNSURED";
}
void ExtendedLyrics::SetErrorLog(string el)
{
    _errorLog = el;
}
string ExtendedLyrics::GetErrorLog()
{
    return _errorLog;
}
ExtendedLyrics::ExtendedLyrics(long long ID)
{
    _status = UNSURED;
    id = ID;
    _errorLog = "";
}
void ExtendedLyrics::FetchOnlineLyrics()
{

    hasOriLyrics = false;
    hasTransLyrics = false;
    SharedFramework::Lyrics tempOriLyric;
    SharedFramework::Lyrics tempTransLyric;
    SharedFramework::HttpRequest hr;

    string sLRC = "", sContent;
    try
    {
        sContent = hr.GetContent("https://music.163.com/api/song/detail/?id=" + to_string(id) + "&ids=[" + to_string(id) + "]"); //这个是仅对确定歌词状态有用的
        smatch result;
        if (regex_search(sContent, result, regex("^\\{\"songs\":\\[]")) ||
            regex_search(sContent, result, regex("^\\{\"code\":400")))
        {
            _errorLog += "<STATUS_ERR>";
            SetStatus(ERRORx);
            return;
        }
        sContent = hr.GetContent("https://music.163.com/api/song/media?id=" + to_string(id));
        if (sContent.substr(0, 4) == "ERR!")
        {
            _errorLog += "<STATUS_ERR>";
            SetStatus(ERRORx);
            return;
        }

        //分析歌词状态
        if (sContent == "{\"code\":200}")
        {
            SetStatus(NOLYRICS);
            return;
        }

        if (regex_search(sContent, result, regex("^\\{\"nolyric\":true")))
        {
            SetStatus(NOLYRICS);
            return;
        }
        else
            SetStatus(EXISTED);
        SetStatus(UNMATCHED);

        //分析原文歌词
        if (!regex_search(sContent, result, regex("\"lyric\"")))
        {
            SetStatus(ERRORx);
            _errorLog += "<NO_LYRIC_LABEL>";
            return;
        }
        regex_search(sContent, result, regex("^.+?,\"lyric\":\""));
        sLRC = result[0];
        int t = sLRC.length();
        sLRC = sContent;
        sLRC.erase(0, t);
        regex_search(sLRC, result, regex("\",\"code\".+?$"));
        string x = result[0];
        t = x.length();
        sLRC.erase(sLRC.length()-t,t);
        tempOriLyric.ArrangeLyrics(sLRC);
        if(tempOriLyric.GetCount() == 0)
        {
            SetStatus(ERRORx);
            _errorLog += "<OriLyric_Count_0>";
            hasOriLyrics = false;
            return;
        }
        hasOriLyrics = true;
        mixedLyrics.ArrangeLyrics(sLRC);

        //=====================翻译=================
        sContent = hr.GetContent("https://music.163.com/api/song/lyric?os=pc&id=" + to_string(id) + "&tv=-1");

        if (sContent.substr(0, 4) == "ERR!")
        {
            _errorLog += "<STATUS_ERR>";
            return;
        }
        regex_search(sContent, result, regex("^.+?,\"lyric\":\""));
        string tmp = result[0];
        t = tmp.length();
        tmp = sContent;
        tmp.erase(0, t);
        regex_search(sContent, result, regex("\"[}],\"code\".+?$"));
        x = result[0];
        t = x.length();
        tmp.erase(tmp.length()-t,t);



        tempTransLyric.ArrangeLyrics(tmp);
        if (tempOriLyric.GetCount() >= tempTransLyric.GetCount() && tempTransLyric.GetCount() != 0)
        {              //翻译可能比外文歌词少，下面会对时间轴来判断配对
            int j = 0; //指示tempOriLyric和mixedLyrics，j为外文歌词的index 下面的循环是将外文歌词下移
            list<SharedFramework::LyricsLine>::iterator o = tempOriLyric.LyricsLineText.begin();
            list<SharedFramework::LyricsLine>::iterator t = tempTransLyric.LyricsLineText.begin();
            list<SharedFramework::LyricsLine>::iterator m = mixedLyrics.LyricsLineText.begin();
            int mixindex = 0; //指示目前的m是第几个
            for (int i = 0; (i < tempTransLyric.GetCount()) && (j < tempOriLyric.GetCount()); o++, j++)
            {
                if (stoi((*o).GetTimeline()) < stoi((*t).GetTimeline()))
                { //此外文歌词可能为空格之类，没有翻译，所以continue只将外文歌词下移
                    continue;
                }
                if (stoi((*o).GetTimeline()) > stoi((*t).GetTimeline()))
                { //正常情况下应该不会出现这种情况，（特例参见song?id=27901389），将翻译下移
                    t++;
                }
                if (stoi((*o).GetTimeline()) == stoi((*t).GetTimeline()) & (*t).GetOriLyrics() != "")
                {
                    while (mixindex != j)
                    {
                        m++;
                        mixindex++;
                    }
                    (*m).SetTransLyrics("#", (*t).GetOriLyrics());
                }
                t++;
            }
        }
        mixedLyrics.Sort();
    }
    catch (const std::exception &e)
    {
        _errorLog += e.what();
    }
}
//应该在GetOnlineLyric()后使用,若无翻译将直接返回ori
string ExtendedLyrics::GetCustomLyric(int mode, int delayMsec)
{
    char **r;
    r = mixedLyrics.GetLatestLyrics(mode, delayMsec);
    return r[0];
}
string ExtendedLyrics::ToString()
{
    return mixedLyrics.ToString();
}

void Music::fetchInfo()
{
    smatch result;
    string sContent, tmp;
    HttpRequest hr = HttpRequest();
    sContent = hr.GetContent("https://music.163.com/api/v3/song/detail?id=" + to_string(id) + "&c=[{\"id\":\"" + to_string(id) + "\"}]");
    regex_search(sContent, result, regex("\\{\"name\":\".*?(?=\",\")"));
    tmp = result[0];
    tmp.erase(0, 9);
    _title = tmp;

    string::const_iterator iterStart = sContent.begin();
    string::const_iterator iterEnd = sContent.end();
    while (regex_search(iterStart, iterEnd, result, regex(",\"name\":\"[^/]+?(?=\",\"tns\")")))
    {
        tmp = result[0];
        tmp.erase(0, 9);
        _artist += tmp + ",";
        iterStart = result[0].second;
    }
    _artist.erase(_artist.end() - 1);

    regex_search(sContent, result, regex(",\"name\":\"[^}]+?(?=\",\"picUrl)"));
    tmp = result[0];
    tmp.erase(0, 9);
    _album = tmp;
}

Music::Music(long long ID, int index)
{
    _title = "";
    _artist = "";
    _album = "";
    id = ID;
    _index = index;
}
int Music::GetIndex()
{
    return _index;
}
void Music::SetIndex(int value)
{
    _index = value;
}
long long Music::GetID()
{
    return id;
}
void Music::SetID(long long value)
{
    id = value;
}
string Music::GetTitle()
{
    if (_title == "")
        fetchInfo();
    return _title;
}
string Music::GetAartist()
{
    if (_artist == "")
        fetchInfo();
    return _artist;
}
string Music::GetAlbum()
{
    if (_album == "")
        fetchInfo();
    return _album;
}

void Playlist::fetchInfo()
{
    string sContent, tmp;
    HttpRequest hr;
    smatch result;
    string x = "https://music.163.com/api/v3/playlist/detail?id=" + to_string(id) + "&c=[{\"id\":\"" + to_string(id) + "\"}]";
    sContent = hr.GetContent("https://music.163.com/api/v3/playlist/detail?id=" + to_string(id) + "&c=[{\"id\":\"" + to_string(id) + "\"}]");
    string::const_iterator iterStart = sContent.begin();
    string::const_iterator iterEnd = sContent.end();
    while (regex_search(iterStart, iterEnd, result, regex("\\{\"id\":\\d+(?=,\"v\":)")))
    {
        tmp = result[0];
        tmp.erase(0, 6);
        _songidInPlaylist.push_back(stoi(tmp));
        _count++;
        iterStart = result[0].second;
    }//"trackIds.+(?=,\"shareCount)"
    regex_search(sContent, result, regex("status[^}]+(?=,\"shareCount)"));
    tmp = result[0];
    regex_search(tmp, result, regex(",\"name\"[:]\".+(?=\",\"id\")"));
    tmp = result[0];
    tmp.erase(0, 9);
    _name = tmp;
}
list<long long> Playlist::GetSongidInPlaylist()
{
    if (_count == 0)
        fetchInfo();
    return _songidInPlaylist;
}
int Playlist::getCount()
{
    if (_count == 0)
        fetchInfo();
    return _count;
}
string Playlist::GetName()
{
    if (_name == "")
        fetchInfo();
    return _name;
}
Playlist::Playlist(long long ID)
{
    _count = 0;
    _name == "";
    id = ID;
}

void Album::fetchInfo()
{
    string sContent, tmp;
    HttpRequest hr;
    smatch result;
    sContent = hr.GetContent("https://music.163.com/api/album/" + to_string(id));
    string::const_iterator iterStart = sContent.begin();
    string::const_iterator iterEnd = sContent.end();
    while (regex_search(iterStart, iterEnd, result, regex("\"id\":\\d*(?=})")))
    {
        tmp = result[0];
        tmp.erase(0, 5);
        _songidInAlbum.push_back(stoi(tmp));
        _count++;
        iterStart = result[0].second;
    }

    int tmpcount = 0; //先跑一次看看匹配总数是多少
    iterStart = sContent.begin();
    iterEnd = sContent.end();
    while (regex_search(iterStart, iterEnd, result, regex("\"name\":\"[^}]+(?=\",\"id\")")))
    {
        tmpcount++;
        tmp = result[0];
        iterStart = result[0].second;
    }
    tmp.erase(0, 8);
    _name = tmp;
}

Album::Album(long long ID)
{
    _count = 0;
    _name == "";
    id = ID;
}
string Album::GetName()
{
    if (_name == "")
        fetchInfo();
    return _name;
}
int Album::getCount()
{
    if (_count == 0)
        fetchInfo();
    return _count;
}
list<long long> Album::GetSongidInAlbum()
{
    if (_count == 0)
        fetchInfo();
    return _songidInAlbum;
}
