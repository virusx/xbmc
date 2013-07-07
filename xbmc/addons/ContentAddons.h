#pragma once
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

#include "BinaryAddonManager.h"
#include "ContentAddon.h"
#include "dialogs/GUIDialogContextMenu.h"
#include "cores/paplayer/ICodec.h"
#include "filesystem/MusicDatabaseDirectory/DirectoryNode.h"

namespace ADDON
{
  typedef std::map<CStdString, boost::shared_ptr<CContentAddon> >                 CONTENT_CLIENTMAP;
  typedef std::map<CStdString, boost::shared_ptr<CContentAddon> >::iterator       CONTENT_CLIENTMAP_ITR;
  typedef std::map<CStdString, boost::shared_ptr<CContentAddon> >::const_iterator CONTENT_CLIENTMAP_CITR;
  typedef boost::shared_ptr<CContentAddon>                                        CONTENT_ADDON;

  class CContentAddons :
    public CBinaryAddonManager<CContentAddon>
  {
  public:
    static CContentAddons& Get(void);
    virtual ~CContentAddons(void) {};

    bool HasAvailableMusicAddons(void) const;
    bool SupportsConcurrentStreams(const CStdString& strPath) const;
    bool MusicGetAddons(CFileItemList& items) const;
    bool MusicGetOverviewItems(CFileItemList& items);
    bool MusicGetArtists(CFileItemList& items);
    bool MusicGetAlbums(CFileItemList& items, const CStdString& strArtistName);
    bool MusicGetSongs(CFileItemList& items, const CStdString& strArtistName, const CStdString& strAlbumName);
    bool MusicGetPlaylists(CFileItemList& items);
    bool MusicGetTop100(CFileItemList& items, CONTENT_TOP100_TYPE type);
    bool MusicSearch(CFileItemList& items, const CStdString& query);
    bool MusicGetContextButtons(const CFileItemPtr& item, CContextButtons &buttons);
    bool MusicClickContextButton(const CFileItemPtr& item, CONTEXT_BUTTON button);

    CONTENT_ADDON GetAddonForPath(const CStdString& strPath) const;
    CONTENT_ADDON GetAddonByID(const CStdString& strID) const;
    bool IsSupported(const CStdString& strPath) const;
    /*!
     * \brief Returns true if the path is to a virtual content add-on item.
     */
    static bool IsPlugin(const CStdString& strFile);
  private:
    CContentAddons(void) :
      CBinaryAddonManager<CContentAddon>(ADDON_CONTENTDLL) {}
  };
}
