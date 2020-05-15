import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.2
import DropShapeAnalysis.Backend 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 800
    height: 480
    title: qsTr("Drop Shape Analysis")

    property bool isHorizontal: width / height > 1.5

    Backend {
        id: backend
        experimentalSeries: mainView.experimentalSeries
        theoreticalSeries: mainView.theoreticalSeries
        errorSeries: mainView.errorSeries

        onProgressChanged: {
            progressPopup.progress = progress
        }

        onOperationCompleted: {
            progressPopup.close()
            inputB.text = b.toFixed(8);
            inputC.text = c.toFixed(8);
            var step = inputStep.text
            var type = (inputType.currentText === "Pendant" ? 0 : 1)
            backend.generateTheoreticalProfile(b, c, type, step, 0)
        }
    }

    FileDialog {
        id: textFileDialog
        selectExisting: true

        onAccepted: {
            if (!backend.loadExperimentalFromTextFile(fileUrl)) {
                errorMessagePopup.errorMessage = backend.lastError()
                errorMessagePopup.open()
            }
        }
    }

    FileDialog {
        id: imageFileDialog
        selectExisting: true

        onAccepted: {
            if (!backend.loadExperimentalFromImageFile(fileUrl)) {
                errorMessagePopup.errorMessage = backend.lastError()
                errorMessagePopup.open()
            }
        }
    }

    Popup {
        property real progress: 0.0

        id: progressPopup
        anchors.centerIn: parent
        width: parent.width * 0.8
        modal: true

        ColumnLayout {
            anchors.fill: parent
            Label {
                Layout.alignment: Qt.AlignCenter
                text: "Calculating: " + Math.ceil(100 * progressPopup.progress) + "%"
            }
            ProgressBar {
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                from: 0
                to: 1
                value: progressPopup.progress
                indeterminate: false
            }
            Button {
                Layout.alignment: Qt.AlignCenter
                text: "Cancel"

                onClicked: {
                    // TODO
                }
            }
        }
    }

    Popup {
        id: errorMessagePopup
        anchors.centerIn: parent
        property string errorMessage: ""
        ColumnLayout {
            anchors.fill: parent
            Label {
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: root.width * 0.8
                color: "red"
                text: errorMessagePopup.errorMessage
                wrapMode: Text.WordWrap
            }
            Button {
                Layout.alignment: Qt.AlignHCenter
                text: "Ok"
                onClicked: {
                    errorMessagePopup.close()
                }
            }
        }
    }

    Popup {
        id: experimentalSelectPopup
        anchors.centerIn: parent
        modal: true
        ColumnLayout {
            ToolButton {
                text: "Open text file"
                onClicked: {
                    experimentalSelectPopup.close()
                    textFileDialog.open();
                }
            }
            ToolButton {
                text: "Open image/photo"
                onClicked: {
                    experimentalSelectPopup.close()
                    imageFileDialog.open();
                }
            }
            ToolButton {
                text: "Take photo"
                onClicked: {
                    experimentalSelectPopup.close()
                    stackView.push(cameraItem)
                }
            }
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainView
        focus: true

        Keys.onReleased: {
            if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
                if (stackView.depth > 1) {
                    if (cameraItem.previewing) {
                        cameraItem.closePreview()
                    } else {
                        stackView.pop()
                    }
                    event.accepted = true
                }
            }
        }
    }

    MainView {
        id: mainView
    }

    CameraItem {
        id: cameraItem
        visible: false
    }
}
