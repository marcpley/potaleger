#ifndef POTAWIDGET_H
#define POTAWIDGET_H

#include "PotaUtils.h"
#include "qapplication.h"
#include "qboxlayout.h"
#include "qclipboard.h"
#include "qdatetimeedit.h"
#include "qevent.h"
#include "qheaderview.h"
#include "qlineedit.h"
#include "qspinbox.h"
#include "qsqlrelationaltablemodel.h"
#include "qtextedit.h"
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
    QSet<QString> generatedColumns;
    QSet<int> nonEditableColumns;
    QSet<QModelIndex> modifiedCells;
    QSet<QModelIndex> commitedCells;
    QSet<QModelIndex> copiedCells;
    QSet<int> rowsToRemove;
    QSet<int> modifiedRows;

    QString FieldName(int index);
    bool SelectShowErr();
    bool SubmitAllShowErr();
    bool RevertAllShowErr();
    bool InsertRowShowErr();
    bool DeleteRowShowErr();

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
        if (role == Qt::DisplayRole) {
            QString columnName = headerData(index.column(), Qt::Horizontal, Qt::DisplayRole).toString();
            if (modifiedRows.contains(index.row()) and  generatedColumns.contains(columnName)) {
                QSqlQuery query;
                QString primaryKey = headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(); //First column must be primaryKey !
                QVariant primaryKeyValue = record(index.row()).value(primaryKey);

                // Construire une requête pour lire directement la colonne générée
                query.prepare(QString("SELECT %1 FROM %2 WHERE %3 = :key")
                                  .arg(columnName)
                                  .arg(tableName())
                                  .arg(primaryKey));
                query.bindValue(":key", primaryKeyValue);

                if (query.exec() && query.next()) {
                    return query.value(0).toString();//+"(q)";
                }
            }
        }
        return QSqlRelationalTableModel::data(index, role);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
        if (role == Qt::EditRole) {
            if (QSqlRelationalTableModel::data(index, role).toString() != value.toString()) {
                modifiedCells.insert(index);
                modifiedRows.insert(index.row());
                if (copiedCells.contains(index))
                    copiedCells.clear();
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

    bool select() override {
        //return QSqlRelationalTableModel::select();
        //If use of QSqlRelationalTableModel select(), the generated columns and null FK value rows are not displayed. #FKNull
        QString sQuery="SELECT * FROM "+tableName();

        if (filter().toStdString()!="") {//Add filter
            sQuery+=" WHERE "+filter();
        }

        if (sOrderByClause.toStdString()!="")//Add order by
            sQuery+=" "+sOrderByClause;

        qInfo() << sQuery;

        QSqlRelationalTableModel::select();//Avoids duplicate display of inserted lines
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
            commitedCells.unite(modifiedCells);
            modifiedCells.clear();
            rowsToRemove.clear();
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

};

class PotaTableView: public QTableView
{
    Q_OBJECT

public:
    PotaTableView() {}

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void copySelectionToClipboard() {
        QClipboard *clipboard = QApplication::clipboard();
        QItemSelectionModel *selectionModel = this->selectionModel();
        if (!selectionModel || !selectionModel->hasSelection()) return;

        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
        if (selectedIndexes.isEmpty()) return;

        dynamic_cast<PotaTableModel*>(model())->copiedCells.clear();
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
            dynamic_cast<PotaTableModel*>(model())->copiedCells.insert(index);
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
                    index=selectedIndexes.value(iSel).sibling(selectedIndexes.value(iSel).row()+iCB,
                                                              selectedIndexes.value(iSel).column()+jCB);
                    if (index.isValid() and
                        (clipboardData[iCB][jCB] != "erbg-Ds45") and
                        !dynamic_cast<PotaTableModel*>(model())->nonEditableColumns.contains(index.column()))
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
            }
        }
    }

};

class PotaHeaderView : public QHeaderView {
    Q_OBJECT
public:
    explicit PotaHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QHeaderView(orientation, parent) {
        //setStretchLastSection(false);
    }
    int TempoCol=-1;
    int iSortCol = 0;

protected:
    bool bSortDes = false;

    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override {

        if (logicalIndex == TempoCol) {
            painter->save();
            int xOffset = -22;
            int yOffset = rect.height()-5;
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(1).left(3));
            painter->drawText(rect.left() + (xOffset+=28), rect.top() + yOffset, locale().monthName(2).left(3));
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(3).left(3));
            painter->drawText(rect.left() + (xOffset+=30), rect.top() + yOffset, locale().monthName(4).left(3));
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(5).left(3));
            painter->drawText(rect.left() + (xOffset+=30), rect.top() + yOffset, locale().monthName(6).left(3));
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(7).left(3));
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(8).left(3));
            painter->drawText(rect.left() + (xOffset+=30), rect.top() + yOffset, locale().monthName(9).left(3));
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(10).left(3));
            painter->drawText(rect.left() + (xOffset+=30), rect.top() + yOffset, locale().monthName(11).left(3));
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(12).left(3));

            painter->restore();
        } else
            QHeaderView::paintSection(painter, rect, logicalIndex);
    }

    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

