#include "VehicleOdometryFactGroupTest.h"

#include "VehicleOdometryFactGroup.h"

namespace {

mavlink_message_t buildOdometryMessage(uint8_t compId)
{
    mavlink_odometry_t odometry{};
    odometry.x = 1.5f;
    odometry.y = -2.5f;
    odometry.z = 3.5f;
    odometry.pose_covariance[0]  = 0.01f; // x variance
    odometry.pose_covariance[6]  = 0.02f; // y variance
    odometry.pose_covariance[11] = 0.03f; // z variance
    odometry.quality = 87;

    mavlink_message_t message{};
    mavlink_msg_odometry_encode(1, compId, &message, &odometry);
    return message;
}

} // namespace

void VehicleOdometryFactGroupTest::_testOdometryFromVioComponent()
{
    VehicleOdometryFactGroup group;
    QVERIFY(!group.telemetryAvailable());

    const mavlink_message_t message = buildOdometryMessage(MAV_COMP_ID_VISUAL_INERTIAL_ODOMETRY);
    group.handleMessage(nullptr, message);

    QVERIFY(group.telemetryAvailable());
    QCOMPARE(group.x()->rawValue().toFloat(), 1.5f);
    QCOMPARE(group.y()->rawValue().toFloat(), -2.5f);
    QCOMPARE(group.z()->rawValue().toFloat(), 3.5f);
    QCOMPARE(group.xVariance()->rawValue().toFloat(), 0.01f);
    QCOMPARE(group.yVariance()->rawValue().toFloat(), 0.02f);
    QCOMPARE(group.zVariance()->rawValue().toFloat(), 0.03f);
    QCOMPARE(group.quality()->rawValue().toInt(), 87);
}

void VehicleOdometryFactGroupTest::_testOdometryIgnoredFromOtherComponent()
{
    VehicleOdometryFactGroup group;

    const mavlink_message_t message = buildOdometryMessage(MAV_COMP_ID_AUTOPILOT1);
    group.handleMessage(nullptr, message);

    QVERIFY(!group.telemetryAvailable());
    QVERIFY(qIsNaN(group.x()->rawValue().toDouble()));
}

void VehicleOdometryFactGroupTest::_testOdometryIgnoredForOtherMessage()
{
    VehicleOdometryFactGroup group;

    mavlink_heartbeat_t heartbeat{};
    mavlink_message_t message{};
    mavlink_msg_heartbeat_encode(1, MAV_COMP_ID_VISUAL_INERTIAL_ODOMETRY, &message, &heartbeat);

    group.handleMessage(nullptr, message);

    QVERIFY(!group.telemetryAvailable());
}

UT_REGISTER_TEST(VehicleOdometryFactGroupTest, TestLabel::Unit)
