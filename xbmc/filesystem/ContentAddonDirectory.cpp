/*
 *      Copyright (C) 2013 Team XBMC
 *      http://www.xbmc.org
 *
 * This Program is free software; you can redistribute it and/or modify
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

#include "ContentAddonDirectory.h"
#include "FileItem.h"
#include "Util.h"
#include "URL.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "guilib/LocalizeStrings.h"
#include "addons/ContentAddons.h"

using namespace std;
using namespace XFILE;
using namespace ADDON;

CContentAddonDirectory::CContentAddonDirectory()
{
}

CContentAddonDirectory::~CContentAddonDirectory()
{
}

bool CContentAddonDirectory::Exists(const char* strPath)
{
  CLog::Log(LOGDEBUG, "TODO: %s(%s)", __FUNCTION__, strPath);
  return false;
}

bool CContentAddonDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
  CLog::Log(LOGDEBUG, "TODO: %s(%s)", __FUNCTION__, strPath.c_str());
  return false;
}

bool CContentAddonDirectory::SupportsWriteFileOperations(const CStdString& strPath)
{
  return false;
}

