/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "TakeoffMissionItem.h"
#include "MissionCommandTree.h"
#include "QGroundControlQmlGlobal.h"
#include "SettingsManager.h"
#include "PlanViewSettings.h"
#include "PlanMasterController.h"
#include "MissionSettingsItem.h"
#include "MultiVehicleManager.h"
#include "Vehicle.h"

TakeoffMissionItem::TakeoffMissionItem(PlanMasterController* masterController, bool flyView, MissionSettingsItem* settingsItem, bool forLoad)
    : SimpleMissionItem (masterController, flyView, forLoad)
    , _settingsItem     (settingsItem)
{
    _masterController = masterController;
    _setupMavlinkSubscription();
    _init(forLoad);
}

TakeoffMissionItem::TakeoffMissionItem(MAV_CMD takeoffCmd, PlanMasterController* masterController, bool flyView, MissionSettingsItem* settingsItem, bool forLoad)
    : SimpleMissionItem (masterController, flyView, false /* forLoad */)
    , _settingsItem     (settingsItem)
{
    setCommand(takeoffCmd);
    _masterController = masterController;
    _setupMavlinkSubscription();
    _init(forLoad);
}

TakeoffMissionItem::TakeoffMissionItem(const MissionItem& missionItem, PlanMasterController* masterController, bool flyView, MissionSettingsItem* settingsItem, bool forLoad)
    : SimpleMissionItem (masterController, flyView, missionItem)
    , _settingsItem     (settingsItem)
{
    _masterController = masterController;
    _setupMavlinkSubscription();
    _init(forLoad);
    
}

TakeoffMissionItem::~TakeoffMissionItem()
{
    _masterController = nullptr;
}

void TakeoffMissionItem::_init(bool forLoad)
{
    _editorQml = QStringLiteral("qrc:/qml/QGroundControl/Controls/SimpleItemEditor.qml");

    connect(_settingsItem, &MissionSettingsItem::coordinateChanged, this, &TakeoffMissionItem::launchCoordinateChanged);

    if (_flyView) {
        _initLaunchTakeoffAtSameLocation();
        return;
    }

    QGeoCoordinate homePosition = _settingsItem->coordinate();
    if (!homePosition.isValid()) {
        Vehicle* activeVehicle = MultiVehicleManager::instance()->activeVehicle();
        if (activeVehicle) {
            homePosition = activeVehicle->homePosition();
            if (homePosition.isValid()) {
                _settingsItem->setCoordinate(homePosition);
            }
        }
    }

    if (forLoad) {
        // Load routines will set the rest up after load
        return;
    }

    _initLaunchTakeoffAtSameLocation();
    if (_launchTakeoffAtSameLocation && homePosition.isValid()) {
        SimpleMissionItem::setCoordinate(homePosition);
    }

    // Wizard mode is set if:
    //  - Launch position is missing - requires prompt to user to click to set launch
    //  - Fixed wing - warn about climb out position adjustment
    if (!homePosition.isValid() || _controllerVehicle->fixedWing()) {
        _wizardMode = true;
    }

    setDirty(false);
}

void TakeoffMissionItem::setLaunchTakeoffAtSameLocation(bool launchTakeoffAtSameLocation)
{
    if (launchTakeoffAtSameLocation != _launchTakeoffAtSameLocation) {
        _launchTakeoffAtSameLocation = launchTakeoffAtSameLocation;
        if (_launchTakeoffAtSameLocation) {
            setLaunchCoordinate(coordinate());
        }
        emit launchTakeoffAtSameLocationChanged(_launchTakeoffAtSameLocation);
        setDirty(true);
    }
}

QGeoCoordinate TakeoffMissionItem::launchCoordinate(void) const
{
    return _settingsItem->coordinate();
}

void TakeoffMissionItem::setCoordinate(const QGeoCoordinate& coordinate)
{
    if (coordinate != this->coordinate()) {
        SimpleMissionItem::setCoordinate(coordinate);
        if (_launchTakeoffAtSameLocation) {
            _settingsItem->setCoordinate(coordinate);
        }
    }
}

bool TakeoffMissionItem::isTakeoffCommand(MAV_CMD command)
{
    return MissionCommandTree::instance()->isTakeoffCommand(command);
}

