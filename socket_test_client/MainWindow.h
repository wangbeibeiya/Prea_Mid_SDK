#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include "SearchableComboBox.h"

#include <QSpinBox>

#include "json.hpp"
using json = nlohmann::json;

class SocketClient;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onConnect();
    void onDisconnect();
    void onSend();

private:
    void appendResponse(const QString& text, bool isError = false);

    SocketClient* m_client;
    QLineEdit* m_hostEdit;
    QSpinBox* m_portSpin;
    QPushButton* m_connectBtn;
    QPushButton* m_disconnectBtn;
    SearchableComboBox* m_cmdCombo;
    QLineEdit* m_jsonPathEdit;
    QLineEdit* m_ppcfPathEdit;
    QLineEdit* m_savePathEdit;
    QLineEdit* m_sessionIdEdit;
    QLineEdit* m_volumeNameEdit;
    QLineEdit* m_opacityEdit;
    QComboBox* m_representationCombo;
    QComboBox* m_enableCombo;
    QComboBox* m_importModeCombo;
    QCheckBox* m_verboseLogCheck;
    QPushButton* m_sendBtn;
    QTextEdit* m_responseEdit;
};

#endif
