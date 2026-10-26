import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: window
    objectName: "mainWindow"
    width: 1100
    height: 700
    minimumWidth: 760
    minimumHeight: 480
    visible: true
    title: qsTr("XML 转 Properties")

    property bool statusError: false

    function showStatus(message, isError) {
        statusLabel.text = message
        window.statusError = isError
    }

    function loadXmlFile(fileUrl) {
        var text = converter.readTextFile(fileUrl.toString())
        if (converter.lastError.length > 0) {
            showStatus(converter.lastError, true)
        } else {
            inputEditor.text = text
            showStatus(qsTr("已载入 XML 文件"), false)
        }
    }

    function savePropertiesFile(fileUrl) {
        if (converter.writeTextFile(fileUrl.toString(), outputEditor.text))
            showStatus(qsTr("已保存 Properties 文件"), false)
        else
            showStatus(converter.lastError, true)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                objectName: "convertButton"
                text: qsTr("转换")
                highlighted: true
                onClicked: {
                    var result = converter.convert(inputEditor.text, ignoreRootCheckBox.checked)
                    if (converter.lastError.length > 0) {
                        outputEditor.text = ""
                        showStatus(converter.lastError, true)
                    } else {
                        outputEditor.text = result
                        showStatus(qsTr("转换完成"), false)
                    }
                }
            }
            Button {
                objectName: "openButton"
                text: qsTr("打开 XML")
                onClicked: openDialog.open()
            }
            Button {
                objectName: "saveButton"
                text: qsTr("保存 Properties")
                onClicked: saveDialog.open()
            }
            Button {
                objectName: "clearButton"
                text: qsTr("清空")
                onClicked: {
                    inputEditor.clear()
                    outputEditor.clear()
                    showStatus("", false)
                }
            }
            CheckBox {
                id: ignoreRootCheckBox
                objectName: "ignoreRootCheckBox"
                text: qsTr("忽略根节点")
                checked: true
            }
            Item { Layout.fillWidth: true }
            Label {
                id: statusLabel
                objectName: "statusLabel"
                color: window.statusError ? "#c62828" : palette.mid
                elide: Text.ElideRight
                Layout.maximumWidth: 480
            }
        }

        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal
            handle: Rectangle {
                implicitWidth: 6
                color: SplitHandle.pressed ? palette.highlight : palette.mid
            }

            Pane {
                SplitView.fillWidth: true
                SplitView.minimumWidth: 280
                padding: 8
                ColumnLayout {
                    anchors.fill: parent
                    Label { text: qsTr("XML 输入"); font.bold: true }
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        TextArea {
                            id: inputEditor
                            objectName: "inputEditor"
                            placeholderText: qsTr("在此输入 XML…")
                            wrapMode: TextEdit.NoWrap
                            selectByMouse: true
                            font.family: "Consolas"
                        }
                    }
                }
            }

            Pane {
                SplitView.fillWidth: true
                SplitView.minimumWidth: 280
                padding: 8
                ColumnLayout {
                    anchors.fill: parent
                    Label { text: qsTr("Properties 输出"); font.bold: true }
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        TextArea {
                            id: outputEditor
                            objectName: "outputEditor"
                            readOnly: true
                            wrapMode: TextEdit.NoWrap
                            selectByMouse: true
                            font.family: "Consolas"
                            placeholderText: qsTr("转换结果将显示在此处")
                        }
                    }
                }
            }
        }
    }

    FileDialog {
        id: openDialog
        objectName: "openDialog"
        title: qsTr("打开 XML 文件")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("XML 文件 (*.xml)"), qsTr("所有文件 (*)")]
        onAccepted: loadXmlFile(selectedFile)
    }

    FileDialog {
        id: saveDialog
        objectName: "saveDialog"
        title: qsTr("保存 Properties 文件")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "properties"
        nameFilters: [qsTr("Properties 文件 (*.properties)"), qsTr("所有文件 (*)")]
        onAccepted: savePropertiesFile(selectedFile)
    }
}
