import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Material 2.14
import DropShapeAnalysis.Backend 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 500
    title: qsTr("Drop Shape Analysis")
    property int configuredTheme: Material.Light
    property int currentTheme: Material.Light
    Material.theme: currentTheme

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
            mainView.setB(b)
            mainView.setC(c)
            mainView.generateTheoreticalProfile()
        }

        onOperationCanceled: {
            progressPopup.close()
            cancelButton.text = "Cancel"
            cancelButton.enabled = true
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
        closePolicy: Popup.NoAutoClose

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
                id: cancelButton
                Layout.alignment: Qt.AlignCenter
                text: "Cancel"

                onClicked: {
                    backend.cancelOperation()
                    text = "Cancelling..."
                    enabled = false
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
                Layout.maximumWidth: window.width * 0.8
                color: Material.color(Material.red)
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
                Layout.fillWidth: true
                text: "Open text file"
                onClicked: {
                    experimentalSelectPopup.close()
                    textFileDialog.open();
                }
            }
            ToolButton {
                Layout.fillWidth: true
                text: "Open image/photo"
                onClicked: {
                    experimentalSelectPopup.close()
                    imageFileDialog.open();
                }
            }
            ToolButton {
                Layout.fillWidth: true
                text: "Take photo"
                onClicked: {
                    experimentalSelectPopup.close()
                    stackView.push(cameraItem)
                    window.currentTheme = Material.Dark
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
                        window.currentTheme = window.configuredTheme
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
