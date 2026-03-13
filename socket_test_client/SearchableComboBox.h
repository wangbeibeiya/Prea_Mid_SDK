#ifndef SEARCHABLECOMBOBOX_H
#define SEARCHABLECOMBOBOX_H

#include <QComboBox>
#include <QSortFilterProxyModel>
#include <QCompleter>

/**
 * 可搜索下拉框：支持在下拉选择时通过输入文字过滤选项
 */
class SearchableComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit SearchableComboBox(QWidget* parent = nullptr);

    void setFilterCaseSensitivity(Qt::CaseSensitivity cs);

protected:
    void showPopup() override;

private:
    void initCompleter();
    QSortFilterProxyModel* m_filterModel;
    QCompleter* m_completer;
};

#endif // SEARCHABLECOMBOBOX_H
