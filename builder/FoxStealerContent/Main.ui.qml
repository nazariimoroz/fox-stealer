

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 6.6
import QtQuick.Controls 6.6
import FoxStealer
import QtQuick.Layouts

Pane {
    id: mainFrame
    padding: mainFrame.baseSpacing
    property int baseSpacing: 10

    ColumnLayout {
        id: columnLayout
        anchors.fill: parent
        spacing: mainFrame.baseSpacing

        RowLayout {
            id: checkRowLayout
            spacing: mainFrame.baseSpacing
            Layout.fillWidth: true
            Layout.fillHeight: false

            TextField {
                id: botTokenTextField
                font: Constants.font
                Layout.fillWidth: true
                placeholderText: qsTr("BOT TOKEN")
            }

            TextField {
                id: chatIdTextField
                font: Constants.font
                Layout.preferredWidth: 120
                placeholderText: qsTr("CHAT ID")
                Layout.fillWidth: false
            }

            Button {
                id: checkButton
                font: Constants.largeFont
                text: qsTr("CHECK")
                Layout.fillWidth: false
            }
        }

        RowLayout {
            id: settingsRowLayout
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                id: baseSettColumn
                spacing: mainFrame.baseSpacing
                Layout.maximumWidth: 200
                Layout.fillHeight: true
                clip: true

                Repeater {
                    id: checkBoxRepeater
                    model: ["ANTI VM", "MELT"]

                    CheckBox {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        font: Constants.font
                        required property string modelData
                        text: modelData
                        checkState: Qt.Checked
                    }
                }

                Button {
                    id: icoButton
                    font: Constants.largeFont
                    text: qsTr("SELECT ICO")
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.fillWidth: true
                }
            }
            ColumnLayout {
                id: stealSettColumn
                clip: true
                Layout.fillHeight: true
                Layout.fillWidth: true

                GridView {
                    id: gridView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: ["CHROME", "FIREFOX", "SCREEN SHOT", "BROWSER PASSWORD", "BROWSER COOKIES", "WEBCAM SNAPSHOT", "SYSTEM INFO"]
                    delegate: CheckBox {
                        required property string modelData
                        font: Constants.font
                        text: modelData
                        checkState: Qt.Checked
                    }

                    cellWidth: gridView.width / 3 > 200 ? gridView.width / 3 : 0
                    cellHeight: gridView.height / 3
                }
            }
        }

        RowLayout {
            id: statusAndBuildRowLayout
            Layout.maximumHeight: 50
            Layout.fillWidth: true

            Label {
                id: buildStatusLabel
                font: Constants.largeFont
                text: qsTr("TODO OK || NOT OK")
                Layout.fillHeight: false
                Layout.fillWidth: true
            }

            Button {
                id: buildButton
                font: Constants.largeFont
                text: qsTr("BUILD")
                Layout.fillHeight: true
            }
        }
    }
}
