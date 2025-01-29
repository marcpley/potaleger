#include "potawidget.h"
#include "data/Data.h"
#include "qapplication.h"
#include "qheaderview.h"
#include "qlineedit.h"
#include "qsqlerror.h"
#include <QSqlQuery>
#include "PotaUtils.h"
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QList>
#include <Qt>
#include <QLabel>
#include "PotaUtils.h"


PotaWidget::PotaWidget(QWidget *parent) : QWidget(parent)
{
    twParent = dynamic_cast<QTabWidget*>(parent);
    model = new PotaTableModel();
    model->setParent(this);
    delegate = new PotaItemDelegate2();
    delegate->setParent(this);
    query = new PotaQuery();
    lTabTitle = new QLabel();

    //Toolbar
    toolbar = new QWidget(this);

    pbRefresh = new QToolButton(this);
    pbRefresh->setIcon(QIcon(":/images/reload.svg"));
    SetButtonSize(pbRefresh);
    pbRefresh->setShortcut( QKeySequence(Qt::Key_F5));
    pbRefresh->setToolTip(tr("Recharger les données depuis le fichier.")+"\n"+
                          tr("Les modifications en cours seront automatiquement enregistrées")+"\n"+
                          "F5");
    connect(pbRefresh, &QToolButton::released, this, &PotaWidget::pbRefreshClick);

    pbCommit = new QToolButton(this);
    pbCommit->setIcon(QIcon(":/images/commit.svg"));
    SetButtonSize(pbCommit);
    // Associer plusieurs raccourcis
    QAction *action = new QAction(pbCommit);
    action->setShortcuts({QKeySequence(Qt::CTRL | Qt::Key_Enter), QKeySequence(Qt::CTRL | Qt::Key_Return)});
    QObject::connect(action, &QAction::triggered, pbCommit, &QToolButton::click);
    pbCommit->addAction(action);
    pbCommit->setToolTip(tr("Enregistrer les modifications en cours dans le fichier.")+"\n"+
                         "Ctrl + Enter");
    pbCommit->setEnabled(false);
    connect(pbCommit, &QToolButton::released, this, &PotaWidget::pbCommitClick);

    pbRollback = new QToolButton(this);
    pbRollback->setIcon(QIcon(":/images/rollback.svg"));
    SetButtonSize(pbRollback);
    pbRollback->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Escape));
    pbRollback->setToolTip(tr("Abandonner les modifications en cours.")+"\n"+
                         "Ctrl + Escape");
    pbRollback->setEnabled(false);
    connect(pbRollback, &QToolButton::released, this, &PotaWidget::pbRollbackClick);

    //Add delete rows
    sbInsertRows = new QSpinBox(this);
    sbInsertRows->setMinimum(1);
    sbInsertRows->setEnabled(false);

    pbInsertRow = new QToolButton(this);
    pbInsertRow->setIcon(QIcon(":/images/insert_row.svg"));
    SetButtonSize(pbInsertRow);
    pbInsertRow->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Insert));
    pbInsertRow->setToolTip(tr("Ajouter des lignes.")+"\n"+
                            "Ctrl + Insert");
    connect(pbInsertRow, &QToolButton::released, this, &PotaWidget::pbInsertRowClick);
    pbInsertRow->setEnabled(false);

    pbDeleteRow = new QToolButton(this);
    pbDeleteRow->setIcon(QIcon(":/images/delete_row.svg"));
    SetButtonSize(pbDeleteRow);
    pbDeleteRow->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Delete));
    pbDeleteRow->setToolTip(tr("Supprimer des lignes.")+"\n"+
                            "Ctrl + Delete (Suppr)");
    connect(pbDeleteRow, &QToolButton::released, this, &PotaWidget::pbDeleteRowClick);
    pbDeleteRow->setEnabled(false);

    //Filtering
    fFilter = new QFrame(this);
    fFilter->setFrameShape(QFrame::NoFrame);
    fFilter->setBackgroundRole(QPalette::Midlight);
    fFilter->setAutoFillBackground(true);
    cbFilter = new QCheckBox(this);
    cbFilter->setLayoutDirection(Qt::RightToLeft);
    cbFilter->setFixedWidth(120);
    cbFilter->setEnabled(false);
    cbFilter->setText(tr("Filtrer"));
    connect(cbFilter, &QCheckBox::checkStateChanged, this, &PotaWidget::cbFilterClick);
    leFilter = new QLineEdit(this);
    leFilter->setFixedWidth(100);
    connect(leFilter, &QLineEdit::returnPressed, this, &PotaWidget::leFilterReturnPressed);
    sbFilter = new QSpinBox(this);
    sbFilter->setMinimum(-1);
    sbFilter->setValue(5);
    connect(sbFilter, &QSpinBox::valueChanged, this, &PotaWidget::sbFilterClick);
    lFilter = new QLabel(this);
    lFilter->setFixedWidth(120);

    lRowSummary = new QLabel(this);

    tv = new PotaTableView();
    tv->setParent(this);
    tv->setModel(model);
    tv->setItemDelegate(delegate);
    auto *header = new PotaHeaderView( Qt::Horizontal, tv);
    tv->setHorizontalHeader(header);

    connect(tv->selectionModel(), &QItemSelectionModel::currentChanged, this, &PotaWidget::curChanged);
    connect(model, &PotaTableModel::dataChanged, this, &PotaWidget::dataChanged);
    connect(tv->verticalHeader(), &QHeaderView::sectionClicked,this, &PotaWidget::headerRowClicked);

    editNotes = new QTextEdit();
    editNotes->setParent(tv);
    editNotes->setVisible(false);
    editNotes->setReadOnly(true);
    //editNotes->setGeometry(400,300,400,300);
    // editNotes->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    // editNotes->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    // editNotes->setAutoFillBackground(true);
    //connect(editNotes, &::PotaWidget::mouseDoubleClickEvent, this, &PotaWidget::editNotesDoubleClicked);

    //Filter layout
    lf = new QHBoxLayout(this);
    lf->setSizeConstraint(QLayout::SetFixedSize);
    lf->setContentsMargins(0,0,0,0);
    lf->setSpacing(2);
    lf->addWidget(cbFilter);
    lf->addWidget(leFilter);
    lf->addWidget(sbFilter);
    lf->addSpacing(5);
    lf->addWidget(lFilter);
    fFilter->setLayout(lf);

    //Toolbar layout
    ltb = new QHBoxLayout(this);
    ltb->setSizeConstraint(QLayout::SetFixedSize);
    ltb->setContentsMargins(2,2,2,2);
    ltb->setSpacing(2);
    ltb->addWidget(pbRefresh);
    ltb->addSpacing(10);
    ltb->addWidget(pbCommit);
    ltb->addWidget(pbRollback);
    ltb->addSpacing(10);
    ltb->addWidget(sbInsertRows);
    ltb->addWidget(pbInsertRow);
    ltb->addWidget(pbDeleteRow);
    ltb->addSpacing(10);
    ltb->addWidget(fFilter);
    ltb->addSpacing(10);
    ltb->addWidget(lRowSummary);
    toolbar->setLayout(ltb);

    //Main layout
    lw = new QVBoxLayout(this);
    lw->setContentsMargins(2,2,2,2);
    lw->setSpacing(2);
    lw->addWidget(toolbar);
    lw->addWidget(tv);
    setLayout(lw);
}

