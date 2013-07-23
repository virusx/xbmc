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

/*!
 * @file xbmc_file_utils.hpp  C++ wrappers for content add-on files
 */

#include "xbmc_content_types.h"
#include <cstddef>
#include <string>
#include <string.h>
#include <malloc.h>
#include <vector>
#include <map>

#if defined(TARGET_WINDOWS) && !defined(strdup)
#define strdup _strdup // Silence warning C4996
#endif

/*!
 * Wrapper for CONTENT_ADDON_FILE_PROPERTY
 */
class AddonFileItemProperty
{
public:
  /*!
   * Create a new property of type string
   * @param strKey The key of this property
   * @param strValue The value of this property
   */
  AddonFileItemProperty(const std::string& strKey, const std::string& strValue) :
    m_strKey(strKey),
    m_type(CONTENT_ADDON_PROPERTY_TYPE_STRING),
    m_strValue(strValue),
    m_iValue(0)
  {
  }

  /*!
   * Create a new property of type integer
   * @param strKey The key of this property
   * @param iValue The value of this property
   */
  AddonFileItemProperty(const std::string& strKey, int iValue) :
    m_strKey(strKey),
    m_type(CONTENT_ADDON_PROPERTY_TYPE_INT),
    m_iValue(iValue)
  {
  }

  /*!
   * Parse prop into a key and value
   * @param prop The property to parse
   */
  AddonFileItemProperty(const CONTENT_ADDON_FILE_PROPERTY* prop) :
    m_strKey(prop->key),
    m_type(prop->type),
    m_strValue(prop->type == CONTENT_ADDON_PROPERTY_TYPE_STRING ? prop->strValue : ""),
    m_iValue(prop->type == CONTENT_ADDON_PROPERTY_TYPE_INT ? prop->iValue : 0)
  {
  }

  /*!
   * @return The type of this property
   */
  CONTENT_ADDON_PROPERTY_TYPE Type(void) const
  {
    return m_type;
  }

  /*!
   * @return The key of this property
   */
  std::string Key(void) const
  {
    std::string val(m_strKey);
    return val;
  }

  /*!
   * @return The string value of this property, or an empty string if this is not a string
   */
  std::string ValueAsString(void) const
  {
    std::string val(m_strValue);
    return val;
  }

  /*!
   * @return The integer value of this property, or 0 if this is not an integer
   */
  int ValueAsInt(void) const
  {
    return m_iValue;
  }

  /*!
   * @return This property as CONTENT_ADDON_FILE_PROPERTY. Must be freed by calling AddonFileItemList::Free()
   */
  CONTENT_ADDON_FILE_PROPERTY AsProperty(void) const
  {
    CONTENT_ADDON_FILE_PROPERTY prop;
    prop.key = strdup(m_strKey.c_str());
    prop.type = m_type;
    if (m_type == CONTENT_ADDON_PROPERTY_TYPE_STRING)
      prop.strValue = strdup(m_strValue.c_str());
    else if (m_type == CONTENT_ADDON_PROPERTY_TYPE_INT)
      prop.iValue = m_iValue;
    return prop;
  }

private:
  std::string                 m_strKey;
  CONTENT_ADDON_PROPERTY_TYPE m_type;
  std::string                 m_strValue;
  int                         m_iValue;
};

/*!
 * Wrapper for CONTENT_ADDON_FILE
 */
class AddonFileItem
{
public:
  /*!
   * Create a new empty file item
   * @param type The type of this file
   */
  AddonFileItem(CONTENT_ADDON_TYPE type, const std::string& strPath, const std::string& strName) :
    m_type(type)
  {
    AddPropertyString("path", strPath);
    AddPropertyString("name", strName);
  }

  /*!
   * Create a fileitem, copying the data from the provided file item.
   * @note Strings will _not_ be strdup'ed
   * @param fileItem The item to copy
   */
  AddonFileItem(const CONTENT_ADDON_FILEITEM* fileItem) :
    m_type(fileItem->type)
  {
    for (unsigned int iPtr = 0; iPtr < fileItem->iSize; iPtr++)
      m_properties.insert(std::make_pair(std::string(fileItem->properties[iPtr].key), new AddonFileItemProperty(&fileItem->properties[iPtr])));
  }

  virtual ~AddonFileItem(void)
  {
    for (std::map<std::string,AddonFileItemProperty*>::iterator it = m_properties.begin(); it != m_properties.end(); it++)
      delete it->second;
  }

