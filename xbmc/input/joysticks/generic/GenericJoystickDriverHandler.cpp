/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
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

#include "GenericJoystickDriverHandler.h"
#include "input/joysticks/IJoystickButtonMap.h"
#include "input/joysticks/IJoystickInputHandler.h"
#include "input/joysticks/JoystickDriverPrimitive.h"
#include "input/joysticks/JoystickTranslator.h"
#include "input/joysticks/JoystickTypes.h"
#include "utils/log.h"

#include <algorithm>

#define ANALOG_DIGITAL_THRESHOLD  0.5f

CGenericJoystickDriverHandler::CGenericJoystickDriverHandler(IJoystickInputHandler* handler, IJoystickButtonMap* buttonMap)
 : m_handler(handler),
   m_buttonMap(buttonMap)
{
}

void CGenericJoystickDriverHandler::OnButtonMotion(unsigned int index, bool bPressed)
{
  const char pressed = bPressed ? 1 : 0;

  if (m_buttonStates.size() <= index)
    m_buttonStates.resize(index + 1);

  if (m_buttonStates[index] == pressed)
    return;

  char& oldState = m_buttonStates[index];

  CJoystickDriverPrimitive button(index);
  JoystickFeatureID feature = m_buttonMap->GetFeature(button);

  if (feature != JoystickIDButtonUnknown)
  {
    CLog::Log(LOGDEBUG, "CGenericJoystickDriverHandler: Feature %d %s",
              feature, bPressed ? "pressed" : "released");

    if (!oldState && pressed)
      m_handler->OnButtonPress(feature, true);
    else if (oldState && !pressed)
      m_handler->OnButtonPress(feature, false);
  }
  else if (bPressed)
  {
    CLog::Log(LOGDEBUG, "CGenericJoystickDriverHandler: No feature for button %u",
              index);
  }

  oldState = pressed;
}

void CGenericJoystickDriverHandler::OnHatMotion(unsigned int index, HatDirection newDirection)
{
  if (m_hatStates.size() <= index)
    m_hatStates.resize(index + 1);

  HatDirection& oldDirection = m_hatStates[index];

  ProcessHatDirection(index, oldDirection, newDirection, HatDirectionUp);
  ProcessHatDirection(index, oldDirection, newDirection, HatDirectionRight);
  ProcessHatDirection(index, oldDirection, newDirection, HatDirectionDown);
  ProcessHatDirection(index, oldDirection, newDirection, HatDirectionLeft);

  oldDirection = newDirection;
}

void CGenericJoystickDriverHandler::ProcessHatDirection(int index,
    HatDirection oldDir, HatDirection newDir, HatDirection targetDir)
{
  if ((oldDir & targetDir) == HatDirectionNone &&
      (newDir & targetDir) != HatDirectionNone)
  {
    CJoystickDriverPrimitive left(index, HatDirectionLeft);
    JoystickFeatureID feature = m_buttonMap->GetFeature(left);
    if (feature != JoystickIDButtonUnknown)
    {
      CLog::Log(LOGDEBUG, "CGenericJoystickDriverHandler: Feature %d activated",
                feature);
      m_handler->OnButtonPress(feature, true);
    }
    else
    {
      CLog::Log(LOGDEBUG, "CGenericJoystickDriverHandler: No feature for hat %u %s",
                index, CJoystickTranslator::HatDirectionToString(targetDir));
    }
  }
  else if ((oldDir & targetDir) != HatDirectionNone &&
           (newDir & targetDir) == HatDirectionNone)
  {
    CJoystickDriverPrimitive left(index, HatDirectionLeft);
    JoystickFeatureID feature = m_buttonMap->GetFeature(left);
    if (feature != JoystickIDButtonUnknown)
    {
      CLog::Log(LOGDEBUG, "CGenericJoystickDriverHandler: Feature %d deactivated",
                feature);
      m_handler->OnButtonPress(feature, false);
    }
  }
}

