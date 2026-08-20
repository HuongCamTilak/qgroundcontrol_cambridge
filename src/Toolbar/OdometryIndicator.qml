import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls

//-------------------------------------------------------------------------
//-- Visual Odometry Indicator
Item {
    id:             control
    width:          odometryRow.width * 1.1
    anchors.top:    parent.top
    anchors.bottom: parent.bottom

    property bool showIndicator: _activeVehicle && _activeVehicle.odometry.telemetryAvailable

    property var    _activeVehicle:  QGroundControl.multiVehicleManager.activeVehicle
    property var    _odometry:       _activeVehicle ? _activeVehicle.odometry : undefined
    property real   _quality:        _odometry ? _odometry.quality.rawValue : -1
    property bool   _qualityValid:   _quality >= 0
    property bool   _qualityHealthy: _quality >= 10

    Component {
        id: odometryInfoPage

        ToolIndicatorPage {
            showExpand: false

            contentComponent: SettingsGroupLayout {
                heading: qsTr("Visual Odometry")

                LabelledLabel {
                    label:      qsTr("Quality")
                    labelText:  _qualityValid ? (_quality + "%") : qsTr("N/A")
                }

                LabelledLabel {
                    label:      qsTr("Position (X, Y, Z)")
                    labelText:  _odometry ? (_odometry.x.valueString + ", " + _odometry.y.valueString + ", " + _odometry.z.valueString + " m") : qsTr("N/A")
                }

                LabelledLabel {
                    label:      qsTr("Position Variance (X, Y, Z)")
                    labelText:  _odometry ? (_odometry.xVariance.valueString + ", " + _odometry.yVariance.valueString + ", " + _odometry.zVariance.valueString + " m²") : qsTr("N/A")
                }
            }
        }
    }

    Row {
        id:             odometryRow
        anchors.top:    parent.top
        anchors.bottom: parent.bottom
        spacing:        ScreenTools.defaultFontPixelWidth

        Image {
            anchors.top:        parent.top
            anchors.bottom:     parent.bottom
            width:              height
            fillMode:           Image.PreserveAspectFit
            sourceSize.height:  height
            // Quality < 10: unhealthy (includes -1 = failed and 0 = unknown); >= 10: healthy
            source:             _qualityHealthy ? "/qmlimages/OdometryHealthy.svg" : "/qmlimages/OdometryUnhealthy.svg"
        }
    }

    MouseArea {
        anchors.fill:   parent
        onClicked:      mainWindow.showIndicatorDrawer(odometryInfoPage, control)
    }
}
