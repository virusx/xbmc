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

#include "DirectoryNodeContentAddon.h"
#include "addons/ContentAddons.h"

using namespace ADDON;
using namespace XFILE::MUSICDATABASEDIRECTORY;

CDirectoryNodeContentAddon::CDirectoryNodeContentAddon(const CStdString& strName, CDirectoryNode* pParent) :
  CContentAddonDirectoryNode(NODE_TYPE_CONTENT_ADDON, strName, pParent)
{}

NODE_TYPE CDirectoryNodeContentAddon::GetChildType() const
{
  return NODE_TYPE_CONTENT_ADDON_OVERVIEW;
}

CStdString CDirectoryNodeContentAddon::GetLocalizedName() const
{
  CONTENT_ADDON addon = GetAddon();
  if (addon.get())
    return addon->Name();
  return "";
}

CONTENT_ADDON CDirectoryNodeContentAddon::GetAddon(void) const
{
  return CContentAddons::Get().GetAddonByID(GetName());
}
