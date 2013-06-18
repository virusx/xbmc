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

#include "DirectoryNodeContentAddonTop100Song.h"
#include "addons/ContentAddons.h"

using namespace XFILE::MUSICDATABASEDIRECTORY;

CDirectoryNodeContentAddonTop100Song::CDirectoryNodeContentAddonTop100Song(const CStdString& strName, CDirectoryNode* pParent)
  : CContentAddonDirectoryNode(NODE_TYPE_CONTENT_ADDON_TOP100SONG, strName, pParent)
{
}

NODE_TYPE CDirectoryNodeContentAddonTop100Song::GetChildType() const
{
  return NODE_TYPE_CONTENT_ADDON_SONG;
}

bool CDirectoryNodeContentAddonTop100Song::GetContent(CFileItemList& items) const
{
  return ADDON::CContentAddons::Get().MusicGetTop100(items, CONTENT_TOP100_TYPE_SONGS);
}