void PotaWidget::Init(QString TableName)
{
    model->setTable(TableName);

    qInfo() << "Open " << TableName; //+" ("+RealTableName+")";

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);//OnFieldChange

    //FK
    QString RealTableName=model->tableName();
    if (RealTableName.contains("__"))
        RealTableName=RealTableName.first(RealTableName.indexOf("__"));

    query->clear();
    query->ExecShowErr("PRAGMA foreign_key_list("+RealTableName+");");
    while (query->next()) {
        QString referencedTable = query->value("table").toString();
        QString localColumn = query->value("from").toString();
        QString referencedClumn = query->value("to").toString();
        int localColumnIndex = model->fieldIndex(localColumn);

        model->setRelation(localColumnIndex, QSqlRelation(referencedTable, referencedClumn, referencedClumn));//Issue #2
    }

    //Generated columns
    query->clear();
    query->ExecShowErr("PRAGMA table_xinfo("+TableName+")");
    while (query->next()){
        if (query->value(6).toInt()==2)
            model->generatedColumns.insert(query->value(1).toString());
    }

    tv->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    tv->verticalHeader()->setDefaultSectionSize(0);//Mini.
    tv->verticalHeader()->setDefaultAlignment(Qt::AlignTop);
    tv->verticalHeader()->hide();
    tv->setTabKeyNavigation(false);
    dynamic_cast<PotaHeaderView*>(tv->horizontalHeader())->iSortCol=NaturalSortCol(TableName);
    dynamic_cast<PotaHeaderView*>(tv->horizontalHeader())->model()->setHeaderData(NaturalSortCol(TableName), Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_BlueDown.svg")), Qt::DecorationRole);

}