void CGenericJoystickDriverHandler::OnAxisMotion(unsigned int index, float newPosition)
{
  if (m_axisStates.size() <= index)
    m_axisStates.resize(index + 1);

  if (m_axisStates[index] == 0.0f && newPosition == 0.0f)
    return;

  float oldPosition = m_axisStates[index];
  m_axisStates[index] = newPosition;

  CJoystickDriverPrimitive positiveAxis(index, SemiAxisDirectionPositive);
  CJoystickDriverPrimitive negativeAxis(index, SemiAxisDirectionNegative);

  JoystickFeatureID positiveFeature = m_buttonMap->GetFeature(positiveAxis);
  JoystickFeatureID negativeFeature = m_buttonMap->GetFeature(negativeAxis);

  if (!positiveFeature && !negativeFeature)
  {
    // No features to send to callback
  }
  else if (positiveFeature == negativeFeature)
  {
    // Feature uses multiple axes, add to OnAxisMotions() batch process
    if (std::find(m_featuresWithMotion.begin(), m_featuresWithMotion.end(), positiveFeature) == m_featuresWithMotion.end())
      m_featuresWithMotion.push_back(positiveFeature);
  }
  else // positiveFeature != negativeFeature
  {
    // Positive and negative directions are mapped to different features, so we
    // must be dealing with a button or trigger

    if (positiveFeature)
    {
      // If new position passes through zero, 0.0f is sent exactly once until
      // the position becomes positive again
      if (newPosition > 0)
        m_handler->OnButtonMotion(positiveFeature, newPosition);
      else if (oldPosition > 0)
        m_handler->OnButtonMotion(positiveFeature, 0.0f);
    }

    if (negativeFeature)
    {
      // If new position passes through zero, 0.0f is sent exactly once until
      // the position becomes negative again
      if (newPosition < 0)
        m_handler->OnButtonMotion(negativeFeature, -1.0f * newPosition); // magnitude is >= 0
      else if (oldPosition < 0)
        m_handler->OnButtonMotion(negativeFeature, 0.0f);
    }
  }
}

void CGenericJoystickDriverHandler::ProcessAxisMotions()
{
  std::vector<JoystickFeatureID> featuresToProcess;
  featuresToProcess.swap(m_featuresWithMotion);

  for (std::vector<JoystickFeatureID>::const_iterator it = featuresToProcess.begin(); it != featuresToProcess.end(); ++it)
  {
    const JoystickFeatureID feature = *it;
    if (CJoystickTranslator::GetInputType(feature) == JoystickAnalogStick)
    {
      int  horizIndex;
      bool horizInverted;
      int  vertIndex;
      bool vertInverted;

      if (m_buttonMap->GetAnalogStick(feature,
                                      horizIndex, horizInverted,
                                      vertIndex,  vertInverted))
      {
        const float horizPos = GetAxisState(horizIndex) * (horizInverted ? -1.0f : 1.0f);
        const float vertPos  = GetAxisState(vertIndex)  * (vertInverted  ? -1.0f : 1.0f);
        m_handler->OnAnalogStickMotion(feature, horizPos, vertPos);
      }
    }
    else if (CJoystickTranslator::GetInputType(feature) == JoystickAccelerometer)
    {
      int  xIndex;
      bool xInverted;
      int  yIndex;
      bool yInverted;
      int  zIndex;
      bool zInverted;

      if (m_buttonMap->GetAccelerometer(feature,
                                        xIndex, xInverted,
                                        yIndex, yInverted,
                                        zIndex, zInverted))
      {
        const float xPos = GetAxisState(xIndex) * (xInverted ? -1.0f : 1.0f);
        const float yPos = GetAxisState(yIndex) * (yInverted ? -1.0f : 1.0f);
        const float zPos = GetAxisState(zIndex) * (zInverted ? -1.0f : 1.0f);
        m_handler->OnAccelerometerMotion(feature, xPos, yPos, zPos);
      }
    }
  }
}

float CGenericJoystickDriverHandler::GetAxisState(int axisIndex) const
{
  return (0 <= axisIndex && axisIndex < (int)m_axisStates.size()) ? m_axisStates[axisIndex] : 0;
}
