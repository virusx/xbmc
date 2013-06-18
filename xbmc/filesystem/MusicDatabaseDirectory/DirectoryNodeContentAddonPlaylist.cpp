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

#include "DirectoryNodeContentAddonPlaylist.h"
#include "DirectoryNodeContentAddonOverview.h"
#include "addons/ContentAddons.h"

using namespace ADDON;
using namespace XFILE::MUSICDATABASEDIRECTORY;

CDirectoryNodeContentAddonPlaylist::CDirectoryNodeContentAddonPlaylist(const CStdString& strName, CDirectoryNode* pParent) :
  CContentAddonDirectoryNode(NODE_TYPE_CONTENT_ADDON_PLAYLIST, strName, pParent)
{
}

NODE_TYPE CDirectoryNodeContentAddonPlaylist::GetChildType() const
{
  return NODE_TYPE_CONTENT_ADDON_SONG;
}

CStdString CDirectoryNodeContentAddonPlaylist::GetLocalizedName() const
{
  CONTENT_ADDON addon = GetAddon();
  if (addon.get())
    return addon->MusicGetPlaylistName(GetName());
  return g_localizeStrings.Get(136);
}

bool CDirectoryNodeContentAddonPlaylist::GetContent(CFileItemList& items) const
{
  CONTENT_ADDON addon = GetAddon();
  if (addon.get())
    return addon->MusicGetPlaylists(items);
  return false;
}
