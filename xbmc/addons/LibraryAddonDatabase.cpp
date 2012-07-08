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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

//#include "system.h"
#include "LibraryAddonDatabase.h"
#include "settings/AdvancedSettings.h"
//#include "settings/GUISettings.h"
//#include "settings/Settings.h"
#include "GUIInfoManager.h"
#include "interfaces/python/XBPython.h"
#include "utils/log.h"

//using namespace std;
//using namespace dbiplus;
//using namespace XFILE;
//using namespace VIDEO;
using namespace ADDON;


//*********************************************************************************************************************
CLibraryAddonDatabase::CLibraryAddonDatabase(const ADDON::LibraryPtr &addon) : m_addon(addon)
{
}

//*********************************************************************************************************************
bool CLibraryAddonDatabase::Open()
{
  // Augment the settings with our database name
  DatabaseSettings dbsettings(g_advancedSettings.m_databaseLibraryAddons);
  // Addons define their own db name, so dbsettings.name is really the prefix (hack for now)
  dbsettings.name = dbsettings.name + GetBaseDBName();
  return CDatabase::Open(dbsettings);
}

bool CLibraryAddonDatabase::CommitTransaction()
{
  if (CDatabase::CommitTransaction())
  { // number of items in the db has likely changed, so reset the infomanager cache
    //g_infoManager.SetLibraryBool(LIBRARY_HAS_MUSIC, GetSongsCount("") > 0);
    //g_infoManager.SetLibraryBool(LIBRARY_HAS_MOVIES, HasContent(VIDEODB_CONTENT_MOVIES));
    //g_infoManager.SetLibraryBool(LIBRARY_HAS_TVSHOWS, HasContent(VIDEODB_CONTENT_TVSHOWS));
    //g_infoManager.SetLibraryBool(LIBRARY_HAS_MUSICVIDEOS, HasContent(VIDEODB_CONTENT_MUSICVIDEOS));
    return true;
  }
  return false;
}

bool CLibraryAddonDatabase::UpdateOldVersion(int version)
{
  if (!CDatabase::UpdateOldVersion(version))
    return false;

  // Call our Python script with the old version
  std::vector<CStdString> argv;
  argv.push_back(m_addon->LibPath());
  argv.push_back("updateoldversion");
  CStdString oldVersion;
  oldVersion.Format("%i", version);
  argv.push_back(oldVersion);
  g_pythonParser.evalFile(argv[0], argv, m_addon);

  return true;
}

int CLibraryAddonDatabase::GetMinVersion() const
{
  return m_addon->GetDatabaseVersion();
}

const char* CLibraryAddonDatabase::GetBaseDBName() const
{
  return m_addon->GetDatabaseName().c_str();
}
