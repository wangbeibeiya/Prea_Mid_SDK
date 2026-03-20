#include "MainWindow.h"
#include "SocketClient.h"
#include "SearchableComboBox.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFont>
#include <QScrollBar>
#include <QMessageBox>
#include <QTextCursor>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_client(new SocketClient()) {
    setWindowTitle("Socket 命令测试客户端");
    setMinimumSize(700, 550);
    resize(800, 600);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    // 连接区
    QGroupBox* connGroup = new QGroupBox("连接");
    QHBoxLayout* connLayout = new QHBoxLayout(connGroup);
    connLayout->addWidget(new QLabel("主机:"));
    m_hostEdit = new QLineEdit("127.0.0.1");
    m_hostEdit->setMaximumWidth(120);
    connLayout->addWidget(m_hostEdit);
    connLayout->addWidget(new QLabel("端口:"));
    m_portSpin = new QSpinBox();
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(12345);
    m_portSpin->setMaximumWidth(80);
    connLayout->addWidget(m_portSpin);
    m_connectBtn = new QPushButton("连接");
    m_disconnectBtn = new QPushButton("断开");
    m_disconnectBtn->setEnabled(false);
    connLayout->addWidget(m_connectBtn);
    connLayout->addWidget(m_disconnectBtn);
    connLayout->addStretch();
    mainLayout->addWidget(connGroup);

    // 命令区
    QGroupBox* cmdGroup = new QGroupBox("发送命令");
    QVBoxLayout* cmdLayout = new QVBoxLayout(cmdGroup);

    QHBoxLayout* cmdRow = new QHBoxLayout();
    cmdRow->addWidget(new QLabel("命令:"));
    m_cmdCombo = new SearchableComboBox();
    QStringList cmdList = {
        "ListCommands",
        "ImportGeometryModel", "ExecuteGeometryProcessing", "ExecuteGeometryMatching",
        "ExecuteMeshGeneration", "ShowGeometry", "ShowMesh", "CloseSession", "DeleteVolumeByName", "SavePpcf", "GetMeshQuality", "ImportPpcf",
        "GetVolumeListNames", "GetFaceGroupNamesByVolume", "GetUnmatchedVolumeNames", "RefreshSessionData",
        "GetMeshWindowHandle", "ResetMeshCamera", "RenderMesh", "SetMeshSize",
        "ToggleMeshEdges", "SetMeshRepresentation", "ToggleMeshWireframe",
        "ToggleMeshPoints", "SetMeshTransparency", "ToggleMeshColorByGroup",
        "GetVolumeWindowHandle", "ResetVolumeCamera", "RenderVolume", "SetVolumeSize",
        "RenderVolumesWithOpacity"
    };
    m_cmdCombo->addItems(cmdList);
    m_cmdCombo->setMinimumWidth(220);
    m_cmdCombo->lineEdit()->setPlaceholderText("输入检索命令...");
    cmdRow->addWidget(m_cmdCombo);
    cmdLayout->addLayout(cmdRow);

    cmdRow = new QHBoxLayout();
    cmdRow->addWidget(new QLabel("jsonPath:"));
    m_jsonPathEdit = new QLineEdit("F:/PeraSimTestPro/f98/T1230/T1230.json");
    m_jsonPathEdit->setPlaceholderText("C:/path/to/project.json");
    cmdRow->addWidget(m_jsonPathEdit);
    cmdLayout->addLayout(cmdRow);

    cmdRow = new QHBoxLayout();
    cmdRow->addWidget(new QLabel("ppcfPath:"));
    m_ppcfPathEdit = new QLineEdit();
    m_ppcfPathEdit->setPlaceholderText("ShowMesh/GetMeshQuality 用，空则从 jsonPath 推导");
    cmdRow->addWidget(m_ppcfPathEdit);
    cmdLayout->addLayout(cmdRow);

    cmdRow = new QHBoxLayout();
    cmdRow->addWidget(new QLabel("savePath:"));
    m_savePathEdit = new QLineEdit();
    m_savePathEdit->setPlaceholderText("SavePpcf 保存路径（必填）");
    cmdRow->addWidget(m_savePathEdit);
    cmdLayout->addLayout(cmdRow);

    cmdRow = new QHBoxLayout();
    cmdRow->addWidget(new QLabel("sessionId:"));
    m_sessionIdEdit = new QLineEdit();
    m_sessionIdEdit->setPlaceholderText("geom_1 (从上一步返回获取)");
    cmdRow->addWidget(m_sessionIdEdit);
    cmdLayout->addLayout(cmdRow);

    cmdRow = new QHBoxLayout();
    cmdRow->addWidget(new QLabel("volumeName:"));
    m_volumeNameEdit = new QLineEdit();
    m_volumeNameEdit->setPlaceholderText("GetFaceGroupNamesByVolume / DeleteVolumeByName / RenderVolumesWithOpacity");
    cmdRow->addWidget(m_volumeNameEdit);
    cmdLayout->addLayout(cmdRow);

    cmdRow = new QHBoxLayout();
    cmdRow->addWidget(new QLabel("opacity:"));
    m_opacityEdit = new QLineEdit("0.5");
    m_opacityEdit->setPlaceholderText("SetMeshTransparency / RenderVolumesWithOpacity 时填写 (0-1)");
    m_opacityEdit->setMaximumWidth(80);
    cmdRow->addWidget(m_opacityEdit);
    cmdRow->addWidget(new QLabel("representation:"));
    m_representationCombo = new QComboBox();
    m_representationCombo->addItems({"surface", "wireframe", "points"});
    m_representationCombo->setMaximumWidth(100);
    cmdRow->addWidget(m_representationCombo);
    cmdRow->addWidget(new QLabel("enable:"));
    m_enableCombo = new QComboBox();
    m_enableCombo->addItems({"true", "false"});
    m_enableCombo->setMaximumWidth(60);
    cmdRow->addWidget(m_enableCombo);
    cmdRow->addWidget(new QLabel("importMode:"));
    m_importModeCombo = new QComboBox();
    m_importModeCombo->addItems({"geometry", "mesh"});
    m_importModeCombo->setMaximumWidth(90);
    m_importModeCombo->setToolTip("ImportPpcf 用: geometry=openDocument, mesh=importMesh");
    cmdRow->addWidget(m_importModeCombo);
    m_verboseLogCheck = new QCheckBox("verboseLog (ExecuteGeometryMatching 详细日志)");
    m_verboseLogCheck->setToolTip("勾选后输出体/面匹配过程、包围盒调试等详细日志");
    cmdRow->addWidget(m_verboseLogCheck);
    cmdRow->addStretch();
    cmdLayout->addLayout(cmdRow);

    m_sendBtn = new QPushButton("发送");
    m_sendBtn->setEnabled(false);
    cmdLayout->addWidget(m_sendBtn);

    mainLayout->addWidget(cmdGroup);

    // 响应区
    QGroupBox* respGroup = new QGroupBox("响应");
    QVBoxLayout* respLayout = new QVBoxLayout(respGroup);
    m_responseEdit = new QTextEdit();
    m_responseEdit->setReadOnly(true);
    m_responseEdit->setFont(QFont("Consolas", 9));
    respLayout->addWidget(m_responseEdit);
    mainLayout->addWidget(respGroup);

    connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::onConnect);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &MainWindow::onDisconnect);
    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSend);
}

