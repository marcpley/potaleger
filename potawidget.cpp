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
//#include "qtimer.h"


PotaWidget::PotaWidget(QWidget *parent) : QWidget(parent)
{
    twParent = dynamic_cast<QTabWidget*>(parent);
    model = new PotaTableModel();
    model->setParent(this);
    delegate = new PotaItemDelegate();
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

    pbEdit = new QToolButton(this);
    pbEdit->setIcon(QIcon(":/images/edit.svg"));
    SetButtonSize(pbEdit);
    pbEdit->setCheckable(true);
    pbEdit->setChecked(false);
    pbEdit->setShortcut( QKeySequence(Qt::Key_F2));
    pbEdit->setToolTip(tr("Basculer le mode édition.")+"\n"+
                          "F2");
    connect(pbEdit, &QToolButton::released, this, &PotaWidget::pbEditClick);

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
    pbCommit->setVisible(false);
    connect(pbCommit, &QToolButton::released, this, &PotaWidget::pbCommitClick);

    pbRollback = new QToolButton(this);
    pbRollback->setIcon(QIcon(":/images/rollback.svg"));
    SetButtonSize(pbRollback);
    pbRollback->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Escape));
    pbRollback->setToolTip(tr("Abandonner les modifications en cours.")+"\n"+
                         "Ctrl + Escape");
    pbRollback->setEnabled(false);
    pbRollback->setVisible(false);
    connect(pbRollback, &QToolButton::released, this, &PotaWidget::pbRollbackClick);

    //Add delete rows
    sbInsertRows = new QSpinBox(this);
    sbInsertRows->setFixedHeight(26);
    sbInsertRows->setMinimum(1);
    sbInsertRows->setEnabled(false);
    sbInsertRows->setVisible(false);

    pbInsertRow = new QToolButton(this);
    pbInsertRow->setIcon(QIcon(":/images/insert_row.svg"));
    SetButtonSize(pbInsertRow);
    pbInsertRow->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Insert));
    pbInsertRow->setToolTip(tr("Ajouter des lignes.")+"\n"+
                            "Ctrl + Insert");
    connect(pbInsertRow, &QToolButton::released, this, &PotaWidget::pbInsertRowClick);
    pbInsertRow->setEnabled(false);
    pbInsertRow->setVisible(false);

    pbDeleteRow = new QToolButton(this);
    pbDeleteRow->setIcon(QIcon(":/images/delete_row.svg"));
    SetButtonSize(pbDeleteRow);
    pbDeleteRow->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Delete));
    pbDeleteRow->setToolTip(tr("Supprimer des lignes.")+"\n"+
                            "Ctrl + Delete (Suppr)");
    connect(pbDeleteRow, &QToolButton::released, this, &PotaWidget::pbDeleteRowClick);
    pbDeleteRow->setEnabled(false);
    pbDeleteRow->setVisible(false);

    //Filter tool
    filterFrame = new QFrame(this);
    filterFrame->setFrameShape(QFrame::NoFrame);
    filterFrame->setBackgroundRole(QPalette::Midlight);
    filterFrame->setAutoFillBackground(true);
    lFilterOn = new QLabel(this);
    lFilterOn->setFixedWidth(80);
    lFilterOn->setText("...");
    cbFilterType = new QComboBox(this);
    cbFilterType->setFixedWidth(125);
    connect(cbFilterType, &QComboBox::currentIndexChanged,this, &PotaWidget::cbFilterTypeChanged);
    leFilter = new QLineEdit(this);
    leFilter->setFixedWidth(80);
    connect(leFilter, &QLineEdit::returnPressed, this, &PotaWidget::leFilterReturnPressed);
    pbFilter = new QPushButton(this);
    pbFilter->setText(tr("Filtrer"));
    pbFilter->setCheckable(true);
    pbFilter->setFixedWidth(70);
    pbFilter->setEnabled(false);
    connect(pbFilter, &QPushButton::clicked,this,&PotaWidget::pbFilterClick);
    lFilterResult = new QLabel(this);
    lFilterResult->setFixedWidth(80);

    //Find tool
    findFrame = new QFrame(this);
    findFrame->setFrameShape(QFrame::NoFrame);
    findFrame->setBackgroundRole(QPalette::Midlight);
    findFrame->setAutoFillBackground(true);
    lFind = new QLabel(this);
    lFind->setText(tr("Rechercher"));
    leFind = new QLineEdit(this);
    leFind->setFixedWidth(80);
    connect(leFind, &QLineEdit::textEdited, this, &PotaWidget::leFindTextEdited);
    connect(leFind, &QLineEdit::returnPressed, this, &PotaWidget::leFindReturnPressed);
    pbFindFirst = new QPushButton(this);
    pbFindFirst->setText(tr("1er"));
    pbFindFirst->setFixedWidth(40);
    pbFindFirst->setEnabled(false);
    connect(pbFindFirst, &QPushButton::clicked,this,&PotaWidget::pbFindFirstClick);
    pbFindNext = new QPushButton(this);
    pbFindNext->setText(tr("Suivant"));
    pbFindNext->setFixedWidth(70);
    pbFindNext->setEnabled(false);
    connect(pbFindNext, &QPushButton::clicked,this,&PotaWidget::pbFindNextClick);
    pbFindPrev = new QPushButton(this);
    pbFindPrev->setText(tr("Précédent"));
    pbFindPrev->setFixedWidth(70);
    pbFindPrev->setEnabled(false);
    connect(pbFindPrev, &QPushButton::clicked,this,&PotaWidget::pbFindPrevClick);

    ffFrame = new QFrame(this);
    ffFrame->setVisible(true);

    lRowSummary = new QLabel(this);
    lRowSummary->setMinimumSize(375,lRowSummary->height());
    lRowSummary->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    lRowSummary->setTextInteractionFlags(Qt::TextSelectableByMouse);
    //lRowSummary->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    lSelect = new QLabel(this);
    lSelect->setText("");
    lSelect->setTextInteractionFlags(Qt::TextSelectableByMouse);

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

    //Toolbar layout
    ltb = new QHBoxLayout(this);
    ltb->setSizeConstraint(QLayout::SetFixedSize);
    ltb->setContentsMargins(2,2,2,2);
    ltb->setSpacing(0);
    ltb->addWidget(pbRefresh);
    ltb->addSpacing(10);
    ltb->addWidget(pbEdit);
    ltb->addSpacing(10);
    ltb->addWidget(pbCommit);
    ltb->addSpacing(5);
    ltb->addWidget(pbRollback);
    ltb->addSpacing(10);
    ltb->addWidget(sbInsertRows);
    ltb->addWidget(pbInsertRow);
    ltb->addSpacing(5);
    ltb->addWidget(pbDeleteRow);
    ltb->addSpacing(10);
    ltb->addWidget(lRowSummary);
    ltb->addSpacing(10);
    ltb->addWidget(lSelect);
    toolbar->setLayout(ltb);

    //Filter layout
    filterLayout = new QHBoxLayout(this);
    filterLayout->setSizeConstraint(QLayout::SetFixedSize);
    filterLayout->setContentsMargins(5,3,5,3);
    filterLayout->setSpacing(5);
    filterLayout->addWidget(lFilterOn);
    filterLayout->addWidget(cbFilterType);
    filterLayout->addWidget(leFilter);
    filterLayout->addWidget(pbFilter);
    filterLayout->addWidget(lFilterResult);
    filterFrame->setLayout(filterLayout);

    //Find layout
    findLayout = new QHBoxLayout(this);
    findLayout->setSizeConstraint(QLayout::SetFixedSize);
    findLayout->setContentsMargins(5,3,5,3);
    findLayout->setSpacing(5);
    findLayout->addWidget(lFind);
    findLayout->addWidget(leFind);
    findLayout->addWidget(pbFindFirst);
    findLayout->addWidget(pbFindNext);
    findLayout->addWidget(pbFindPrev);
    findFrame->setLayout(findLayout);

    //Filter find layout
    ffLayout = new QHBoxLayout(this);
    ffLayout->setSizeConstraint(QLayout::SetFixedSize);
    ffLayout->setContentsMargins(0,0,0,0);
    ffLayout->addWidget(filterFrame);
    ffLayout->addSpacing(10);
    ffLayout->addWidget(findFrame);
    ffFrame->setLayout(ffLayout);

    //Main layout
    lw = new QVBoxLayout(this);
    lw->setContentsMargins(2,2,2,2);
    lw->setSpacing(2);
    lw->addWidget(toolbar);
    lw->addWidget(ffFrame);
    lw->addWidget(tv);
    setLayout(lw);
}

