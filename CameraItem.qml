import QtQuick 2.14
import QtQuick.Controls 2.14
import QtMultimedia 5.14

Item {
    id: root

    property bool previewing: false
    property string photoLocation: ""

    function closePreview() {
        photoPreview.source = ""
        root.previewing = false
    }

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

        exposure {
            exposureCompensation: -1.0
            exposureMode: Camera.ExposurePortrait
        }

        flash.mode: Camera.FlashRedEyeReduction

        imageCapture {
            onImageCaptured: {
                photoPreview.source = preview  // Show the preview in an Image
                console.log(preview)
                console.log(photoPreview.source)
                root.previewing = true
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
        visible: !root.previewing
    }

    Image {
        id: photoPreview
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: width / 2
        anchors.horizontalCenter: parent.horizontalCenter

        width: Math.min(root.width, root.height) / 8
        height: width
        color: "white"
        border.width: 3
        border.color: "grey"
        radius: width * 0.5
        visible: !root.previewing
        opacity: 0.5

        MouseArea {
            anchors.fill: parent

            onClicked: {
                root.photoLocation = ""
                camera.imageCapture.captureToLocation(backend.getTempDir())
            }
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: width / 2
        anchors.bottomMargin: width / 2

        width: Math.min(root.width, root.height) / 8
        height: width
        color: "lightblue"
        border.width: 3
        border.color: "grey"
        radius: width * 0.5
        visible: root.previewing

        MouseArea {
            anchors.fill: parent

            onClicked: {
                backend.setPhoto(root.photoLocation)
                closePreview()
                stackView.pop()
            }
        }
    }
}
