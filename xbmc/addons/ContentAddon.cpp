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

//TODO implement todo's

#include "ContentAddon.h"
#include "ContentAddons.h"
#include "music/Song.h"
#include "music/Artist.h"
#include "music/Album.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "URL.h"
#include "settings/AdvancedSettings.h"
#include "MediaSource.h"
#include "cores/AudioEngine/Utils/AEChannelInfo.h"
#include "interfaces/AnnouncementManager.h"
#include "ApplicationMessenger.h"
#include "filesystem/MusicDatabaseDirectory/DirectoryNode.h"
#include "include/xbmc_file_utils.hpp"

using namespace ADDON;
using namespace ANNOUNCEMENT;
using namespace XFILE::MUSICDATABASEDIRECTORY;
using namespace std;

CContentAddon::CContentAddon(const AddonProps& props) :
    CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>(props),
    m_apiVersion("0.0.0"),
    m_bReadyToUse(false),
    m_playState(CONTENT_ADDON_PLAYSTATE_STOP),
    m_bProvidesMusicCodec(false),
    m_bProvidesMusicFiles(false),
    m_bSupportsConcurrentStreams(false),
    m_bProvidesFiles(false)
{
  m_strUserPath          = CSpecialProtocol::TranslatePath(Profile()).c_str();
  m_strClientPath        = CSpecialProtocol::TranslatePath(Path()).c_str();
  m_pInfo                = new CONTENT_PROPERTIES;
  m_pInfo->strUserPath   = m_strUserPath.c_str();
  m_pInfo->strClientPath = m_strClientPath.c_str();
}

CContentAddon::CContentAddon(const cp_extension_t *ext) :
    CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>(ext),
    m_apiVersion("0.0.0"),
    m_bReadyToUse(false),
    m_playState(CONTENT_ADDON_PLAYSTATE_STOP)
{
  CStdString strProvidesMusicCodec = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_music_codec");
  CStdString strProvidesMusicFiles = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_music_files");
  CStdString strProvidesFiles = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_files");
  CStdString strSupportsConcurrentStreams = CAddonMgr::Get().GetExtValue(ext->configuration, "@supports_concurrent_streams");

  m_fileTypes                  = StringUtils::Split(CAddonMgr::Get().GetExtValue(ext->configuration, "@filetypes"), "|");
  m_bProvidesMusicCodec        = strProvidesMusicCodec.Equals("true") || strProvidesMusicCodec.Equals("1");
  m_bProvidesMusicFiles        = strProvidesMusicFiles.Equals("true") || strProvidesMusicFiles.Equals("1");
  m_bProvidesFiles             = strProvidesFiles.Equals("true") || strProvidesFiles.Equals("1");
  m_bSupportsConcurrentStreams = strSupportsConcurrentStreams.Equals("true") || strSupportsConcurrentStreams.Equals("1");
  m_strUserPath                = CSpecialProtocol::TranslatePath(Profile()).c_str();
  m_strClientPath              = CSpecialProtocol::TranslatePath(Path()).c_str();
  m_pInfo                      = new CONTENT_PROPERTIES;
  m_pInfo->strUserPath         = m_strUserPath.c_str();
  m_pInfo->strClientPath       = m_strClientPath.c_str();

  //TODO hack
  if (!m_bProvidesMusicFiles)
    MarkAsDisabled();
}

CContentAddon::~CContentAddon(void)
{
  Destroy();
  SAFE_DELETE(m_pInfo);
}

void CContentAddon::ResetProperties(void)
{
  CSingleLock lock(m_critSection);
  m_apiVersion  = AddonVersion("0.0.0");
  m_bReadyToUse = false;
}

ADDON_STATUS CContentAddon::Create(void)
{
  ADDON_STATUS status(ADDON_STATUS_UNKNOWN);

  /* ensure that a previous instance is destroyed */
  Destroy();

  bool bReadyToUse(false);
  CLog::Log(LOGDEBUG, "%s - creating content add-on instance '%s'", __FUNCTION__, Name().c_str());
  try
  {
    status = CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>::Create();
    bReadyToUse = (status == ADDON_STATUS_OK);
  }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  {
    CSingleLock lock(m_critSection);
    m_bReadyToUse = bReadyToUse;
  }

  if (bReadyToUse)
    CAnnouncementManager::AddAnnouncer(this);

  return status;
}