void PotaWidget::Init(QString TableName)
{
    model->setTable(TableName);

    qInfo() << "Open " << TableName; +" ("+model->RealTableName()+")";

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);//OnFieldChange

    //FK
    query->clear();
    query->ExecShowErr("PRAGMA foreign_key_list("+model->RealTableName()+");");
    while (query->next()) {
        QString referencedTable = query->value("table").toString();
        QString localColumn = query->value("from").toString();
        QString referencedClumn = query->value("to").toString();
        int localColumnIndex = model->fieldIndex(localColumn);

        if (localColumnIndex>-1){
            model->setRelation(localColumnIndex, QSqlRelation(referencedTable, referencedClumn, referencedClumn));//Issue #2
            model->relationModel(localColumnIndex)->setFilter(FkFilter(model->RealTableName(),localColumn,model->index(0,0))); //todo crash on Cultures à terminer ?
        }
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

void PotaWidget::curChanged(const QModelIndex cur)//, const QModelIndex pre
{
    if (bUserCurrChanged){
        tv->clearSpans();//Force redraw of grid, for selected ligne visibility.

        if (cur.row()>-1)
            lRowSummary->setText(RowSummary(model->tableName(),model->record(cur.row())));
        else
            lRowSummary->setText("...");

        QModelIndexList selectedIndexes = tv->selectionModel()->selectedIndexes();
        int nbSelected=tv->selectionModel()->selectedIndexes().count();
        int nbOkSelected=0;
        if (nbSelected>1){
            float sumSelected=0;
            float minSelected=0;
            float maxSelected=0;
            bool bOk;
            for (const QModelIndex &index : selectedIndexes) {
                sumSelected+=index.data().toFloat(&bOk);
                if (bOk){
                    if (nbOkSelected==0) {
                        minSelected=index.data().toFloat();
                        maxSelected=index.data().toFloat();
                    } else {
                        minSelected=min(minSelected,index.data().toFloat());
                        maxSelected=fmax(maxSelected,index.data().toFloat());
                    }
                    nbOkSelected++;
                }
            }
            lSelect->setText(tr("Sélection: ")+str(nbSelected)+
                             iif(nbOkSelected>0," - "+
                                 iif(nbOkSelected<nbSelected,tr("num: ")+str(nbOkSelected)+" - ","").toString()+
                                 tr("somme: ")+str(sumSelected)+
                                 iif(nbOkSelected>1," - "+
                                     tr("moy: ")+str(sumSelected/nbOkSelected)+" - "+
                                     tr("min: ")+str(minSelected)+" - "+
                                     tr("max: ")+str(maxSelected),"").toString(),"").toString());
            lSelect->setVisible(true);
        } else {
            lSelect->setVisible(false);
        }

        QString FieldName=model->headerData(cur.column(),Qt::Horizontal,Qt::DisplayRole).toString();

        if (filterFrame->isVisible()) {
            if (!pbFilter->isChecked()) {
                //Filtering
                sDataNameFilter=FieldName;
                sDataFilter=model->index(cur.row(),cur.column()).data(Qt::DisplayRole).toString();

                lFilterOn->setText(sDataNameFilter);

                pbFilter->setEnabled(true);

                SetFilterTypeCombo(sDataNameFilter);
                SetFilterParamsFrom(sDataNameFilter,sDataFilter);
            }
        }


        editNotes->setReadOnly(true);
        mEditNotes->setChecked(!editNotes->isReadOnly());

        if (FieldName=="Notes" or FieldName.startsWith("N_")){
            QString text = model->data(cur,Qt::DisplayRole).toString();
            if (text.contains("\n")){
                SetVisibleEditNotes(true);
            } else {
                QVariant fontVariant = tv->model()->data(cur, Qt::FontRole);
                QFont font = fontVariant.isValid() ? fontVariant.value<QFont>() : tv->font();
                QFontMetrics fontMetrics(font);
                QString elidedText = fontMetrics.elidedText(text, Qt::ElideRight, tv->columnWidth(cur.column())-7);

                SetVisibleEditNotes(elidedText != text);
                qDebug() << text;
                qDebug() << elidedText;
            }
        } else {
            SetVisibleEditNotes(false);
        }
    }
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
            //tv->selectionModel()->setCurrentIndex(model->index(i,iPositionCol),QItemSelectionModel::Current);
            tv->setCurrentIndex(model->index(i,iPositionCol));
            break;
        }
        if (sPositionRow2==model->index(i,0).data(Qt::DisplayRole).toString()+
                           model->index(i,1).data(Qt::DisplayRole).toString()+
                           model->index(i,2).data(Qt::DisplayRole).toString()) {
            //Normal row probably not in the the new row set.
            //tv->selectionModel()->setCurrentIndex(model->index(i,iPositionCol),QItemSelectionModel::Current);
            tv->setCurrentIndex(model->index(i,iPositionCol));
            break;
        }
    }
    //tv->setFocus();
}

