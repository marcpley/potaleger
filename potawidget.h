#ifndef POTAWIDGET_H
#define POTAWIDGET_H

#include "PotaUtils.h"
#include "potagraph.h"
#include "qapplication.h"
#include "qboxlayout.h"
#include "qclipboard.h"
#include "qdatetimeedit.h"
#include "qevent.h"
#include "qheaderview.h"
#include "qlineedit.h"
#include "qpushbutton.h"
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
#include "muParser/muParser.h"

class PotaTableModel: public QSqlRelationalTableModel
{
    Q_OBJECT

public:
    PotaTableModel() {}
    // explicit PotaTableModel(QObject *parent=nullptr)
    //     : QSqlRelationalTableModel(parent) {}
    QSqlDatabase *db;
    QString sPrimaryKey;
    QString sOrderByClause="";
    QSet<QString> generatedColumns;
    QStringList dataTypes;
    QSet<int> nonEditableColumns,dateColumns,moneyColumns;
    //QSet<QModelIndex> modifiedCells;
    QSet<QModelIndex> commitedCells;
    QSet<QModelIndex> copiedCells;
    QSet<int> rowsToRemove,rowsToInsert;
    //QSet<int> modifiedRows;
    QProgressBar* progressBar;
    QString tempTableName;
    bool bBatch;

    int FieldIndex(QString sFieldName);
    QString FieldName(int index);

    bool SelectShowErr();
    bool SubmitAllShowErr();
    bool RevertAllShowErr();
    bool InsertRowShowErr();
    bool DuplicRowShowErr();
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
        if (orientation==Qt::Horizontal && role==Qt::FontRole) {
            if (nonEditableColumns.contains(section)) {
                QFont font;
                font.setItalic(true);
                return font;
            }
        }

        if (orientation==Qt::Horizontal && role==Qt::TextAlignmentRole) {
            return Qt::AlignLeft;
        }

