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

#include "ProgressBarTester.h"
#include "GUIDialogExtendedProgressBar.h"
#include "Application.h"
#include "guilib/GUIWindowManager.h"
#include "threads/CriticalSection.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

#include <boost/shared_ptr.hpp>
#include <stdlib.h>
#include <time.h>
#include <vector>

using namespace std;

int GetRand(int n)
{
  static bool seeded = false;
  if (!seeded)
  {
    srand(time(NULL));
    seeded = true;
  }
  static CCriticalSection mutex;
  CSingleLock lock(mutex);
  return rand() % n;
}

CProgressBarTester::CProgressBarTester() : CThread("ProgressBarTester")
{
}

CProgressBarTester &CProgressBarTester::Get()
{
  static CProgressBarTester _instance;
  return _instance;
}

void CProgressBarTester::Start()
{
  Create(true, THREAD_MINSTACKSIZE);
}

void CProgressBarTester::Process()
{
  CLog::Log(LOGDEBUG, "ProgressBarTester: warming up");
  // Give XBMC a chance to warm up
  Sleep(2000); // ms
  
  vector<boost::shared_ptr<CProgressBarTesterThread> > workers;
  for (unsigned int i = 0; i < 10; i++)
  {
    CStdString name;
    name.Format("%u", i);
    boost::shared_ptr<CProgressBarTesterThread> worker(new CProgressBarTesterThread(name, i * 100));
    workers.push_back(worker);
    worker->Create(false, THREAD_MINSTACKSIZE);
  }

  while (!g_application.m_bStop)
  {
    Sleep(100); // ms
  }

  for (vector<boost::shared_ptr<CProgressBarTesterThread> >::iterator it = workers.begin(); it != workers.end(); ++it)
    (*it)->StopThread();
  for (vector<boost::shared_ptr<CProgressBarTesterThread> >::iterator it = workers.begin(); it != workers.end(); ++it)
    (*it)->StopThread(true);
}

CProgressBarTesterThread::CProgressBarTesterThread(const string &name, unsigned int period)
    : CThread(("ProgressBarTester: " + name).c_str()), m_threadName(name), m_period(period)
{
}

void CProgressBarTesterThread::Process()
{
  CGUIDialogExtendedProgressBar* pDlgProgress = dynamic_cast<CGUIDialogExtendedProgressBar*>(g_windowManager.GetWindow(WINDOW_DIALOG_EXT_PROGRESS));
  CGUIDialogProgressBarHandle* handle = pDlgProgress->GetHandle(m_threadName);
  
  unsigned int percent = 0;
  const unsigned int MAX_PERCENT = 100;
  const unsigned int STEPS = 100;
  const unsigned int STEP_SIZE = m_period / STEPS;
  vector<unsigned int> stepsizes;
  for (unsigned i = 0; i < MAX_PERCENT + 1; i++)
    stepsizes.push_back(GetRand(2 * STEP_SIZE));

  while (!m_bStop)
  {
    CStdString strText;
    strText.Format("%d%%", percent);
    handle->SetText(strText);
    handle->SetPercentage((float)percent);

    //Sleep(stepsizes[percent]);
    //Sleep(10);
    percent++;
    if (percent > MAX_PERCENT) // Actually yields a period of 1.01 * m_period, but who's counting
      percent = 0;
  }

  handle->MarkFinished();
}
