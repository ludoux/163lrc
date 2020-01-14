#include <iostream>
#include <iomanip>
#include <regex>
#include <math.h>
#include <list>
#include <windows.h>
#include <string>
#include <stdio.h>
using namespace std;
using std::string;
#include "SharedFramework.h"
using namespace SharedFramework;


    HINSTANCE hInst;
    WSADATA wsaData;

    LyricsLine::LyricsLine(string tl, string ori)
    {
        _break = "";
        SetTimeline(tl);
        SetOriLyrics(ori);
    }
    LyricsLine::LyricsLine() {_break = "";}
    void LyricsLine::SetTimeline(string in) //进来不带[]
    {
        int MSec = 0, Sec = 0, Min = 0; //此处Msec为10毫秒
        smatch result;
        string tmp;
        regex_search(in, result, regex("^\\d+(?=:)"));
        Min = stoi(result[0]);

        regex_search(in, result, regex("[:]\\d+(?=.)"));
        tmp = result[0];
        if (tmp != "")
        {
            tmp.erase(0, 1); //将首位的;抹去
            Sec = stoi(tmp);
        }
        else
        { //考虑像 00:37 这样的玩意
            regex_search(in, result, regex("[:]\\d+$"));
            tmp = result[0];
            tmp.erase(0, 1); //将首位的;抹去
            Sec = stoi(tmp);
        }

        regex_search(in, result, regex("[.]\\d+$"));
        tmp = result[0];
        if (tmp != "")
        {
            tmp.erase(0, 1); //将首位的;抹去
            MSec = stoi(tmp);
        }
        else
        { //考虑像 00:37 这样的玩意
            MSec = 0;
        }
        if (MSec > 99)
            MSec = round(MSec / 10.0);
        int tl = MSec + Sec * 100 + Min * 100 * 60;
        if (tl > 0)
            _timeline = tl;
        else
            _timeline = 0;
    }
    string LyricsLine::GetTimeline() //出来不带[]
    {
        int _tmptimeline = _timeline;
        int MSec = 0, Sec = 0, Min = 0; //此处Msec为10毫秒
        if (_tmptimeline > 99)
        {
            MSec = _tmptimeline % 100;
            Sec = (floor(_tmptimeline / 100.0));
        }
        else
        {
            if (_tmptimeline < 10)
                return "00:00.0" + to_string(_tmptimeline);
            else
                return "00:00." + to_string(_tmptimeline);
        }
        if (Sec > 59)
        {
            Min = floor(Sec / 60.0);
            Sec = Sec % 60;
        }
        stringstream ss;
        ss << setw(2) << setfill('0') << Min << ":" << setw(2) << setfill('0') << Sec << "." << setw(2) << setfill('0') << MSec;
        return ss.str();
    }
    int LyricsLine::GetTimelineint()
    {
        return _timeline;
    }
    void LyricsLine::SetOriLyrics(string in)
    {
        _oriLyrics = in;
    }
    string LyricsLine::GetOriLyrics()
    {
        return _oriLyrics;
    }
    void LyricsLine::SetBreak(string in)
    {
        _break = in;
    }
    string LyricsLine::GetBreak()
    {
        return _break;
    }
    void LyricsLine::SetTransLyrics(string in)
    {
        _transLyrics = in;
    }
    string LyricsLine::GetTransLyrics()
    {
        return _transLyrics;
    }
    bool LyricsLine::HasTrans()
    {
        if (_break != "" && _transLyrics != "")
            return true;
        else
            return false;
    }
    string LyricsLine::ToString()
    {
        if (_break == "")
        {
            return _oriLyrics;
        }
        else
        {

            return _oriLyrics + _break + _transLyrics;
        }
    }
    void LyricsLine::SetTransLyrics(string breakText, string transLyricsText, bool claerIt) //ClearIt清除
    {
        if (claerIt)
        {
            _break = "";
            _transLyrics = "";
            return;
        }
        _break = breakText;
        _transLyrics = transLyricsText;
    }
    void LyricsLine::DelayTimeline(int mSec) //以十毫秒数来保存，100=1000ms=1s
    {
        if (_timeline + mSec > 0)
            _timeline = _timeline + mSec;
    }
    int LyricsLine::CompareTo(LyricsLine other)
    {
        if(_timeline > other._timeline)
            return 1;
        if(_timeline = other._timeline)
            return 0;
        if(_timeline < other._timeline)
            return -1;
    }

    Lyrics::Lyrics(list<LyricsLine> Lyrics)
    {
        _count = 0;
        LyricsLineText = Lyrics;
        Sort();
    }
    Lyrics::Lyrics() {_count = 0;}
    Lyrics::Lyrics(string text, string breakText)
    {
        _count = 0;
        ArrangeLyrics(text, breakText);
    }
    void Lyrics::Sort()
    {
        LyricsLineText.sort([](const LyricsLine & a, const LyricsLine & b) { return a._timeline < b._timeline; });
    }
    int Lyrics::GetCount()
    {
        return _count;
    }
    string Lyrics::ToString()
    {
        if (_count == 0)
        {
            return "";
        }
        string r;
        for (list<LyricsLine>::iterator it = LyricsLineText.begin(); it != LyricsLineText.end(); it++)
        {
            r += (*it).ToString() + "\n";
        }
        r.erase(r.end() - 1);
        return r;
    }
    void Lyrics::ArrangeLyrics(string text, string breakText)
    {

        //首先处理\r \n 之类的，将换行符用\n表示
        text = regex_replace(text, regex("\\\\r"), "");
        text = regex_replace(text, regex("\\\\n"), "\n");                //将明码出的\r \n 成为转义符
        text = regex_replace(text, regex("\\r"), "");
        text = regex_replace(text, regex("\\r\\n"), "\n");
        

        text = regex_replace(text, regex("^(\\n)+"), "");             //去除开头的换行
        text = regex_replace(text, regex("(\\n)+$"), "");             //去除结尾的换行
        text = regex_replace(text, regex("(\\n(\\s*)){2,}"), "\n"); //将连续多个换行（无论其中是否有空格）转为一个换行
        smatch resultline;
        //迭代器声明
        string::const_iterator iterStart = text.begin();
        string::const_iterator iterEnd = text.end();
        string temp;
        while (regex_search(iterStart, iterEnd, resultline, regex("(^|\\n).+($|\\n)")))
        {
            temp = resultline[0];
            smatch resultgeci;
            temp = regex_replace(temp, regex("\\n$"), ""); //理论上开出来的temp是每一行，没有换行符
            regex_search(temp, resultgeci, regex("[^(\\])]*$"));
            string geci = resultgeci[0]; //为歌词（如果有翻译的化，是歌词#翻译 样式的
            //下面是考虑到会有同行多个时间轴，在while中添加歌词line
            smatch resulttl;
            string::const_iterator tliterStart = temp.begin();
            string::const_iterator tliterEnd = temp.end();
            while (regex_search(tliterStart, tliterEnd, resulttl, regex("\\[\\d+:\\d+[.]\\d+\\]")))
            {
                string tl = resulttl[0];
                tl.erase(0, 1);         //将首位的[抹去
                tl.erase(tl.end() - 1); ////将末位的]抹去
                LyricsLine ll(tl, "");  //tl,ori
                if (breakText != "")
                { //有翻译，此时geci是类似 歌词#翻译 形式的
                    ll.SetBreak(breakText);
                    regex_search(geci, resultgeci, regex("^.*[" + breakText + "]")); //将real歌词提取出来
                    string realgeci = resultgeci[0];
                    realgeci.erase(realgeci.end() - 1); //将末位的分隔符去掉
                    ll.SetOriLyrics(realgeci);

                    regex_search(geci, resultgeci, regex("[" + breakText + "].*$")); //将翻译提取出来，这里就用上面的变量了
                    realgeci = resultgeci[0];
                    realgeci.erase(0, 1); //将首位的分隔符去掉
                    ll.SetTransLyrics(realgeci);
                }
                else
                {
                    ll.SetOriLyrics(geci);
                }
                LyricsLineText.push_back(ll);
                _count++;

                tliterStart = resulttl[0].second; //更新搜索起始位置,搜索剩下时间轴
            }

            iterStart = resultline[0].second; //更新搜索起始位置,搜索剩下行
        }
        string x;
        string r="";
        for (list<LyricsLine>::iterator it = LyricsLineText.begin(); it != LyricsLineText.end(); it++)
        {
            r = r + (*it).ToString() + "\r\n";
        }
        Sort();
    }
    char** Lyrics::GetLatestLyrics(int mode, int delayMsec)
    {
        string r="";
        string errorLog = "";
        char **rr=(char **)calloc(1,2*sizeof(char*));;
        //rr[0]=(char*)calloc(1,10000);
        //rr[1]=(char*)calloc(1,100);
        
        if (_count == 0)
        {
            errorLog += "<MixedLyric COUNT ERROR>";
            rr[0] = new char[1];
            rr[0] = (char*)"";
            rr[1] = new char[errorLog.length() + 1];
            strcpy(rr[1],errorLog.c_str());
            return rr;
        }
        

        switch (mode)
        {
        case 0:
            try
            {
                for (list<LyricsLine>::iterator it = LyricsLineText.begin(); it != LyricsLineText.end(); it++)
                    r += "[" + (*it).GetTimeline() + "]" + (*it).ToString() + "\n";
                r.erase(r.end() - 1);
                rr[0] = new char[r.length() + 1];
                rr[1] = new char[errorLog.length() + 1];
                strcpy(rr[0],r.c_str());
                strcpy(rr[1],errorLog.c_str());
                return rr;
            }
            catch(const std::exception& e)
            {
                errorLog += e.what();
                rr[0] = new char[1];
                rr[0] = (char*)"";
                rr[1] = new char[errorLog.length() + 1];
                strcpy(rr[1],errorLog.c_str());
                return rr;
            }
            
        break;
        case 1://翻译延迟，作为新行出现
            try
            {
                for (list<LyricsLine>::iterator it = LyricsLineText.begin(); it != LyricsLineText.end(); it++)
                {
                    if((*it).HasTrans())
                    {
                        if( r != "")
                        {
                            r += "\n[" + (*it).GetTimeline() + "]" + (delayMsec >= 0 ? (*it).GetOriLyrics() : (*it).GetTransLyrics());//三目是有人希望可以让翻译先展示，过一会再显示原文
                            (*it).DelayTimeline(delayMsec >= 0 ? delayMsec : -delayMsec);
                            r += "\n[" + (*it).GetTimeline() + "]" + (delayMsec >= 0 ? (*it).GetTransLyrics() : (*it).GetOriLyrics());
                            (*it).DelayTimeline(delayMsec >= 0 ? -delayMsec : delayMsec);//复原原本的时间轴
                        }
                        else
                        {
                            r += "[" + (*it).GetTimeline() + "]" + (delayMsec >= 0 ? (*it).GetOriLyrics() : (*it).GetTransLyrics());//三目是有人希望可以让翻译先展示，过一会再显示原文
                            (*it).DelayTimeline(delayMsec >= 0 ? delayMsec : -delayMsec);
                            r += "\n[" + (*it).GetTimeline() + "]" + (delayMsec >= 0 ? (*it).GetTransLyrics() : (*it).GetOriLyrics());
                            (*it).DelayTimeline(delayMsec >= 0 ? -delayMsec : delayMsec);//复原原本的时间轴
                        }
                    }
                    else if(!(*it).HasTrans())
                    {
                        if( r != "")
                            r += "\n[" + (*it).GetTimeline() + "]" + (*it).ToString();
                        else
                            r += "[" + (*it).GetTimeline() + "]" + (*it).ToString();
                    }
                    else
                    {
                        errorLog += "<Interesting things happened...>";
                        rr[0] = new char[1];
                        rr[0] = (char*)"";
                        rr[1] = new char[errorLog.length() + 1];
                        strcpy(rr[1],errorLog.c_str());
                        return rr;
                    }
                }
                rr[0] = new char[r.length() + 1];
                rr[1] = new char[errorLog.length() + 1];
                strcpy(rr[0],r.c_str());
                strcpy(rr[1],errorLog.c_str());
                return rr;
            }
            catch(const std::exception& e)
            {
                errorLog += e.what();
                rr[0] = new char[1];
                rr[0] = (char*)"";
                rr[1] = new char[errorLog.length() + 1];
                strcpy(rr[1],errorLog.c_str());
                return rr;
            }
            
            break;
        
        default:
            break;
        }
    }



    
    void HttpRequest::mParseUrl(char *mUrl, string &serverName, string &filepath, string &filename)
    {
        string::size_type n;
        string url = mUrl;

        if (url.substr(0, 7) == "http://")
            url.erase(0, 7);

        if (url.substr(0, 8) == "https://")
            url.erase(0, 8);

        n = url.find('/');
        if (n != string::npos)
        {
            serverName = url.substr(0, n);
            filepath = url.substr(n);
            n = filepath.rfind('/');
            filename = filepath.substr(n + 1);
        }

        else
        {
            serverName = url;
            filepath = "/";
            filename = "";
        }
    }

    SOCKET HttpRequest::connectToServer(char *szServerName, WORD portNum)
    {
        struct hostent *hp;
        unsigned int addr;
        struct sockaddr_in server;
        SOCKET conn;

        conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (conn == INVALID_SOCKET)
            return NULL;

        if (inet_addr(szServerName) == INADDR_NONE)
        {
            hp = gethostbyname(szServerName);
        }
        else
        {
            addr = inet_addr(szServerName);
            hp = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
        }

        if (hp == NULL)
        {
            closesocket(conn);
            return NULL;
        }

        server.sin_addr.s_addr = *((unsigned long *)hp->h_addr);
        server.sin_family = AF_INET;
        server.sin_port = htons(portNum);
        if (connect(conn, (struct sockaddr *)&server, sizeof(server)))
        {
            closesocket(conn);
            return NULL;
        }
        return conn;
    }

    int HttpRequest::getHeaderLength(char *content)
    {
        const char *srchStr1 = "\r\n\r\n", *srchStr2 = "\n\r\n\r";
        char *findPos;
        int ofset = -1;

        findPos = strstr(content, srchStr1);
        if (findPos != NULL)
        {
            ofset = findPos - content;
            ofset += strlen(srchStr1);
        }

        else
        {
            findPos = strstr(content, srchStr2);
            if (findPos != NULL)
            {
                ofset = findPos - content;
                ofset += strlen(srchStr2);
            }
        }
        return ofset;
    }

    char* HttpRequest::readUrl2(char *szUrl, long &bytesReturnedOut, char **headerOut)
    {
        const int bufSize = 512;
        char readBuffer[bufSize], sendBuffer[bufSize], tmpBuffer[bufSize];
        char *tmpResult = NULL, *result;
        SOCKET conn;
        string server, filepath, filename;
        long totalBytesRead, thisReadSize, headerLen;

        mParseUrl(szUrl, server, filepath, filename);

        ///////////// step 1, connect //////////////////////
        conn = connectToServer((char *)server.c_str(), 80);

        ///////////// step 2, send GET request /////////////
        sprintf(tmpBuffer, "GET %s HTTP/1.0", filepath.c_str());
        strcpy(sendBuffer, tmpBuffer);
        strcat(sendBuffer, "\r\n");
        sprintf(tmpBuffer, "Host: %s", server.c_str());
        strcat(sendBuffer, tmpBuffer);
        strcat(sendBuffer, "\r\n");
        strcat(sendBuffer, "\r\n");
        send(conn, sendBuffer, strlen(sendBuffer), 0);

        ///////////// step 3 - get received bytes ////////////////
        // Receive until the peer closes the connection
        totalBytesRead = 0;
        while (1)
        {
            memset(readBuffer, 0, bufSize);
            thisReadSize = recv(conn, readBuffer, bufSize, 0);

            if (thisReadSize <= 0)
                break;

            tmpResult = (char *)realloc(tmpResult, thisReadSize + totalBytesRead);

            memcpy(tmpResult + totalBytesRead, readBuffer, thisReadSize);
            totalBytesRead += thisReadSize;
        }

        headerLen = getHeaderLength(tmpResult);
        long contenLen = totalBytesRead - headerLen;
        result = new char[contenLen + 1];
        memcpy(result, tmpResult + headerLen, contenLen);
        result[contenLen] = 0x0;
        char *myTmp;

        myTmp = new char[headerLen + 1];
        strncpy(myTmp, tmpResult, headerLen);
        myTmp[headerLen] = NULL;
        delete (tmpResult);
        *headerOut = myTmp;

        bytesReturnedOut = contenLen;
        closesocket(conn);
        return (result);
    }

    string HttpRequest::GetContent(string Urlstr)
    {
        const int bufLen = 1024;
        char szUrl[100];
        strcpy(szUrl, Urlstr.c_str());
        long fileSize;
        char *memBuffer, *headerBuffer;
        FILE *fp;

        memBuffer = headerBuffer = NULL;

        if (WSAStartup(0x101, &wsaData) != 0)
            return "-1";

        memBuffer = readUrl2(szUrl, fileSize, &headerBuffer);

        if (fileSize != 0)
        {
            delete (headerBuffer);
            return memBuffer;
        }
    }

