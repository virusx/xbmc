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

#include "ContentAddons.h"
#include "Application.h"
#include "filesystem/AddonsDirectory.h"
#include "filesystem/MusicDatabaseDirectory.h"
#include "MediaSource.h"
#include "utils/StringUtils.h"
#include "URL.h"

using namespace std;
using namespace ADDON;
using namespace XFILE;
using namespace XFILE::MUSICDATABASEDIRECTORY;

//TODO remove musicdb:// hacks

CContentAddons& CContentAddons::Get(void)
{
  static CContentAddons instance;
  return instance;
}

bool CContentAddons::HasAvailableMusicAddons(void) const
{
  CONTENT_CLIENTMAP map;
  {
    CSingleLock lock(m_critSection);
    map = m_addonMap;
  }
  for (CONTENT_CLIENTMAP_CITR it = map.begin(); it != map.end(); it++)
  {
    if (it->second->ProvidesMusicCodec())//TODO
      return true;
  }
  return false;
}

bool CContentAddons::SupportsConcurrentStreams(const CStdString& strPath) const
{
  CONTENT_ADDON addon = GetAddonForPath(strPath);
  if (!addon.get())
    return true;
  return addon->SupportsConcurrentStreams();
}

bool CContentAddons::MusicGetAddons(CFileItemList& items) const
{
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
  {
    if (it->second->ProvidesMusicFiles())
    {
      AddonPtr addon = boost::dynamic_pointer_cast<IAddon>(it->second);
      items.Add(CAddonsDirectory::FileItemFromAddon(addon, MUSIC_VIRTUAL_NODE, true));
    }
  }
  return true;
}

bool CContentAddons::MusicGetOverviewItems(CFileItemList& items)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetOverviewItems(items);
  return bReturn;
}

bool CContentAddons::MusicGetArtists(CFileItemList& items)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetArtists(items);
  return bReturn;
}

bool CContentAddons::MusicGetAlbums(CFileItemList& items, const CStdString& strArtistName)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetAlbums(items, strArtistName);
  return bReturn;
}

bool CContentAddons::MusicGetSongs(CFileItemList& items, const CStdString& strArtistName, const CStdString& strAlbumName)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetSongs(items, strArtistName, strAlbumName);
  return bReturn;
}

bool CContentAddons::MusicGetPlaylists(CFileItemList& items)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetPlaylists(items);
  return bReturn;
}

bool CContentAddons::MusicGetTop100(CFileItemList& items, CONTENT_TOP100_TYPE type)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetTop100(items, type);
  return bReturn;
}

bool CContentAddons::MusicSearch(CFileItemList& items, const CStdString& strQuery)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicSearch(items, strQuery);
  return bReturn;
}

bool CContentAddons::MusicGetContextButtons(const CFileItemPtr& item, CContextButtons &buttons)
{
  CONTENT_ADDON addon = GetAddonForPath(item->GetPath());
  if (addon.get())
    return addon->MusicGetContextButtons(item, buttons);
  return false;
}

bool CContentAddons::MusicClickContextButton(const CFileItemPtr& item, CONTEXT_BUTTON button)
{
  CONTENT_ADDON addon = GetAddonForPath(item->GetPath());
  if (addon.get())
    return addon->MusicClickContextButton(item, button);
  return false;
}

bool CContentAddons::IsSupported(const CStdString& strPath) const
{
  CONTENT_ADDON addon = GetAddonForPath(strPath);
  return addon.get() != NULL;
}

bool CContentAddons::IsPlugin(const CStdString& strPath)
{
  CURL url(strPath);
  if (url.GetProtocol().Equals("content"))
    return true;
  if (url.GetProtocol().Equals("musicdb") && CMusicDatabaseDirectory::IsContentAddonDir(strPath))
    return true;
  return false;
}

CONTENT_ADDON CContentAddons::GetAddonForPath(const CStdString& strPath) const
{
  CONTENT_ADDON retval;

  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
  {
    CONTENT_ADDON addon = it->second;
    if (addon->SupportsFile(strPath))
    {
      retval = addon;
      break;
    }
  }

  return retval;
}

CONTENT_ADDON CContentAddons::GetAddonByID(const CStdString& strID) const
{
  CONTENT_ADDON retval;

  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
  {
    if (it->second->ID().Equals(strID))
    {
      retval = it->second;
      break;
    }
  }

  return retval;
}
