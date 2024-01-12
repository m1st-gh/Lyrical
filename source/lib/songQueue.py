from yt_dlp import YoutubeDL
YDL_OPTIONS = {'format': 'bestaudio', 'noplaylist': 'True'}

class SongQueue:
    def __init__(self):
        self._songlist = []
        self._currentSong = None

    def push(self, url):
        try:
            self._songlist.append(YoutubeDL(YDL_OPTIONS).extract_info(url, download=False))
        except Exception as e:
            return False
        return True
    
    def peek(self, index: int):
        if index > len(self._songlist):
            return False
        return self._songlist[index]
    
    def isEmpty(self):
        return len(self._songlist) == 0
    
    def size(self):
        return len(self._songlist)
    
    def clear(self):
        self._songlist.clear()

    def currentIndex(self):
        return self._songlist.index(self._currentSong)
    
    def getCurrentSong(self):
        return self._currentSong
    
    def setCurrentSong(self, song):
        self._currentSong = song
        
    def __list__(self):
        return self._songlist.copy()

    