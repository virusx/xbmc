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

#include "DirectoryNodeContentAddonAlbum.h"
#include "DirectoryNodeContentAddonOverview.h"
#include "addons/ContentAddons.h"

using namespace ADDON;
using namespace XFILE::MUSICDATABASEDIRECTORY;

CDirectoryNodeContentAddonAlbum::CDirectoryNodeContentAddonAlbum(const CStdString& strName, CDirectoryNode* pParent) :
  CContentAddonDirectoryNode(NODE_TYPE_CONTENT_ADDON_ALBUM, strName, pParent)
{
}

NODE_TYPE CDirectoryNodeContentAddonAlbum::GetChildType() const
{
  return NODE_TYPE_CONTENT_ADDON_SONG;
}

CStdString CDirectoryNodeContentAddonAlbum::GetLocalizedName() const
{
  CStdString strReturn;
  CONTENT_ADDON addon = GetAddon();
  if (addon.get())
    strReturn = addon->MusicGetAlbumName(Artist(), GetName());
  return strReturn;
}

bool CDirectoryNodeContentAddonAlbum::GetContent(CFileItemList& items) const
{
  CONTENT_ADDON addon = GetAddon();
  if (addon.get())
    return addon->MusicGetAlbums(items, Artist());
  return false;
}
