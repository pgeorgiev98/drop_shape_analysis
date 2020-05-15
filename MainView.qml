import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtCharts 2.14

Item {
    property LineSeries theoreticalSeries: qtheoreticalSeries
    property LineSeries experimentalSeries: qexperimentalSeries
    property LineSeries errorSeries: qerrorSeries

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
                            width: 2
                        }

                        LineSeries {
                            id: qexperimentalSeries
                            color: "green"
                            name: "Experimental"
                            width: 2
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
                            width: 2
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
