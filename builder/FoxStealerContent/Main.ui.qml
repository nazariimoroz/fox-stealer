

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
    width: Constants.width
    height: Constants.height
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
                Layout.fillWidth: true
                placeholderText: qsTr("Bot Token")
            }

            TextField {
                id: chatIdTextField
                Layout.preferredWidth: 120
                placeholderText: qsTr("Chat Id")
                Layout.fillWidth: false
            }

            Button {
                id: checkButton
                text: qsTr("CHECK")
                Layout.fillWidth: false
            }
        }

        RowLayout {
            id: settingsRowLayout
            width: 100
            height: 100

            GroupBox {
                id: baseSettGroupBox
                width: 200
                height: 200
                title: qsTr("BASE")
                Layout.preferredWidth: 200
                Layout.fillWidth: false
                Layout.fillHeight: true

                Column {
                    id: baseSettColumn
                    anchors.fill: parent
                    anchors.topMargin: 22
                    padding: 10

                    Repeater {
                        id: checkBoxrepeater
                        model: ["Anti VM", "Melt"]

                        CheckBox {
                            id: antiVMCheckBox
                            required property string modelData
                            text: antiVMCheckBox.modelData
                            checkState: Qt.Checked
                        }
                    }
                }
            }
            Column {
                id: stealSettColumn
                width: 200
                height: 400
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }
}
