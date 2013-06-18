#ifndef CONTENTADDONCODEC_H_
#define CONTENTADDONCODEC_H_
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

#include "ICodec.h"
#include "addons/ContentAddons.h"

class ContentAddonCodec : public ICodec
{
public:
  ContentAddonCodec(void) {}
  virtual ~ContentAddonCodec(void) {}

  bool Init(const CStdString& strFile, unsigned int filecache);
  void DeInit(void);
  int64_t Seek(int64_t iSeekTime);
  int ReadPCM(BYTE* pBuffer, int size, int* actualsize);
  bool CanInit(void);
  CAEChannelInfo GetChannelInfo(void);

private:
  ADDON::CONTENT_ADDON m_addon;
};

#endif /* CONTENTADDONCODEC_H_ */
