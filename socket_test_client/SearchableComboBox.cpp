#include "SearchableComboBox.h"
#include <QLineEdit>
#include <QAbstractItemView>
#include <QTimer>

SearchableComboBox::SearchableComboBox(QWidget* parent)
    : QComboBox(parent)
    , m_filterModel(new QSortFilterProxyModel(this))
    , m_completer(nullptr)
{
    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);

    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterModel->setFilterKeyColumn(0);
    QTimer::singleShot(0, this, [this]() { initCompleter(); });
}

void SearchableComboBox::setFilterCaseSensitivity(Qt::CaseSensitivity cs) {
    m_filterModel->setFilterCaseSensitivity(cs);
}

void SearchableComboBox::initCompleter() {
    if (m_completer || count() == 0) return;
    m_filterModel->setSourceModel(model());
    m_completer = new QCompleter(m_filterModel, this);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    setCompleter(m_completer);

    connect(lineEdit(), &QLineEdit::textEdited, this, [this](const QString& text) {
        m_filterModel->setFilterFixedString(text);
    });
    connect(m_completer, QOverload<const QString&>::of(&QCompleter::activated),
            this, [this](const QString& text) {
        if (!text.isEmpty()) {
            int idx = findText(text);
            if (idx >= 0) setCurrentIndex(idx);
        }
    });
}

void SearchableComboBox::showPopup() {
    initCompleter();
    m_filterModel->setFilterFixedString(lineEdit()->text());
    QComboBox::showPopup();
}