void TakeoffMissionItem::_initLaunchTakeoffAtSameLocation(void)
{
    if (specifiesCoordinate()) {
        if (_controllerVehicle->fixedWing() || _controllerVehicle->vtol()) {
            setLaunchTakeoffAtSameLocation(false);
        } else {
            // PX4 specifies a coordinate for takeoff even for multi-rotor. But it makes more sense to not have a coordinate
            // from and end user standpoint. So even for PX4 we try to keep launch and takeoff at the same position. Unless the
            // user has moved/loaded launch at a different location than takeoff.
            if (coordinate().isValid() && _settingsItem->coordinate().isValid()) {
                setLaunchTakeoffAtSameLocation(coordinate().latitude() == _settingsItem->coordinate().latitude() && coordinate().longitude() == _settingsItem->coordinate().longitude());
            } else {
                setLaunchTakeoffAtSameLocation(true);
            }

        }
    } else {
        setLaunchTakeoffAtSameLocation(true);
    }
}

bool TakeoffMissionItem::load(QTextStream &loadStream)
{
    bool success = SimpleMissionItem::load(loadStream);
    if (success) {
        _initLaunchTakeoffAtSameLocation();
    }
    _wizardMode = false; // Always be off for loaded items
    return success;
}

bool TakeoffMissionItem::load(const QJsonObject& json, int sequenceNumber, QString& errorString)
{
    bool success = SimpleMissionItem::load(json, sequenceNumber, errorString);
    if (success) {
        _initLaunchTakeoffAtSameLocation();
    }
    _wizardMode = false; // Always be off for loaded items
    return success;
}

void TakeoffMissionItem::setLaunchCoordinate(const QGeoCoordinate& launchCoordinate)
{
    if (!launchCoordinate.isValid()) {
        return;
    }

    _settingsItem->setCoordinate(launchCoordinate);

    if (!coordinate().isValid()) {
        QGeoCoordinate takeoffCoordinate;
        if (_launchTakeoffAtSameLocation) {
            takeoffCoordinate = launchCoordinate;
        } else {
            double distance = SettingsManager::instance()->planViewSettings()->vtolTransitionDistance()->rawValue().toDouble(); // Default distance is VTOL transition to takeoff point distance
            if (_controllerVehicle->fixedWing()) {
                double altitude = this->altitude()->rawValue().toDouble();

                if (altitudeMode() == QGroundControlQmlGlobal::AltitudeModeRelative) {
                    // Offset for fixed wing climb out of 30 degrees to specified altitude
                    if (altitude != 0.0) {
                        distance = altitude / tan(qDegreesToRadians(30.0));
                    }
                } else {
                    distance = altitude * 1.5;
                }
            }
            _setCurrentVehicleAttitude();
            takeoffCoordinate = launchCoordinate.atDistanceAndAzimuth(distance, _currentYaw);
        }
        SimpleMissionItem::setCoordinate(takeoffCoordinate);
    }
}

void TakeoffMissionItem::_setupMavlinkSubscription()
{
    // Connect to receive all MAVLink messages
    connect(MAVLinkProtocol::instance(), &MAVLinkProtocol::messageReceived, 
            this, &TakeoffMissionItem::_handleMavlinkMessage);
}

void TakeoffMissionItem::_setCurrentVehicleAttitude()
{
    // Use the stored values from MAVLink messages
    missionItem().setParam1(_currentPitch);  // Pitch
    missionItem().setParam4(_currentYaw);    // Yaw
}

void TakeoffMissionItem::_handleMavlinkMessage(LinkInterface* link, const mavlink_message_t& message)
{
    Q_UNUSED(link)
    
    // Handle ATTITUDE message (ID 30)
    if (message.msgid == MAVLINK_MSG_ID_ATTITUDE) {
        mavlink_attitude_t attitude;
        mavlink_msg_attitude_decode(&message, &attitude);
        
        _currentPitch = qRadiansToDegrees(attitude.pitch);
        _currentYaw = qRadiansToDegrees(attitude.yaw);
        
        if (_currentYaw < 0.0) {
            _currentYaw += 360.0;
        }
        
    }
    
    // Handle ATTITUDE_QUATERNION message (ID 31)
    else if (message.msgid == MAVLINK_MSG_ID_ATTITUDE_QUATERNION) {
        mavlink_attitude_quaternion_t attitudeQuat;
        mavlink_msg_attitude_quaternion_decode(&message, &attitudeQuat);
        
        float quaternion[4] = {
            attitudeQuat.q1,  // w
            attitudeQuat.q2,  // x
            attitudeQuat.q3,  // y
            attitudeQuat.q4   // z
        };
        
        float roll, pitch, yaw;
        mavlink_quaternion_to_euler(quaternion, &roll, &pitch, &yaw);
        
        _currentPitch = qRadiansToDegrees(pitch);
        _currentYaw = qRadiansToDegrees(yaw);
        
        if (_currentYaw < 0.0) {
            _currentYaw += 360.0;
        }
        
    }
}
