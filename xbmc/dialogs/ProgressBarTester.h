#pragma once
/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "threads/Thread.h"

#include <string>

class CProgressBarTester : public CThread
{
public:
  CProgressBarTester();
  static CProgressBarTester &Get();
  virtual ~CProgressBarTester() { }
  void Start();

protected:
  virtual void Process();
};

class CProgressBarTesterThread : public CThread
{
public:
  CProgressBarTesterThread(const std::string &name, unsigned int period);
  virtual ~CProgressBarTesterThread() { }

protected:
  virtual void Process();

private:
  std::string m_threadName;
  unsigned int m_period;
};