        return QSqlRelationalTableModel::headerData(section, orientation, role);
    }

    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const override {
        // if (role==Qt::DisplayRole && rowsToRemove.contains(index.row()))
        //     return QVariant(); //Don't show text for rows to delete.
        if (role==Qt::FontRole && nonEditableColumns.contains(index.column())) {
            QFont font;
            font.setItalic(true);
            return font;
        }
        if (role==Qt::DisplayRole and !data(index,Qt::EditRole).isNull()) {
            if (dateColumns.contains(index.column())) {// #DateFormat
                if (data(index,Qt::EditRole).toDate().toString("dd/MM/yyyy").isEmpty())
                    return data(index,Qt::EditRole).toString()+"!"; //Not date format
                else
                    return data(index,Qt::EditRole).toDate().toString("dd/MM/yyyy");
            } else if (moneyColumns.contains(index.column())) {
                if (data(index,Qt::EditRole).toFloat()==0 and data(index,Qt::EditRole).toString()!="0")
                    return data(index,Qt::EditRole).toString()+"!"; //Not number format
                else
                    return QString::number(data(index,Qt::EditRole).toFloat(),'f', 2);
            } else if (headerData(index.column(), Qt::Horizontal, Qt::EditRole).toString().endsWith("_pc")) {
                return data(index,Qt::EditRole).toString()+"%";
            } else if (headerData(index.column(), Qt::Horizontal, Qt::EditRole).toString().startsWith("S_")) {
                return data(index,Qt::EditRole).toString()+" ("+QDate(2001,1,1).addDays((data(index,Qt::EditRole).toInt()-1)*7).toString("dd/MM")+")";
            } else if (data(index,Qt::EditRole).toString().startsWith(".") and data(index,Qt::EditRole).toString().length()>1) {//Invisible data
                return QVariant();
            }
        }
        if (role==Qt::TextAlignmentRole) {
            if (StrLast(data(index,Qt::EditRole).toString(),1)=='%' or
                moneyColumns.contains(index.column())) {
                return Qt::AlignRight;
            }
        }

        return QSqlRelationalTableModel::data(index, role);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override {
        if (role==Qt::EditRole) {
            // if (QSqlRelationalTableModel::data(index, role).toString() != value.toString() and
            //     !nonEditableColumns.contains(index.column())) {
            //     if (copiedCells.contains(index))
            //         copiedCells.clear();//The cell must be shown as modified before copied.
            // }
            if (relation(index.column()).isValid()) {
                //Column with FK. #FKnull
                QVariant currentValue=QSqlTableModel::data(index, role);
                if ((!value.isValid() || value.toString().isEmpty())and
                    !currentValue.isNull()) {
                    // Si la valeur est invalide ou vide, insérer NULL dans la base
                    if (QSqlTableModel::setData(index, QVariant(), role))
                      emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
                    return true;
                }
            }

            if (dateColumns.contains(index.column())) {// #DateFormat
                QString dateString=value.toString();
                QDate date=QDate::fromString(dateString, "dd/MM/yyyy");
                if (date.isValid()) {
                    return QSqlRelationalTableModel::setData(index, date, role);
                }
            } else if (value.toString().startsWith("=") and
                      (dataTypes[index.column()]=="REAL" or dataTypes[index.column()].startsWith("INT"))) {
                mu::string_type expr;
                #ifdef _WIN32
                    expr=value.toString().mid(1).trimmed().toStdWString();
                #else
                    expr=value.toString().mid(1).trimmed().toStdString();
                #endif
                //std::string expr=value.toString().mid(1).trimmed().toStdString();
                mu::Parser parser;
                parser.SetExpr(expr);
                QString result;
                try {
                    if (dataTypes[index.column()].startsWith("INT"))
                        result=QString::number(std::round(parser.Eval()));
                    else
                        result=QString::number(parser.Eval());
                    QLocale locale;
                    QString decimalSep=QString(locale.decimalPoint());
                    result.replace(decimalSep,".");
                    return QSqlRelationalTableModel::setData(index, result, role);
                } catch (mu::Parser::exception_type &e) {};
            }
        }

        return QSqlRelationalTableModel::setData(index, value, role);
    }

    bool select() override;

    void setOrderBy(int column,Qt::SortOrder so);

    bool submitAll()  {
        if (QSqlRelationalTableModel::submitAll()) {
            rowsToRemove.clear();
            rowsToInsert.clear();
            return true;
        }
        return false;
    }

    void revertAll()  {
        QSqlRelationalTableModel::revertAll();
        // QApplication::clipboard()->setText("");
        rowsToRemove.clear();
        rowsToInsert.clear();
    }

    QString RealTableName() const {
        if (tableName().contains("__"))
            return tableName().first(tableName().indexOf("__"));
        else
            return tableName();
    }

    bool removeRow(int row, const QModelIndex &parent=QModelIndex()) {
        if (!rowsToRemove.contains(row) and !rowsToInsert.contains(row))
            rowsToRemove.insert(row);
        return QSqlRelationalTableModel::removeRow(row, parent);
    }

// private slots:
//     void selectTimer();
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
        // QClipboard *clipboard=QApplication::clipboard();
        QItemSelectionModel *selectionModel=this->selectionModel();
        if (!selectionModel || !selectionModel->hasSelection()) return;

        QModelIndexList selectedIndexes=selectionModel->selectedIndexes();
        if (selectedIndexes.isEmpty()) return;

        PotaTableModel *m=dynamic_cast<PotaTableModel*>(model());
        m->copiedCells.clear();
        QApplication::clipboard()->setText("");

        // Déterminer le rectangle englobant
        int minRow=std::numeric_limits<int>::max();
        int maxRow=std::numeric_limits<int>::min();
        int minCol=std::numeric_limits<int>::max();
        int maxCol=std::numeric_limits<int>::min();

        for (const QModelIndex &index : selectedIndexes) {
            minRow=qMin(minRow, index.row());
            maxRow=qMax(maxRow, index.row());
            minCol=qMin(minCol, index.column());
            maxCol=qMax(maxCol, index.column());
        }

        // Créer une grille pour stocker les valeurs copiées
        QList<QList<QString>> clipboardGrid(maxRow - minRow + 1, QList<QString>(maxCol - minCol + 1, ""));
        QLocale locale;
        QString decimalSep=QString(locale.decimalPoint());

        // Remplir la grille avec les données des cellules sélectionnées
        for (const QModelIndex &index : selectedIndexes) {
            int row=index.row() - minRow;
            int col=index.column() - minCol;
            clipboardGrid[row][col]=StrReplace(model()->data(index, Qt::EditRole).toString(),"\n","\\n"); //sdff+54rg
            m->copiedCells.insert(index);
        }

        // Convertir la grille en texte formaté pour le presse-papier
        QString clipboardText;
        for (const QList<QString> &row : clipboardGrid) {
            clipboardText += row.join('\t') + '\n';
        }
        clipboardText.chop(1); // Supprimer le dernier saut de ligne

        // Copier le texte formaté dans le presse-papier
        QApplication::clipboard()->setText(clipboardText);

    }

    void cutSelectionToClipboard() {
        copySelectionToClipboard(); // Copie les données
        clearSelectionData();       // Efface les données sélectionnées
    }

    void pasteFromClipboard() {
        // QClipboard *clipboard=QApplication::clipboard();
        QString clipboardText=QApplication::clipboard()->text();
        if (clipboardText.isEmpty()) return;

        QItemSelectionModel *selectionModel=this->selectionModel();
        if (!selectionModel || !selectionModel->hasSelection()) return;

        QModelIndexList selectedIndexes=selectionModel->selectedIndexes();
        if (selectedIndexes.isEmpty()) return;

        // Trier les cellules sélectionnées pour un parcours cohérent (par ligne/colonne)
        std::sort(selectedIndexes.begin(), selectedIndexes.end(), [](const QModelIndex &a, const QModelIndex &b) {
            return (a.row()==b.row()) ? (a.column() < b.column()) : (a.row() < b.row());
        });

        // Convertir le texte du presse-papier en lignes et colonnes
        QStringList rows=clipboardText.split('\n');
        QList<QStringList> clipboardData;
        for (const QString &row : rows) {
            clipboardData.append(row.split('\t'));
        }
        int clipboardRowCount=clipboardData.size();
        int clipboardColCount;

        int SelectedCount=selectionModel->selectedIndexes().count();
        QModelIndex index;
        PotaTableModel *m=dynamic_cast<PotaTableModel*>(model());
        QStringList formats={"yyyy-MM-dd", "dd/MM/yyyy", "dd/MM/yy", "yy-MM-dd", "MM-dd-yyyy"};
        QDate date;
        bool noCentury=false;
        QLocale locale;
        QString decimalSep=QString(locale.decimalPoint());

        //Loop on selected cells for pasting
        for (int iSel=0;iSel<SelectedCount;iSel++)
        {
            //Paste
            //Loop on clipboard rows.
            for (int iCB=0;iCB<clipboardRowCount;iCB++)
            {
                clipboardColCount=clipboardData[iCB].size();
                //Loop on clipboard columns.
                for (int jCB=0;jCB<clipboardColCount;jCB++)
                {
                    index=selectedIndexes.value(iSel).sibling(selectedIndexes.value(iSel).row()+iCB,
                                                              selectedIndexes.value(iSel).column()+jCB);
                    //qDebug() << "clipboardData[iCB][jCB] : "+clipboardData[iCB][jCB];
                    if (index.isValid() and
                        (!clipboardData[iCB][jCB].isEmpty()) and
                        !m->nonEditableColumns.contains(index.column())) {
                        if (DataType(m->db,m->tableName(),m->headerData(index.column(),Qt::Horizontal,Qt::EditRole).toString())=="DATE") {
                            for (const QString &format : formats) {
                                date=QDate::fromString(clipboardData[iCB][jCB], format);
                                if (date.isValid()) {
                                    noCentury=(format.contains("yy") and !format.contains("yyyy"));
                                    break;
                                }
                            }
                            if (!date.isValid())
                                m->setData(index,clipboardData[iCB][jCB], Qt::EditRole);//Try raw paste
                            else {//Paste the formated date
                                if (noCentury)
                                    m->setData(index,date.toString("20yy-MM-dd"), Qt::EditRole);
                                else
                                    m->setData(index,date.toString("yyyy-MM-dd"), Qt::EditRole);
                            }
                        } else if (m->dataTypes[index.column()]=="REAL"){
                            m->setData(index,StrReplace(clipboardData[iCB][jCB],"\\n","\n").replace(decimalSep,"."), Qt::EditRole); //sdff+54rg
                        } else {
                            m->setData(index,StrReplace(clipboardData[iCB][jCB],"\\n","\n"), Qt::EditRole); //sdff+54rg
                        }
                    }
                }
            }
        }
    }

    void clearSelectionData() {
        QItemSelectionModel *selectionModel=this->selectionModel();
        if (!selectionModel || !selectionModel->hasSelection()) return;

        QModelIndexList selectedIndexes=selectionModel->selectedIndexes();
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
    explicit PotaHeaderView(Qt::Orientation orientation, QWidget *parent=nullptr)
        : QHeaderView(orientation, parent) {
        //setStretchLastSection(false);
    }
    int iSortCol=0;
    mutable bool inPaintSection=false;

protected:
    bool bSortDes=false;

    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

class PotaItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit PotaItemDelegate(QObject *parent=nullptr) : QStyledItemDelegate(parent) {}

    QColor cTableColor;
    QColor cColColors[50];
    int RowColorCol=-1;
    //QSet<int> PaintedCols;
    QStringList PaintedColsTypes;
    QStringList PaintedColsTitles;
    int FilterCol=-1;
    QString FindText="";
    bool FindTextchanged;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // Création de l'éditeur
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override ;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QComboBox *comboBox=qobject_cast<QComboBox *>(editor);
        if (comboBox) {
            QVariant currentValue=index.data(Qt::EditRole);
            int comboIndex=comboBox->findData(currentValue);
            comboBox->setCurrentIndex(comboIndex != -1 ? comboIndex : 0);
            return;
        }
        QLineEdit *lineEdit=qobject_cast<QLineEdit *>(editor);
        if (lineEdit) {
            lineEdit->setText(index.data(Qt::EditRole).toString());
            return;
        }
        QDateEdit *dateEdit=qobject_cast<QDateEdit *>(editor);
        if (dateEdit) {
            if (index.data(Qt::EditRole).isNull())
                dateEdit->setDate(QDate::currentDate());
            else {
                // #DateFormat
                QString dateString=index.data(Qt::EditRole).toString();
                QDate date=QDate::fromString(dateString, "dd/MM/yyyy");
                //if (date.isValid()) {
                dateEdit->setDate(date);//index.data().toDate()
            }
            return;
        }

        QStyledItemDelegate::setEditorData(editor, index); // Éditeur standard
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

private:
    void paintTempo(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

class PotaWidget: public QWidget
{
    Q_OBJECT

public:
    PotaWidget(QWidget *parent=0);

    void Init(QString TableName);
    PotaTableModel *model;
    QTableView *tv;
    PotaItemDelegate *delegate;
    //PotaQuery *query;//for specials coded querys.
    QTabWidget *twParent;
    bool isCommittingError=false;
    QLabel *lTabTitle;
    QTextEdit *editNotes;
    QAction *aEditNotes;
    QTextEdit *editSelInfo;

    QWidget *toolbar;
    QToolButton *pbRefresh;
    QToolButton *pbEdit;
    QToolButton *pbCommit;
    QToolButton *pbRollback;
    QSpinBox *sbInsertRows;
    QToolButton *pbInsertRow;
    QToolButton *pbDuplicRow;
    QToolButton *pbDeleteRow;
    bool bAllowInsert=false;
    bool bAllowDelete=false;

    QLabel *lRowSummary;
    QLabel *lSelect;
    QHBoxLayout *filterLayout;
    QHBoxLayout *findLayout;
    QHBoxLayout *pageFilterLayout;
    QHBoxLayout *ffLayout;
    QHBoxLayout *ltb;
    QHBoxLayout *ltbCV;
    QVBoxLayout *lw;

    QFrame * ffFrame;

    QFrame * filterFrame;
    QLabel *lFilterOn;
    QComboBox * cbFilterType;
    QLineEdit *leFilter;
    QPushButton *pbFilter;
    QLabel *lFilterResult;

    QFrame * findFrame;
    QLabel *lFind;
    QLineEdit *leFind;
    QPushButton *pbFindFirst;
    QPushButton *pbFindNext;
    QPushButton *pbFindPrev;

    QFrame * pageFilterFrame;
    QLabel *lPageFilter;
    QComboBox *cbPageFilter;
    QStringList pageFilterFilters;

    QLabel *lErr;
    QAction *mEditNotes;
    QComboBox *cbFontSize;

    QWidget *toolbarCV;
    QToolButton *pbRefreshCV;
    QPushButton *pbCloseCV;
    QToolButton *helpCV;

    PotaGraph *graph=nullptr;
    PotaChartView *chartView=nullptr;
    QStringList sGraph;

    //bool *userDataEditing;
    bool bUserCurrChanged=true;
    int iTypeText=0;
    int iTypeTextNbCar=5;
    int iTypeDate=0;
    int iTypeReal=0;

    void SetVisibleEditNotes(bool bVisible, bool autoSize);
    void PositionSave();
    void PositionRestore();
    void RefreshHorizontalHeader();
    void SetSizes();

private:
    //Filtering
    bool bSetType=false;
    QString sFieldNameFilter,sDataTypeFilter,sDataFilter;
    void SetLeFilterWith(QString sFieldName, QString sDataType, QString sData);
    void SetFilterTypeCombo(QString sDataType);
    int iPositionCol=-1;
    QString sPositionRow="";
    QString sPositionRowPrev="";
    QString sPositionRowNext="";
    QString sPositionRow3="";
    QString sPositionRow4="";
    void FindFrom(int row, int column, bool Backward);

private slots:
    void dataChanged(const QModelIndex &topLeft);//,const QModelIndex &bottomRight,const QList<int> &roles
    void headerRowClicked();//int logicalIndex
    void pbCommitClick();
    void pbRollbackClick();
    void pbInsertRowClick();
    void pbDuplicRowClick();
    void pbDeleteRowClick();
    void cbFilterTypeChanged(int i);
    void leFilterReturnPressed();
    void leFindReturnPressed();
    void leFindTextEdited(const QString &text);
    void cbPageFilterChanged();
    void showContextMenu(const QPoint& pos);
    void hDefColWidth();
    void hEditNotes(const QModelIndex index);

public slots:
    void curChanged(const QModelIndex cur, const QModelIndex pre);
    void selChanged();
    void showSelInfo();
    void pbRefreshClick();
    void pbEditClick();
    void pbFilterClick(bool checked);
    void pbFindFirstClick();
    void pbFindNextClick();
    void pbFindPrevClick();
    void showGraphDialog();

};

#endif // POTAWIDGET_H
