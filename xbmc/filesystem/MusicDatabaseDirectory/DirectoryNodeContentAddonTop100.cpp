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

#include "DirectoryNodeContentAddonTop100.h"
#include "addons/ContentAddons.h"

using namespace ADDON;
using namespace XFILE::MUSICDATABASEDIRECTORY;

CDirectoryNodeContentAddonTop100::CDirectoryNodeContentAddonTop100(const CStdString& strName, CDirectoryNode* pParent)
  : CContentAddonDirectoryNode(NODE_TYPE_CONTENT_ADDON_TOP100, strName, pParent)
{
}

NODE_TYPE CDirectoryNodeContentAddonTop100::GetChildType() const
{
  CStdString strName = GetName();
  if (strName.Equals(MUSIC_ARTIST))
    return NODE_TYPE_CONTENT_ADDON_ARTISTTOP100;
  else if (strName.Equals(MUSIC_ALBUM))
    return NODE_TYPE_CONTENT_ADDON_ALBUMTOP100;
  else if (strName.Equals(MUSIC_SONG))
    return NODE_TYPE_CONTENT_ADDON_TOP100SONG;
  return NODE_TYPE_NONE;
}

CStdString CDirectoryNodeContentAddonTop100::GetLocalizedName() const
{
  CStdString strName = GetName();
  if (strName.Equals(MUSIC_ARTIST))
    return g_localizeStrings.Get(10512);
  else if (strName.Equals(MUSIC_ALBUM))
    return g_localizeStrings.Get(10505);
  else if (strName.Equals(MUSIC_SONG))
    return g_localizeStrings.Get(10504);

  return "";
}

bool CDirectoryNodeContentAddonTop100::GetContent(CFileItemList& items) const
{
  CONTENT_ADDON addon = GetAddon();
  if (addon.get())
    return addon->MusicGetTop100Overview(items);

  return false;
}