void PotaWidget::curChanged(const QModelIndex cur, const QModelIndex pre)
{
    tv->clearSpans();//Force redraw of grid, for selected ligne visibility.

    lRowSummary->setText(RowSummary(model->tableName(),cur));

    QString FieldName=model->headerData(cur.column(),Qt::Horizontal,Qt::DisplayRole).toString();
    if (!cbFilter->isChecked()) {
        //Filtering
        sDataNameFilter=FieldName;
        sDataFilter=model->index(cur.row(),cur.column()).data(Qt::DisplayRole).toString();

        cbFilter->setText(sDataNameFilter);

        cbFilter->setEnabled(true);//todo: when false ?

        SetFilterParamsFrom(sDataNameFilter,sDataFilter);
    }

    editNotes->setReadOnly(true);
    mEditNotes->setChecked(!editNotes->isReadOnly());
    SetVisibleEditNotes((FieldName=="Notes" or FieldName.startsWith("N_"))and
                         model->data(cur,Qt::DisplayRole).toString().contains("\n"));
}

void PotaWidget::SetVisibleEditNotes(bool bVisible){
    if (bVisible){
        int x = tv->columnViewportPosition(tv->currentIndex().column())+5;
        int y = tv->rowViewportPosition(tv->currentIndex().row())+
                tv->horizontalHeader()->height()+
                tv->rowHeight(tv->currentIndex().row());
        int EditNotesWidth = 400;
        int EditNotesHeight = 200;
        x=min(x,tv->width()-EditNotesWidth-20);
        editNotes->setGeometry(x,y,EditNotesWidth,EditNotesHeight);

        if (editNotes->isReadOnly()) {
            editNotes->setMarkdown(model->data(tv->currentIndex(),Qt::DisplayRole).toString());
        } else {
            editNotes->setPlainText(model->data(tv->currentIndex(),Qt::DisplayRole).toString());

        }

        editNotes->setVisible(true);
    } else {
        editNotes->setVisible(false);
    }
}

void PotaWidget::PositionSave() {
    iPositionCol=tv->selectionModel()->currentIndex().column();
    //Normal row to retreive.
    sPositionRow=model->index(tv->selectionModel()->currentIndex().row(),0).data(Qt::DisplayRole).toString()+
                 model->index(tv->selectionModel()->currentIndex().row(),1).data(Qt::DisplayRole).toString()+
                 model->index(tv->selectionModel()->currentIndex().row(),2).data(Qt::DisplayRole).toString();
    //Alternative row to retreive if normal row desapear.
    sPositionRow2=model->index(tv->selectionModel()->currentIndex().row()+1,0).data(Qt::DisplayRole).toString()+
                  model->index(tv->selectionModel()->currentIndex().row()+1,1).data(Qt::DisplayRole).toString()+
                  model->index(tv->selectionModel()->currentIndex().row()+1,2).data(Qt::DisplayRole).toString();
}

void PotaWidget::PositionRestore() {
    for (int i=0;i<model->rowCount();i++) {
        if (sPositionRow==model->index(i,0).data(Qt::DisplayRole).toString()+
                          model->index(i,1).data(Qt::DisplayRole).toString()+
                          model->index(i,2).data(Qt::DisplayRole).toString()) {
            //Normal row retreived.
            tv->selectionModel()->setCurrentIndex(model->index(i,iPositionCol),QItemSelectionModel::Current);
            break;
        }
        if (sPositionRow2==model->index(i,0).data(Qt::DisplayRole).toString()+
                           model->index(i,1).data(Qt::DisplayRole).toString()+
                           model->index(i,2).data(Qt::DisplayRole).toString()) {
            //Normal row probably not in the the new row set.
            tv->selectionModel()->setCurrentIndex(model->index(i,iPositionCol),QItemSelectionModel::Current);
            break;
        }
    }
    tv->setFocus();
}