  /*!
   * Add a property to this item
   * @param strKey The key of this property
   * @param strValue The value of this property
   */
  void AddPropertyString(const std::string& strKey, const std::string& strValue)
  {
    std::map<std::string,AddonFileItemProperty*>::iterator it = m_properties.find(strKey);
    if (it != m_properties.end())
      delete it->second;
    m_properties.insert(std::make_pair(strKey, new AddonFileItemProperty(strKey, strValue)));
  }

  /*!
   * Add a property to this item
   * @param strKey The key of this property
   * @param iValue The value of this property
   */
  void AddPropertyInt(const std::string& strKey, int iValue)
  {
    std::map<std::string,AddonFileItemProperty*>::iterator it = m_properties.find(strKey);
    if (it != m_properties.end())
      delete it->second;
    m_properties.insert(std::make_pair(strKey, new AddonFileItemProperty(strKey, iValue)));
  }

  /*!
   * Assign all properties to the provided file item. Must be freed by calling AddonFileItemList::Free()
   * @param fileItem The file item to assign the values to
   */
  void FileItem(CONTENT_ADDON_FILEITEM* fileItem) const
  {
    fileItem->type       = m_type;
    fileItem->iSize      = m_properties.size();
    fileItem->properties = (CONTENT_ADDON_FILE_PROPERTY*) malloc(sizeof(CONTENT_ADDON_FILE_PROPERTY) * fileItem->iSize);
    int iPtr(0);
    for (std::map<std::string,AddonFileItemProperty*>::const_iterator it = m_properties.begin(); it != m_properties.end(); it++)
      fileItem->properties[iPtr++] = it->second->AsProperty();
  }

  /*!
   * Look up and remove a property of type string
   * @param strKey The key of the property to look up
   * @param strDefault The default value to use, if the key could not be found
   * @return The value
   */
  std::string GetAndRemovePropertyString(const std::string& strKey, const std::string& strDefault = "")
  {
    std::string strReturn(strDefault);
    std::map<std::string,AddonFileItemProperty*>::iterator it = m_properties.find(strKey);
    if (it != m_properties.end() && it->second->Type() == CONTENT_ADDON_PROPERTY_TYPE_STRING)
    {
      strReturn = it->second->ValueAsString();
      delete it->second;
      m_properties.erase(it);
    }
    return strReturn;
  }

  /*!
   * Look up a property of type string
   * @param strKey The key of the property to look up
   * @param strDefault The default value to use, if the key could not be found
   * @return The value
   */
  std::string GetPropertyString(const std::string& strKey, const std::string& strDefault = "") const
  {
    std::string strReturn(strDefault);
    std::map<std::string,AddonFileItemProperty*>::const_iterator it = m_properties.find(strKey);
    if (it != m_properties.end() && it->second->Type() == CONTENT_ADDON_PROPERTY_TYPE_STRING)
      strReturn = it->second->ValueAsString();
    return strReturn;
  }

  /*!
   * Look up and remove a property of type integer
   * @param strKey The key of the property to look up
   * @param iDefault The default value to use, if the key could not be found
   * @return The value
   */
  int GetAndRemovePropertyInt(const std::string& strKey, int iDefault = 0)
  {
    int iReturn(iDefault);
    std::map<std::string,AddonFileItemProperty*>::iterator it = m_properties.find(strKey);
    if (it != m_properties.end() && it->second->Type() == CONTENT_ADDON_PROPERTY_TYPE_INT)
    {
      iReturn = it->second->ValueAsInt();
      delete it->second;
      m_properties.erase(it);
    }
    return iReturn;
  }

  /*!
   * Look up a property of type integer
   * @param strKey The key of the property to look up
   * @param iDefault The default value to use, if the key could not be found
   * @return The value
   */
  int GetPropertyInt(const std::string& strKey, int iDefault = 0) const
  {
    int iReturn(iDefault);
    std::map<std::string,AddonFileItemProperty*>::const_iterator it = m_properties.find(strKey);
    if (it != m_properties.end() && it->second->Type() == CONTENT_ADDON_PROPERTY_TYPE_INT)
      iReturn = it->second->ValueAsInt();
    return iReturn;
  }

  std::string Path(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("path") : GetPropertyString("path"); }
  std::string Path() const { return GetPropertyString("path"); }

  std::string Name(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("name") : GetPropertyString("name"); }
  std::string Name() const { return GetPropertyString("name"); }

  std::string Thumb(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("thumb") : GetPropertyString("thumb"); }
  std::string Thumb() const { return GetPropertyString("thumb"); }
  void SetThumb(const std::string& strThumb) { AddPropertyString("thumb", strThumb); }

  std::string Fanart(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("fanart_image") : GetPropertyString("fanart_image"); }
  std::string Fanart() const { return GetPropertyString("fanart_image"); }
  void SetFanart(const std::string& strFanart) { AddPropertyString("fanart_image", strFanart); }

