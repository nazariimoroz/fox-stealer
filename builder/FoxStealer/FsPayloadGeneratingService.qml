import QtQuick 6.6
import Service

Item {
    id: root

    required property var icoFileDialog

    property var payloadGeneratingService: PayloadGeneratingService {}


    Connections {
        target: icoFileDialog

        function onAccepted() {
            payloadGeneratingService.selectIco(
                        icoFileDialog.selectedFile.toString())
        }
    }
}
