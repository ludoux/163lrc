#ifndef __SHAREDFRAMEWORK_H__
#define __SHAREDFRAMEWORK_H__
namespace SharedFramework
{
class LyricsLine
{
private:
    
    string _oriLyrics;
    string _break = ""; // 用是否为空来判断是否有翻译
    string _transLyrics;

public:
    int _timeline; //以十毫秒数来保存，100=1000ms=1s 为了排序必须要公开
    LyricsLine(string tl, string ori);
    LyricsLine();
    void SetTimeline(string in);//进来不带[]
    string GetTimeline(); //出来不带[];
    int GetTimelineint();
    void SetOriLyrics(string in);
    string GetOriLyrics();
    void SetBreak(string in);
    string GetBreak();
    void SetTransLyrics(string in);
    string GetTransLyrics();
    bool HasTrans();
    string ToString();
    void SetTransLyrics(string breakText, string transLyricsText, bool claerIt = false); //ClearIt清除
    void DelayTimeline(int mSec); //以十毫秒数来保存，100=1000ms=1s
    int CompareTo(LyricsLine other);
};

class Lyrics
{
private:
    int _count = 0;
    
public:
    list<LyricsLine> LyricsLineText;
    Lyrics(list<LyricsLine> Lyrics);
    Lyrics();
    Lyrics(string text, string breakText = "");
    void Sort();
    int GetCount();
    string ToString();
    void ArrangeLyrics(string text, string breakText = "");
    char** GetLatestLyrics(int mode, int delayMsec = 0);
};

class HttpRequest
{//面向 https://stackoverflow.com/questions/1011339/how-do-you-make-a-http-request-with-c 编程
private:
    void mParseUrl(char *mUrl, string &serverName, string &filepath, string &filename);
    SOCKET connectToServer(char *szServerName, WORD portNum);
    int getHeaderLength(char *content);
    char *readUrl2(char *szUrl, long &bytesReturnedOut, char **headerOut);
public:
    string GetContent(string Urlstr);
};
}
#endif
