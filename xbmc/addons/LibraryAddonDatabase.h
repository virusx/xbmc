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
#pragma once

#include "dbwrappers/Database.h"
//#include "addons/Scraper.h"
#include "addons/AddonManager.h"
#include "addons/Library.h"

//#include <set>

class CLibraryAddonDatabase : public CDatabase
{
public:
  CLibraryAddonDatabase(const ADDON::LibraryPtr &libraryAddon);
  virtual ~CLibraryAddonDatabase() { }

  virtual bool Open();
  virtual bool CommitTransaction();
  
  /* housekeeping */
  /*
  // videodb
  void CleanDatabase(VIDEO::IVideoInfoScannerObserver* pObserver=NULL, const std::set<int>* paths=NULL);
  // musicdb
  void EmptyCache();
  void Clean();
  int  Cleanup(CGUIDialogProgress *pDlgProgress=NULL);
  
  /* adding */
  
  /* scrapers */
  /*
  // videodb
  void SetScraperForPath(const CStdString& filePath, const ADDON::ScraperPtr& info, const VIDEO::SScanSettings& settings);
  ADDON::ScraperPtr GetScraperForPath(const CStdString& strPath);
  ADDON::ScraperPtr GetScraperForPath(const CStdString& strPath, VIDEO::SScanSettings& settings);
  // musicdb
  bool SetScraperForPath(const CStdString& strPath, const ADDON::ScraperPtr& info);
  bool GetScraperForPath(const CStdString& strPath, ADDON::ScraperPtr& info, const ADDON::TYPE &type);

  /* import/export */
  /*
  // videodb
  void ExportToXML(const CStdString &path, bool singleFiles = false, bool images=false, bool actorThumbs=false, bool overwrite=false);
  bool ExportSkipEntry(const CStdString &nfoFile);
  void ExportActorThumbs(const CStdString &path, const CVideoInfoTag& tag, bool singleFiles, bool overwrite=false);
  void ImportFromXML(const CStdString &path);
  void DumpToDummyFiles(const CStdString &path);
  // musicdb
  void ExportToXML(const CStdString &xmlFile, bool singleFiles = false, bool images=false, bool overwrite=false);
  void ImportFromXML(const CStdString &xmlFile);
  /**/

protected:
  virtual bool UpdateOldVersion(int version);
  virtual int GetMinVersion() const;
  virtual const char* GetBaseDBName() const;

private:
  ADDON::LibraryPtr m_addon;
};
