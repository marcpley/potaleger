#ifndef POTAWIDGET_H
#define POTAWIDGET_H

#include "PotaUtils.h"
#include "qapplication.h"
#include "qboxlayout.h"
#include "qclipboard.h"
#include "qevent.h"
#include "qspinbox.h"
#include "qsqlrelationaltablemodel.h"
#include "qtoolbutton.h"
#include <QTableView>
#include <QCheckBox>
#include <Qt>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QLabel>
#include "qsqlerror.h"

class PotaTableModel: public QSqlRelationalTableModel
{
    Q_OBJECT

public:
    PotaTableModel() {}
    // explicit PotaTableModel(QObject *parent = nullptr)
    //     : QSqlRelationalTableModel(parent) {}
    QString sOrderByClause="";

    QString FieldName(int index);
    bool SelectShowErr();
    bool SubmitAllShowErr();
    bool RevertAllShowErr();
    bool InsertRowShowErr();
    bool DeleteRowShowErr();

    void setColumnEditable(int column, bool editable) {
        if (editable) {
            nonEditableColumns.remove(column);
        } else {
            nonEditableColumns.insert(column);
        }
    }

    bool isColumnEditable(int column) const {
        return !nonEditableColumns.contains(column);
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override {
        if (!index.isValid())
            return Qt::NoItemFlags;

        if (nonEditableColumns.contains(index.column())) {
            return QSqlRelationalTableModel::flags(index) & ~Qt::ItemIsEditable;
        }

        return QSqlRelationalTableModel::flags(index);
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::FontRole) {
            if (nonEditableColumns.contains(section)) {
                QFont font;
                font.setItalic(true);
                return font;
            }
        }

        // if (orientation == Qt::Horizontal && role == Qt::BackgroundRole) {
        //     if (!nonEditableColumns.contains(section))
        //         return QBrush(QColor(127,127,127));
        // }

        if (orientation == Qt::Horizontal && role == Qt::TextAlignmentRole) {
            return Qt::AlignLeft;
        }

        return QSqlRelationalTableModel::headerData(section, orientation, role);
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && rowsToRemove.contains(index.row())) {
            return QVariant(); //Don't show text for rows to delete.
        }
        if (role == Qt::FontRole && nonEditableColumns.contains(index.column())) {
            QFont font;
            font.setItalic(true);
            return font;
        }
        return QSqlRelationalTableModel::data(index, role);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
        if (role == Qt::EditRole) {
            if (QSqlRelationalTableModel::data(index, role).toString() != value.toString()) {
                modifiedCells.insert(index);
                if (isCellCopied(index)) clearCellCopied();
            }
            if (relation(index.column()).isValid()) {
                //Column with FK. #FKnull
                QVariant currentValue = QSqlTableModel::data(index, role);
                if ((!value.isValid() || value.toString().isEmpty())and
                    !currentValue.isNull()) {
                    // Si la valeur est invalide ou vide, insérer NULL dans la base
                    if (QSqlTableModel::setData(index, QVariant(), role))
                      emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
                    return true;
                }
            }
        }

