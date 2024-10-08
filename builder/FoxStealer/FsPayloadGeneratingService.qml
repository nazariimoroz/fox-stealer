import QtQuick 6.6
import Service

Item {
    id: root

    required property var mainUi

    property var payloadGeneratingService: PayloadGeneratingService {}

    Connections {
        target: mainUi

        function onCheckPressed() {
            payloadGeneratingService.checkTgBotToken(mainUi.botTokenText
                , mainUi.chatIdText)
        }
    }

    Connections {
        target: mainUi.icoFileDialog

        function onAccepted() {
            payloadGeneratingService.selectIco(
                        icoFileDialog.selectedFile.toString())
        }
    }
}
