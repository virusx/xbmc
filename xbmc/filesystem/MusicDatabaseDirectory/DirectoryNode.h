#pragma once
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

#include "utils/StdString.h"
#include "utils/UrlOptions.h"
#include <boost/shared_ptr.hpp>

class CFileItemList;

namespace ADDON { class CContentAddon; }

namespace XFILE
{
  namespace MUSICDATABASEDIRECTORY
  {
    class CQueryParams;

    typedef enum _NODE_TYPE
    {
      NODE_TYPE_NONE=0,
      NODE_TYPE_ROOT,
      NODE_TYPE_OVERVIEW,
      NODE_TYPE_TOP100,
      NODE_TYPE_GENRE,
      NODE_TYPE_ARTIST,
      NODE_TYPE_ARTIST_TOP100,
      NODE_TYPE_ALBUM,
      NODE_TYPE_ALBUM_RECENTLY_ADDED,
      NODE_TYPE_ALBUM_RECENTLY_ADDED_SONGS,
      NODE_TYPE_ALBUM_RECENTLY_PLAYED,
      NODE_TYPE_ALBUM_RECENTLY_PLAYED_SONGS,
      NODE_TYPE_ALBUM_TOP100,
      NODE_TYPE_ALBUM_TOP100_SONGS,
      NODE_TYPE_ALBUM_COMPILATIONS,
      NODE_TYPE_ALBUM_COMPILATIONS_SONGS,
      NODE_TYPE_SONG,
      NODE_TYPE_SONG_TOP100,
      NODE_TYPE_YEAR,
      NODE_TYPE_YEAR_ALBUM,
      NODE_TYPE_YEAR_SONG,
      NODE_TYPE_SINGLES,
      NODE_TYPE_CONTENT_ADDON,
      NODE_TYPE_CONTENT_ADDON_OVERVIEW,
      NODE_TYPE_CONTENT_ADDON_ARTIST,
      NODE_TYPE_CONTENT_ADDON_ALBUM,
      NODE_TYPE_CONTENT_ADDON_SONG,
      NODE_TYPE_CONTENT_ADDON_TOP100,
      NODE_TYPE_CONTENT_ADDON_TOP100SONG,
      NODE_TYPE_CONTENT_ADDON_ALBUMTOP100,
      NODE_TYPE_CONTENT_ADDON_ARTISTTOP100,
      NODE_TYPE_CONTENT_ADDON_PLAYLIST
    } NODE_TYPE;

    typedef struct {
      NODE_TYPE node;
      int       id;
      int       label;
    } Node;

    class CDirectoryNode
    {
    public:
      static CDirectoryNode* ParseURL(const CStdString& strPath);
      static void GetDatabaseInfo(const CStdString& strPath, CQueryParams& params);
      virtual ~CDirectoryNode();

      NODE_TYPE GetType() const;

      bool GetChilds(CFileItemList& items);
      virtual NODE_TYPE GetChildType() const;
      virtual CStdString GetLocalizedName() const;

      CDirectoryNode* GetParent() const;
      bool CanCache() const;

    protected:
      CDirectoryNode(NODE_TYPE Type, const CStdString& strName, CDirectoryNode* pParent);
      static CDirectoryNode* CreateNode(NODE_TYPE Type, const CStdString& strName, CDirectoryNode* pParent);

      void AddOptions(const CStdString &options);
      void CollectQueryParams(CQueryParams& params) const;

      const CStdString& GetName() const;
      int GetID() const;
      void RemoveParent();

      virtual bool GetContent(CFileItemList& items) const;

      CStdString BuildPath() const;

    private:
      void AddQueuingFolder(CFileItemList& items) const;

    private:
      NODE_TYPE m_Type;
      CStdString m_strName;
      CDirectoryNode* m_pParent;
      CUrlOptions m_options;
    };

    class CContentAddonDirectoryNode : public CDirectoryNode
    {
    public:
      CContentAddonDirectoryNode(NODE_TYPE Type, const CStdString& strName, CDirectoryNode* pParent) :
        CDirectoryNode(Type, strName, pParent) {}
      virtual boost::shared_ptr<ADDON::CContentAddon> GetAddon(void) const;

      virtual CStdString Artist(void) const;
      virtual CStdString Album(void) const;
      virtual CStdString Song(void) const;
      virtual CStdString Playlist(void) const;
      virtual CStdString Filename(void) const { CStdString strName(GetName()); return strName; }
    };
  }
}