void PotaWidget::SetFilterParamsFrom(QString sDataName, QString sData){
    //Filtering
    QString sDataType = DataType(model->tableName(),sDataName);
    leFilter->setToolTip("");
    if (sDataType=="" or sDataType=="TEXT" or sDataType.startsWith("BOOL")) {
        if (cbFilterType->currentText()==tr("égal à") or
            cbFilterType->currentText()==tr("différent de"))
            leFilter->setText(sData);
        else if (cbFilterType->currentText()==tr("fini par"))
            leFilter->setText(StrLast(sData,iTypeTextNbCar));
        else
            leFilter->setText(StrFirst(sData,iTypeTextNbCar));
    } else if (sDataType=="DATE") {
        if (cbFilterType->currentText()==tr("est de l'année"))
            leFilter->setText(StrFirst(sData,4));
        else if (cbFilterType->currentText()==tr("est du mois"))
            leFilter->setText(StrFirst(sData,7));
        else
            leFilter->setText(StrFirst(sData,10));
    } else if (sDataType=="REAL" or sDataType.startsWith("INT")) {
        leFilter->setText(sData);
        if(cbFilterType->currentText().startsWith(StrFirst(tr("proche de (%1)").arg(" 5%"),10))){
            float proche=StrFirst(StrLast(cbFilterType->currentText(),4),2).toInt();
            leFilter->setToolTip(tr("'%1' est compris entre %2 et %3.")
                                     .arg(sDataName)
                                     .arg(str(leFilter->text().toFloat()*(1-proche/100)))
                                     .arg(str(leFilter->text().toFloat()*(1+proche/100))));
        }
    }
    if (leFilter->text()==""){
        if (cbFilterType->currentText()==tr("ne contient pas") or
            cbFilterType->currentText()==tr("différent de"))
            leFilter->setToolTip(tr("'%1' n'est pas vide.")
                                     .arg(sDataName));
        else
            leFilter->setToolTip(tr("'%1' est vide.")
                                     .arg(sDataName));
    }
    if (!pbFilter->isChecked())
        leFilter->setToolTip(leFilter->toolTip());
    // leFilter->setToolTip(tr("Filtrer les lignes:")+"\n"+
    //                      leFilter->toolTip());
}

