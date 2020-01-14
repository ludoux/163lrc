#ifndef __NETEASEMUSIC_H__
#define __NETEASEMUSIC_H__
namespace NeteaseMusic
{
class ExtendedLyrics
{
public:
    enum LyricsStatus {UNMATCHED = -2, ERRORx = -1,NOTSUPPLIED = 0,  EXISTED = 1,  NOLYRICS = 2,  UNSURED = 3 };
    //[Description("未命中")]  [Description("错误")] [Description("无人上传歌词")] [Description("有词")] [Description("纯音乐")] Description("初始值（以防状态被多次更改）")]
private:
    long long id;
    LyricsStatus _status;
    bool hasOriLyrics;
    bool hasTransLyrics;
    string _errorLog;
    SharedFramework::Lyrics mixedLyrics;////翻译作为trans来保存
public:
    void SetStatus(LyricsStatus ls);
    string GetStatus();
    void SetErrorLog(string el);
    string GetErrorLog();
    ExtendedLyrics(long long ID);
    void FetchOnlineLyrics();
    string GetCustomLyric(int mode = 0, int delayMsec = 0);//应该在GetOnlineLyric()后使用,若无翻译将直接返回ori
    string ToString();
};
class Music
{
    private:
    int _index;
    long long id;
    string _title;//由于titile和artist是一个api，所以获取任意一个都会给两个值进行获取（倘若为空才获取）
    string _artist;
    string _album;
    void fetchInfo();
public:
    Music(long long ID, int index = 1);
    int GetIndex();
    void SetIndex(int value);
    long long GetID();
    void SetID(long long value);
    string GetTitle();
    string GetAartist();
    string GetAlbum();
};
class Playlist
{
private:
    long long id;
    int _count;
    list<long long> _songidInPlaylist;
    string _name;
    void fetchInfo();
public:
    list<long long> GetSongidInPlaylist();
    string GetName();
    int getCount();
    Playlist(long long ID);
};
class Album
{
private:
    long long id;
    int _count;
    string _name;
    list<long long> _songidInAlbum;
    void fetchInfo();
public:
    Album(long long ID);
    string GetName();
    int getCount();
    list<long long> GetSongidInAlbum();
};
}
#endif