  std::string ProviderIcon(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("provider_icon") : GetPropertyString("provider_icon"); }
  std::string ProviderIcon() const { return GetPropertyString("provider_icon"); }
  void SetProviderIcon(const std::string& strIcon) { AddPropertyString("provider_icon", strIcon); }

  std::map<std::string,AddonFileItemProperty*> m_properties;

  CONTENT_ADDON_TYPE Type(void) const { return m_type; }

private:
  CONTENT_ADDON_TYPE m_type;
};

/*!
 * Wrapper for CONTENT_ADDON_FILELIST
 */
class AddonFileItemList
{
public:
  /*!
   * Create a new empty file item list
   */
  AddonFileItemList(void) :
    m_items(NULL)
  {
  }

  /*!
   * Create a new file item list, copying the data from the provided file item list.
   * @param items
   */
  AddonFileItemList(CONTENT_ADDON_FILELIST* items) :
    m_items(items)
  {
    for (unsigned int iPtr = 0; iPtr < items->iSize; iPtr++)
      m_fileItems.push_back(new AddonFileItem(&items->items[iPtr]));
  }

  virtual ~AddonFileItemList(void)
  {
    for (std::vector<AddonFileItem*>::iterator it = m_fileItems.begin(); it != m_fileItems.end(); it++)
      delete (*it);
  }

  /*!
   * Free all data in the provided file item list
   * @param items The list to free
   */
  static void Free(CONTENT_ADDON_FILELIST* items)
  {
    if (!items)
      return;

    for (unsigned int iPtr = 0; iPtr < items->iSize; iPtr++)
    {
      for (unsigned int iPtr2 = 0; iPtr2 < items->items[iPtr].iSize; iPtr2++)
      {
        free(items->items[iPtr].properties[iPtr2].key);
        if (items->items[iPtr].properties[iPtr2].type == CONTENT_ADDON_PROPERTY_TYPE_STRING)
          free(items->items[iPtr].properties[iPtr2].strValue);
      }
      free(items->items[iPtr].properties);
    }

    free(items->items);
    free(items);
  }

  /*!
   * This file item list as CONTENT_ADDON_FILELIST*
   * Must be freed by calling Free()
   * @return
   */
  CONTENT_ADDON_FILELIST* AsFileList(void)
  {
    if (!m_items)
    {
      m_items = (CONTENT_ADDON_FILELIST*)malloc(sizeof(CONTENT_ADDON_FILELIST));
      memset(m_items, 0, sizeof(CONTENT_ADDON_FILELIST));

      m_items->items = (CONTENT_ADDON_FILEITEM*)malloc(sizeof(CONTENT_ADDON_FILEITEM) * m_fileItems.size());
      memset(m_items->items, 0, sizeof(CONTENT_ADDON_FILEITEM) * m_fileItems.size());

      for (std::vector<AddonFileItem*>::const_iterator it = m_fileItems.begin(); it != m_fileItems.end(); it++)
        (*it)->FileItem(&m_items->items[m_items->iSize++]);
    }
    return m_items;
  }

  /*!
   * Add a file item to this list
   * @param item The item to add
   */
  void AddFileItem(AddonFileItem* item)
  {
    if (item)
      m_fileItems.push_back(item);
  }

  std::vector<AddonFileItem*> m_fileItems;

private:
  CONTENT_ADDON_FILELIST* m_items;
};

class AddonFileSong : public AddonFileItem
{
public:
  AddonFileSong(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_SONG, strPath, strName) {}
  virtual ~AddonFileSong(void) {}

  int Track(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("track") : GetPropertyInt("track"); }
  void SetTrack(int iTrack) { AddPropertyInt("track", iTrack); }

  int Duration(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("duration") : GetPropertyInt("duration"); }
  void SetDuration(int iDuration) { AddPropertyInt("duration", iDuration); }

  int Rating(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("rating") : GetPropertyInt("rating"); }
  void SetRating(int iRating) { AddPropertyInt("rating", iRating); }

  std::string Artists(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("artists") : GetPropertyString("artists"); }
  void SetArtists(const std::string& strArtists) { AddPropertyString("artists", strArtists); }

  int Year(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("year") : GetPropertyInt("year"); }
  void SetYear(int iYear) { AddPropertyInt("year", iYear); }

  std::string Album(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("album") : GetPropertyString("album"); }
  void SetAlbum(const std::string& strAlbum) { AddPropertyString("album", strAlbum); }

  std::string AlbumArtists(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("album_artists") : GetPropertyString("album_artists"); }
  void SetAlbumArtists(const std::string& strAlbumArtists) { AddPropertyString("album_artists", strAlbumArtists); }
};

