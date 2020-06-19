import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14
import QtMultimedia 5.14

Item {
    id: root

    property string photoLocation: ""

    onVisibleChanged: {
        if (visible) {
            camera.start()
        } else {
            camera.stop()
        }
    }

    Camera {
        id: camera

        imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceFlash

        captureMode: Camera.CaptureStillImage

        exposure {
            exposureMode: Camera.ExposureAuto
        }

        focus {
            focusMode: Camera.FocusContinuous
            focusPointMode: Camera.FocusPointCustom
        }

        flash.mode: Camera.FlashRedEyeReduction

        imageCapture {
            onImageCaptured: {
                stackView.push(editTool)
                editTool.source = preview
            }
            onImageSaved: {
                root.photoLocation = path
            }
        }
    }

    VideoOutput {
        id: videoOutput
        source: camera
        anchors.fill: parent
        focus : visible // to receive focus and capture key events when visible
        orientation: camera.orientation
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: width / 2
        anchors.horizontalCenter: parent.horizontalCenter

        width: Math.min(root.width, root.height) / 8
        height: width
        color: "white"
        border.width: 3
        border.color: Material.color(Material.Grey)
        radius: width * 0.5
        opacity: 0.5

        MouseArea {
            anchors.fill: parent

            onClicked: {
                root.photoLocation = ""
                camera.imageCapture.captureToLocation(backend.getTempDir())
            }
        }
    }
}
