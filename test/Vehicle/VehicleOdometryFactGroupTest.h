#pragma once

#include "UnitTest.h"

/// Tests ODOMETRY message handling in VehicleOdometryFactGroup, scoped to messages from the
/// visual-inertial-odometry component (MAV_COMP_ID_VISUAL_INERTIAL_ODOMETRY).
class VehicleOdometryFactGroupTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _testOdometryFromVioComponent();
    void _testOdometryIgnoredFromOtherComponent();
    void _testOdometryIgnoredForOtherMessage();
};
