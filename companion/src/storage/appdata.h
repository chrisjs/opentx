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

// All temporary and permanent global variables are defined here to make
// initialization and storage safe and visible.
// Do not access variables in QSettings directly, it is not type safe!


#ifndef _APPDATA_H_
#define _APPDATA_H_

#include <QByteArray>
#include <QStringList>
#include <QString>
#include <QSettings>
#include <QStandardPaths>

#include "constants.h"
#include "simulator.h"

#define COMPANY            "OpenTX"
#define COMPANY_DOMAIN     "open-tx.org"
#define PRODUCT            "Companion 2.2"
#define APP_COMPANION      "OpenTX Companion"
#define APP_SIMULATOR      "OpenTX Simulator"

#define MAX_PROFILES 15
#define MAX_JOYSTICKS 8

/**
  This macro declares and defines a CompStoreObj member variable and its access functions:
    public:
      inline T name() const;            // getter
      void name(const T &, bool=true);  // sets value and optionally saves to storage. If new value == default value, the storage is cleared.
      void nameReset(bool=true);        // reset to default and optionally save
      inline T nameDefault() const;     // returns the default value
    private:
      void name_init();                 // sets value from saved settings, if any, otherwise sets default value
      T _name;                          // member variable

  @param type  The data type (T).
  @param name  Name of the property.
  @param key   Settings key name, can be fixed string or dynamic function.
  @param dflt  Default value, must be compatible with @a type.
  @sa PROPERTY()

  @note @a type must be a standard or registered Qt type. See notes in CompStoreObject.
*/
#define PROPERTY4(type, name, key, dflt) \
    public:                                                                        \
      inline type name() const { return _##name; }                                 \
      void name(const type &val, bool store = true) {                              \
        _##name = val;                                                             \
        if (!store) return;                                                        \
        if (QVariant::fromValue(val) == QVariant::fromValue(name##Default()))      \
          clear((key), settingsPath());                                            \
        else                                                                       \
          this->store(val, (key));                                                 \
      }                                                                            \
      inline void name##Reset(bool store = true) { name(name##Default(), store); } \
      inline type name##Default() const { return (type)(dflt); }                   \
    private:                                                                       \
      void name##_init() {                                                         \
        load(_##name, (key), QVariant::fromValue(name##Default()));                \
      }                                                                            \
      type _##name;


/**
  Convenience equivalent to @p PROPERTY4() macro when the @a name and @a key are the same (recommended).

  @param type  The data type (T).
  @param name  Name of the property, and also the settings key name.
  @param dflt  Default value, must be compatible with @a type.
  @sa PROPERTY4()
*/
#define PROPERTY(type, name, dflt)     PROPERTY4(type, name, QStringLiteral(#name), dflt)

//! Convenience macros for QString types with either null or custom default value.
#define PROPERTYSTR(name)              PROPERTY(QString, name, QStringLiteral(""))
#define PROPERTYSTRD(name, dflt)       PROPERTY(QString, name, QString(dflt))
#define PROPERTYSTR2(name, key)        PROPERTY4(QString, name, QStringLiteral(key), QStringLiteral(""))
#define PROPERTYSTR3(name, key, dflt)  PROPERTY4(QString, name, QStringLiteral(key), QString(dflt))

//! Convenience macros for QByteArray types with null value.
#define PROPERTYQBA(name)              PROPERTY(QByteArray, name, QByteArray())
#define PROPERTYQBA2(name, key)        PROPERTY4(QByteArray, name, QStringLiteral(key), QByteArray())

class CompStoreObj
{
  protected:
    CompStoreObj() : m_settings(COMPANY, PRODUCT) { }

    //! reimplement this function to return the default settings group (used in \p store() and \p load() if their \a group is not specified).
    virtual QString settingsPath() const { return QString(); }

    //! Deletes any saved settings in \a key, which is (optionally) a subkey of \a group
    void clear(const QString & key, const QString & group = QString());

    //! Utility function to return a fully qualified settings path for given \a key in optional \a group. If \a group is null, \p settingsPath() is used.
    QString pathForKey(const QString &key, const QString &group = QString()) const
    {
      QString path = (group.isNull() ? settingsPath() : group);
      if (!path.isEmpty() && !path.endsWith('/'))
        path.append('/');
      return path.append(key);
    }

    // NOTE: T must be a standard or registered Qt type. To use custom types,
    //   either Q_DECLARE_METATYPE() them or add additional logic in load() to handle QMetaType::User.
    //   Custom types will also need QDataStream streaming operators defined so that they can be read/written by QSettings.
    //   See SimulatorOptions class for an example of using a custom type with stream operators,
    //     or e.g. https://stackoverflow.com/questions/18144377/writing-and-reading-custom-class-to-qsettings

    //! Save property value to persistent storage.
    template <typename T>
    void store(const T & newValue, const QString & key, const QString & group = QString())
    {
      m_settings.setValue(pathForKey(key, group), QVariant::fromValue(newValue));
    }

    //! Set property to value saved in persistent storage, if any, otherwise to \a def default value.
    template <typename T>
    void load(T & destValue, const QString & key, const QVariant & def, const QString & group = QString())
    {
      QVariant val = m_settings.value(pathForKey(key, group), def);
      if (val.canConvert<T>())
        destValue = val.value<T>();
    }

    //! Same as calling \p load() and then \p store()
    template <typename T>
    void getset(T & value, const QString & key, const QVariant & def, const QString & group = QString())
    {
      load(value, key, def, group);
      store(value, key, group);
    }

    QSettings m_settings;
};

class FwRevision: protected CompStoreObj
{
  public:
    FwRevision() : CompStoreObj() {}
    int get(const QString & fwType);
    void set(const QString & fwType, const int fwRevision);
    void remove(const QString & tag);
  protected:
    virtual QString settingsPath() const { return QStringLiteral("FwRevisions/"); }
};

class JStickData: protected CompStoreObj
{
  public:
    JStickData() : CompStoreObj(), index(-1) {}
    void reset();
    bool existsOnDisk();
    void init(int index);

  private:
    PROPERTY4(int, stick_axe, QString("stick%1_axe").arg(index), -1)
    PROPERTY4(int, stick_min, QString("stick%1_min").arg(index), -32767)
    PROPERTY4(int, stick_med, QString("stick%1_med").arg(index), 0)
    PROPERTY4(int, stick_max, QString("stick%1_max").arg(index), 32767)
    PROPERTY4(int, stick_inv, QString("stick%1_inv").arg(index), 0)
    int index;

  protected:
    virtual QString settingsPath() const { return QStringLiteral("JsCalibration/"); }
};

class Profile: protected CompStoreObj
{
  public:
    Profile() : CompStoreObj(), index(-1) {}
    Profile & operator=(const Profile & rhs);
    void remove();
    bool existsOnDisk();
    void init(int newIndex);
    void resetFwVariables();

  private:
    // !! When adding properties, remember to also add the matching name_init() call in AppData::init() !! //

    PROPERTYSTR2(name,       "Name")
    PROPERTYSTR2(splashFile, "SplashFileName")
    PROPERTYSTR(fwName)
    PROPERTYSTR(fwType)
    PROPERTYSTR(sdPath)
    PROPERTYSTR(pBackupDir)

    PROPERTY4(int, channelOrder, "default_channel_order",  0)
    PROPERTY4(int, defaultMode,  "default_mode",           1)
    PROPERTY (int, volumeGain,   10)

    PROPERTY4(bool, renameFwFiles, "rename_firmware_files", false)
    PROPERTY (bool, burnFirmware,  false)
    PROPERTY (bool, penableBackup, false)

    // Simulator variables
    PROPERTY(SimulatorOptions, simulatorOptions,  SimulatorOptions())

    // Firmware Variables
    PROPERTYSTR2(beeper,        "Beeper")
    PROPERTYSTR2(countryCode,   "countryCode")
    PROPERTYSTR2(display,       "Display")
    PROPERTYSTR2(haptic,        "Haptic")
    PROPERTYSTR2(speaker,       "Speaker")
    PROPERTYSTR2(stickPotCalib, "StickPotCalib")
    PROPERTYSTR2(timeStamp,     "TimeStamp")
    PROPERTYSTR2(trainerCalib,  "TrainerCalib")
    PROPERTYSTR2(controlTypes,  "ControlTypes")
    PROPERTYSTR2(controlNames,  "ControlNames")

    PROPERTY4(int, gsStickMode,   "GSStickMode",    0)
    PROPERTY4(int, ppmMultiplier, "PPM_Multiplier", 0)
    PROPERTY4(int, vBatWarn,      "vBatWarn",       0)
    PROPERTY4(int, vBatMin,       "VbatMin",        0)
    PROPERTY4(int, vBatMax,       "VbatMax",        0)
    PROPERTY4(int, txCurrentCalibration, "currentCalib",  0)
    PROPERTY4(int, txVoltageCalibration, "VbatCalib",     0)

    int index;

  protected:
    virtual QString settingsPath() const { return QString("Profiles/profile%1/").arg(index); }
};

class AppData: protected CompStoreObj
{
  public:
    AppData() :
      CompStoreObj(),
      m_profileId(0),
      m_sessionId(0)
    {}

    void init();

    // TODO: better profile handling... e.g. dynamic list, change signals... etc.

    //! Get the currently active radio profile ID. This may or may not be the same as \p id(). \sa currentProfile()
    inline int sessionId() const { return m_sessionId; }
    //! Set the current profile ID, but do not save it in persisted settings. To set and save the ID, use \p id(int)
    inline void sessionId(int x)
    {
      if (x > -1 && x < MAX_PROFILES)
        m_sessionId = x;
    }

    //! Get the last user-selected radio profile ID, this setting is saved between user sessions.
    inline int id() const { return m_profileId; }
    //! Set the current profile ID, and save it to persistent storage. Use this e.g. when switching profiles in GUI.
    void id(int index)
    {
      if (index < 0 || index >= MAX_PROFILES)
        return;
      m_profileId = index;
      sessionId(m_profileId);
      store(m_profileId, "profileId");
    }

    //! Get a modifiable (non-const) reference to the currently active Profile.  \sa sessionId()
    inline Profile & currentProfile() { return getProfile(m_sessionId); }
    //! Get a non-modifiable (const) reference to the currently active Profile.  \sa sessionId()
    inline const Profile & currentProfile() const { return getProfile(m_sessionId); }

    //! Get a modifiable (non-const) reference to the Profile at \a index. Returns the default profile if \a index is invalid.
    Profile & getProfile(int index)
    {
      if (index > -1 && index < MAX_PROFILES)
        return profile[index];
      return profile[0];
    }

    //! Get a non-modifiable (const) reference to the Profile at \a index. Returns the default profile if \a index is invalid.
    const Profile & getProfile(int index) const
    {
      if (index > -1 && index < MAX_PROFILES)
        return profile[index];
      return profile[0];
    }


    inline bool isFirstUse()         const { return firstUse; }
    inline QString previousVersion() const { return upgradeFromVersion; }
    inline bool hasCurrentSettings() const { return m_settings.contains(settingsVersionKey); }

    QMap<int, QString> getActiveProfiles() const;
    bool findPreviousVersionSettings(QString * version) const;
    bool importSettings(QString fromVersion);

    inline DownloadBranchType boundedOpenTxBranch() const {
#if defined(ALLOW_NIGHTLY_BUILDS)
      return qBound(BRANCH_RELEASE_STABLE, DownloadBranchType(OpenTxBranch()), BRANCH_NIGHTLY_UNSTABLE);
#else
      return qBound(BRANCH_RELEASE_STABLE, DownloadBranchType(OpenTxBranch()), BRANCH_RC_TESTING);
#endif
    }

    Profile    profile[MAX_PROFILES];
    JStickData joystick[MAX_JOYSTICKS];
    FwRevision fwRev;

  private:

    // !! When adding properties, remember to also add the matching name_init() call in AppData::init() !! //

    PROPERTY4(QStringList, recentFiles,  "recentFileList", QStringList())

    PROPERTYQBA2(mainWinGeo,   "mainWindowGeometry")
    PROPERTYQBA2(mainWinState, "mainWindowState")
    PROPERTYQBA2(modelEditGeo, "modelEditGeometry")
    PROPERTYQBA (mdiWinGeo)
    PROPERTYQBA (mdiWinState)
    PROPERTYQBA (compareWinGeo)

    PROPERTYSTR3(armMcu,          "arm_mcu",         QStringLiteral("at91sam3s4-9x"))
    PROPERTYSTR2(avrArguments,    "avr_arguments")
    PROPERTYSTR2(avrPort,         "avr_port")
    PROPERTYSTR2(avrdudeLocation, "avrdudeLocation")
    PROPERTYSTR3(dfuArguments,    "dfu_arguments",   QStringLiteral("-a 0"))
    PROPERTYSTR2(dfuLocation,     "dfu_location")
    PROPERTYSTR2(sambaLocation,   "samba_location")
    PROPERTYSTR3(sambaPort,       "samba_port",      QStringLiteral("\\USBserial\\COM23"))
    PROPERTYSTR2(backupDir,       "backupPath")
    PROPERTYSTR2(eepromDir,       "lastDir")
    PROPERTYSTR2(flashDir,        "lastFlashDir")
    PROPERTYSTR2(imagesDir,       "lastImagesDir")
    PROPERTYSTR2(logDir,          "lastLogDir")
    PROPERTYSTR2(libDir,          "libraryPath")
    PROPERTYSTR2(snapshotDir,     "snapshotpath")
    PROPERTYSTR2(updatesDir,      "lastUpdatesDir")

    PROPERTYSTR (locale)
    PROPERTYSTR (gePath)
    PROPERTYSTRD(mcu,        QStringLiteral("m64"))
    PROPERTYSTRD(programmer, QStringLiteral("usbasp"))
    PROPERTYSTRD(appLogsDir, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) % "/" COMPANY "/DebugLogs")

    PROPERTY(unsigned, OpenTxBranch,   BRANCH_RELEASE_STABLE)
    PROPERTY(unsigned, newModelAction, 1)  // 0=no action; 1=model wizard; 2=model edit

    PROPERTY4(int, embedSplashes,   "embedded_splashes",  0)
    PROPERTY4(int, fwServerFails,   "fwserver",           0)
    PROPERTY4(int, iconSize,        "icon_size",          2)
    PROPERTY4(int, jsCtrl,          "js_ctrl",            0)
    PROPERTY4(int, historySize,     "history_size",      10)
    PROPERTY (int, generalEditTab,  0)
    PROPERTY (int, theme,           1)
    PROPERTY (int, warningId,       0)

    PROPERTY4(bool, jsSupport,       "js_support",              false)
    PROPERTY4(bool, showSplash,      "show_splash",             true)
    PROPERTY4(bool, snapToClpbrd,    "snapshot_to_clipboard",   false)
    PROPERTY4(bool, autoCheckApp,    "startup_check_companion", true)
    PROPERTY4(bool, autoCheckFw,     "startup_check_fw",        true)

    PROPERTY(bool, enableBackup,               false)
    PROPERTY(bool, backupOnFlash,              true)
    PROPERTY(bool, outputDisplayDetails,       false)
    PROPERTY(bool, checkHardwareCompatibility, true)
    PROPERTY(bool, removeModelSlots,           true)
    PROPERTY(bool, maximized,                  false)
    PROPERTY(bool, tabbedMdi,                  false)
    PROPERTY(bool, appDebugLog,                false)
    PROPERTY(bool, fwTraceLog,                 false)

    // Simulator global (non-profile) settings
    PROPERTY(QStringList, simuDbgFilters, QStringList())
    PROPERTY(int, backLight,       0)  // TODO: move to profile
    PROPERTY(int, simuLastProfId, -1)
    PROPERTY(bool, simuSW,      true)

    //! Add setting key names here which are unused and should be removed entirely. This list should be updated when settings PRODUCT changes.
    static QStringList deprecatedSettings()
    {
      static QStringList list = QStringList()
        << "avrdude_location" // named avrdudeLocation since a long time ago but old one keeps getting imported.
        << "last_simulator"   // removed in 2.1
        << "companionBranch" << "useCompanionNightlyBuilds" << "useFirmwareNightlyBuilds"  // removed in 2.2
      ;
      return list;
    }

    void convertSettings(QSettings & settings);

    int m_profileId;  // last user-selected radio profile ID, persisted to saved settings
    int m_sessionId;  // currently active radio profile ID, NOT saved to persistent storage

    bool firstUse;
    QString upgradeFromVersion;
    static QString settingsVersionKey;

};

extern AppData g;

#endif // _APPDATA_H_
