/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "appdata.h"

// Global data and storage object
AppData g;

// ** CompStoreObj class********************

void CompStoreObj::clear (const QString &key, const QString &group)
{
  if (!group.isEmpty())
    m_settings.beginGroup(group);
  m_settings.remove(key);
  if (!group.isEmpty())
    m_settings.endGroup();
}


// ** FwRevision class********************

int FwRevision::get(const QString & fwType)
{
  QString result;
  load( result, fwType, QString(""));
  return result.toInt();
}

void FwRevision::set(const QString & fwType, const int fwRevision)
{
  QString tempString = QString("%1").arg(fwRevision);
  store( tempString, tempString, fwType);
}

void FwRevision::remove(const QString & tag)
{
  clear(tag, settingsPath());
}


// ** JStickData class********************

void JStickData::init(int newIndex)
{
  index = newIndex;

  stick_axe_init();
  stick_min_init();
  stick_med_init();
  stick_max_init();
  stick_inv_init();
}

void JStickData::reset()
{
  // Reset all JStickData variables to initial values
  stick_axeReset();
  stick_minReset();
  stick_medReset();
  stick_maxReset();
  stick_invReset();
}

bool JStickData::existsOnDisk()
{
  return (m_settings.value(settingsPath() + QString("stick%1_axe").arg(index), -1).toInt() > -1);
}


// ** Profile class********************

// The default copy operator can not be used since the index variable would be destroyed
Profile & Profile::operator= (const Profile & rhs)
{
  name         ( rhs.name()          );
  fwName       ( rhs.fwName()        );
  fwType       ( rhs.fwType()        );
  sdPath       ( rhs.sdPath()        );
  pBackupDir   ( rhs.pBackupDir()    );
  splashFile   ( rhs.splashFile()    );

  channelOrder ( rhs.channelOrder()  );
  defaultMode  ( rhs.defaultMode()   );
  volumeGain   ( rhs.volumeGain()    );
  burnFirmware ( rhs.burnFirmware()  );
  penableBackup( rhs.penableBackup() );
  renameFwFiles( rhs.renameFwFiles() );

  beeper       ( rhs.beeper()        );
  countryCode  ( rhs.countryCode()   );
  display      ( rhs.display()       );
  haptic       ( rhs.haptic()        );
  speaker      ( rhs.speaker()       );
  stickPotCalib( rhs.stickPotCalib() );
  trainerCalib ( rhs.trainerCalib()  );
  controlTypes ( rhs.controlTypes()  );
  controlNames ( rhs.controlNames()  );
  gsStickMode  ( rhs.gsStickMode()   );
  ppmMultiplier( rhs.ppmMultiplier() );
  vBatWarn     ( rhs.vBatWarn()      );
  vBatMin      ( rhs.vBatMin()       );
  vBatMax      ( rhs.vBatMax()       );
  txCurrentCalibration ( rhs.txCurrentCalibration() );
  txVoltageCalibration ( rhs.txVoltageCalibration() );

  simulatorOptions( rhs.simulatorOptions() );

  return *this;
}

void Profile::remove()
{
  // Remove all profile values from settings file
  m_settings.remove(settingsPath());
  // Reset all profile variables to initial values
  init(index);
}

bool Profile::existsOnDisk()
{
  return m_settings.contains(settingsPath() + "Name");
}

void Profile::resetFwVariables()
{
  beeperReset();
  countryCodeReset();
  displayReset();
  hapticReset();
  speakerReset();
  stickPotCalibReset();
  timeStampReset();
  trainerCalibReset();
  controlTypesReset();
  controlNamesReset();
  txCurrentCalibrationReset();
  gsStickModeReset();
  ppmMultiplierReset();
  txVoltageCalibrationReset();
  vBatWarnReset();
  vBatMinReset();
  vBatMaxReset();
}

void Profile::init(int newIndex)
{
  index = newIndex;

  // Initialize all variables. Use default values if no saved settings.

  name_init();
  fwName_init();
  fwType_init();
  sdPath_init();
  pBackupDir_init();
  splashFile_init();

  channelOrder_init();
  defaultMode_init();
  volumeGain_init();
  burnFirmware_init();
  penableBackup_init();
  renameFwFiles_init();

  simulatorOptions_init();

  beeper_init();
  countryCode_init();
  display_init();
  haptic_init();
  speaker_init();
  stickPotCalib_init();
  timeStamp_init();
  trainerCalib_init();
  controlTypes_init();
  controlNames_init();
  gsStickMode_init();
  ppmMultiplier_init();
  vBatWarn_init();
  vBatMin_init();
  vBatMax_init();
  txCurrentCalibration_init();
  txVoltageCalibration_init();
}

// ** AppData class********************

QString AppData::settingsVersionKey = QStringLiteral("settings_version");

