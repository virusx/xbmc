/*
 *      Copyright (C) 2005-2012 Team XBMC
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
#include "addons/ContentAddons.h"
#include "threads/SystemClock.h"
#include "MusicSearchDirectory.h"
#include "music/MusicDatabase.h"
#include "URL.h"
#include "FileItem.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"
#include "utils/URIUtils.h" // for MUSICSEARCH_TARGET_* defines
#include "guilib/LocalizeStrings.h"

using namespace ADDON;
using namespace XFILE;

CMusicSearchDirectory::CMusicSearchDirectory(void)
{
}

CMusicSearchDirectory::~CMusicSearchDirectory(void)
{
}

bool CMusicSearchDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
  // break up our path
  // format is: musicsearch://<target>/<search string>
  // target is: MUSICSEARCH_TARGET_ALL, MUSICSEARCH_TARGET_LIBRARY or content add-on ID
  CURL url(strPath);
  CStdString target(url.GetHostName());
  CStdString search(url.GetFileName());

  if (search.IsEmpty())
    return false;

  CONTENT_ADDON addon;
  if (!target.Equals(MUSICSEARCH_TARGET_ALL) && !target.Equals(MUSICSEARCH_TARGET_LIBRARY) &&
      !(addon = CContentAddons::Get().GetAddonByID(target)))
    return false;

  // and retrieve the search details
  items.SetPath(strPath);
  unsigned int time = XbmcThreads::SystemClockMillis();
  if (target.Equals(MUSICSEARCH_TARGET_ALL))
    CContentAddons::Get().MusicSearch(items, search);
  else if (!target.Equals(MUSICSEARCH_TARGET_LIBRARY))
  {
    // target is a content add-on ID
    if (addon->ProvidesMusicFiles())
      addon->MusicSearch(items, search);
  }
  if (target.Equals(MUSICSEARCH_TARGET_ALL) || target.Equals(MUSICSEARCH_TARGET_LIBRARY))
  {
    CMusicDatabase db;
    if (db.Open())
    {
      db.Search(search, items);
      db.Close();
    }
  }
  CLog::Log(LOGDEBUG, "%s (%s) took %u ms",
            __FUNCTION__, strPath.c_str(), XbmcThreads::SystemClockMillis() - time);
  items.SetLabel(g_localizeStrings.Get(137)); // Search
  return true;
}

bool CMusicSearchDirectory::Exists(const char* strPath)
{
  return true;
}
