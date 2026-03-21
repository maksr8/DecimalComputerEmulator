import QtQuick
import QtGraphs

Rectangle {
    id: root
    color: "#1e1e1e"

    component BaseSlice: PieSlice {
        labelVisible: true
        labelColor: "#cccccc"
        borderColor: "#1e1e1e"
        borderWidth: 2
        labelPosition: PieSlice.LabelPosition.Outside
        labelArmLengthFactor: 0.35
    }

    GraphsView {
        anchors.fill: parent
        antialiasing: true
        
        theme: GraphsTheme { 
            colorScheme: GraphsTheme.ColorScheme.Dark
            backgroundColor: "transparent"
            plotAreaBackgroundColor: "transparent"
        }
        
        PieSeries {
            id: cpuSeries
            pieSize: 0.55
            
            BaseSlice { 
                label: `ALU (${cpuData.alu})`
                value: cpuData.alu 
                color: "#0e639c" 
            }
            BaseSlice { 
                label: `Mem (${cpuData.memory})`
                value: cpuData.memory
                color: "#264f78" 
            }
            BaseSlice { 
                label: `Ctrl (${cpuData.control})`
                value: cpuData.control
                color: "#007fd4" 
            }
            BaseSlice { 
                label: `I/O (${cpuData.io})`
                value: cpuData.io
                color: "#3f3f46" 
            }
        }
    }
}