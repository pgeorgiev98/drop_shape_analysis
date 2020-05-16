import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtCharts 2.14
import QtQuick.Controls.Material 2.14

Item {
    property LineSeries theoreticalSeries: qtheoreticalSeries
    property LineSeries experimentalSeries: qexperimentalSeries
    property LineSeries errorSeries: qerrorSeries

    id: root

    function setB(b) {
        inputB.text = b.toFixed(8)
    }

    function setC(c) {
        inputC.text = c.toFixed(8)
    }

    function generateTheoreticalProfile() {
        var b = inputB.text
        var c = inputC.text
        var step = inputStep.text
        var type = (inputType.currentText === "Pendant" ? 0 : 1)
        backend.generateTheoreticalProfile(b, c, type, step, 0)
    }

    GridLayout {
        anchors.fill: parent
        columns: isHorizontal ? 2 : 1

        Item {
            clip: true
            Layout.preferredWidth: (isHorizontal ? root.width - controls.width : root.width) - 16
            Layout.preferredHeight: (isHorizontal ? root.height : root.height - controls.height) - 16
            Layout.fillWidth: true
            Layout.fillHeight: true

            SwipeView {
                id: view
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                width: parent.width
                height: parent.height - tabBar.height
                currentIndex: tabBar.currentIndex

                ChartView {
                    id: profilesChart
                    theme: Material.theme === Material.Light ? ChartView.ChartThemeLight : ChartView.ChartThemeDark
                    antialiasing: true

                    LineSeries {
                        id: qtheoreticalSeries
                        color: Material.color(Material.Blue)
                        name: "Theoretical"
                        width: 2
                    }

                    LineSeries {
                        id: qexperimentalSeries
                        color: Material.color(Material.Green)
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
                        color: Material.color(Material.Red)
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

        ScrollView {
            Layout.minimumWidth: isHorizontal ? controls.width : 50
            Layout.minimumHeight: isHorizontal ? 50 : root.height / 3
            Layout.preferredWidth: isHorizontal ? Layout.minimumWidth : root.width
            Layout.preferredHeight: isHorizontal ? root.height : Layout.minimumHeight
            contentWidth: controls.width
            contentHeight: controls.height
            clip: true

            Item {
                id: controls
                property real margin: 16
                width: isHorizontal ? controlsGrid.width + 2 * margin : Math.max(root.width, controlsGrid.width + 2 * margin)
                height: isHorizontal ? Math.max(root.height, controlsGrid.height + 2 * margin) : controlsGrid.height + 2 * margin

                GridLayout {
                    id: controlsGrid
                    columns: 2
                    width: children.width
                    height: children.height
                    anchors.centerIn: parent

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
                        horizontalAlignment: Qt.AlignRight
                    }

                    Label { text: "b: " }
                    TextField {
                        id: inputB
                        text: "1.8"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        horizontalAlignment: Qt.AlignRight
                    }

                    Label { text: "c: " }
                    TextField {
                        id: inputC
                        text: "-2.9"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        horizontalAlignment: Qt.AlignRight
                    }

                    Button {
                        text: "Generate theoretical profile"
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignCenter
                        onClicked: {
                            generateTheoreticalProfile()
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
                        Layout.columnSpan: 2
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
