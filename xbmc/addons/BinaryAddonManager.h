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

#include "AddonManager.h"
#include "AddonDatabase.h"
#include "utils/Observer.h"
#include "threads/Thread.h"
#include "utils/log.h"
#include "addons/include/xbmc_addon_types.h"
#include <vector>

namespace ADDON
{
  template <typename _AddonType>
  class CBinaryAddonManager :
    public IAddonMgrCallback,
    public Observer,
    protected CThread
  {
  public:
    virtual ~CBinaryAddonManager(void)
    {
      Stop();
    }

    virtual bool GetConnectedClient(const CStdString strClientId, boost::shared_ptr<_AddonType>& addon) const
    {
      if (GetClient(strClientId, addon))
        return addon->ReadyToUse();
      return false;
    }

    virtual bool GetClient(const CStdString& strAddonProfile, boost::shared_ptr<_AddonType>& addon) const
    {
      CSingleLock lock(m_critSection);
      typename std::map<CStdString, boost::shared_ptr<_AddonType> >::const_iterator itr = m_addonMap.find(strAddonProfile);
      if (itr != m_addonMap.end())
      {
        addon = itr->second;
        return true;
      }

      return false;
    }

    virtual CStdString GetClientId(const AddonPtr client) const
    {
      CStdString strReturn;
      CSingleLock lock(m_critSection);

      for (typename std::map<CStdString, boost::shared_ptr<_AddonType> >::const_iterator itr = m_addonMap.begin(); itr != m_addonMap.end() && strReturn.empty(); itr++)
        if (itr->second->Profile() == client->Profile())
          strReturn = itr->first;

      return strReturn;
    }

    virtual bool RequestRestart(AddonPtr addon, bool datachanged)
    {
      return StopClient(addon, true);
    }

    virtual bool RequestRemoval(AddonPtr addon)
    {
      return StopClient(addon, false);
    }

    virtual bool StopClient(AddonPtr client, bool bRestart)
    {
      CSingleLock lock(m_critSection);
      boost::shared_ptr<_AddonType> mappedClient;
      if (GetClient(client->Profile(), mappedClient))
      {
        CLog::Log(LOGDEBUG, "%s - %s add-on '%s'", __FUNCTION__, bRestart ? "restarting" : "stopping", mappedClient->Name().c_str());
        if (bRestart)
          mappedClient->ReCreate();
        else
          mappedClient->Destroy();

        return bRestart ? mappedClient->ReadyToUse() : true;
      }

      return false;
    }

    virtual bool IsConnectedClient(const CStdString& strClientId) const
    {
      boost::shared_ptr<_AddonType> client;
      return GetConnectedClient(strClientId, client);
    }

    virtual bool IsConnectedClient(const AddonPtr addon)
    {
      for (typename std::map<CStdString, boost::shared_ptr<_AddonType> >::const_iterator itr = m_addonMap.begin(); itr != m_addonMap.end(); itr++)
        if (itr->second->Profile() == addon->Profile())
          return itr->second->ReadyToUse();
      return false;
    }

    virtual void Start(void)
    {
      Stop();

      m_addonDb.Open();
      Create();
      SetPriority(-1);
    }

    virtual void Stop(void)
    {
      StopThread();
      for (typename std::map<CStdString, boost::shared_ptr<_AddonType> >::const_iterator itr = m_addonMap.begin(); itr != m_addonMap.end(); itr++)
        itr->second->Destroy();
      m_addonDb.Close();
    }

    virtual void Notify(const Observable &obs, const ObservableMessage msg)
    {
      if (msg == ObservableMessageAddons)
        UpdateAddons();
    }

  protected:
    CBinaryAddonManager(TYPE type) :
      CThread("binary add-on updater"),
      m_addonType(type) {}

    virtual void Process(void)
    {
      CAddonMgr::Get().RegisterAddonMgrCallback(m_addonType, this);
      CAddonMgr::Get().RegisterObserver(this);
      UpdateAddons();

      while (!m_bStop)
      {
        UpdateAndInitialiseClients();
        Sleep(1000);
      }

      CAddonMgr::Get().UnregisterAddonMgrCallback(m_addonType);
      CAddonMgr::Get().UnregisterObserver(this);
    }

    virtual bool UpdateAddons(void)
    {
      ADDON::VECADDONS addons;
      bool bReturn(CAddonMgr::Get().GetAddons(m_addonType, addons, true));
      for (ADDON::VECADDONS::const_iterator it = addons.begin(); it != addons.end(); it++)
      {
        if (!m_addonDb.HasAddon((*it)->ID()))
          m_addonDb.AddAddon(*it, -1);
      }

      if (bReturn)
      {
        CSingleLock lock(m_critSection);
        m_addons = addons;
      }

      return bReturn;
    }

    virtual bool UpdateAndInitialiseClients()
    {
      bool bReturn(true);
      ADDON::VECADDONS map;
      ADDON::VECADDONS disableAddons;
      {
        CSingleLock lock(m_critSection);
        map = m_addons;
      }

      if (map.size() == 0)
        return false;

      for (unsigned iClientPtr = 0; iClientPtr < map.size(); iClientPtr++)
      {
        const AddonPtr clientAddon = map.at(iClientPtr);
        bool bEnabled = clientAddon->Enabled() &&
            !m_addonDb.IsAddonDisabled(clientAddon->ID());

        if (!bEnabled)
        {
          StopClient(clientAddon, false);
          ADDON::VECADDONS::iterator addonPtr = find(m_addons.begin(), m_addons.end(), clientAddon);
          if (addonPtr != m_addons.end())
            m_addons.erase(addonPtr);
        }
        else if (bEnabled && !IsConnectedClient(clientAddon))
        {
          bool bDisabled(false);

          // register the add-on in the db, and create the _AddonType instance
          if (!RegisterClient(clientAddon))
          {
            // failed to register or create the add-on, disable it
            CLog::Log(LOGERROR, "%s - failed to register add-on '%s', disabling it", __FUNCTION__, clientAddon->Name().c_str());
            disableAddons.push_back(clientAddon);
            bDisabled = true;
          }
          else
          {
            ADDON_STATUS status(ADDON_STATUS_UNKNOWN);
            boost::shared_ptr<_AddonType> addon;
            {
              if (!GetClient(clientAddon->Profile(), addon))
              {
                CLog::Log(LOGERROR, "%s - failed to find add-on '%s', disabling it", __FUNCTION__, clientAddon->Name().c_str());
                disableAddons.push_back(clientAddon);
                bDisabled = true;
              }
            }

            // throttle connection attempts, no more than 1 attempt per 5 seconds
            if (!bDisabled && addon->Enabled())
            {
              time_t now;
              CDateTime::GetCurrentDateTime().GetAsTime(now);
              std::map<CStdString, time_t>::iterator it = m_connectionAttempts.find(clientAddon->Profile());
              if (it != m_connectionAttempts.end() && now < it->second)
                continue;
              m_connectionAttempts[clientAddon->Profile()] = now + 5;
            }

            // re-check the enabled status. newly installed clients get disabled when they're added to the db
            if (!bDisabled && addon->Enabled() && (status = addon->Create()) != ADDON_STATUS_OK)
            {
              CLog::Log(LOGERROR, "%s - failed to create add-on '%s', status = %d", __FUNCTION__, clientAddon->Name().c_str(), status);
              if (!addon.get() || !addon->DllLoaded() || status == ADDON_STATUS_PERMANENT_FAILURE)
              {
                // failed to load the dll of this add-on, disable it
                CLog::Log(LOGERROR, "%s - failed to load the dll for add-on '%s', disabling it", __FUNCTION__, clientAddon->Name().c_str());
                disableAddons.push_back(clientAddon);
                bDisabled = true;
              }
            }
          }
          if (bDisabled)
            DisplayWarningAddonFailed(clientAddon);
        }
      }

      // disable add-ons that failed to initialise
      if (disableAddons.size() > 0)
      {
        CSingleLock lock(m_critSection);
        for (ADDON::VECADDONS::iterator it = disableAddons.begin(); it != disableAddons.end(); it++)
        {
          // disable in the add-on db
          m_addonDb.DisableAddon((*it)->ID(), true);

          ADDON::VECADDONS::iterator addonPtr = find(m_addons.begin(), m_addons.end(), *it);
          if (addonPtr != m_addons.end())
            m_addons.erase(addonPtr);
        }
      }

      return bReturn;
    }

    virtual bool RegisterClient(AddonPtr client)
    {
      if (!client->Enabled())
        return false;

      boost::shared_ptr<_AddonType> addon;
      // load and initialise the client libraries
      {
        CSingleLock lock(m_critSection);
        typename std::map<CStdString, boost::shared_ptr<_AddonType> >::iterator existingClient = m_addonMap.find(client->Profile());
        if (existingClient != m_addonMap.end())
        {
          // return existing client
          addon = existingClient->second;
        }
        else
        {
          // create a new client instance
          addon = boost::dynamic_pointer_cast<_AddonType>(client);
          m_addonMap.insert(std::make_pair(client->Profile(), addon));
        }
      }

      return true;
    }

    virtual void DisplayWarningAddonFailed(AddonPtr client) {}

    CCriticalSection                                     m_critSection;
    CAddonDatabase                                       m_addonDb;
    TYPE                                                 m_addonType;
    VECADDONS                                            m_addons;
    std::map<CStdString, time_t>                         m_connectionAttempts;
    std::map<CStdString, boost::shared_ptr<_AddonType> > m_addonMap;
    // TODO use the profile path to map. when adding support for multiple instances of an add-on, look into a way
    // to use one LibPath() but multiple Profile() paths.
  };
}
