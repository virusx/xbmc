/*
 *      Copyright (C) 2012 Team XBMC
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

#include "PictureDatabase.h"
#include "PictureInfoTag.h"
#include "dbwrappers/dataset.h"
#include "dialogs/GUIDialogProgress.h"
#include "settings/AdvancedSettings.h"
#include "threads/SystemClock.h"
#include "utils/JSONVariantParser.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"
#include "FileItem.h"

#include <vector>
#include <set>
#include <queue>

using namespace std;

CPictureDatabase::CPictureDatabase() : CDynamicDatabase("picture")
{
  BeginDeclarations();
  DeclareIndex("file", "VARCHAR(256)");
  DeclareOneToMany("path", "VARCHAR(512)");
  DeclareOneToMany("folder", "VARCHAR(64)");
  DeclareOneToMany("year", "INTEGER");
  DeclareOneToMany("camera", "VARCHAR(72)"); // EXIF model is 32 bytes, make is 40

  // Version 2
  DeclareManyToMany("tag", "VARCHAR(64)");

  // Version 3
  DeclareIndex("aperturefnumber", "FLOAT");
  DeclareIndex("author", "VARCHAR(256)"); // MAX_IPTC_STRING
  DeclareIndex("byline", "VARCHAR(256)");
  DeclareIndex("bylinetitle", "VARCHAR(256)");
  DeclareIndex("cameramake", "VARCHAR(256)");
  DeclareIndex("cameramodel", "VARCHAR(256)");
  DeclareIndex("caption", "VARCHAR(256)");
  DeclareIndex("category", "VARCHAR(256)");
  DeclareIndex("ccdwidth", "FLOAT");
  DeclareOneToMany("city", "VARCHAR(256)");
  DeclareIndex("comments", "VARCHAR(256)");
  DeclareIndex("copyright", "VARCHAR(256)");
  DeclareIndex("copyrightnotice", "VARCHAR(256)");
  DeclareOneToMany("country", "VARCHAR(256)");
  DeclareIndex("countrycode", "VARCHAR(256)");
  DeclareIndex("credit", "VARCHAR(256)");
  DeclareIndex("date", "VARCHAR(256)");
  DeclareIndex("datetime", "VARCHAR(19)");
  DeclareIndex("datetimeoffsets", "VARCHAR(256)");
  DeclareIndex("description", "VARCHAR(256)");
  DeclareIndex("digitalzoomratio", "FLOAT");
  DeclareIndex("distance", "FLOAT");
  DeclareIndex("exposurebias", "FLOAT");
  DeclareIndex("exposuremode", "INTEGER");
  DeclareIndex("exposureprogram", "INTEGER");
  DeclareIndex("exposuretime", "FLOAT");
  DeclareIndex("flashused", "INTEGER");
  DeclareIndex("focallength", "FLOAT");
  DeclareIndex("focallength35mmequiv", "INTEGER");
  // Ignore GPS info tags for now
  DeclareIndex("gpsinfopresent", "INTEGER");
  DeclareIndex("headline", "VARCHAR(256)");
  DeclareIndex("height", "INTEGER");
  DeclareIndex("iscolor", "INTEGER");
  DeclareIndex("isoequivalent", "INTEGER");
  DeclareIndex("keywords", "VARCHAR(256)");
  DeclareIndex("largestexifoffset", "INTEGER");
  DeclareIndex("lightsource", "INTEGER");
  DeclareIndex("meteringmode", "INTEGER");
  DeclareIndex("numdatetimetags", "INTEGER");
  DeclareIndex("objectname", "VARCHAR(256)");
  DeclareIndex("orientation", "INTEGER");
  DeclareIndex("process", "INTEGER");
  DeclareIndex("referenceservice", "VARCHAR(256)");
  DeclareIndex("size", "INTEGER");
  DeclareIndex("source", "VARCHAR(256)");
  DeclareIndex("specialinstructions", "VARCHAR(256)");
  DeclareIndex("state", "VARCHAR(256)");
  DeclareIndex("supplementalcategories", "VARCHAR(256)");
  DeclareIndex("thumbnailatend", "INTEGER");
  DeclareIndex("thumbnailoffset", "INTEGER");
  DeclareIndex("thumbnailsize", "INTEGER");
  DeclareIndex("thumbnailsizeoffset", "INTEGER");
  DeclareIndex("transmissionreference", "VARCHAR(256)");
  DeclareIndex("whitebalance", "INTEGER");
  DeclareIndex("width", "INTEGER");
}

bool CPictureDatabase::Open()
{
  return CDynamicDatabase::Open(g_advancedSettings.m_databasePictures);
}

bool CPictureDatabase::CreateTables()
{
  try
  {
    BeginTransaction();
    if (!CDynamicDatabase::CreateTables())
      return false;

    // Add the hash column to the path table
    CStdString strSQL = PrepareSQL("ALTER TABLE path ADD hash TEXT");
    m_pDS->exec(strSQL.c_str());

    CommitTransaction();
    return true;
  }
  catch (dbiplus::DbErrors&)
  {
    CLog::Log(LOGERROR, "PictureDatabase: unable to create tables (error %i)", (int)GetLastError());
    RollbackTransaction();
  }
  return false;
}

bool CPictureDatabase::UpdateOldVersion(int version)
{
  if (version < 1)
  {
    BeginDeclarations();
    DeclareIndex("file", "VARCHAR(128)");
    DeclareOneToMany("path", "VARCHAR(512)");
    DeclareOneToMany("folder", "VARCHAR(64)");
    DeclareOneToMany("year", "INTEGER");
    DeclareOneToMany("camera", "VARCHAR(72)");

    // Add the hash column to the path table
    CStdString strSQL = PrepareSQL("ALTER TABLE path ADD hash TEXT");
    m_pDS->exec(strSQL.c_str());
  }
  if (version < 2)
  {
    BeginDeclarations();
    DeclareIndex("file", "VARCHAR(128)");
    DeclareOneToMany("path", "VARCHAR(512)");
    DeclareOneToMany("folder", "VARCHAR(64)");
    DeclareOneToMany("year", "INTEGER");
    DeclareOneToMany("camera", "VARCHAR(72)");

    AddManyToMany("tag", "VARCHAR(64)");
  }
  if (version < 3)
  {
    BeginDeclarations();
    DeclareIndex("file", "VARCHAR(128)");
    DeclareOneToMany("path", "VARCHAR(512)");
    DeclareOneToMany("folder", "VARCHAR(64)");
    DeclareOneToMany("year", "INTEGER");
    DeclareOneToMany("camera", "VARCHAR(72)"); // EXIF model is 32 bytes, make is 40
    DeclareManyToMany("tag", "VARCHAR(64)");

    CLog::Log(LOGDEBUG, "DynDB: Expanding tables...");
    unsigned int start = XbmcThreads::SystemClockMillis();

    AddIndex("aperturefnumber", "FLOAT");
    AddIndex("author", "VARCHAR(256)"); // MAX_IPTC_STRING
    AddIndex("byline", "VARCHAR(256)");
    AddIndex("bylinetitle", "VARCHAR(256)");
    AddIndex("cameramake", "VARCHAR(256)");
    AddIndex("cameramodel", "VARCHAR(256)");
    AddIndex("caption", "VARCHAR(256)");
    AddIndex("category", "VARCHAR(256)");
    AddIndex("ccdwidth", "FLOAT");
    AddOneToMany("city", "VARCHAR(256)");
    AddIndex("comments", "VARCHAR(256)");
    AddIndex("copyright", "VARCHAR(256)");
    AddIndex("copyrightnotice", "VARCHAR(256)");
    AddOneToMany("country", "VARCHAR(256)");
    AddIndex("countrycode", "VARCHAR(256)");
    AddIndex("credit", "VARCHAR(256)");
    AddIndex("date", "VARCHAR(256)");
    AddIndex("datetime", "VARCHAR(19)");
    AddIndex("datetimeoffsets", "VARCHAR(256)");
    AddIndex("description", "VARCHAR(256)");
    AddIndex("digitalzoomratio", "FLOAT");
    AddIndex("distance", "FLOAT");
    AddIndex("exposurebias", "FLOAT");
    AddIndex("exposuremode", "INTEGER");
    AddIndex("exposureprogram", "INTEGER");
    AddIndex("exposuretime", "FLOAT");
    AddIndex("flashused", "INTEGER");
    AddIndex("focallength", "FLOAT");
    AddIndex("focallength35mmequiv", "INTEGER");
    // Ignore GPS info tags for now
    AddIndex("gpsinfopresent", "INTEGER");
    AddIndex("headline", "VARCHAR(256)");
    AddIndex("height", "INTEGER");
    AddIndex("iscolor", "INTEGER");
    AddIndex("isoequivalent", "INTEGER");
    AddIndex("keywords", "VARCHAR(256)");
    AddIndex("largestexifoffset", "INTEGER");
    AddIndex("lightsource", "INTEGER");
    AddIndex("meteringmode", "INTEGER");
    AddIndex("numdatetimetags", "INTEGER");
    AddIndex("objectname", "VARCHAR(256)");
    AddIndex("orientation", "INTEGER");
    AddIndex("process", "INTEGER");
    AddIndex("referenceservice", "VARCHAR(256)");
    AddIndex("size", "INTEGER");
    AddIndex("source", "VARCHAR(256)");
    AddIndex("specialinstructions", "VARCHAR(256)");
    AddIndex("state", "VARCHAR(256)");
    AddIndex("supplementalcategories", "VARCHAR(256)");
    AddIndex("thumbnailatend", "INTEGER");
    AddIndex("thumbnailoffset", "INTEGER");
    AddIndex("thumbnailsize", "INTEGER");
    AddIndex("thumbnailsizeoffset", "INTEGER");
    AddIndex("transmissionreference", "VARCHAR(256)");
    AddIndex("whitebalance", "INTEGER");
    AddIndex("width", "INTEGER");

    unsigned int duration = XbmcThreads::SystemClockMillis() - start;
    CLog::Log(LOGDEBUG, "DynDB: Expanding tables took %d ms", duration);
  }

  return true;
}

bool CPictureDatabase::Exists(const CVariant &object, int &idObject)
{
  if (!IsValid(object))
    return false;
  CStdString strSQL = PrepareSQL(
    "SELECT picture.idpicture "
    "FROM picture JOIN path ON path.idpath=picture.idpath "
    "WHERE file='%s' AND path='%s'",
    object["file"].asString().c_str(), object["path"].asString().c_str()
  );
  if (m_pDS->query(strSQL.c_str()))
  {
    bool bFound = false;
    if (m_pDS->num_rows() != 0)
    {
      idObject = m_pDS->fv(0).get_asInt();
      bFound = true;
    }
    m_pDS->close();
    return bFound;
  }
  return false;
}

bool CPictureDatabase::IsValid(const CVariant &object) const
{
  return !object["file"].isNull() && !object["path"].isNull() &&
      object["file"].asString().length() != 0 && object["path"].asString().length() != 0;
}

CFileItem* CPictureDatabase::CreateFileItem(const string &json, int id) const
{
  CPictureInfoTag p;
  DeserializeJSON(json, id, &p);
  CStdString title = p.GetFilename();
  URIUtils::RemoveExtension(title);

  CFileItem *item = new CFileItem(title);
  item->SetPath(URIUtils::AddFileToFolder(p.GetPath(), p.GetFilename()));
  *item->GetPictureInfoTag() = p;
  item->m_dwSize = p.GetFileSize();
  CDateTime datetime;
  if (p.GetDateTime(datetime))
    item->m_dateTime = datetime;
  item->m_bIsFolder = false;
  return item;
}

CFileItem* CPictureDatabase::CreateFileItem2(const string &file, const string &path, int id) const
{
  CPictureInfoTag p;
  p.Load(URIUtils::AddFileToFolder(path, file));
  
  if (!p.Loaded())
  {
    CLog::Log(LOGDEBUG, "DynDB: Failed to load pic info tag: %s", URIUtils::AddFileToFolder(path, file).c_str());
    throw "";
  }

  CStdString title = p.GetFilename();
  URIUtils::RemoveExtension(title);

  CFileItem *item = new CFileItem(title);
  item->SetPath(URIUtils::AddFileToFolder(path, file));
  *item->GetPictureInfoTag() = p;
  item->m_dwSize = p.GetFileSize();
  CDateTime datetime;
  if (p.GetDateTime(datetime))
    item->m_dateTime = datetime;
  item->m_bIsFolder = false;
  return item;
}

CFileItem* CPictureDatabase::CreateFileItem3(const string &file, const string &path, int id) const
{
  CStdString title = file;
  URIUtils::RemoveExtension(title);

  CFileItem *item = new CFileItem(title);
  item->SetPath(URIUtils::AddFileToFolder(path, file));
  item->m_bIsFolder = false;
  return item;
}

CFileItem* CPictureDatabase::CreateFileItem4(auto_ptr<dbiplus::Dataset> &pDS) const
{
  // Partially simulate accessing 66 of the info tag properties
  for (int i = 0; i < 66; i++)
    (void)pDS->fv("file").get_asString();

  CStdString title = pDS->fv("file").get_asString();
  URIUtils::RemoveExtension(title);

  CFileItem *item = new CFileItem(title);
  item->SetPath(URIUtils::AddFileToFolder(pDS->fv("path").get_asString(), pDS->fv("file").get_asString()));
  item->m_bIsFolder = false;
  return item;
}

bool CPictureDatabase::GetPaths(set<string> &paths)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  CStdString strSQL;
  paths.clear();
  try
  {
    strSQL = PrepareSQL("SELECT path FROM path");

    if (m_pDS->query(strSQL.c_str()))
    {
      while (!m_pDS->eof())
      {
        paths.insert(m_pDS->fv("path").get_asString());
        m_pDS->next();
      }
      m_pDS->close();
    }
  }
  catch (dbiplus::DbErrors &e)
  {
    CLog::Log(LOGERROR, "%s failed (%s). SQL: %s", __FUNCTION__, e.getMsg(), strSQL.c_str());
  }
  return paths.size() > 0;
}

bool CPictureDatabase::GetPathHash(const string &strDirectory, string &dbHash)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  CStdString strSQL;
  try
  {
    strSQL = PrepareSQL("SELECT hash FROM path WHERE path='%s'", strDirectory.c_str());

    if (m_pDS->query(strSQL.c_str()))
    {
      if (m_pDS->num_rows() != 0)
      {
        dbHash = m_pDS->fv("hash").get_asString();
        m_pDS->close();
        return true;
      }
      m_pDS->close();
    }
  }
  catch (dbiplus::DbErrors e)
  {
    CLog::Log(LOGERROR, "%s failed (%s). SQL: %s", __FUNCTION__, e.getMsg(), strSQL.c_str());
  }
  return false;
}

bool CPictureDatabase::SetPathHash(const string &strDirectory, const string &dbHash)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  CStdString strSQL;
  try
  {
    strSQL = PrepareSQL("UPDATE path SET hash='%s' WHERE path='%s'", dbHash.c_str(), strDirectory.c_str());

    if (m_pDS->exec(strSQL.c_str()))
      return true;
  }
  catch (dbiplus::DbErrors &e)
  {
    CLog::Log(LOGERROR, "%s failed (%s). SQL: %s", __FUNCTION__, e.getMsg(), strSQL.c_str());
  }
  return false;
}

bool CPictureDatabase::HasPath(const string &strPath)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  CStdString strSQL;
  try
  {
    strSQL = PrepareSQL("SELECT path FROM path WHERE path='%s'", strPath.c_str());

    if (m_pDS->query(strSQL.c_str()))
    {
      bool bFound = false;
      if (m_pDS->num_rows() != 0)
        bFound = true;
      m_pDS->close();
      return bFound;
    }
  }
  catch (dbiplus::DbErrors &e)
  {
    CLog::Log(LOGERROR, "%s failed (%s). SQL: %s", __FUNCTION__, e.getMsg(), strSQL.c_str());
  }
  return false;
}

bool CPictureDatabase::GetPicturesByPath(const string &path, vector<CPictureInfoTag> &pictures)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  CStdString strSQL;
  try
  {
    strSQL = PrepareSQL(
      "SELECT picture.idpicture, picture.strContent "
      "FROM picture JOIN path ON picture.idpath=path.idpath "
      "WHERE path.path='%s'",
      path.c_str()
    );

    if (m_pDS->query(strSQL.c_str()))
    {
      pictures.clear();
      while (!m_pDS->eof())
      {
        CPictureInfoTag p;
        DeserializeJSON(m_pDS->fv("strContent").get_asString(), m_pDS->fv("idpicture").get_asInt(), &p);
        pictures.push_back(p);
        m_pDS->next();
      }
      m_pDS->close();
      return true;
    }
  }
  catch (dbiplus::DbErrors &e)
  {
    CLog::Log(LOGERROR, "%s failed (%s). SQL: %s", __FUNCTION__, e.getMsg(), strSQL.c_str());
  }
  return false;
}

bool CPictureDatabase::DeletePicturesByPath(const string &path, bool recursive, CGUIDialogProgress* pDialogProgress)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  CStdString strSQL;
  try
  {
    strSQL = "SELECT idpicture "
             "FROM picture JOIN path ON path.idpath=picture.idpath ";
    if (recursive)
      strSQL += PrepareSQL("WHERE SUBSTR(path,1,%i)='%s'", StringUtils::utf8_strlen(path.c_str()), path.c_str());
    else
      strSQL += PrepareSQL("WHERE path='%s'", path.c_str());

    queue<int> ids;
    if (m_pDS->query(strSQL.c_str()))
    {
      while (!m_pDS->eof())
      {
        ids.push(m_pDS->fv(0).get_asInt());
        m_pDS->next();
      }
      m_pDS->close();
    }

    if (pDialogProgress)
    {
      // Show progress dialog if we have to connect to freedb.org
      pDialogProgress->SetHeading(1); // Pictures
      pDialogProgress->SetLine(0, 703); // Removing pictures from the library
      pDialogProgress->SetLine(1, "");
      pDialogProgress->SetLine(2, "");
      pDialogProgress->SetPercentage(0);
      pDialogProgress->ShowProgressBar(false);
      pDialogProgress->StartModal();
    }

    size_t total = ids.size();
    while (ids.size())
    {
      DeleteObject(ids.front());
      ids.pop();
      if (pDialogProgress)
      {
        pDialogProgress->SetPercentage(100 * (total - ids.size()) / total);
        pDialogProgress->Progress();
        if (pDialogProgress->IsCanceled())
        {
          pDialogProgress->Close();
          m_pDS->close();
          return false;
        }
      }
    }

    if (pDialogProgress)
      pDialogProgress->Close();

    return true;
  }
  catch (dbiplus::DbErrors &e)
  {
    CLog::Log(LOGERROR, "%s failed (%s). SQL: %s", __FUNCTION__, e.getMsg(), strSQL.c_str());
  }
  if (pDialogProgress && pDialogProgress->IsActive())
    pDialogProgress->Close();
  return false;
}