class PotaItemDelegate2 : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit PotaItemDelegate2(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QColor cTableColor;
    QColor cColColors[50];
    int RowColorCol=-1;
    int TempoCol=-1;
    int FilterCol=-1;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const    override;

    // Création de l'éditeur
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        const QSqlRelationalTableModel *constModel = qobject_cast<const QSqlRelationalTableModel *>(index.model());
        if (!constModel) {
            return QStyledItemDelegate::createEditor(parent, option, index); // Standard editor
        }

        QSqlRelationalTableModel *model = const_cast<QSqlRelationalTableModel *>(constModel);
        QString sFieldName=model->headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString();
        QString sDataType=DataType(model->tableName(),sFieldName);
        if (model->relation(index.column()).isValid()) {
            //Create QComboBox for relational columns
            QComboBox *comboBox = new QComboBox(parent);
            QSqlTableModel *relationModel = model->relationModel(index.column());
            int relationIndex = relationModel->fieldIndex(model->relation(index.column()).displayColumn());

            // qDebug() << objectName();
            // qDebug() << parent->objectName();
            // qDebug() << parent->parent()->objectName();

            comboBox->addItem("", QVariant()); // Option for setting a NULL
            for (int i = 0; i < relationModel->rowCount(); ++i) {
                QString value = relationModel->record(i).value(relationIndex).toString();
                QString displayValue = relationModel->record(i).value(0).toString();
                if (!relationModel->record(i).value(1).toString().isEmpty())
                    displayValue+=" | "+relationModel->record(i).value(1).toString();
                if (!relationModel->record(i).value(2).toString().isEmpty())
                    displayValue+=" | "+relationModel->record(i).value(2).toString();
                comboBox->addItem( displayValue,value);
            }
            return comboBox;
        } else if (sDataType=="REAL"){
            return new QLineEdit(parent);
        } else if (sDataType.startsWith("BOOL")){
            return new QLineEdit(parent);
        } else if (sDataType=="DATE"){
            QDateEdit *dateEdit = new QDateEdit(parent);
            dateEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
            //dateEdit->setDisplayFormat("yyyy-MM-dd");
            return dateEdit;
        }
        return QStyledItemDelegate::createEditor(parent, option, index); // Standard editor

    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
        if (comboBox) {
            QVariant currentValue = index.data();
            int comboIndex = comboBox->findData(currentValue);
            comboBox->setCurrentIndex(comboIndex != -1 ? comboIndex : 0);
            return;
        }
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
        if (lineEdit) {
            lineEdit->setText(index.data().toString());
            return;
        }
        QDateEdit *dateEdit = qobject_cast<QDateEdit *>(editor);
        if (dateEdit) {
            if (index.data().isNull())
                dateEdit->setDate(QDate::currentDate());
            else
                dateEdit->setDate(index.data().toDate());
            return;
        }

        QStyledItemDelegate::setEditorData(editor, index); // Éditeur standard
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
            return;
        }
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
        if (lineEdit) {
            if (lineEdit->text().isEmpty()) {
                model->setData(index, QVariant(), Qt::EditRole); // Définit à NULL
            } else {
                model->setData(index, lineEdit->text(), Qt::EditRole);
            }
            return;
        }
        QDateEdit *dateEdit = qobject_cast<QDateEdit *>(editor);
        if (dateEdit) {
                model->setData(index, dateEdit->date(), Qt::EditRole);
            return;
        }

        QStyledItemDelegate::setModelData(editor, model, index); // Éditeur standard
    }

private:
    void paintTempo(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

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
    PotaQuery *query;//for specials coded querys.
    QTabWidget *twParent;
    bool isCommittingError=false;
    QLabel *lTabTitle;
    QTextEdit *editNotes;

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
    QLabel *lRowSummary;
    QHBoxLayout *lf;
    QHBoxLayout *ltb;
    QVBoxLayout *lw;

    QLabel *lErr;
    QAction *mEditNotes;

    void SetVisibleEditNotes(bool bVisible);

private:
    //Filtering
    int iNCharDate=10;
    int iNCharText=5;
    int iNCharReal=1;
    QString sDataNameFilter;
    QString sDataFilter;
    void SetFilterParamsFrom(QString sDataName, QString sData);
    int iPositionCol=-1;
    QString sPositionRow="";
    QString sPositionRow2="";
    void PositionSave();
    void PositionRestore();

private slots:
    void curChanged(const QModelIndex cur, const QModelIndex pre);
    void dataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight,const QList<int> &roles);
    void headerRowClicked();//int logicalIndex
    void pbRefreshClick();
    void pbCommitClick();
    void pbRollbackClick();
    void pbInsertRowClick();
    void pbDeleteRowClick();
    void cbFilterClick(Qt::CheckState state);
    void sbFilterClick(int i);
    void leFilterReturnPressed();

};

#endif // POTAWIDGET_H