void PotaWidget::SetFilterParamsFrom(QString sDataName, QString sData){
    //Filtering
    QString sDataType = DataType(model->tableName(),sDataName);
    leFilter->setToolTip("");
    if (sDataType=="TEXT" or sDataType.startsWith("BOOL")) {
        if (!cbFilter->isChecked())
            sbFilter->setValue(iNCharText);
        sbFilter->setMaximum(100);
        sbFilter->setMinimum(-1);
        sbFilter->setSingleStep(1);
        sbFilter->setEnabled(true);
        leFilter->setText(StrFirst(sData,sbFilter->value()));
        leFilter->setToolTip(tr("'%1' commence par '%2'.")
                                 .arg(sDataName)
                                 .arg(leFilter->text()));
    } else if (sDataType=="DATE") {
        if (!cbFilter->isChecked())
            sbFilter->setValue(iNCharDate);
        sbFilter->setMaximum(10);
        sbFilter->setMinimum(4);
        sbFilter->setSingleStep(3);
        sbFilter->setEnabled(true);
        leFilter->setText(StrFirst(sData,sbFilter->value()));
        if (sbFilter->value()==4) {
            leFilter->setToolTip(tr("'%1' est de l'année '%2'.")
                                     .arg(sDataName)
                                     .arg(leFilter->text()));
        } else if (sbFilter->value()==7) {
            leFilter->setToolTip(tr("'%1' est du mois '%2'.")
                                     .arg(sDataName)
                                     .arg(leFilter->text()));
        } else {
            leFilter->setToolTip(tr("'%1' est '%2'.")
                                     .arg(sDataName)
                                     .arg(leFilter->text()));
        }
    } else if (sDataType=="REAL" or sDataType.startsWith("INT")) {
        if (!cbFilter->isChecked())
            sbFilter->setValue(iNCharReal);
        sbFilter->setMaximum(1000);
        sbFilter->setMinimum(0);
        sbFilter->setSingleStep(1);
        sbFilter->setEnabled(true);
        leFilter->setText(sData);
        leFilter->setToolTip(tr("'%1' égal à %2 plus ou moins %3.")
                                 .arg(sDataName)
                                 .arg(leFilter->text())
                                 .arg(sbFilter->value()));
    } else {
        sbFilter->setEnabled(false);
    }
    if (leFilter->text()=="")
        leFilter->setToolTip(tr("'%1' est vide.")
                                 .arg(sDataName));
    if (!cbFilter->isChecked())
        leFilter->setToolTip(leFilter->toolTip()+"\n"+
                             tr("Cocher la case pour activer la filtration."));
    leFilter->setToolTip(tr("Filtrer les lignes:")+"\n"+
                         leFilter->toolTip());
}

void PotaWidget::dataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight,const QList<int> &roles)
{
    pbCommit->setEnabled(true);
    pbRollback->setEnabled(true);
    cbFilter->setEnabled(false);
    twParent->setTabIcon(twParent->currentIndex(),QIcon(":/images/toCommit.svg"));
    //lTabTitle->setStyleSheet(lTabTitle->styleSheet().append("color: red;"));

    QVariant variant;
    if (!topLeft.data(Qt::EditRole).isNull() and
        (topLeft.data(Qt::EditRole) == ""))
        model->setData(topLeft,variant,Qt::EditRole);//To avoid empty non null values.
}

void PotaWidget::headerRowClicked() //int logicalIndex
{
    //pbDeleteRow->setEnabled(true);
}

void PotaWidget::pbRefreshClick()
{
    if (pbCommit->isEnabled())
        model->SubmitAllShowErr();
    model->copiedCells.clear();
    model->commitedCells.clear();

    PositionSave();
    model->SelectShowErr();
    PositionRestore();
}

void PotaWidget::pbCommitClick()
{
    PositionSave();

    if (model->SubmitAllShowErr())
        PositionRestore();
}

void PotaWidget::pbRollbackClick()
{
    PositionSave();

    if (model->RevertAllShowErr()){
        model->copiedCells.clear();
        PositionRestore();
    }
}

void PotaWidget::pbInsertRowClick()
{
    model->InsertRowShowErr();
    tv->setFocus();
}

