import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtCharts 2.14
import QtQuick.Layouts 1.14
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
    }

    ScrollView {
        anchors.fill: parent
        clip: true

        GridLayout {
            width: root.width
            columns: isHorizontal ? 2 : 1

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
                    }

                    Label { text: "b: " }
                    TextField {
                        id: inputB
                        text: "1.8"
                    }

                    Label { text: "c: " }
                    TextField {
                        id: inputC
                        text: "-2.9"
                    }

                    Button {
                        text: "Generate theoretical profile"
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignCenter
                        onClicked: {
                            var b = inputB.text
                            var c = inputC.text
                            var step = inputStep.text
                            var type = 0
                            var profile = backend.generateTheoreticalProfile(b, c, type, step, 0, theoreticalSeries)
                        }
                    }

                    Button {
                        text: "Minimize theoretical error"
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignCenter
                    }

                    Button {
                        text: "Load experimental"
                        Layout.columnSpan: isHorizontal ? 2 : 4
                        Layout.alignment: Qt.AlignCenter
                    }
                }
            }

            Item {
                clip: true
                Layout.minimumWidth: Math.min(root.height, root.width)
                Layout.minimumHeight: Math.min(root.height, root.width)
                Layout.preferredWidth: (isHorizontal ? root.width - controls.width : root.width) - 16
                Layout.preferredHeight: (isHorizontal ? root.height : root.height - controls.height) - 16

                SwipeView {
                    id: view
                    anchors.top: parent.top
                    anchors.left: parent.left
                    width: parent.width
                    height: parent.height - tabBar.height
                    currentIndex: tabBar.currentIndex

                    ChartView {
                        id: profilesChart
                        theme: ChartView.ChartThemeLight
                        antialiasing: true

                        LineSeries {
                            id: theoreticalSeries
                            color: "blue"
                            name: "Theoretical"
                        }

                        LineSeries {
                            color: "green"
                            name: "Experimental"
                        }
                    }

                    ChartView {
                        id: errorChart
                        theme: ChartView.ChartThemeLight
                        antialiasing: true

                        LineSeries {
                            color: "red"
                            name: "Error"
                        }
                    }
                }

                TabBar {
                    id: tabBar
                    currentIndex: view.currentIndex
                    anchors.top: view.bottom
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
        }
    }



}