        return QSqlRelationalTableModel::setData(index, value, role);
    }

    bool isCellModified(const QModelIndex &index) const {
        return modifiedCells.contains(index);
    }

    bool isCellCopied(const QModelIndex &index) const {
        return copiedCells.contains(index);
    }

    void setCellCopied(const QModelIndex &index) {
        copiedCells.insert(index);
    }

    void clearCellCopied() {
        copiedCells.clear();
    }

    bool select() override {
        //If use of QSqlRelationalTableModel select(), the generated columns and null FK value rows are not displayed. #FKNull
        QString sQuery="SELECT * FROM "+tableName();

        if (filter().toStdString()!="")//Add filter
            sQuery+=" WHERE "+filter();

        if (sOrderByClause.toStdString()!="")//Add order by
            sQuery+=" "+sOrderByClause;

        qDebug() << sQuery << " (" << sOrderByClause << ")";

        setQuery(sQuery);

        return (lastError().type() == QSqlError::NoError);
    }

    void sort(int column,Qt::SortOrder so) override {

        if (column<0)
            sOrderByClause="";
        else {
            sOrderByClause="ORDER BY "+FieldName(column);
            if (so==Qt::SortOrder::DescendingOrder)
                sOrderByClause+=" DESC";
            else
                sOrderByClause+=" ASC";
        }
        select();
    }

    bool submitAll()  {
        if (QSqlRelationalTableModel::submitAll()) {
            modifiedCells.clear();
            rowsToRemove.clear();
            //setQuery("SELECT * FROM "+tableName());//To preserve dispay of ligne with null foreign key. #FKNull
            return true;
        }
        return false;
    }

    void revertAll()  {
        QSqlRelationalTableModel::revertAll();
        modifiedCells.clear(); // Vider les cellules modifiées après un rollback
        copiedCells.clear();
        QApplication::clipboard()->setText("");
        rowsToRemove.clear();
    }

    bool removeRow(int row, const QModelIndex &parent = QModelIndex()) {
        if (!rowsToRemove.contains(row)) {
            rowsToRemove.insert(row);

            // Effacer le contenu des cellules de la ligne avant de marquer pour suppression
            for (int col = 0; col < columnCount(); ++col) {
                setData(index(row, col), QVariant(), Qt::EditRole);
            }
        }
        return QSqlRelationalTableModel::removeRow(row, parent);
    }

    bool isRowMarkedForRemoval(int row) const {
        return rowsToRemove.contains(row);
    }

    void clearRemovedRows() {
        rowsToRemove.clear();
    }


private:
    QSet<int> nonEditableColumns;
    QSet<QModelIndex> modifiedCells;
    QSet<QModelIndex> copiedCells;
    QSet<int> rowsToRemove;
};

class PotaTableView: public QTableView
{
    Q_OBJECT

public:
    PotaTableView() {}

protected:
    void keyPressEvent(QKeyEvent *event) override {
        QModelIndex currentIndex = selectionModel()->currentIndex();

        if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && !(event->modifiers() == Qt::ControlModifier)) {
            if (currentIndex.isValid()) {
                edit(currentIndex);
            }
        } else if (event->key() == Qt::Key_Delete) {
            clearSelectionData();
        } else if (event->matches(QKeySequence::Copy)) {
            copySelectionToClipboard();
        } else if (event->matches(QKeySequence::Cut)) {
            cutSelectionToClipboard();
        } else if (event->matches(QKeySequence::Paste)) {
            pasteFromClipboard();
        } else {
            QTableView::keyPressEvent(event);
        }
    }