void PotaWidget::pbDeleteRowClick()
{
    model->DeleteRowShowErr();
    tv->setFocus();
}

void PotaWidget::cbFilterClick(Qt::CheckState state)
{
    //Filtering
    model->commitedCells.clear();
    QString filter="";
    if (state==Qt::CheckState::Checked) {
        //Filter
        //QModelIndex index = tv->selectionModel()->currentIndex();

        QString sDataType = DataType(model->tableName(),sDataNameFilter);
        if (sDataType=="TEXT" or sDataType=="DATE" or sDataType.startsWith("BOOL")) {
            if (leFilter->text()=="")
                filter=cbFilter->text()+" ISNULL";
            else
                filter=cbFilter->text()+" LIKE '"+StrReplace(leFilter->text(),"'","\'")+"%'";
        } else if (sDataType=="REAL" or sDataType.startsWith("INT")) {
            if (leFilter->text()=="")
                filter=cbFilter->text()+" ISNULL";
            else
                filter=cbFilter->text()+" BETWEEN "+str(leFilter->text().toFloat()-sbFilter->value())+" AND "+
                                                    str(leFilter->text().toFloat()+sbFilter->value());
        }
        delegate->FilterCol=model->fieldIndex(sDataNameFilter);
        fFilter->setFrameShape(QFrame::Box);

    } else {
        //Reset filter
        delegate->FilterCol=-1;
        fFilter->setFrameShape(QFrame::NoFrame);
    }

    PositionSave();

    model->setFilter(filter);

    PositionRestore();

    lFilter->setText(str(model->rowCount())+" "+tr("lignes"));
}

void PotaWidget::sbFilterClick(int i)
{
    //Filtering
    QModelIndex index = tv->selectionModel()->currentIndex();
    QString sDataType = DataType(model->tableName(),model->headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString());
    if (sDataType=="TEXT") {
        iNCharText=i;
        //leFilter->setText(StrFirst(model->index(index.row(),index.column()).data(Qt::DisplayRole).toString(),sbFilter->value()));
    } else if (sDataType=="DATE") {
        iNCharDate=i;
        //leFilter->setText(StrFirst(model->index(index.row(),index.column()).data(Qt::DisplayRole).toString(),sbFilter->value()));
    } else if (sDataType=="REAL" or sDataType.startsWith("INT")){
        iNCharReal=i;
        //leFilter->setText(StrFirst(model->index(index.row(),index.column()).data(Qt::DisplayRole).toString(),sbFilter->value()));
    }

    if (cbFilter->isChecked()) {
        SetFilterParamsFrom(sDataNameFilter,sDataFilter);
        cbFilterClick(Qt::CheckState::Checked);
    } else {
        SetFilterParamsFrom(model->headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString(),
                            model->index(index.row(),index.column()).data(Qt::DisplayRole).toString());
    }
}

void PotaWidget::leFilterReturnPressed()
{
    if (cbFilter->isEnabled())
    {
        if(cbFilter->isChecked())
            cbFilterClick(Qt::CheckState::Checked);
        else
            cbFilter->setChecked(true);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//                           PotaTableModel
//////////////////////////////////////////////////////////////////////////////////////////////////

QString PotaTableModel::FieldName(int index)
{
    PotaQuery *q = dynamic_cast<PotaWidget*>(parent())->query;
    q->ExecShowErr("PRAGMA table_xinfo("+tableName()+");");

    if (q->seek(index))
        return q->value("name").toString();
    else
        return "";
}

bool PotaTableModel::SelectShowErr()
{
    int i;
    for (i=0;i<columnCount();i++) {
        if (relationModel(i))
            relationModel(i)->select();
    }

    i = rowCount();

    setLastError(QSqlError());

    select();

    if (i != rowCount()) {
        //Display modified, commited or copied cells could be unconsistent.
        modifiedCells.clear();
        commitedCells.clear();
        copiedCells.clear();
    }

    if ((lastError().type() == QSqlError::NoError)and(parent()->objectName().startsWith("PW")))
    {
        SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,tableName()+" - "+str(rowCount()),"Ok");
        return true;
    }
    {
        SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,lastError().text(),"Err");
        return false;
    }
}

