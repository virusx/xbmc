#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "addons/Addon.h"
#include "addons/AddonDll.h"
#include "addons/DllContentAddon.h"
#include "interfaces/IAnnouncer.h"
#include "dialogs/GUIDialogContextMenu.h"

class CAEChannelInfo;

namespace ADDON
{
  #define CONTENT_NODE           "content://"
  #define MUSIC_VIRTUAL_NODE     "musicdb://99/"
  #define MUSIC_PLAYLIST         "playlist"
  #define MUSIC_ARTIST           "artist"
  #define MUSIC_ALBUM            "album"
  #define MUSIC_SONG             "song"
  #define MUSIC_TOP100           "top100"

  class CAddonCallbacksContent;

  class CContentAddon : public ADDON::CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>,
                        public ANNOUNCEMENT::IAnnouncer
  {
    friend class CAddonCallbacksContent;

  public:
    CContentAddon(const ADDON::AddonProps& props);
    CContentAddon(const cp_extension_t *ext);
    ~CContentAddon(void);

    ADDON_STATUS Create(void);
    bool         DllLoaded(void) const;
    void         Destroy(void);
    void         ReCreate(void);
    bool         ReadyToUse(void) const;
    void         Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char* sender, const char* message, const CVariant& data);

    bool         SupportsFile(const CStdString& strPath) const;
    bool         SupportsFileType(const std::string& strType) const;

    bool         OnDemand(void) const { return !m_bProvidesMusicFiles && ProvidesMusicCodec(); }
    bool         ProvidesMusicCodec(void) const { return m_bReadyToUse && m_bProvidesMusicCodec; }
    bool         ProvidesMusicFiles(void) const { return m_bReadyToUse && m_bProvidesMusicFiles; }
    bool         ProvidesFiles(void) const { return m_bReadyToUse && m_bProvidesFiles; }
    bool         SupportsConcurrentStreams(void) const { return m_bReadyToUse && m_bSupportsConcurrentStreams; }

    bool         FileOpen(const CStdString& strFileName, CONTENT_HANDLE* handle);
    void         FileClose(CONTENT_HANDLE handle);
    unsigned int FileRead(CONTENT_HANDLE handle, void* pBuffer, int64_t iBufLen);
    bool         FileExists(const CStdString& strFileName);
    int64_t      FileSeek(CONTENT_HANDLE handle, int64_t iFilePosition, int iWhence);
    int64_t      FileGetPosition(CONTENT_HANDLE handle);
    int64_t      FileGetLength(CONTENT_HANDLE handle);
    bool         FileGetDirectory(CFileItemList& items, const CStdString& strPath);

    const char*  GetServerName(void);
    bool         MusicGetOverview(CFileItemList& items);
    bool         MusicGetTop100Overview(CFileItemList& items);
    bool         MusicGetPlaylists(CFileItemList& items);
    bool         MusicGetPlaylistContent(CFileItemList& items, const CStdString& strName);
    bool         MusicGetArtists(CFileItemList& items);
    bool         MusicGetAlbums(CFileItemList& items, const CStdString& strArtist);
    bool         MusicGetSongs(CFileItemList& items, const CStdString& strArtist, const CStdString& strAlbum);
    bool         MusicGetTop100(CFileItemList& items, CONTENT_TOP100_TYPE type);
    bool         MusicGetOverviewItems(CFileItemList& items);
    bool         MusicSearch(CFileItemList& items, const CStdString& query);
    bool         MusicGetContextButtons(const CFileItemPtr& item, CContextButtons &buttons);
    bool         MusicClickContextButton(const CFileItemPtr& item, CONTEXT_BUTTON button);
    bool         MusicOpenFile(const std::string& strPath);
    bool         MusicPreloadFile(const std::string& strPath);
    void         MusicCloseFile(void);
    bool         MusicGetCodecInfo(const std::string& strPath, CONTENT_ADDON_CODEC_INFO* info);
    void         MusicPause(bool bPause);
    int64_t      MusicSeek(int64_t iSeekTime);
    int          MusicReadPCM(BYTE* pBuffer, int size, int* actualsize);
    CStdString   MusicGetPlaylistName(const CStdString& strPlaylist) const;
    CStdString   MusicGetAlbumName(const CStdString& strArtist, const CStdString& strAlbum) const;
    CStdString   MusicGetArtistName(const CStdString& strArtist) const;

  private:
    CStdString   ContentBuildPath(const CStdString& strPath);
    CStdString   MusicBuildPath(CONTENT_ADDON_TYPE type, const CStdString& strFilename, const CStdString& strArtist = "", const CStdString& strAlbum = "") const;
    CStdString   GetFilename(const CStdString& strPath) const;
    bool CreateOnDemand(void);
    void SetPlaystate(CONTENT_ADDON_PLAYSTATE newState);
    void FreeFileList(CONTENT_ADDON_FILELIST* items);
    void LogException(const std::exception& e, const char* strFunctionName) const;
    void ResetProperties(void);
    bool GetAddonCapabilities(void);

    CStdString GetPropertyString(std::map<std::string, CONTENT_ADDON_FILE_PROPERTY> m, const CStdString& strKey, const CStdString& strDefault = "");
    int        GetPropertyInt(std::map<std::string, CONTENT_ADDON_FILE_PROPERTY> m, const CStdString& strKey, int iDefault = 0);
    void       ReadFileThumbArt(std::map<std::string, CONTENT_ADDON_FILE_PROPERTY> item, CFileItemPtr& fileItem);

    void ReadFileArtist(std::map<std::string, CONTENT_ADDON_FILE_PROPERTY> item, CFileItemList& xbmcItems);
    void ReadFileAlbum(std::map<std::string, CONTENT_ADDON_FILE_PROPERTY> item, CFileItemList& xbmcItems, const std::string& strArtist = "");
    void ReadFileSong(std::map<std::string, CONTENT_ADDON_FILE_PROPERTY> item, CFileItemList& xbmcItems, const std::string& strArtist = "", const std::string& strAlbum = "");
    void ReadFilePlaylist(std::map<std::string, CONTENT_ADDON_FILE_PROPERTY> item, CFileItemList& xbmcItems);

    void ReadFileDirectory(std::map<std::string, CONTENT_ADDON_FILE_PROPERTY> item, CFileItemList& xbmcItems);
    void ReadFileFile(std::map<std::string, CONTENT_ADDON_FILE_PROPERTY> item, CFileItemList& xbmcItems);

    void ReadFiles(CONTENT_ADDON_FILELIST* addonItems, CFileItemList& xbmcItems, const std::string& strArtist = "", const std::string& strAlbum = "");

    CCriticalSection                                        m_critSection;
    AddonVersion                                            m_apiVersion;
    bool                                                    m_bReadyToUse;
    CONTENT_ADDON_PLAYSTATE                                 m_playState;
    std::map<CStdString, CStdString>                        m_playlistNames;
    std::map<CStdString, CStdString>                        m_artistNames;
    std::map<CStdString, std::map<CStdString, CStdString> > m_albumNames;
    std::string                                             m_strUserPath;
    std::string                                             m_strClientPath;
    std::vector<std::string>                                m_fileTypes;
    bool                                                    m_bOnDemand;
    bool                                                    m_bProvidesMusicCodec;
    bool                                                    m_bProvidesMusicFiles;
    bool                                                    m_bSupportsConcurrentStreams;
    bool                                                    m_bProvidesFiles;
  };
}
