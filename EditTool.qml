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

    id: root

    Image {
        id: photoPreview
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: root.source
    }

    Rectangle {
        property real handlesSize: 16
        property real initWidth: 100
        property real initHeight: 100

        id: selection

        anchors.left: left.left
        anchors.right: right.right
        anchors.top: top.top
        anchors.bottom: bottom.bottom

        anchors.leftMargin: handlesSize
        anchors.rightMargin: handlesSize
        anchors.topMargin: handlesSize
        anchors.bottomMargin: handlesSize

        height: 100
        color: "red"
        opacity: 0.25

        MouseArea {
            anchors.fill: parent
            anchors.margins: selection.handlesSize
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: 0
            drag.maximumX: root.width
        }
    }

    Rectangle {
        id: left
        color: leftArea.containsMouse || leftArea.pressed ? hoveredColor : normalColor
        width: selection.handlesSize
        anchors.top: top.bottom
        anchors.bottom: bottom.top
        x: (root.width - photoPreview.paintedWidth) / 2
        MouseArea {
            id: leftArea
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: (root.width - photoPreview.paintedWidth) / 2
            drag.maximumX: right.x - selection.handlesSize
        }
    }

    Rectangle {
        id: right
        color: rightArea.containsMouse || rightArea.pressed ? hoveredColor : normalColor
        width: selection.handlesSize
        anchors.top: top.bottom
        anchors.bottom: bottom.top
        x: root.width - selection.handlesSize - (root.width - photoPreview.paintedWidth) / 2
        MouseArea {
            id: rightArea
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: left.x + selection.handlesSize
            drag.maximumX: root.width - selection.handlesSize - (root.width - photoPreview.paintedWidth) / 2
        }
    }

    Rectangle {
        id: top
        color: topArea.containsMouse || topArea.pressed ? hoveredColor : normalColor
        height: selection.handlesSize
        anchors.left: left.right
        anchors.right: right.left
        y: (root.height - photoPreview.paintedHeight) / 2

        MouseArea {
            id: topArea
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.YAxis
            drag.minimumY: (root.height - photoPreview.paintedHeight) / 2
            drag.maximumY: bottom.y - selection.handlesSize
        }
    }

    Rectangle {
        id: bottom
        color: bottomArea.containsMouse || bottomArea.pressed ? hoveredColor : normalColor
        height: selection.handlesSize
        anchors.left: left.right
        anchors.right: right.left
        y: root.height - selection.handlesSize - (root.height - photoPreview.paintedHeight) / 2
        MouseArea {
            id: bottomArea
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.YAxis
            drag.minimumY: top.y + selection.handlesSize
            drag.maximumY: root.height - selection.handlesSize - (root.height - photoPreview.paintedHeight) / 2
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: width / 2
        anchors.bottomMargin: width / 2

        width: Math.min(root.width, root.height) / 8
        height: width
        color: Material.color(Material.LightBlue)
        border.width: 3
        border.color: Material.color(Material.Grey)
        radius: width * 0.5

        MouseArea {
            anchors.fill: parent

            onClicked: {
                var dw = root.width - photoPreview.paintedWidth
                var dh = root.height - photoPreview.paintedHeight
                var x0 = (left.x - dw / 2 + 16) / photoPreview.paintedWidth
                var y0 = (top.y - dh / 2 + 16) / photoPreview.paintedHeight
                var x1 = (right.x - dw / 2) / photoPreview.paintedWidth
                var y1 = (bottom.y - dh / 2) / photoPreview.paintedHeight
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