bool PotaTableModel::SubmitAllShowErr()
{
    PotaWidget *pw = dynamic_cast<PotaWidget*>(parent());
    int i = rowCount();
    setLastError(QSqlError());
    submitAll();
    if (lastError().type() == QSqlError::NoError)
    {
        SetColoredText(pw->lErr,tableName()+": "+tr("modifications enregistrées."),"Ok");
        pw->isCommittingError=false;
        pw->pbCommit->setEnabled(false);
        pw->pbRollback->setEnabled(false);
        pw->cbFilter->setEnabled(true);
        pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(""));
        //pw->lTabTitle->setStyleSheet(pw->lTabTitle->styleSheet().replace("color: red;", ""));
        if (i != rowCount()) {
            //Display modified, commited or copied cells could be unconsistent.
            modifiedCells.clear();
            commitedCells.clear();
            copiedCells.clear();
        }
    }
    else
    {
        SetColoredText(pw->lErr,lastError().text(),"Err");
        pw->isCommittingError=true;
        return false;
    }
    return true;
}

bool PotaTableModel::RevertAllShowErr()
{
    PotaWidget *pw = dynamic_cast<PotaWidget*>(parent());
    setLastError(QSqlError());
    revertAll();
    if (lastError().type() == QSqlError::NoError)
    {
        SetColoredText(pw->lErr,tableName()+": "+tr("modifications abandonnées."),"Info");
        pw->isCommittingError=false;
        pw->pbCommit->setEnabled(false);
        pw->pbRollback->setEnabled(false);
        pw->cbFilter->setEnabled(true);
        pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(""));
        //pw->lTabTitle->setStyleSheet(pw->lTabTitle->styleSheet().replace("color: red;", ""));

    }
    else
    {
        SetColoredText(pw->lErr,lastError().text(),"Err");
        pw->isCommittingError=true;
        return false;
    }
    return true;
}

bool PotaTableModel::InsertRowShowErr()
{
    PotaWidget *pw = dynamic_cast<PotaWidget*>(parent());
    if (insertRows(pw->tv->currentIndex().row()+1,
                   pw->sbInsertRows->value()))
    {
        pw->pbCommit->setEnabled(true);
        pw->pbRollback->setEnabled(true);
        pw->cbFilter->setEnabled(false);
        pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(":/images/toCommit.svg"));
        //pw->lTabTitle->setStyleSheet(pw->lTabTitle->styleSheet().append("color: red;"));
    }
    else
    {
        SetColoredText(pw->lErr,"insertRows(x,y)","Err");
        return false;
    }
    return true;
}

