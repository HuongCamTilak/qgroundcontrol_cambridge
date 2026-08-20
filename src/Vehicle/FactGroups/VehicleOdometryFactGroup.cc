#include "VehicleOdometryFactGroup.h"
#include "Vehicle.h"

VehicleOdometryFactGroup::VehicleOdometryFactGroup(QObject *parent)
    : FactGroup(1000, QStringLiteral(":/json/Vehicle/OdometryFact.json"), parent)
{
    _addFact(&_xFact);
    _addFact(&_yFact);
    _addFact(&_zFact);
    _addFact(&_xVarianceFact);
    _addFact(&_yVarianceFact);
    _addFact(&_zVarianceFact);
    _addFact(&_qualityFact);

    _xFact.setRawValue(qQNaN());
    _yFact.setRawValue(qQNaN());
    _zFact.setRawValue(qQNaN());
    _xVarianceFact.setRawValue(qQNaN());
    _yVarianceFact.setRawValue(qQNaN());
    _zVarianceFact.setRawValue(qQNaN());
    _qualityFact.setRawValue(-1);
}

void VehicleOdometryFactGroup::handleMessage(Vehicle *vehicle, const mavlink_message_t &message)
{
    Q_UNUSED(vehicle);

    // Only the visual-inertial odometry component's own estimate is relevant here, not the
    // autopilot's fused local position (which can arrive as the same ODOMETRY message id).
    if ((message.msgid != MAVLINK_MSG_ID_ODOMETRY) || (message.compid != MAV_COMP_ID_VISUAL_INERTIAL_ODOMETRY)) {
        return;
    }

    mavlink_odometry_t odometry{};
    mavlink_msg_odometry_decode(&message, &odometry);

    x()->setRawValue(odometry.x);
    y()->setRawValue(odometry.y);
    z()->setRawValue(odometry.z);

    // pose_covariance holds the upper-right triangle of the row-major 6x6 [x,y,z,roll,pitch,yaw]
    // covariance matrix; the x/y/z variance (diagonal) terms sit at indices 0, 6, and 11.
    xVariance()->setRawValue(odometry.pose_covariance[0]);
    yVariance()->setRawValue(odometry.pose_covariance[6]);
    zVariance()->setRawValue(odometry.pose_covariance[11]);

    quality()->setRawValue(odometry.quality);

    _setTelemetryAvailable(true);
}