private:
    void copySelectionToClipboard() {
        QClipboard *clipboard = QApplication::clipboard();
        QItemSelectionModel *selectionModel = this->selectionModel();
        if (!selectionModel || !selectionModel->hasSelection()) return;

        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
        if (selectedIndexes.isEmpty()) return;

        dynamic_cast<PotaTableModel*>(model())->clearCellCopied();
        QApplication::clipboard()->setText("");

        // Déterminer le rectangle englobant
        int minRow = std::numeric_limits<int>::max();
        int maxRow = std::numeric_limits<int>::min();
        int minCol = std::numeric_limits<int>::max();
        int maxCol = std::numeric_limits<int>::min();

        for (const QModelIndex &index : selectedIndexes) {
            minRow = qMin(minRow, index.row());
            maxRow = qMax(maxRow, index.row());
            minCol = qMin(minCol, index.column());
            maxCol = qMax(maxCol, index.column());
        }

        // Créer une grille pour stocker les valeurs copiées
        QString placeholder = "erbg-Ds45";
        QVector<QVector<QString>> clipboardGrid(maxRow - minRow + 1, QVector<QString>(maxCol - minCol + 1, placeholder));

        // Remplir la grille avec les données des cellules sélectionnées
        for (const QModelIndex &index : selectedIndexes) {
            int row = index.row() - minRow;
            int col = index.column() - minCol;
            clipboardGrid[row][col] = model()->data(index, Qt::DisplayRole).toString();
            dynamic_cast<PotaTableModel*>(model())->setCellCopied(index);
        }

        // Convertir la grille en texte formaté pour le presse-papier
        QString clipboardText;
        for (const QVector<QString> &row : clipboardGrid) {
            clipboardText += row.join('\t') + '\n';
        }
        clipboardText.chop(1); // Supprimer le dernier saut de ligne

        // Copier le texte formaté dans le presse-papier
        clipboard->setText(clipboardText);

    }

    void cutSelectionToClipboard() {
        copySelectionToClipboard(); // Copie les données
        clearSelectionData();       // Efface les données sélectionnées
    }

    void pasteFromClipboard() {
        QClipboard *clipboard = QApplication::clipboard();
        QString clipboardText = clipboard->text();
        if (clipboardText.isEmpty()) return;
        qDebug() << clipboardText;

        QItemSelectionModel *selectionModel = this->selectionModel();
        if (!selectionModel || !selectionModel->hasSelection()) return;

        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
        if (selectedIndexes.isEmpty()) return;

        // Trier les cellules sélectionnées pour un parcours cohérent (par ligne/colonne)
        std::sort(selectedIndexes.begin(), selectedIndexes.end(), [](const QModelIndex &a, const QModelIndex &b) {
            return (a.row() == b.row()) ? (a.column() < b.column()) : (a.row() < b.row());
        });

        // Convertir le texte du presse-papier en lignes et colonnes
        QStringList rows = clipboardText.split('\n');
        QVector<QStringList> clipboardData;
        for (const QString &row : rows) {
            clipboardData.append(row.split('\t'));
        }
        int clipboardRowCount = clipboardData.size();
        int clipboardColCount = clipboardData.first().size();

        int SelectedCount=selectionModel->selectedIndexes().count();
        QModelIndex index;
        //Loop on selected cells for pasting
        for (int iSel=0;iSel<SelectedCount;iSel++)
        {
            //Paste
            //Loop on clipboard rows.
            for (int iCB=0;iCB<clipboardRowCount;iCB++)
            {
                //Loop on clipboard columns.
                for (int jCB=0;jCB<clipboardColCount;jCB++)
                {
                    qDebug() << "SelectedCount" << SelectedCount;
                    qDebug() << "CB " << iCB << " " << jCB;
                    qDebug() << "Sel " << iSel ;//<< " " << jSel;
                    index=selectedIndexes.value(iSel).sibling(selectedIndexes.value(iSel).row()+iCB,
                                                              selectedIndexes.value(iSel).column()+jCB);
                    if (index.isValid() and
                        (clipboardData[iCB][jCB] != "erbg-Ds45") and
                        dynamic_cast<PotaTableModel*>(model())->isColumnEditable(index.column()))
                        model()->setData(index,clipboardData[iCB][jCB], Qt::EditRole);
                }
            }
        }
    }

    void clearSelectionData() {
        QItemSelectionModel *selectionModel = this->selectionModel();
        if (!selectionModel || !selectionModel->hasSelection()) return;

        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
        for (const QModelIndex &index : selectedIndexes) {
            if (!model()->data(index,Qt::EditRole).isNull()) {
                model()->setData(index, QVariant(), Qt::EditRole);
                // qDebug() << "del " << index.row() << index.column();
                // QSqlRelation relation = dynamic_cast<PotaTableModel*>(model())->relation(index.column());
                // if (relation.isValid()) {
                //     qDebug() << "Relation index col " << relation.indexColumn() << " display col " << relation.displayColumn();
                //     qDebug() << relation.tableName();
                // }
            }
        }
    }

};