void PotaWidget::SetFilterTypeCombo(QString sDataName){
    //Filtering
    QString sDataType = DataType(model->tableName(),sDataName);
    bSetType=true;
    cbFilterType->clear();
    if (sDataType=="" or sDataType=="TEXT" or sDataType.startsWith("BOOL")) {
        cbFilterType->addItem(tr("commence par"));
        cbFilterType->addItem(tr("contient"));
        cbFilterType->addItem(tr("ne contient pas"));
        cbFilterType->addItem(tr("égal à"));
        cbFilterType->addItem(tr("différent de"));
        cbFilterType->addItem(tr("fini par"));
        cbFilterType->setCurrentIndex(iTypeText);
    } else if (sDataType=="DATE") {
        cbFilterType->addItem(tr("égal à"));
        cbFilterType->addItem(tr("différent de"));
        cbFilterType->addItem(tr("est du mois"));
        cbFilterType->addItem(tr("est de l'année"));
        cbFilterType->setCurrentIndex(iTypeDate);
    } else if (sDataType=="REAL" or sDataType.startsWith("INT")) {
        cbFilterType->addItem(tr("égal à"));
        cbFilterType->addItem(tr("supérieur à"));
        cbFilterType->addItem(tr("sup. ou = à"));
        cbFilterType->addItem(tr("inférieur à"));
        cbFilterType->addItem(tr("inf. ou = à"));
        cbFilterType->addItem(tr("différent de"));
        cbFilterType->addItem(tr("proche de (%1)").arg(" 5%"));
        cbFilterType->addItem(tr("proche de (%1)").arg("10%"));
        cbFilterType->addItem(tr("proche de (%1)").arg("20%"));
        cbFilterType->addItem(tr("proche de (%1)").arg("50%"));
        cbFilterType->setCurrentIndex(iTypeReal);
    }
    bSetType=false;
}