bool PotaTableModel::DeleteRowShowErr()
{
    PotaWidget *pw = dynamic_cast<PotaWidget*>(parent());
    QModelIndexList selectedIndexes = pw->tv->selectionModel()->selectedIndexes();
    int rr=0;
    for (const QModelIndex &index : selectedIndexes) {
        if (removeRow(index.row()))
            rr++;
        else
            SetColoredText(pw->lErr,"removeRow("+str(index.row())+")","Err");
    }
    if (rr>0) {
        pw->pbCommit->setEnabled(true);
        pw->pbRollback->setEnabled(true);
        pw->cbFilter->setEnabled(false);
        pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(":/images/toCommit.svg"));
        //pw->lTabTitle->setStyleSheet(pw->lTabTitle->styleSheet().append("color: red;"));
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                                  PotaTableView
//////////////////////////////////////////////////////////////////////////////////////////////////

void PotaTableView::keyPressEvent(QKeyEvent *event) {
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

//////////////////////////////////////////////////////////////////////////////////////////////////
//                                  PotaHeaderView
//////////////////////////////////////////////////////////////////////////////////////////////////

void PotaHeaderView::mouseDoubleClickEvent(QMouseEvent *event)  {
    int logicalIndex = logicalIndexAt(event->pos());
    if (logicalIndex != -1) {

        if (iSortCol==logicalIndex)//Already sorted on this column.
        {
            if (!bSortDes)
            {
                model()->sort(logicalIndex,Qt::SortOrder::DescendingOrder);
                model()->setHeaderData(logicalIndex, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_RedUp.svg")), Qt::DecorationRole);
                bSortDes=true;
            }
            else//Reset sorting.
            {
                model()->setHeaderData(iSortCol, Qt::Horizontal, 0, Qt::DecorationRole);
                model()->sort( -1,Qt::SortOrder::AscendingOrder);
                int iCol=NaturalSortCol(dynamic_cast<PotaTableModel*>(model())->tableName());
                model()->setHeaderData(iCol, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_BlueDown.svg")), Qt::DecorationRole);
                iSortCol=iCol;
                bSortDes=false;
            }
        }
        else//Actual sort on another column.
        {
            model()->setHeaderData(iSortCol, Qt::Horizontal, 0, Qt::DecorationRole);
            model()->sort(logicalIndex,Qt::SortOrder::AscendingOrder);
            model()->setHeaderData(logicalIndex, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_GreenDown.svg")), Qt::DecorationRole);
            iSortCol=logicalIndex;
            bSortDes=false;
        }

        emit sectionClicked(logicalIndex);
    }

    // Appeler la méthode parente pour le comportement standard
    QHeaderView::mouseDoubleClickEvent(event);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                                  PotaItemDelegate
//////////////////////////////////////////////////////////////////////////////////////////////////


void PotaItemDelegate2::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    PotaWidget* pw = dynamic_cast<PotaWidget*>(parent());
    //if (isDarkTheme()) todo

    QBrush b;
    b.setStyle(Qt::SolidPattern);
    QColor c,cFiltered,cCopied,cModified,cModifiedError;
    QItemSelectionModel *selection = pw->tv->selectionModel();

    if (pw->model && pw->model->rowsToRemove.contains(index.row())) {
        //Not painting the rows to remove.
        QStyleOptionViewItem opt = option;
        opt.backgroundBrush = QBrush(QColor(220, 220, 220));
        return;
    // } else if (index.row() == selection->currentIndex().row()) {//Line selected
    //     //c=Qt::blue;
    //     c=QApplication::palette().color(QPalette::Highlight);
    //     c.setAlpha(70);
    } else {//Row data color
        if (!index.data(Qt::EditRole).isNull() and
            (index.data(Qt::EditRole) == "")) {
            //Hightlight not null empty value. They are not normal, it causes SQL failure.
            c=Qt::red;
            c.setAlpha(150);
        } else if (RowColorCol>-1) {
            c=RowColor(index.model()->index(index.row(),RowColorCol).data(Qt::DisplayRole).toString());
        }
    }
    if (!c.isValid()) {//Table color.
        c=cColColors[index.column()];
        if (!c.isValid() and index.column()!=TempoCol)
            c=cTableColor;
        c.setAlpha(30);
    }
    if (c.isValid()) {
        b.setColor(c);
        painter->fillRect(option.rect,b);
    }

    if (index.column()==FilterCol) { //Filtering column.
        if (!isDarkTheme())
            cFiltered=QColor("#000000");//palette
        else
            cFiltered=QColor("#ffffff");
        cFiltered.setAlpha(150);
        painter->save();
        painter->setPen(cFiltered);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
        painter->restore();
    }

    if (index.row() == selection->currentIndex().row()) {//Line selected
        c=QApplication::palette().color(QPalette::Highlight);
        c.setAlpha(255);
        b.setColor(c);
        QRect r;
        r.setTop(option.rect.bottom()-1);
        r.setBottom(option.rect.bottom());
        r.setLeft(option.rect.left());
        r.setRight(option.rect.right());
        painter->fillRect(r,b);
    }

    if (index.column()==TempoCol)
         paintTempo(painter,option,index);
    else
        QStyledItemDelegate::paint(painter, option, index);

    if (pw->model && pw->model->copiedCells.contains(index)) {
        cCopied=QColor("#00ab00");//green
        painter->save();
        painter->setPen(cCopied);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
        painter->restore();
    }
    if (pw->model && pw->model->modifiedCells.contains(index)) {
        cModified=QColor("#0086ff");//blue
        cModifiedError=QColor("#ff0000");//red
        painter->save();
        painter->setPen(pw->isCommittingError ? cModifiedError : cModified);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
        painter->restore();
    }
    if (pw->model && pw->model->commitedCells.contains(index)) {
        cModified=QColor("#0086ff");//blue
        cModified.setAlpha(150);
        painter->save();
        painter->setPen(cModified);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
        painter->restore();
    }
}

void PotaItemDelegate2::paintTempo(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    double const coef=1;
    QBrush b;
    b.setStyle(Qt::SolidPattern);
    QColor c;
    QRect r;

    //Mois
    c=QColor(128,128,128);
    b.setColor(c);
    r.setBottom(option.rect.bottom());
    r.setTop(option.rect.top());
    r.setLeft(option.rect.left()+30*coef);//January 31 day - half of month bar width
    r.setWidth(2);painter->fillRect(r,b);
    r.setLeft(r.left()+29*coef); r.setWidth(2); painter->fillRect(r,b);
    r.setLeft(r.left()+31*coef); r.setWidth(2); painter->fillRect(r,b);
    r.setLeft(r.left()+30*coef); r.setWidth(2); painter->fillRect(r,b);
    r.setLeft(r.left()+31*coef); r.setWidth(2); painter->fillRect(r,b);
    r.setLeft(r.left()+30*coef); r.setWidth(2); painter->fillRect(r,b);
    r.setLeft(r.left()+31*coef); r.setWidth(2); painter->fillRect(r,b);
    r.setLeft(r.left()+31*coef); r.setWidth(2); painter->fillRect(r,b);
    r.setLeft(r.left()+30*coef); r.setWidth(2); painter->fillRect(r,b);
    r.setLeft(r.left()+31*coef); r.setWidth(2); painter->fillRect(r,b);
    r.setLeft(r.left()+30*coef); r.setWidth(2); painter->fillRect(r,b);
    r.setLeft(r.left()+31*coef); r.setWidth(2); painter->fillRect(r,b);

    QStringList ql = index.data(Qt::DisplayRole).toString().split(":");
    if (ql.count()!=6)
        return;
    int const attente=ql[0].toInt()*coef;
    int const semis=ql[1].toInt()*coef;
    int const semisF=ql[2].toInt()*coef;
    int const plant=ql[3].toInt()*coef;
    int const plantF=ql[4].toInt()*coef;
    int const recolte=ql[5].toInt()*coef;

    r.setBottom(option.rect.bottom()-2);
    if (semis>0){
        //Période de semis
        c=cSousAbris;
        c.setAlpha(255);
        b.setColor(c);
        r.setTop(r.bottom()-4);
        r.setLeft(option.rect.left()+attente);
        r.setRight(option.rect.left()+attente+semis);
        painter->fillRect(r,b);
    }
    if (semisF>0){
        //Semis fait, attente plantation
        c=cSousAbris;
        c.setAlpha(100);
        b.setColor(c);
        r.setTop(r.bottom()-4);
        r.setLeft(option.rect.left()+attente+semis);
        r.setRight(option.rect.left()+attente+semis+semisF);
        painter->fillRect(r,b);
    }
    if (plant>0){
        //Période de plantation
        c=cEnPlace;
        c.setAlpha(255);
        b.setColor(c);
        r.setTop(r.bottom()-10);
        r.setLeft(option.rect.left()+attente+semis+semisF);
        r.setRight(option.rect.left()+attente+semis+semisF+plant);
        painter->fillRect(r,b);
    }
    if (plantF>0){
        //Plantation faite, attente récolte
        c=cEnPlace;
        c.setAlpha(150);
        b.setColor(c);
        r.setTop(r.bottom()-10);
        r.setLeft(option.rect.left()+attente+semis+semisF+plant);
        r.setRight(option.rect.left()+attente+semis+semisF+plant+plantF);
        painter->fillRect(r,b);
    }
    if (recolte>0){
        //Période de récolte
        c=cATerminer;
        c.setAlpha(255);
        b.setColor(c);
        r.setTop(r.bottom()-12);
        r.setBottom(r.bottom()-6);
        r.setLeft(option.rect.left()+attente+semis+semisF+plant+plantF);
        r.setRight(option.rect.left()+attente+semis+semisF+plant+plantF+recolte);
        painter->fillRect(r,b);
    }
}

void PotaItemDelegate2::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const  {
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    if (comboBox) {
        QVariant selectedValue = comboBox->currentData();
        if (!selectedValue.isValid() || selectedValue.toString().isEmpty()) {
            model->setData(index, QVariant(), Qt::EditRole); // Définit à NULL
        } else {
            model->setData(index, selectedValue, Qt::EditRole);
        }
        dynamic_cast<PotaWidget*>(model->parent())->tv->setFocus();
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
