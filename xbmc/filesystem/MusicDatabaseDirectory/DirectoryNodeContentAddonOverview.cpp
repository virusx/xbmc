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

#include "DirectoryNodeContentAddonOverview.h"
#include "DirectoryNodeContentAddon.h"
#include "addons/ContentAddons.h"

using namespace XFILE::MUSICDATABASEDIRECTORY;
using namespace ADDON;

CDirectoryNodeContentAddonOverview::CDirectoryNodeContentAddonOverview(const CStdString& strName, CDirectoryNode* pParent) :
    CContentAddonDirectoryNode(NODE_TYPE_CONTENT_ADDON_OVERVIEW, strName, pParent)
{
}

NODE_TYPE CDirectoryNodeContentAddonOverview::GetChildType() const
{
  CStdString strName = GetName();
  if (strName.Equals(MUSIC_ARTIST))
    return NODE_TYPE_CONTENT_ADDON_ARTIST;
  else if (strName.Equals(MUSIC_ALBUM))
    return NODE_TYPE_CONTENT_ADDON_ALBUM;
  else if (strName.Equals(MUSIC_SONG))
    return NODE_TYPE_CONTENT_ADDON_SONG;
  else if (strName.Equals(MUSIC_TOP100))
    return NODE_TYPE_CONTENT_ADDON_TOP100;
  else if (strName.Equals(MUSIC_PLAYLIST))
    return NODE_TYPE_CONTENT_ADDON_PLAYLIST;
  return NODE_TYPE_NONE;
}

CStdString CDirectoryNodeContentAddonOverview::GetLocalizedName() const
{
  CStdString strName = GetName();
  if (strName.Equals(MUSIC_ARTIST))
    return g_localizeStrings.Get(133);
  else if (strName.Equals(MUSIC_ALBUM))
    return g_localizeStrings.Get(132);
  else if (strName.Equals(MUSIC_SONG))
    return g_localizeStrings.Get(134);
  else if (strName.Equals(MUSIC_TOP100))
    return g_localizeStrings.Get(271);
  else if (strName.Equals(MUSIC_PLAYLIST))
    return g_localizeStrings.Get(136);

  return "";
}

bool CDirectoryNodeContentAddonOverview::GetContent(CFileItemList& items) const
{
  CONTENT_ADDON addon = GetAddon();
  if (addon.get())
    return addon->MusicGetOverview(items);

  return false;
}