void PotaWidget::dataChanged(const QModelIndex &topLeft)//,const QModelIndex &bottomRight,const QList<int> &roles
{
    pbCommit->setEnabled(true);
    pbRollback->setEnabled(true);
    pbFilter->setEnabled(false);
    twParent->setTabIcon(twParent->currentIndex(),QIcon(":/images/toCommit.svg"));
    //lTabTitle->setStyleSheet(lTabTitle->styleSheet().append("color: red;"));

    if (!topLeft.data(Qt::EditRole).isNull() and
        (topLeft.data(Qt::EditRole) == ""))
        model->setData(topLeft,QVariant(),Qt::EditRole);//To avoid empty non null values.
}

void PotaWidget::headerRowClicked() //int logicalIndex
{
    //pbDeleteRow->setEnabled(true);
}

void PotaWidget::pbRefreshClick(){
    if (pbCommit->isEnabled())
        model->SubmitAllShowErr();
    model->copiedCells.clear();
    model->commitedCells.clear();

    bUserCurrChanged=false;
    PositionSave();
    model->SelectShowErr();
    PositionRestore();
    bUserCurrChanged=true;
}

void PotaWidget::pbEditClick(){
    if (pbEdit->isChecked()){
        pbCommit->setVisible(true);
        pbRollback->setVisible(true);
        sbInsertRows->setVisible(true);
        pbInsertRow->setVisible(true);
        pbDeleteRow->setVisible(true);
        model->nonEditableColumns.clear();
        for (int i=0; i<model->columnCount();i++) {
            if (ReadOnly(model->RealTableName(),model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString()))
                model->nonEditableColumns.insert(i);
        }
        pbEdit->setIcon(QIcon(":/images/editOn.svg"));
    } else {
        if (!pbCommit->isEnabled() or
            model->SubmitAllShowErr()){
            pbCommit->setVisible(false);
            pbRollback->setVisible(false);
            sbInsertRows->setVisible(false);
            pbInsertRow->setVisible(false);
            pbDeleteRow->setVisible(false);
            model->nonEditableColumns.clear();
            for (int i=0; i<model->columnCount();i++){
                model->nonEditableColumns.insert(i);
            }
            pbEdit->setIcon(QIcon(":/images/edit.svg"));
        }
    }

    //Force columns redraw
    tv->setFocus();
    pbEdit->setFocus();
}

void PotaWidget::pbCommitClick()
{
    bUserCurrChanged=false;
    PositionSave();
    if (model->SubmitAllShowErr())
        PositionRestore();
    bUserCurrChanged=true;
}