bool CContentAddon::DllLoaded(void) const
{
  try { return CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>::DllLoaded(); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  return false;
}

void CContentAddon::Destroy(void)
{
  CAnnouncementManager::RemoveAnnouncer(this);

  MusicCloseFile();

  /* reset 'ready to use' to false */
  {
    CSingleLock lock(m_critSection);
    if (!m_bReadyToUse)
      return;
    m_bReadyToUse = false;
  }

  CLog::Log(LOGDEBUG, "%s - destroying add-on '%s'", __FUNCTION__, Name().c_str());

  /* destroy the add-on */
  try { CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>::Destroy(); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  /* reset all properties to defaults */
  ResetProperties();
}

void CContentAddon::ReCreate(void)
{
  Destroy();
  Create();
}

bool CContentAddon::ReadyToUse(void) const
{
  CSingleLock lock(m_critSection);
  return m_bReadyToUse;
}

const char* CContentAddon::GetServerName(void)
{
  try { return m_pStruct->GetServerName(); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
  return NULL;
}

void CContentAddon::FreeFileList(CONTENT_ADDON_FILELIST* items)
{
  try { return m_pStruct->FreeFileList(items); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
}

map<string, CONTENT_ADDON_FILE_PROPERTY> FileListToMap(CONTENT_ADDON_FILE_PROPERTY* properties, int len)
{
  map<string, CONTENT_ADDON_FILE_PROPERTY> retval;
  for (int i = 0; i < len; i++)
    retval.insert(make_pair(properties[i].key, properties[i]));
  return retval;
}

void CContentAddon::AddCommonProperties(AddonFileItem* file, CFileItemPtr& fileItem)
{
  CStdString strThumb = ContentBuildPath(file->Thumb(true));
  if (!strThumb.empty())
    fileItem->SetArt("thumb", strThumb);
  CStdString strArt   = ContentBuildPath(file->Fanart(true));
  if (!strArt.empty())
    fileItem->SetProperty("fanart_image", strArt);

  for (std::map<std::string,AddonFileItemProperty*>::const_iterator it = file->m_properties.begin(); it != file->m_properties.end(); it++)
  {
    if (it->second->Type() == CONTENT_ADDON_PROPERTY_TYPE_STRING)
      fileItem->SetProperty(it->first, CVariant(it->second->ValueAsString()));
    else if (it->second->Type() == CONTENT_ADDON_PROPERTY_TYPE_INT)
      fileItem->SetProperty(it->first, CVariant(it->second->ValueAsInt()));
  }

  // Make sure the provider logo is set to a valid full path
  CStdString strProvider(fileItem->GetProperty("provider_icon").asString());
  if (strProvider.empty())
    fileItem->SetProperty("provider_icon", Icon());
  else if (!CURL::IsFullPath(strProvider))
    fileItem->SetProperty("provider_icon", URIUtils::AddFileToFolder(Path(), strProvider));
}

void CContentAddon::ReadFilePlaylist(AddonFileItem* file, CFileItemList& fileList)
{
  CMediaSource playlist;
  AddonFilePlaylist* playlistFile = reinterpret_cast<AddonFilePlaylist*>(file);
  if (!playlistFile)
    return;

  playlist.strPath = MusicBuildPath(CONTENT_ADDON_TYPE_PLAYLIST, playlistFile->Path(true));
  playlist.strName = playlistFile->Name(true);
  if (playlist.strPath.empty() || playlist.strName.empty()) return;

  CFileItemPtr pItem(new CFileItem(playlist));
  AddCommonProperties(file, pItem);

  {
    CSingleLock lock(m_critSection);
    m_playlistNames.insert(make_pair(playlist.strPath, playlist.strName));
  }
  fileList.Add(pItem);
}

void CContentAddon::ReadFileSong(AddonFileItem* file, CFileItemList& fileList, const string& strArtist /* = "" */, const string& strAlbum /* = "" */)
{
  CSong song;
  AddonFileSong* songFile = reinterpret_cast<AddonFileSong*>(file);
  if (!songFile)
    return;
  song.strFileName = MusicBuildPath(CONTENT_ADDON_TYPE_SONG, songFile->Path(true), strArtist, strAlbum);
  song.strTitle    = songFile->Name(true);
  if (song.strFileName.empty() || song.strTitle.empty()) return;

  song.iTrack      = songFile->Track(true);
  song.iDuration   = songFile->Duration(true);
  song.rating      = songFile->Rating(true);
  song.artist      = StringUtils::Split(songFile->Artists(true), g_advancedSettings.m_musicItemSeparator);
  song.iYear       = songFile->Year(true);
  song.strAlbum    = songFile->Album(true);
  song.albumArtist = StringUtils::Split(songFile->AlbumArtists(true), g_advancedSettings.m_musicItemSeparator);

  CFileItemPtr pItem(new CFileItem(song));
  AddCommonProperties(file, pItem);

  fileList.AddAutoJoin(pItem);
}

void CContentAddon::ReadFileAlbum(AddonFileItem* file, CFileItemList& fileList, const string& strArtist /* = "" */)
{
  AddonFileAlbum* albumFile = reinterpret_cast<AddonFileAlbum*>(file);
  if (!albumFile)
    return;

  CAlbum album;
  album.strAlbum = albumFile->Name(true);
  album.artist = StringUtils::Split(albumFile->Artists(true), g_advancedSettings.m_musicItemSeparator);
  album.genre = StringUtils::Split(albumFile->Genres(true), g_advancedSettings.m_musicItemSeparator);
  album.iYear = albumFile->Year(true);
  album.iRating = albumFile->Rating(true);
  album.strReview = albumFile->Review(true);
  album.styles = StringUtils::Split(albumFile->Styles(true), g_advancedSettings.m_musicItemSeparator);
  album.moods = StringUtils::Split(albumFile->Moods(true), g_advancedSettings.m_musicItemSeparator);
  album.themes = StringUtils::Split(albumFile->Themes(true), g_advancedSettings.m_musicItemSeparator);
  album.strLabel = albumFile->Label(true);
  album.strType = albumFile->Type(true);
  album.bCompilation = albumFile->Compilation(true);
  album.iTimesPlayed = albumFile->TimesPlayed(true);

  const CStdString strAlbumArtist(strArtist.empty() && !album.artist.empty() ? album.artist.at(0) : strArtist);
  const CStdString strPath = MusicBuildPath(CONTENT_ADDON_TYPE_ALBUM, albumFile->Path(true), strAlbumArtist);
  CFileItemPtr pItem(new CFileItem(strPath, album));

  AddCommonProperties(file, pItem);

  {
    CSingleLock lock(m_critSection);
    map<CStdString, map<CStdString, CStdString> >::iterator it = m_albumNames.find(strAlbumArtist);
    if (it != m_albumNames.end())
      it->second.insert(make_pair(pItem->GetPath(), album.strAlbum));
    else
    {
      map<CStdString, CStdString> m;
      m.insert(make_pair(pItem->GetPath(), album.strAlbum));
      m_albumNames.insert(make_pair(strAlbumArtist, m));
    }
  }
  fileList.Add(pItem);
}

void CContentAddon::ReadFileArtist(AddonFileItem* file, CFileItemList& fileList)
{
  CArtist artist;
  AddonFileArtist* artistFile = reinterpret_cast<AddonFileArtist*>(file);
  if (!artistFile)
    return;

  CStdString strPath  = MusicBuildPath(CONTENT_ADDON_TYPE_ARTIST, artistFile->Path(true));
  artist.strArtist    = artistFile->Name(true);
  if (strPath.empty() || artist.strArtist.empty()) return;

  artist.genre        = StringUtils::Split(artistFile->Genres(true), g_advancedSettings.m_musicItemSeparator);
  artist.strBiography = artistFile->Biography(true);
  artist.styles       = StringUtils::Split(artistFile->Styles(true), g_advancedSettings.m_musicItemSeparator);
  artist.moods        = StringUtils::Split(artistFile->Moods(true), g_advancedSettings.m_musicItemSeparator);
  artist.strBorn      = artistFile->Born(true);
  artist.strFormed    = artistFile->Formed(true);
  artist.strDied      = artistFile->Died(true);
  artist.strDisbanded = artistFile->Disbanded(true);
  artist.yearsActive  = StringUtils::Split(artistFile->YearsActive(true), g_advancedSettings.m_musicItemSeparator);
  artist.instruments  = StringUtils::Split(artistFile->Genres(true), g_advancedSettings.m_musicItemSeparator);

  CFileItemPtr pItem(new CFileItem(artist));
  pItem->SetPath(strPath);
  pItem->SetIconImage("DefaultArtist.png");
  pItem->SetProperty("artist_description", artist.strBiography);
  AddCommonProperties(file, pItem);

  {
    CSingleLock lock(m_critSection);
    m_artistNames.insert(make_pair(strPath, artist.strArtist));
  }
  fileList.AddAutoJoin(pItem);
}

void CContentAddon::ReadFileDirectory(AddonFileItem* file, CFileItemList& fileList)
{
  CMediaSource m;
  AddonFileDirectory* directoryFile = reinterpret_cast<AddonFileDirectory*>(file);
  if (!directoryFile)
    return;

  m.strPath = ContentBuildPath(directoryFile->Path(true));
  m.strName = directoryFile->Name(true);

  if (m.strPath.empty() || m.strName.empty())
    return;

  CFileItemPtr pItem(new CFileItem(m));
  AddCommonProperties(file, pItem);

  fileList.Add(pItem);
}

void CContentAddon::ReadFileFile(AddonFileItem* file, CFileItemList& fileList)
{
  CMediaSource m;
  AddonFileFile* fileFile = reinterpret_cast<AddonFileFile*>(file);
  if (!fileFile)
    return;

  m.strPath = ContentBuildPath(fileFile->Path(true));
  m.strName = fileFile->Name(true);

  if (m.strPath.empty() || m.strName.empty())
    return;

  CFileItemPtr pItem(new CFileItem(m));
  AddCommonProperties(file, pItem);

  fileList.Add(pItem);
}

void CContentAddon::ReadFiles(CONTENT_ADDON_FILELIST* addonItems, CFileItemList& xbmcItems, const string& strArtist /* = "" */, const string& strAlbum /* = "" */)
{
  AddonFileItemList list(addonItems);
  for (vector<AddonFileItem*>::iterator it = list.m_fileItems.begin(); it != list.m_fileItems.end(); it++)
  {
    switch ((*it)->Type())
    {
    case CONTENT_ADDON_TYPE_SONG:
      ReadFileSong(*it, xbmcItems, strArtist, strAlbum);
      break;
    case CONTENT_ADDON_TYPE_ARTIST:
      ReadFileArtist(*it, xbmcItems);
      break;
    case CONTENT_ADDON_TYPE_ALBUM:
      ReadFileAlbum(*it, xbmcItems, strArtist);
      break;
    case CONTENT_ADDON_TYPE_PLAYLIST:
      ReadFilePlaylist(*it, xbmcItems);
      break;
    case CONTENT_ADDON_TYPE_DIRECTORY:
      ReadFileDirectory(*it, xbmcItems);
      break;
    case CONTENT_ADDON_TYPE_FILE:
      ReadFileFile(*it, xbmcItems);
      break;
    default:
      CLog::Log(LOGWARNING, "invalid filetype: %d", (*it)->Type());
      break;
    }
  }
}

bool CContentAddon::MusicGetPlaylists(CFileItemList& items)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "getting playlists from add-on '%s'", Name().c_str());
  try { err = m_pStruct->MusicGetPlaylists(&retVal); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadFiles(retVal, items);

    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetArtists(CFileItemList& items)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "getting artists from add-on '%s'", Name().c_str());
  try { err = m_pStruct->MusicGetArtists(&retVal); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadFiles(retVal, items);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetPlaylistContent(CFileItemList& items, const CStdString& strName)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "getting playlist '%s' from add-on '%s'", MusicGetPlaylistName(strName).c_str(), Name().c_str());
  try { err = m_pStruct->MusicGetPlaylist(&retVal, strName.c_str()); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadFiles(retVal, items);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetAlbums(CFileItemList& items, const CStdString& strArtist)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  if (strArtist.empty())
    CLog::Log(LOGDEBUG, "getting all albums from add-on '%s'", Name().c_str());
  else
    CLog::Log(LOGDEBUG, "getting albums for artist '%s' from add-on '%s'", MusicGetArtistName(strArtist).c_str(), Name().c_str());
  try { err = m_pStruct->MusicGetAlbums(&retVal, strArtist.c_str()); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadFiles(retVal, items, strArtist);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetSongs(CFileItemList& items, const CStdString& strArtist, const CStdString& strAlbum)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  if (strArtist.empty())
    CLog::Log(LOGDEBUG, "getting all songs from add-on '%s'", Name().c_str());
  else if (strAlbum.empty())
    CLog::Log(LOGDEBUG, "getting songs from artist '%s' from add-on '%s'", MusicGetArtistName(strArtist).c_str(), Name().c_str());
  else
    CLog::Log(LOGDEBUG, "getting songs for album '%s' from artist '%s' from add-on '%s'", MusicGetAlbumName(strArtist, strAlbum).c_str(), MusicGetArtistName(strArtist).c_str(), Name().c_str());

  try { err = m_pStruct->MusicGetSongs(&retVal, strArtist.c_str(), strAlbum.c_str()); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadFiles(retVal, items, strArtist, strAlbum);

    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetCodecInfo(const string& strPath, CONTENT_ADDON_CODEC_INFO* info)
{
  if (!ReadyToUse() || !ProvidesMusicCodec())
    return false;

  CStdString strFilePath = GetFilename(strPath);

  try { return m_pStruct->MusicGetCodecInfo(strFilePath.c_str(), info) == CONTENT_ERROR_NO_ERROR; }
  catch (exception &e) { LogException(e, __FUNCTION__); }
  return false;
}

CStdString CContentAddon::GetFilename(const CStdString& strPath) const
{
  CStdString retval(strPath);

  // check whether the filename starts with content://id/
  CStdString strContentNode;
  strContentNode.Format("%s%s/", CONTENT_NODE, ID().c_str());
  if (strPath.Left(strContentNode.length()).Equals(strContentNode))
    return strPath.Right(strPath.length() - strContentNode.length());

  // check whether the file resolves to a directory node that we created
  auto_ptr<CDirectoryNode> pNode(CDirectoryNode::ParseURL(strPath));
  if (pNode.get())
  {
    CContentAddonDirectoryNode* pAddonNode = dynamic_cast<CContentAddonDirectoryNode*>(pNode.get());
    if (pAddonNode)
    {
      CONTENT_ADDON addon = pAddonNode->GetAddon();
      if (addon.get() && addon->ID().Equals(ID()))
        retval = pAddonNode->Filename();
    }
  }

  return retval;
}

bool CContentAddon::MusicOpenFile(const string& strPath)
{
  if (!CreateOnDemand() || !ProvidesMusicCodec() || !SupportsFile(strPath))
    return false;

  CStdString strFilePath = GetFilename(strPath);

  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);
  try { err = m_pStruct->MusicOpenFile(strFilePath.c_str()); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err != CONTENT_ERROR_NO_ERROR)
    CLog::Log(LOGERROR, "add-on '%s' returned an error from OpenMusicFile(%s): %d", Name().c_str(), strPath.c_str(), err);

  return err == CONTENT_ERROR_NO_ERROR;
}

bool CContentAddon::MusicPreloadFile(const string& strPath)
{
  if (!CreateOnDemand() || !ProvidesMusicCodec())
    return false;

  CStdString strFilePath = GetFilename(strPath);

  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);
  try { err = m_pStruct->MusicPreloadFile(strFilePath.c_str()); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err != CONTENT_ERROR_NO_ERROR)
    CLog::Log(LOGERROR, "add-on '%s' returned an error from MusicPreloadFile(%s): %d", Name().c_str(), strPath.c_str(), err);

  return err == CONTENT_ERROR_NO_ERROR;
}

void CContentAddon::MusicCloseFile(void)
{
  if (!ReadyToUse() || !ProvidesMusicCodec())
    return;

  try { m_pStruct->MusicCloseFile(); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
}

void CContentAddon::MusicPause(bool bPause)
{
  if (!ReadyToUse() || !ProvidesMusicCodec())
    return;

  {
    CSingleLock lock(m_critSection);
    if ((m_playState == CONTENT_ADDON_PLAYSTATE_PAUSE && bPause) ||
        (m_playState == CONTENT_ADDON_PLAYSTATE_PLAY && !bPause) ||
        m_playState == CONTENT_ADDON_PLAYSTATE_STOP)
      return;
    m_playState = bPause ? CONTENT_ADDON_PLAYSTATE_PAUSE : CONTENT_ADDON_PLAYSTATE_PLAY;
  }

  try { m_pStruct->MusicPause(bPause ? 1 : 0); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
}

int64_t CContentAddon::MusicSeek(int64_t iSeekTime)
{
  if (!ReadyToUse() || !ProvidesMusicCodec())
    return -1;

  try { return m_pStruct->MusicSeek(iSeekTime); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
  return 0;
}

int CContentAddon::MusicReadPCM(BYTE* pBuffer, int size, int* actualsize)
{
  if (!ReadyToUse() || !ProvidesMusicCodec())
    return -1;

  try { return m_pStruct->MusicReadPCM(pBuffer, size, actualsize); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
  return -1;
}

void CContentAddon::LogException(const exception &e, const char *strFunctionName) const
{
  CLog::Log(LOGERROR, "exception '%s' caught while trying to call '%s' on add-on '%s'. Please contact the developer of this add-on: %s", e.what(), strFunctionName, Name().c_str(), Author().c_str());
}

CStdString CContentAddon::MusicGetPlaylistName(const CStdString& strPlaylist) const
{
  CStdString strReturn(strPlaylist);
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return strReturn;
  CSingleLock lock(m_critSection);
  map<CStdString, CStdString>::const_iterator it = m_playlistNames.find(strPlaylist);
  if (it != m_playlistNames.end())
    strReturn = it->second;
  return strReturn;
}

CStdString CContentAddon::MusicGetArtistName(const CStdString& strArtist) const
{
  CStdString strReturn(strArtist);
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return strReturn;
  CSingleLock lock(m_critSection);
  map<CStdString, CStdString>::const_iterator it = m_artistNames.find(strArtist);
  if (it != m_artistNames.end())
    strReturn = it->second;
  return strReturn;
}

CStdString CContentAddon::MusicGetAlbumName(const CStdString& strArtist, const CStdString& strAlbum) const
{
  CStdString strReturn(strAlbum);
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return strReturn;
  CSingleLock lock(m_critSection);

  if (!strArtist.empty())
  {
    map<CStdString, map<CStdString, CStdString> >::const_iterator it = m_albumNames.find(strArtist.c_str());
    if (it != m_albumNames.end())
    {
      map<CStdString, CStdString>::const_iterator it2 = it->second.find(strAlbum.c_str());
      if (it2 != it->second.end())
        return it2->second;
    }
  }

  // return the first match
  for (map<CStdString, map<CStdString, CStdString> >::const_iterator it = m_albumNames.begin(); it != m_albumNames.end(); it++)
  {
    map<CStdString, CStdString>::const_iterator it2 = it->second.find(strAlbum.c_str());
    if (it2 != it->second.end())
    {
      strReturn = it2->second;
      break;
    }
  }
  return strReturn;
}

void CContentAddon::Announce(AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data)
{
  if (!strcmp(sender, "xbmc") && !strcmp(message, "OnStop"))
    MusicCloseFile();
  else if (!strcmp(sender, "xbmc") && !strcmp(message, "OnPause"))
    MusicPause(true);
  else if (!strcmp(sender, "xbmc") && !strcmp(message, "OnPlay"))
    MusicPause(false);
}

bool CContentAddon::MusicGetTop100(CFileItemList& items, CONTENT_TOP100_TYPE type)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "getting top100 from add-on '%s'", Name().c_str());
  try { err = m_pStruct->MusicGetTop100(&retVal, type); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadFiles(retVal, items);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetOverviewItems(CFileItemList& items)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "getting overview items from add-on '%s'", Name().c_str());
  try { err = m_pStruct->MusicGetOverviewItems(&retVal); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadFiles(retVal, items);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetOverview(CFileItemList& items)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CStdString strPrepend;
  strPrepend.Format("%s%s/", MUSIC_VIRTUAL_NODE, ID().c_str());

  // TODO only add items that this add-on supports

  CFileItemPtr pArtistItem(new CFileItem(g_localizeStrings.Get(133)));
  CStdString strDir;
  strDir.Format("%s%s/", strPrepend.c_str(), MUSIC_ARTIST);
  pArtistItem->SetPath(strDir);
  pArtistItem->m_bIsFolder = true;
  pArtistItem->SetCanQueue(false);
  items.Add(pArtistItem);

  CFileItemPtr pAlbumItem(new CFileItem(g_localizeStrings.Get(132)));
  strDir.Format("%s%s/", strPrepend.c_str(), MUSIC_ALBUM);
  pAlbumItem->SetPath(strDir);
  pAlbumItem->m_bIsFolder = true;
  pAlbumItem->SetCanQueue(false);
  items.Add(pAlbumItem);

  CFileItemPtr pSongItem(new CFileItem(g_localizeStrings.Get(134)));
  strDir.Format("%s%s/", strPrepend.c_str(), MUSIC_SONG);
  pSongItem->SetPath(strDir);
  pSongItem->m_bIsFolder = true;
  pSongItem->SetCanQueue(false);
  items.Add(pSongItem);

  CFileItemPtr pTop100Item(new CFileItem(g_localizeStrings.Get(271)));
  strDir.Format("%s%s/", strPrepend.c_str(), MUSIC_TOP100);
  pTop100Item->SetPath(strDir);
  pTop100Item->m_bIsFolder = true;
  pTop100Item->SetCanQueue(false);
  items.Add(pTop100Item);

  CFileItemPtr pPlaylistItem(new CFileItem(g_localizeStrings.Get(136)));
  strDir.Format("%s%s/", strPrepend.c_str(), MUSIC_PLAYLIST);
  pPlaylistItem->SetPath(strDir);
  pPlaylistItem->m_bIsFolder = true;
  pPlaylistItem->SetCanQueue(false);
  items.Add(pPlaylistItem);

  CFileItemPtr pSearchItem(new CFileItem(g_localizeStrings.Get(137)));
  strDir = URIUtils::MakeMusicSearchPath(ID());
  pSearchItem->SetPath(strDir);
  pSearchItem->m_bIsFolder = true;
  pSearchItem->SetCanQueue(false);
  items.Add(pSearchItem);

  return true;
}

bool CContentAddon::MusicGetTop100Overview(CFileItemList& items)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CStdString strPrepend;
  strPrepend.Format("%s%s/%s/", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_TOP100);

  // TODO only add items that this add-on supports

  CFileItemPtr pArtistItem(new CFileItem(g_localizeStrings.Get(133)));
  CStdString strDir;
  strDir.Format("%s%s/", strPrepend.c_str(), MUSIC_ARTIST);
  pArtistItem->SetPath(strDir);
  pArtistItem->m_bIsFolder = true;
  pArtistItem->SetCanQueue(false);
  items.Add(pArtistItem);

  CFileItemPtr pAlbumItem(new CFileItem(g_localizeStrings.Get(132)));
  strDir.Format("%s%s/", strPrepend.c_str(), MUSIC_ALBUM);
  pAlbumItem->SetPath(strDir);
  pAlbumItem->m_bIsFolder = true;
  pAlbumItem->SetCanQueue(false);
  items.Add(pAlbumItem);

  CFileItemPtr pSongItem(new CFileItem(g_localizeStrings.Get(134)));
  strDir.Format("%s%s/", strPrepend.c_str(), MUSIC_SONG);
  pSongItem->SetPath(strDir);
  pSongItem->m_bIsFolder = true;
  pSongItem->SetCanQueue(false);
  items.Add(pSongItem);

  return true;
}

bool CContentAddon::MusicSearch(CFileItemList& items, const CStdString& strQuery, CONTENT_ADDON_SEARCH_TYPE type /* = CONTENT_SEARCH_ALL */)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "searching for '%s' in add-on '%s'", strQuery.c_str(), Name().c_str());
  try { err = m_pStruct->MusicSearch(&retVal, strQuery.c_str(), type); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadFiles(retVal, items);

    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetContextButtons(const CFileItemPtr& item, CContextButtons &buttons)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  // TODO
  return false;
}

bool CContentAddon::MusicClickContextButton(const CFileItemPtr& item, CONTEXT_BUTTON button)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  // TODO
  return false;
}

void CContentAddon::SetPlaystate(CONTENT_ADDON_PLAYSTATE newState)
{
  CLog::Log(LOGDEBUG, "%s(%d)", __FUNCTION__, (int)newState);
  CSingleLock lock(m_critSection);
  switch (newState)
  {
  case CONTENT_ADDON_PLAYSTATE_PAUSE:
    if (m_playState == CONTENT_ADDON_PLAYSTATE_PLAY)
    {
      m_playState = CONTENT_ADDON_PLAYSTATE_PAUSE;
      CApplicationMessenger::Get().MediaPause();
    }
    break;
  case CONTENT_ADDON_PLAYSTATE_PLAY:
    if (m_playState == CONTENT_ADDON_PLAYSTATE_PAUSE)
    {
      m_playState = CONTENT_ADDON_PLAYSTATE_PLAY;
      CApplicationMessenger::Get().MediaPause();
    }
    break;
  case CONTENT_ADDON_PLAYSTATE_STOP:
    if (m_playState != CONTENT_ADDON_PLAYSTATE_STOP)
    {
      m_playState = CONTENT_ADDON_PLAYSTATE_STOP;
      CApplicationMessenger::Get().MediaStop();
    }
    break;
  default:
    CLog::Log(LOGWARNING, "add-on '%s' tried to set an invalid playstate (%d)", Name().c_str(), (int)newState);
    break;
  }
}

bool CContentAddon::SupportsFile(const CStdString& strPath) const
{
  // check whether the file resolves to a directory node that we created
  CStdString strContentNode;
  strContentNode.Format("%s%s/", CONTENT_NODE, ID().c_str());
  if (strPath.Left(strContentNode.length()).Equals(strContentNode))
    return true;

  auto_ptr<CDirectoryNode> pNode(CDirectoryNode::ParseURL(strPath));
  if (pNode.get())
  {
    CContentAddonDirectoryNode* pAddonNode = dynamic_cast<CContentAddonDirectoryNode*>(pNode.get());
    if (pAddonNode)
    {
      CONTENT_ADDON addon = pAddonNode->GetAddon();
      return (addon.get() && addon->ID().Equals(ID()));
    }
  }

  // check whether the filetype matches any of the filetypes in addon.xml
  CURL url(strPath);
  if (SupportsFileType(url.GetFileType()))
    return true;

  // if this add-on provides a music codec, ask the add-on whether it supports this filename
  if (ProvidesMusicCodec())
  {
    try { return m_pStruct->SupportsFile(strPath.c_str()); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  // no match, the add-on doesn't support this file
  return false;
}

bool CContentAddon::SupportsFileType(const string& strType) const
{
  return find(m_fileTypes.begin(), m_fileTypes.end(), strType) != m_fileTypes.end();
}

bool CContentAddon::CreateOnDemand(void)
{
  if (ReadyToUse())
    return true;
  if (OnDemand())
  {
    CLog::Log(LOGDEBUG, "Creating on-demand add-on '%s'", Name().c_str());
    return Create() == ADDON_STATUS_OK;
  }
  return false;
}

CStdString CContentAddon::MusicBuildPath(CONTENT_ADDON_TYPE type, const CStdString& strFilename, const CStdString& strArtist /* = "" */, const CStdString& strAlbum /* = "" */) const
{
  if (strFilename.empty())
    return strFilename;

  CURL fileName(strFilename);
  if (!URIUtils::IsInternetStream(fileName))
  {
    CStdString strReturn;
    switch (type)
    {
    case CONTENT_ADDON_TYPE_SONG:
      {
        if (!strArtist.empty())
        {
          if (!strAlbum.empty())
            strReturn.Format("%s%s/%s/%s/%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_ARTIST, strArtist.c_str(), strAlbum.c_str(), strFilename);
          else
            strReturn.Format("%s%s/%s/%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_ARTIST, strArtist.c_str(), strFilename);
        }
        else
          strReturn.Format("%s%s/%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_SONG, strFilename);
      }
      break;
    case CONTENT_ADDON_TYPE_ARTIST:
      strReturn.Format("%s%s/%s/%s/", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_ARTIST, strFilename.c_str());
      break;
    case CONTENT_ADDON_TYPE_PLAYLIST:
      strReturn.Format("%s%s/%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_PLAYLIST, strFilename.c_str());
      break;
    case CONTENT_ADDON_TYPE_DIRECTORY:
      strReturn.Format("%s%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), strFilename.c_str());
      break;
    case CONTENT_ADDON_TYPE_ALBUM:
      {
        if (strArtist.empty())
          strReturn.Format("%s%s/%s/%s/", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_ALBUM, strFilename.c_str());
        else
          strReturn.Format("%s%s/%s/%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_ARTIST, strArtist.c_str(), strFilename.c_str());
      }
      break;
    default:
      break;
    }

    return strReturn;
  }

  return strFilename;
}

CStdString CContentAddon::ContentBuildPath(const CStdString& strPath)
{
  if (strPath.empty() || URIUtils::IsInternetStream(CURL(strPath)))
    return strPath;

  CStdString retVal;
  if (strPath.Equals("[logo]"))
    retVal = Icon();
  else
    retVal.Format("%s%s/%s", CONTENT_NODE, ID().c_str(), strPath.c_str());
  return retVal;
}

bool CContentAddon::FileOpen(const CStdString& strFileName, CONTENT_HANDLE* handle)
{
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  if (ProvidesFiles())
  {
    CStdString strFilePath = GetFilename(strFileName);
    try { err = m_pStruct->FileOpen(strFilePath.c_str(), handle); }
    catch (exception &e) { LogException(e, __FUNCTION__); return false; }

    if (err != CONTENT_ERROR_NO_ERROR)
      CLog::Log(LOGERROR, "add-on '%s' returned an error from FileOpen(%s): %d", Name().c_str(), strFileName.c_str(), err);
  }

  return err == CONTENT_ERROR_NO_ERROR;
}

void CContentAddon::FileClose(CONTENT_HANDLE handle)
{
  if (ProvidesFiles())
  {
    try { m_pStruct->FileClose(handle); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }
}

unsigned int CContentAddon::FileRead(CONTENT_HANDLE handle, void* pBuffer, int64_t iBufLen)
{
  if (ProvidesFiles())
  {
    try { return m_pStruct->FileRead(handle, pBuffer, iBufLen); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  return 0;
}

bool CContentAddon::FileExists(const CStdString& strFileName)
{
  if (ProvidesFiles())
  {
    CStdString strFilePath = GetFilename(strFileName);
    try { return m_pStruct->FileExists(strFilePath.c_str()) == 0; }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  return false;
}

int64_t CContentAddon::FileSeek(CONTENT_HANDLE handle, int64_t iFilePosition, int iWhence)
{
  if (ProvidesFiles())
  {
    try { return m_pStruct->FileSeek(handle, iFilePosition, iWhence); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  return 0;
}

int64_t CContentAddon::FileGetPosition(CONTENT_HANDLE handle)
{
  if (ProvidesFiles())
  {
    try { return m_pStruct->FileGetPosition(handle); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  return 0;
}

int64_t CContentAddon::FileGetLength(CONTENT_HANDLE handle)
{
  if (ProvidesFiles())
  {
    try { return m_pStruct->FileGetLength(handle); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  return 0;
}

bool CContentAddon::FileGetDirectory(CFileItemList& items, const CStdString& strPath)
{
  if (ProvidesFiles())
  {
    CONTENT_ADDON_FILELIST* retVal = NULL;
    CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);
    CLog::Log(LOGDEBUG, "getting directory '%s' from add-on '%s'", strPath.c_str(), Name().c_str());

    try { err = m_pStruct->FileGetDirectory(&retVal, strPath.c_str()); }
    catch (exception &e) { LogException(e, __FUNCTION__); }

    if (err == CONTENT_ERROR_NO_ERROR && retVal)
    {
      ReadFiles(retVal, items);
      FreeFileList(retVal);
      return true;
    }
  }

  return false;
}