class AddonFileArtist : public AddonFileItem
{
public:
  AddonFileArtist(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_ARTIST, strPath, strName) {}
  virtual ~AddonFileArtist(void) {}

  std::string Genres(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("genres") : GetPropertyString("genres"); }
  void SetGenres(const std::string& strGenres) { AddPropertyString("genres", strGenres); }

  std::string Biography(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("biography") : GetPropertyString("biography"); }
  void SetBiography(const std::string& strBiography) { AddPropertyString("biography", strBiography); }

  std::string Styles(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("styles") : GetPropertyString("styles"); }
  void SetStyles(const std::string& strStyles) { AddPropertyString("styles", strStyles); }

  std::string Moods(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("moods") : GetPropertyString("moods"); }
  void SetMoods(const std::string& strMoods) { AddPropertyString("moods", strMoods); }

  std::string Born(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("born") : GetPropertyString("born"); }
  void SetBorn(const std::string& strBorn) { AddPropertyString("born", strBorn); }

  std::string Formed(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("formed") : GetPropertyString("formed"); }
  void SetFormed(const std::string& strFormed) { AddPropertyString("formed", strFormed); }

  std::string Died(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("died") : GetPropertyString("died"); }
  void SetDied(const std::string& strDied) { AddPropertyString("died", strDied); }

  std::string Disbanded(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("disbanded") : GetPropertyString("disbanded"); }
  void SetDisbanded(const std::string& strDisbanded) { AddPropertyString("disbanded", strDisbanded); }

  std::string YearsActive(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("years_active") : GetPropertyString("years_active"); }
  void SetYearsActive(const std::string& strYearsActive) { AddPropertyString("years_active", strYearsActive); }

  std::string Instruments(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("instruments") : GetPropertyString("instruments"); }
  void SetInstruments(const std::string& strInstruments) { AddPropertyString("instruments", strInstruments); }
};

class AddonFileAlbum : public AddonFileItem
{
public:
  AddonFileAlbum(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_ALBUM, strPath, strName) {}
  virtual ~AddonFileAlbum(void) {}

  int Year(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("year") : GetPropertyInt("year"); }
  void SetYear(int iYear) { AddPropertyInt("year", iYear); }

  std::string Artists(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("artists") : GetPropertyString("artists"); }
  void SetArtists(const std::string& strArtists) { AddPropertyString("artists", strArtists); }

  std::string Genres(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("genres") : GetPropertyString("genres"); }
  void SetGenres(const std::string& strGenres) { AddPropertyString("genres", strGenres); }

  int Rating(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("rating") : GetPropertyInt("rating"); }
  void SetRating(int iRating) { AddPropertyInt("rating", iRating); }

  std::string Review(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("review") : GetPropertyString("review"); }
  void SetReview(const std::string& strReview) { AddPropertyString("review", strReview); }

  std::string Styles(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("styles") : GetPropertyString("styles"); }
  void SetStyles(const std::string& strStyles) { AddPropertyString("styles", strStyles); }

  std::string Moods(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("moods") : GetPropertyString("moods"); }
  void SetMoods(const std::string& strMoods) { AddPropertyString("moods", strMoods); }

  std::string Themes(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("themes") : GetPropertyString("themes"); }
  void SetThemes(const std::string& strThemes) { AddPropertyString("themes", strThemes); }

  std::string Label(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("label") : GetPropertyString("label"); }
  void SetLabel(const std::string& strLabel) { AddPropertyString("label", strLabel); }

  std::string Type(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("type") : GetPropertyString("type"); }
  void SetType(const std::string& strType) { AddPropertyString("type", strType); }

  bool Compilation(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("compilation") == 1 : GetPropertyInt("compilation") == 1; }
  void SetCompilation(bool bCompilation) { AddPropertyInt("compilation", bCompilation?1:0); }

  int TimesPlayed(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("times_played") : GetPropertyInt("times_played"); }
  void SetTimesPlayed(int iTimesPlayed) { AddPropertyInt("times_played", iTimesPlayed); }
};

class AddonFilePlaylist : public AddonFileItem
{
public:
  AddonFilePlaylist(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_PLAYLIST, strPath, strName) {}
  virtual ~AddonFilePlaylist(void) {}
};

class AddonFileDirectory : public AddonFileItem
{
public:
  AddonFileDirectory(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_DIRECTORY, strPath, strName) {}
  virtual ~AddonFileDirectory(void) {}
};

class AddonFileFile : public AddonFileItem
{
public:
  AddonFileFile(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_FILE, strPath, strName) {}
  virtual ~AddonFileFile(void) {}
};