void PotaWidget::pbRollbackClick()
{
    bUserCurrChanged=false;
    PositionSave();
    if (model->RevertAllShowErr()){
        model->copiedCells.clear();
        PositionRestore();
    }
    bUserCurrChanged=true;
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

void PotaWidget::pbFilterClick(bool checked)
{
    //Filtering
    model->copiedCells.clear();
    model->commitedCells.clear();
    QString filter="";
    if (checked) {
        //Filter
        //QModelIndex index = tv->selectionModel()->currentIndex();

        QString sDataType = DataType(model->tableName(),sDataNameFilter);
        if (sDataType=="" or sDataType=="TEXT" or sDataType=="DATE" or sDataType.startsWith("BOOL")) {
            if (leFilter->text()==""){
                if (cbFilterType->currentText()==tr("ne contient pas") or
                    cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" NOTNULL";
                else
                    filter=lFilterOn->text()+" ISNULL";
            } else {
                if (cbFilterType->currentText()==tr("contient"))
                    filter=lFilterOn->text()+" LIKE '%"+StrReplace(leFilter->text(),"'","\'")+"%'";
                else if (cbFilterType->currentText()==tr("ne contient pas"))
                    filter=lFilterOn->text()+" NOT LIKE '%"+StrReplace(leFilter->text(),"'","\'")+"%'";
                else if (cbFilterType->currentText()==tr("égal à"))
                    filter=lFilterOn->text()+" = '"+StrReplace(leFilter->text(),"'","\'")+"'";
                else if (cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" != '"+StrReplace(leFilter->text(),"'","\'")+"'";
                else if (cbFilterType->currentText()==tr("fini par"))
                    filter=lFilterOn->text()+" LIKE '%"+StrReplace(leFilter->text(),"'","\'")+"'";
                else//commence par
                    filter=lFilterOn->text()+" LIKE '"+StrReplace(leFilter->text(),"'","\'")+"%'";
            }
        } else if (sDataType=="REAL" or sDataType.startsWith("INT")) {
            if (leFilter->text()==""){
                if (cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" NOTNULL";
                else
                    filter=lFilterOn->text()+" ISNULL";
            } else {
                if (cbFilterType->currentText()==tr("supérieur à"))
                    filter=lFilterOn->text()+" > '"+StrReplace(leFilter->text(),"'","\'")+"'";
                else if (cbFilterType->currentText()==tr("sup. ou = à"))
                    filter=lFilterOn->text()+" >= '"+StrReplace(leFilter->text(),"'","\'")+"'";
                else if (cbFilterType->currentText()==tr("inférieur à"))
                    filter=lFilterOn->text()+" < '"+StrReplace(leFilter->text(),"'","\'")+"'";
                else if (cbFilterType->currentText()==tr("inf. ou = à"))
                    filter=lFilterOn->text()+" <= '"+StrReplace(leFilter->text(),"'","\'")+"'";
                else if (cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" != '"+StrReplace(leFilter->text(),"'","\'")+"'";
                else if (cbFilterType->currentText().startsWith(StrFirst(tr("proche de (%1)").arg(" 5%"),10))){
                    float proche=StrFirst(StrLast(cbFilterType->currentText(),4),2).toInt();
                    filter=lFilterOn->text()+" BETWEEN "+str(leFilter->text().toFloat()*(1-proche/100))+" AND "+
                                                        str(leFilter->text().toFloat()*(1+proche/100));
                } else//égal à
                    filter=lFilterOn->text()+" = '"+StrReplace(leFilter->text(),"'","\'")+"'";
            }
        }
        delegate->FilterCol=model->fieldIndex(sDataNameFilter);
        //fFilter->setFrameShape(QFrame::Box);
        SetFontWeight(pbFilter,QFont::Bold);
        SetFontWeight(lFilterResult,QFont::Bold);
        pbFilter->setText(tr("Filtré")+" ->");

    } else {
        //Reset filter
        delegate->FilterCol=-1;
        //fFilter->setFrameShape(QFrame::NoFrame);
        SetFontWeight(pbFilter,QFont::Normal);
        SetFontWeight(lFilterResult,QFont::Normal);
        pbFilter->setText(tr("Filtrer"));
    }

    bUserCurrChanged=false;
    PositionSave();
    model->setFilter(filter);
    PositionRestore();
    bUserCurrChanged=true;

    lFilterResult->setText(str(model->rowCount())+" "+tr("lignes"));
}

void PotaWidget::cbFilterTypeChanged(int i){
    if (!bSetType) {
        //Filtering
        QModelIndex index = tv->selectionModel()->currentIndex();
        QString sDataType = DataType(model->tableName(),model->headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString());
        if (sDataType=="" or sDataType=="TEXT") {
            iTypeText=i;
        } else if (sDataType=="DATE") {
            iTypeDate=i;
        } else if (sDataType=="REAL" or sDataType.startsWith("INT")){
            iTypeReal=i;
        }

        if(model->headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString()==sDataNameFilter)
            sDataFilter=index.data(Qt::DisplayRole).toString();

        if (pbFilter->isChecked()) {
            SetFilterParamsFrom(sDataNameFilter,sDataFilter);
            pbFilterClick(true);
        } else {
            SetFilterParamsFrom(model->headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString(),
                                model->index(index.row(),index.column()).data(Qt::DisplayRole).toString());
        }
    }
}

void PotaWidget::leFilterReturnPressed(){
    if (pbFilter->isEnabled()){
            pbFilterClick(true);
            pbFilter->setChecked(true);
    }
}

void PotaWidget::leFindTextEdited(const QString &text){
    pbFindFirst->setEnabled(!text.isEmpty());
    pbFindNext->setEnabled(!text.isEmpty());
    pbFindPrev->setEnabled(!text.isEmpty());
    delegate->FindText=text.toLower();
}

void PotaWidget::leFindReturnPressed() {
    if (pbFindNext->isEnabled())
        pbFindNextClick();
}

void PotaWidget::FindFrom(int row, int column, bool Backward){
    SetFontColor(leFind,QColor());
    if (!Backward){
        for (int i=row;i<model->rowCount();i++) {
            for (int j=column;j<model->columnCount();j++) {
                if (model->index(i,j).data(Qt::DisplayRole).toString().toLower().contains(leFind->text().toLower())){
                    tv->setCurrentIndex(model->index(i,j));
                    return;
                }
            }
            column=0;//Find in all columns since 2nd row
        }
    } else {
        for (int i=row;i>=0;i--) {
            for (int j=column;j>=0;j--) {
                if (model->index(i,j).data(Qt::DisplayRole).toString().toLower().contains(leFind->text().toLower())){
                    tv->setCurrentIndex(model->index(i,j));
                    return;
                }
            }
            column=model->columnCount();//Find in all columns since 2nd row
        }
    }
    //Not found
    SetFontColor(leFind,Qt::red);
}

void PotaWidget::pbFindFirstClick(){
    FindFrom(0,0,false);
}

void PotaWidget::pbFindNextClick(){
    FindFrom(tv->selectionModel()->currentIndex().row(),tv->selectionModel()->currentIndex().column()+1,false);
}

void PotaWidget::pbFindPrevClick(){
    FindFrom(tv->selectionModel()->currentIndex().row(),tv->selectionModel()->currentIndex().column()-1,true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                           PotaTableModel
//////////////////////////////////////////////////////////////////////////////////////////////////

int PotaTableModel::FieldIndex(QString FieldName){
    for (int i=0;i<columnCount();i++){
        if (headerData(i,Qt::Horizontal,Qt::DisplayRole).toString()==FieldName)
            return i;
    }
    return 0;
}


QString PotaTableModel::FieldName(int index)
{
    return headerData(index,Qt::Horizontal,Qt::DisplayRole).toString();
    // PotaQuery *q = dynamic_cast<PotaWidget*>(parent())->query;
    // q->ExecShowErr("PRAGMA table_xinfo("+tableName()+");");

    // if (q->seek(index))
    //     return q->value("name").toString();
    // else
    //     return "";
}

bool PotaTableModel::select()  {
    //return QSqlRelationalTableModel::select();
    //If use of QSqlRelationalTableModel select(), the generated columns and null FK value rows are not displayed. #FKNull


    progressBar->setValue(0);
    progressBar->setMaximum(0);
    progressBar->setFormat(tableName()+" %p%");
    progressBar->setVisible(true);

    QString sQuery="SELECT * FROM "+tableName();

    if (filter().toStdString()!="") {//Add filter
        sQuery+=" WHERE "+filter();
    }

    if (sOrderByClause.toStdString()!="")//Add order by
        sQuery+=" "+sOrderByClause;

    qInfo() << sQuery;

    QSqlQuery countQuery("SELECT COUNT(*) FROM "+tableName()+
                         iif(filter().toStdString()!=""," WHERE "+filter(),"").toString());
    int totalRows = 0;
    if (countQuery.next())
        totalRows = countQuery.value(0).toInt();
    progressBar->setMaximum(totalRows);
    qDebug() << "totalRows " << totalRows;

    // QTimer *timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, this, &PotaTableModel::selectTimer);
    // timer->start(1000);

    setLastError(QSqlError());

    QSqlRelationalTableModel::select();//Avoids duplicate display of inserted lines
    qDebug() << rowCount() << "(select)";
    setQuery(sQuery);
    progressBar->setValue(rowCount());
    progressBar->setValue(1);
    qDebug() << rowCount() << "(setQuery)";

    while (canFetchMore()) {
        fetchMore();
        progressBar->setValue(rowCount());
        qDebug() << rowCount();
    }

    // timer->stop();
    // timer->deleteLater();
    progressBar->setVisible(false);
    return (lastError().type() == QSqlError::NoError);
}

// void PotaTableModel::selectTimer() {
//     progressBar->setValue(rowCount());
//     qDebug() << rowCount() << " timer";
// };


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
        pw->pbFilter->setEnabled(true);
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
        pw->pbFilter->setEnabled(true);
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
        pw->pbFilter->setEnabled(false);
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
        pw->pbFilter->setEnabled(false);
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
        PotaWidget *pw=dynamic_cast<PotaWidget*>(parent()->parent());
        if (!pw->pbCommit->isEnabled()){
            PotaTableModel *pm=dynamic_cast<PotaTableModel*>(model());
            pm->commitedCells.clear();
            pm->copiedCells.clear();

            //Change sort
            if (iSortCol==logicalIndex) {//Already sorted on this column.
                if (!bSortDes) {
                    model()->sort(logicalIndex,Qt::SortOrder::DescendingOrder);
                    model()->setHeaderData(logicalIndex, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_RedUp.svg")), Qt::DecorationRole);
                    bSortDes=true;
                } else {//Reset sorting.
                    model()->setHeaderData(iSortCol, Qt::Horizontal, 0, Qt::DecorationRole);
                    model()->sort( -1,Qt::SortOrder::AscendingOrder);
                    int iCol=NaturalSortCol(dynamic_cast<PotaTableModel*>(model())->tableName());
                    model()->setHeaderData(iCol, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_BlueDown.svg")), Qt::DecorationRole);
                    iSortCol=iCol;
                    bSortDes=false;
                }
            } else {//Actual sort on another column.
                model()->setHeaderData(iSortCol, Qt::Horizontal, 0, Qt::DecorationRole);
                model()->sort(logicalIndex,Qt::SortOrder::AscendingOrder);
                model()->setHeaderData(logicalIndex, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_GreenDown.svg")), Qt::DecorationRole);
                iSortCol=logicalIndex;
                bSortDes=false;
            }

            emit sectionClicked(logicalIndex);
        }
    }

    // Appeler la méthode parente pour le comportement standard
    QHeaderView::mouseDoubleClickEvent(event);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                                  PotaItemDelegate
//////////////////////////////////////////////////////////////////////////////////////////////////


void PotaItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    PotaWidget* pw = dynamic_cast<PotaWidget*>(parent());

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

    if (index.column()==FilterCol or //Filtering column.
        (!FindText.isEmpty() and index.data(Qt::DisplayRole).toString().toLower().contains(FindText))) { //Find
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

void PotaItemDelegate::paintTempo(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

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
    r.setLeft(option.rect.left()+30*coef);//January 31 day - 1 for half of month vert bar width
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
    //Year 2
    r.setLeft(r.left()+31*coef); r.setWidth(2); painter->fillRect(r,b);
    r.setLeft(r.left()+29*coef); r.setWidth(2); painter->fillRect(r,b);
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

QWidget *PotaItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const   {
    const PotaTableModel *constModel = qobject_cast<const PotaTableModel *>(index.model());
    if (!constModel) {
        return QStyledItemDelegate::createEditor(parent, option, index); // Standard editor
    }

    PotaTableModel *model = const_cast<PotaTableModel *>(constModel);
    QString sFieldName=model->headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString();
    QString sDataType=DataType(model->tableName(),sFieldName);
    if (model->relation(index.column()).isValid()) {
        //Create QComboBox for relational columns
        QComboBox *comboBox = new QComboBox(parent);
        QSqlTableModel *relationModel = model->relationModel(index.column());
        int relationIndex = relationModel->fieldIndex(model->relation(index.column()).displayColumn());

        QString filter=FkFilter(model->RealTableName(),sFieldName,index);
        if (filter!="")
            model->relationModel(index.column())->setFilter(filter);
        comboBox->addItem("", QVariant()); // Option for setting a NULL
        for (int i = 0; i < relationModel->rowCount(); ++i) {
            QString value = relationModel->record(i).value(relationIndex).toString();
            QString displayValue;// = relationModel->record(i).value(0).toString();
            // if (!relationModel->record(i).value(1).toString().isEmpty())
            //     displayValue+=" | "+relationModel->record(i).value(1).toString();
            // if (!relationModel->record(i).value(2).toString().isEmpty())
            //     displayValue+=" | "+relationModel->record(i).value(2).toString();
            displayValue=RowSummary(relationModel->tableName(),relationModel->record(i));
            comboBox->addItem( displayValue,value);
        }
        qDebug() << "set combo FK";

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

void PotaItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const  {
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
