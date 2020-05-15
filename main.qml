import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtCharts 2.14
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
        experimentalSeries: qexperimentalSeries
        theoreticalSeries: qtheoreticalSeries
        errorSeries: qerrorSeries

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
                    // TODO
                }
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        clip: true

        GridLayout {
            width: root.width
            columns: isHorizontal ? 2 : 1

            Item {
                clip: true
                Layout.minimumWidth: Math.min(root.height, root.width)
                Layout.minimumHeight: Math.min(root.height, root.width)
                Layout.preferredWidth: (isHorizontal ? root.width - controls.width : root.width) - 16
                Layout.preferredHeight: (isHorizontal ? root.height : root.height - controls.height) - 16

                SwipeView {
                    id: view
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    width: parent.width
                    height: parent.height - tabBar.height
                    currentIndex: tabBar.currentIndex

                    ChartView {
                        id: profilesChart
                        theme: ChartView.ChartThemeLight
                        antialiasing: true

                        LineSeries {
                            id: qtheoreticalSeries
                            color: "blue"
                            name: "Theoretical"
                        }

                        LineSeries {
                            id: qexperimentalSeries
                            color: "green"
                            name: "Experimental"
                        }
                    }

                    ChartView {
                        id: errorChart
                        theme: ChartView.ChartThemeLight
                        antialiasing: true

                        LineSeries {
                            id: qerrorSeries
                            color: "red"
                            name: "Error"
                        }
                    }
                }

                TabBar {
                    id: tabBar
                    currentIndex: view.currentIndex
                    anchors.bottom: view.top
                    anchors.left: view.left
                    width: view.width

                    TabButton {
                        text: "Profiles"
                    }
                    TabButton {
                        text: "Error"
                    }
                }
            }

            Item {
                property real margin: 16
                id: controls
                Layout.alignment: Qt.AlignCenter
                Layout.minimumWidth: controlsGrid.width + 2 * margin
                Layout.minimumHeight: controlsGrid.height + 2 * margin

                GridLayout {
                    id: controlsGrid
                    anchors.centerIn: parent
                    columns: isHorizontal ? 2 : 4

                    Label { text: "Type: " }
                    ComboBox {
                        id: inputType
                        model: ["Pendant", "Rotating"]
                        Layout.fillWidth: true
                    }

                    Label { text: "Step: " }
                    TextField {
                        id: inputStep
                        text: "0.1"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }

                    Label { text: "b: " }
                    TextField {
                        id: inputB
                        text: "1.8"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }

                    Label { text: "c: " }
                    TextField {
                        id: inputC
                        text: "-2.9"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }

                    Button {
                        text: "Generate theoretical profile"
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignCenter
                        onClicked: {
                            var b = inputB.text
                            var c = inputC.text
                            var step = inputStep.text
                            var type = (inputType.currentText === "Pendant" ? 0 : 1)
                            backend.generateTheoreticalProfile(b, c, type, step, 0)
                        }
                    }

                    Button {
                        text: "Minimize theoretical error"
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignCenter

                        onClicked: {
                            var step = inputStep.text
                            var type = (inputType.currentText === "Pendant" ? 0 : 1)
                            if (backend.minimizeError(type, step)) {
                                progressPopup.open()
                            } else {
                                errorMessagePopup.errorMessage = backend.lastError()
                                errorMessagePopup.open()
                            }
                        }
                    }

                    Button {
                        text: "Load experimental"
                        Layout.columnSpan: isHorizontal ? 2 : 4
                        Layout.alignment: Qt.AlignCenter

                        onClicked: {
                            experimentalSelectPopup.open()
                        }
                    }
                }
            }
        }
    }
}
