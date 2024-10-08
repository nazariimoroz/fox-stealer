// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 6.6
import FoxStealer

Window {
    width: Constants.width
    height: Constants.height

    visible: true
    title: "FoxStealer"

    Main {
        id: mainScreen
        anchors.fill: parent
    }

    FsPayloadGeneratingService {
        mainUi: mainScreen
    }
}

