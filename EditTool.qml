import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtCharts 2.14
import QtQuick.Controls.Material 2.14
import QtQuick 2.14

Item {
    property string source
    property color normalColor: "white"
    property color hoveredColor: "grey"

    property real frameMargin: 32

    id: root

    Image {
        id: photoPreview
        anchors.fill: parent
        anchors.margins: root.frameMargin
        fillMode: Image.PreserveAspectFit
        source: root.source
    }

    Rectangle {
        property real handlesSize: 32
        property real initWidth: 100
        property real initHeight: 100

        id: selection

        anchors.left: left.left
        anchors.right: right.right
        anchors.top: top.top
        anchors.bottom: bottom.bottom

        anchors.margins: handlesSize

        height: 100
        color: "white"
        opacity: 0.25

        MouseArea {
            anchors.fill: parent
            anchors.margins: selection.handlesSize
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: 0
            drag.maximumX: photoPreview.width
        }
    }

    Rectangle {
        id: left
        color: leftArea.containsMouse || leftArea.pressed ? hoveredColor : normalColor
        width: selection.handlesSize
        anchors.top: top.bottom
        anchors.bottom: bottom.top
        x: root.frameMargin + (photoPreview.width - photoPreview.paintedWidth) / 2

        MouseArea {
            id: leftArea
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: root.frameMargin + (photoPreview.width - photoPreview.paintedWidth) / 2
            drag.maximumX: root.frameMargin + right.x - selection.handlesSize
        }
    }

    Rectangle {
        id: right
        color: rightArea.containsMouse || rightArea.pressed ? hoveredColor : normalColor
        width: selection.handlesSize
        anchors.top: top.bottom
        anchors.bottom: bottom.top
        x: root.frameMargin + photoPreview.width - selection.handlesSize - (photoPreview.width - photoPreview.paintedWidth) / 2
        MouseArea {
            id: rightArea
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: root.frameMargin + left.x + selection.handlesSize
            drag.maximumX: root.frameMargin + photoPreview.width - selection.handlesSize - (photoPreview.width - photoPreview.paintedWidth) / 2
        }
    }

    Rectangle {
        id: top
        color: topArea.containsMouse || topArea.pressed ? hoveredColor : normalColor
        height: selection.handlesSize
        anchors.left: left.right
        anchors.right: right.left
        y: root.frameMargin + (photoPreview.height - photoPreview.paintedHeight) / 2

        MouseArea {
            id: topArea
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.YAxis
            drag.minimumY: root.frameMargin + (photoPreview.height - photoPreview.paintedHeight) / 2
            drag.maximumY: root.frameMargin + bottom.y - selection.handlesSize
        }
    }

    Rectangle {
        id: bottom
        color: bottomArea.containsMouse || bottomArea.pressed ? hoveredColor : normalColor
        height: selection.handlesSize
        anchors.left: left.right
        anchors.right: right.left
        y: root.frameMargin + photoPreview.height - selection.handlesSize - (photoPreview.height - photoPreview.paintedHeight) / 2
        MouseArea {
            id: bottomArea
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.YAxis
            drag.minimumY: root.frameMargin + top.y + selection.handlesSize
            drag.maximumY: root.frameMargin + photoPreview.height - selection.handlesSize - (photoPreview.height - photoPreview.paintedHeight) / 2
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: width / 2
        anchors.bottomMargin: width / 2

        width: Math.min(photoPreview.width, photoPreview.height) / 8
        height: width
        color: Material.color(Material.LightBlue)
        border.width: 3
        border.color: Material.color(Material.Grey)
        radius: width * 0.5

        MouseArea {
            anchors.fill: parent

            onClicked: {
                var dw = photoPreview.width - photoPreview.paintedWidth
                var dh = photoPreview.height - photoPreview.paintedHeight
                var x0 = (left.x - root.frameMargin - dw / 2 + selection.handlesSize) / photoPreview.paintedWidth
                var y0 = (top.y - root.frameMargin - dh / 2 + selection.handlesSize) / photoPreview.paintedHeight
                var x1 = (right.x - root.frameMargin - dw / 2) / photoPreview.paintedWidth
                var y1 = (bottom.y - root.frameMargin - dh / 2) / photoPreview.paintedHeight
                var w = x1 - x0
                var h = y1 - y0
                var croppedRect = Qt.rect(x0, y0, w, h)
                backend.setPhoto(cameraItem.photoLocation, croppedRect)
                stackView.pop()
                window.currentTheme = window.configuredTheme
                stackView.pop()
            }
        }
    }
}