void MainWindow::onConnect() {
    if (m_client->connect(m_hostEdit->text().toStdString(), m_portSpin->value())) {
        m_connectBtn->setEnabled(false);
        m_disconnectBtn->setEnabled(true);
        m_sendBtn->setEnabled(true);
        appendResponse("已连接到 " + m_hostEdit->text() + ":" + QString::number(m_portSpin->value()));
    } else {
        QMessageBox::warning(this, "连接失败", "无法连接到服务器，请确认 MappingGeometry 已启动。");
    }
}

void MainWindow::onDisconnect() {
    m_client->disconnect();
    m_connectBtn->setEnabled(true);
    m_disconnectBtn->setEnabled(false);
    m_sendBtn->setEnabled(false);
    appendResponse("已断开连接");
}

void MainWindow::onSend() {
    if (!m_client->isConnected()) {
        QMessageBox::warning(this, "未连接", "请先连接服务器");
        return;
    }

    QString cmd = m_cmdCombo->currentText();
    json params;

    if (cmd == "ListCommands") {
        // 无需参数
    } else if (cmd == "ImportGeometryModel") {
        params["jsonPath"] = m_jsonPathEdit->text().trimmed().toStdString();
    } else if (cmd == "ExecuteGeometryProcessing") {
        params["sessionId"] = m_sessionIdEdit->text().trimmed().toStdString();
    } else if (cmd == "ExecuteGeometryMatching") {
        params["sessionId"] = m_sessionIdEdit->text().trimmed().toStdString();
        params["jsonPath"] = m_jsonPathEdit->text().trimmed().toStdString();
        if (m_verboseLogCheck->isChecked())
            params["verboseLog"] = true;
    } else if (cmd == "ExecuteMeshGeneration") {
        params["jsonPath"] = m_jsonPathEdit->text().trimmed().toStdString();
        QString sid = m_sessionIdEdit->text().trimmed();
        if (!sid.isEmpty()) params["sessionId"] = sid.toStdString();  // 复用几何 session，可跳过 SavePpcf
        QString jp = m_jsonPathEdit->text().trimmed();
        QString ppcf = jp.endsWith(".json") ? jp.left(jp.size() - 5) + ".ppcf" : (jp.endsWith(".ppcf") ? jp : jp + ".ppcf");
        params["ppcfPath"] = ppcf.toStdString();
    } else if (cmd == "SavePpcf") {
        QString sp = m_savePathEdit->text().trimmed();
        if (sp.isEmpty()) {
            QString jp = m_jsonPathEdit->text().trimmed();
            sp = jp.endsWith(".json") ? jp.left(jp.size() - 5) + ".ppcf" : (jp.endsWith(".ppcf") ? jp : jp + ".ppcf");
        }
        if (!sp.isEmpty()) params["savePath"] = sp.toStdString();
    } else if (cmd == "CloseSession") {
        params["sessionId"] = m_sessionIdEdit->text().trimmed().toStdString();
    } else if (cmd == "DeleteVolumeByName") {
        params["sessionId"] = m_sessionIdEdit->text().trimmed().toStdString();
        params["volumeName"] = m_volumeNameEdit->text().trimmed().toStdString();
    } else if (cmd == "GetVolumeListNames") {
        QString sid = m_sessionIdEdit->text().trimmed();
        if (!sid.isEmpty()) params["sessionId"] = sid.toStdString();
        else params["ppcfPath"] = m_jsonPathEdit->text().trimmed().toStdString();
    } else if (cmd == "GetFaceGroupNamesByVolume") {
        params["sessionId"] = m_sessionIdEdit->text().trimmed().toStdString();
        params["volumeName"] = m_volumeNameEdit->text().trimmed().toStdString();
        params["ppcfPath"] = m_jsonPathEdit->text().trimmed().toStdString();
    } else if (cmd == "GetUnmatchedVolumeNames" || cmd == "RefreshSessionData") {
        params["sessionId"] = m_sessionIdEdit->text().trimmed().toStdString();
    } else if (cmd == "ShowGeometry") {
        params["sessionId"] = m_sessionIdEdit->text().trimmed().toStdString();
    } else if (cmd == "ShowMesh" || cmd == "GetMeshQuality") {
        QString ppcf = m_ppcfPathEdit->text().trimmed();
        if (ppcf.isEmpty()) {
            QString jp = m_jsonPathEdit->text().trimmed();
            if (!jp.isEmpty()) {
                ppcf = jp.endsWith(".json") ? jp.left(jp.size() - 5) + ".ppcf" : (jp.endsWith(".ppcf") ? jp : jp + ".ppcf");
            }
        }
        if (!ppcf.isEmpty()) params["ppcfPath"] = ppcf.toStdString();
    } else if (cmd == "ImportPpcf") {
        QString ppcf = m_ppcfPathEdit->text().trimmed();
        if (ppcf.isEmpty()) {
            QString jp = m_jsonPathEdit->text().trimmed();
            if (!jp.isEmpty()) {
                ppcf = jp.endsWith(".json") ? jp.left(jp.size() - 5) + ".ppcf" : (jp.endsWith(".ppcf") ? jp : jp + ".ppcf");
            }
        }
        if (!ppcf.isEmpty()) params["ppcfPath"] = ppcf.toStdString();
        params["importMode"] = m_importModeCombo->currentText().toStdString();
    } else if (cmd == "SetMeshSize" || cmd == "SetVolumeSize") {
        params["width"] = 800;
        params["height"] = 600;
    } else if (cmd == "SetMeshTransparency") {
        bool ok = false;
        double op = m_opacityEdit->text().trimmed().toDouble(&ok);
        params["opacity"] = ok && op >= 0 && op <= 1 ? op : 0.5;
    } else if (cmd == "SetMeshRepresentation") {
        params["representation"] = m_representationCombo->currentText().toStdString();
    } else if (cmd == "ToggleMeshEdges" || cmd == "ToggleMeshWireframe" || cmd == "ToggleMeshPoints") {
        params["enable"] = (m_enableCombo->currentText() == "true");
    } else if (cmd == "RenderVolumesWithOpacity") {
        params["volumeName"] = m_volumeNameEdit->text().trimmed().toStdString();
        bool ok = false;
        double op = m_opacityEdit->text().trimmed().toDouble(&ok);
        params["opacity"] = ok && op >= 0 && op <= 1 ? op : 0.5;
    }
    // 其他命令（可视化等）使用空 params，服务端使用默认值

    appendResponse("\n>>> " + cmd);
    json response;
    if (m_client->sendCommand(cmd.toStdString(), params, response)) {
        std::string respStr = response.dump(2);
        bool isError = response.contains("success") && response["success"].is_boolean() && !response["success"].get<bool>();
        appendResponse(QString::fromStdString(respStr), isError);

        if (response.contains("sessionId") && response["sessionId"].is_string()) {
            m_sessionIdEdit->setText(QString::fromStdString(response["sessionId"].get<std::string>()));
        }
    } else {
        appendResponse("发送失败或连接已断开", true);
    }
}

void MainWindow::appendResponse(const QString& text, bool isError) {
    if (isError) {
        QTextCursor cursor = m_responseEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        QString escaped = text.toHtmlEscaped().replace("\n", "<br>");
        cursor.insertHtml("<span style='color:#c00'>" + escaped + "</span>");
        cursor.insertBlock();
    } else {
        m_responseEdit->append(text);
    }
    m_responseEdit->verticalScrollBar()->setValue(m_responseEdit->verticalScrollBar()->maximum());
}