void AppData::init()
{
  qRegisterMetaTypeStreamOperators<SimulatorOptions>("SimulatorOptions");

  firstUse = !hasCurrentSettings();

  qDebug() << "Settings init with" << m_settings.organizationName() << m_settings.applicationName() << "First use:" << firstUse;

  convertSettings(m_settings);

  // Initialize the profiles
  for (int i=0; i<MAX_PROFILES; i++)
    profile[i].init( i );

  // Initialize the joysticks
  for (int i=0; i<MAX_JOYSTICKS; i++)
    joystick[i].init( i );

  QString _tempString;                               // Do not touch. Do not change the settings version before a new verson update!
  getset( _tempString, settingsVersionKey, "220" );  // This is a version marker. Will be used to upgrade the settings later on.

  // Initialize all variables. Use default values if no saved settings.

  load(m_profileId, "profileId", 0);
  sessionId(id());

  recentFiles_init();
  simuDbgFilters_init();
  mainWinGeo_init();
  mainWinState_init();
  modelEditGeo_init();
  mdiWinGeo_init();
  mdiWinState_init();
  compareWinGeo_init();

  armMcu_init();
  avrArguments_init();
  avrPort_init();
  avrdudeLocation_init();
  dfuArguments_init();
  dfuLocation_init();
  locale_init();
  mcu_init();
  programmer_init();
  sambaLocation_init();
  sambaPort_init();

  backupDir_init();
  gePath_init();
  eepromDir_init();
  flashDir_init();
  imagesDir_init();
  logDir_init();
  libDir_init();
  snapshotDir_init();
  updatesDir_init();
  appLogsDir_init();

  OpenTxBranch_init();
  newModelAction_init();
  backLight_init();
  embedSplashes_init();
  fwServerFails_init();
  generalEditTab_init();
  iconSize_init();
  jsCtrl_init();
  historySize_init();
  theme_init();
  warningId_init();
  simuLastProfId_init();

  enableBackup_init();
  backupOnFlash_init();
  outputDisplayDetails_init();
  checkHardwareCompatibility_init();
  removeModelSlots_init();
  maximized_init();
  simuSW_init();
  tabbedMdi_init();
  appDebugLog_init();
  fwTraceLog_init();
  jsSupport_init();
  showSplash_init();
  snapToClpbrd_init();
  autoCheckApp_init();
  autoCheckFw_init();

}

QMap<int, QString> AppData::getActiveProfiles() const
{
  QMap<int, QString> active;
  for (int i=0; i<MAX_PROFILES; i++) {
    if (g.profile[i].existsOnDisk())
      active.insert(i, g.profile[i].name());
  }
  return active;
}

void AppData::convertSettings(QSettings & settings)
{
  // convert old settings to new
  // NOTE: this function should be re-visited after version updates which change settings destination product name.

  if (settings.contains("useWizard")) {
    if (!settings.contains("newModelAction")) {
      newModelAction(settings.value("useWizard").toBool() ? 1 : 2);
    }
    settings.remove("useWizard");
  }
  if (settings.contains("warningId") && settings.value("warningId").toInt() == 7) {
    // meaning of warningId changed during v2.2 development but value of "7" indicates old setting, removing it will restore defaults
    warningIdReset();
  }

  // other deprecated settings
  for (const QString & key : deprecatedSettings())
    settings.remove(key);
}

bool AppData::findPreviousVersionSettings(QString * version) const
{
  QSettings * fromSettings = NULL;
  *version = "";

  QSettings settings21("OpenTX", "Companion 2.1");
  if (settings21.contains(settingsVersionKey)) {
    fromSettings = &settings21;
    *version = "2.1";
  }
  else {
    settings21.clear();
  }

  QSettings settings20("OpenTX", "Companion 2.0");
  if (settings20.contains(settingsVersionKey)) {
    if (!fromSettings) {
      fromSettings = &settings20;
      *version = "2.0";
    }
  }
  else {
    settings20.clear();
  }

  QSettings settings16("OpenTX", "OpenTX Companion");
  if (settings16.contains(settingsVersionKey)) {
    if (!fromSettings) {
      fromSettings = &settings16;
      *version = "1.x";
    }
  }
  else {
    settings16.clear();
  }

  if (!fromSettings)
    return false;

  return true;
}

bool AppData::importSettings(QString fromVersion)
{
  QString fromCompany;
  QString fromProduct;

  upgradeFromVersion = "";

  if (fromVersion == "2.1") {
    fromCompany = "OpenTX";
    fromProduct = "Companion 2.1";
  }
  else if (fromVersion == "2.0") {
    fromCompany = "OpenTX";
    fromProduct = "Companion 2.0";
  }
  else if (fromVersion == "1.x") {
    fromCompany = "OpenTX";
    fromProduct = "OpenTX Companion";
  }
  else
    return false;

  upgradeFromVersion = fromVersion;

  QSettings fromSettings(fromCompany, fromProduct);

  // do not copy these settings
  QStringList excludeKeys = deprecatedSettings();
  excludeKeys << "compilation-server";
#ifdef WIN32
  // locations of tools which come with Companion distros
  excludeKeys << "avrdude_location" << "avrdudeLocation" << "dfu_location";
  // install-specific keys;  "." is the "default" key which may contain install path
  excludeKeys << "Start Menu Folder" << ".";
#endif

  // import settings
  foreach (const QString & key, fromSettings.allKeys()) {
    if (fromSettings.value(key).isValid() && !excludeKeys.contains(key)) {
      m_settings.setValue(key, fromSettings.value(key));
    }
  }

  return true;
}
