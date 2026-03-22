import QtQuick
import QtQuick.Layouts
import QtGraphs

Rectangle {
    id: root
    color: "#1e1e1e"

    component BaseSlice: PieSlice {
        labelVisible: false 
        borderColor: "#1e1e1e"
        borderWidth: 2
    }

    component LegendItem: Row {
        spacing: 8
        property string name
        property PieSlice targetSlice

        Rectangle {
            width: 14
            height: 14
            radius: 3
            color: targetSlice.color
            anchors.verticalCenter: parent.verticalCenter
        }
        
        Text {
            text: `${name} (${targetSlice.value})`
            color: "#cccccc"
            font.family: "Segoe UI"
            font.pixelSize: 13
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        //anchors.rightMargin: 20
        //anchors.topMargin: 20
        //anchors.bottomMargin: 20
        spacing: 30

        Column {
            Layout.alignment: Qt.AlignVCenter
            spacing: 12

            LegendItem { name: "ALU"; targetSlice: aluSlice }
            LegendItem { name: "Mem"; targetSlice: memSlice }
            LegendItem { name: "Ctrl"; targetSlice: ctrlSlice }
            LegendItem { name: "I/O"; targetSlice: ioSlice }
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            antialiasing: true
            
            theme: GraphsTheme { 
                colorScheme: GraphsTheme.ColorScheme.Dark
                backgroundColor: "transparent"
                plotAreaBackgroundColor: "transparent"
            }
            
            PieSeries {
                id: cpuSeries
                pieSize: 1
                
                BaseSlice { id: aluSlice; value: cpuData.alu; color: "#0e639c" }
                BaseSlice { id: memSlice; value: cpuData.memory; color: "#264f78" }
                BaseSlice { id: ctrlSlice; value: cpuData.control; color: "#007fd4" }
                BaseSlice { id: ioSlice; value: cpuData.io; color: "#3f3f46" }
            }
        }
    }
}