class PotaItemDelegate2 : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit PotaItemDelegate2(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QColor cTableColor;
    QColor cColColors[50];

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const    override;

    // Création de l'éditeur
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        const QSqlRelationalTableModel *constModel = qobject_cast<const QSqlRelationalTableModel *>(index.model());
        if (!constModel) {
            return QStyledItemDelegate::createEditor(parent, option, index); // Standard editor
        }

        QSqlRelationalTableModel *model = const_cast<QSqlRelationalTableModel *>(constModel);
        if (!model->relation(index.column()).isValid()) {
            return QStyledItemDelegate::createEditor(parent, option, index); // Standard editor
        }

        //Create QComboBox for relational columns
        QComboBox *comboBox = new QComboBox(parent);
        QSqlTableModel *relationModel = model->relationModel(index.column());
        int relationIndex = relationModel->fieldIndex(model->relation(index.column()).displayColumn());

        comboBox->addItem("", QVariant()); // Option for setting a NULL
        for (int i = 0; i < relationModel->rowCount(); ++i) {
            QString value = relationModel->record(i).value(relationIndex).toString();
            QString displayValue = relationModel->record(i).value(0).toString();
            if (!relationModel->record(i).value(1).toString().isEmpty())
                displayValue+=" | "+relationModel->record(i).value(1).toString();
            if (!relationModel->record(i).value(2).toString().isEmpty())
                displayValue+=" | "+relationModel->record(i).value(2).toString();
            comboBox->addItem(value, value);
        }

        return comboBox;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
        if (comboBox) {
            QVariant currentValue = index.data();
            int comboIndex = comboBox->findData(currentValue);
            comboBox->setCurrentIndex(comboIndex != -1 ? comboIndex : 0);
        } else {
            QStyledItemDelegate::setEditorData(editor, index); // Éditeur standard
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
        if (comboBox) {
            QVariant selectedValue = comboBox->currentData();
            if (!selectedValue.isValid() || selectedValue.toString().isEmpty()) {
                model->setData(index, QVariant(), Qt::EditRole); // Définit à NULL
            } else {
                model->setData(index, selectedValue, Qt::EditRole);
            }
        } else {
            QStyledItemDelegate::setModelData(editor, model, index); // Éditeur standard
        }
    }
};


class PotaWidget: public QWidget
{
    Q_OBJECT

public:
    PotaWidget(QWidget *parent = 0);

    void Init(QString TableName);
    PotaTableModel *model;
    QTableView *tv;
    PotaItemDelegate2 *delegate;
    //PotaItemDelegateFK *delegateFK;
    PotaQuery *query;//for specials coded querys.
    QTabWidget *twParent;
    int iSortCol = 0;
    bool bSortDes = false;
    bool isCommittingError=false;
    bool isView = false;

    QWidget *toolbar;
    QToolButton *pbRefresh;
    QToolButton *pbCommit;
    QToolButton *pbRollback;
    QSpinBox *sbInsertRows;
    QToolButton *pbInsertRow;
    QToolButton *pbDeleteRow;
    QFrame * fFilter;
    QCheckBox *cbFilter;
    QLineEdit *leFilter;
    QLabel *lFilter;
    QSpinBox *sbFilter;
    QHBoxLayout *lf;
    QHBoxLayout *ltb;
    QVBoxLayout *lw;

    QLabel *lErr;

private slots:
    void curChanged(const QModelIndex cur, const QModelIndex pre);
    void dataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight,const QList<int> &roles);
    void headerColClicked(int logicalIndex);
    void headerRowClicked();//int logicalIndex
    void pbRefreshClick();
    void pbCommitClick();
    void pbRollbackClick();
    void pbInsertRowClick();
    void pbDeleteRowClick();
    void cbFilterClick(Qt::CheckState state);
    void leFilterReturnPressed();
};

#endif // POTAWIDGET_H
