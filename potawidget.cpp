#include "potawidget.h"
#include "data/Data.h"
#include "qapplication.h"
#include "qheaderview.h"
#include "qlineedit.h"
#include "qsqlerror.h"
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
    //query = new PotaQuery(nullptr);
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
    sbInsertRows->setMinimum(0);
    sbInsertRows->setValue(1);
    sbInsertRows->setEnabled(false);
    sbInsertRows->setVisible(false);

    pbInsertRow = new QToolButton(this);
    pbInsertRow->setIcon(QIcon(":/images/insert_row.svg"));
    SetButtonSize(pbInsertRow);
    pbInsertRow->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Insert));
    pbInsertRow->setToolTip(tr("Ajouter des lignes.\nSi le nombre de ligne à ajouter en mis à 0, l'enregistrement courant sera dupliqué.")+"\n"+
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
    pbFilter->setShortcut(QKeySequence(Qt::Key_F6));
    pbFilter->setToolTip("F6");
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
    pbFindFirst->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_F3));
    pbFindFirst->setToolTip("Ctrl + F3");
    connect(pbFindFirst, &QPushButton::clicked,this,&PotaWidget::pbFindFirstClick);
    pbFindNext = new QPushButton(this);
    pbFindNext->setText(tr("Suivant"));
    pbFindNext->setFixedWidth(70);
    pbFindNext->setEnabled(false);
    pbFindNext->setShortcut( QKeySequence(Qt::Key_F3));
    pbFindNext->setToolTip("F3");
    connect(pbFindNext, &QPushButton::clicked,this,&PotaWidget::pbFindNextClick);
    pbFindPrev = new QPushButton(this);
    pbFindPrev->setText(tr("Précédent"));
    pbFindPrev->setFixedWidth(70);
    pbFindPrev->setEnabled(false);
    pbFindPrev->setShortcut( QKeySequence(Qt::Key_F4));
    pbFindPrev->setToolTip("Ctrl + F4");
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
    lSelect->setTextInteractionFlags(Qt::TextBrowserInteraction);//Qt::TextSelectableByMouse &
    lSelect->setOpenExternalLinks(false);
    SetFontWeight(lSelect,QFont::Weight::Bold);

    connect(lSelect, &QLabel::linkActivated, this, &PotaWidget::showSelInfo);

    tv = new PotaTableView();
    tv->setParent(this);
    tv->setModel(model);
    tv->setItemDelegate(delegate);
    auto *header = new PotaHeaderView( Qt::Horizontal, tv);
    tv->setHorizontalHeader(header);
    tv->setLocale(QLocale::C );

    connect(tv->selectionModel(), &QItemSelectionModel::currentChanged, this, &PotaWidget::curChanged);
    connect(tv->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PotaWidget::selChanged);
    connect(model, &PotaTableModel::dataChanged, this, &PotaWidget::dataChanged);
    connect(tv->verticalHeader(), &QHeaderView::sectionClicked,this, &PotaWidget::headerRowClicked);

    editNotes = new QTextEdit();
    editNotes->setParent(tv);
    editNotes->setVisible(false);
    editNotes->setReadOnly(true);
    QPalette palette = editNotes->palette();
    palette.setColor(QPalette::Base, QApplication::palette().color(QPalette::ToolTipBase));
    palette.setColor(QPalette::Text, QApplication::palette().color(QPalette::ToolTipText));
    editNotes->setPalette(palette);

    editSelInfo = new QTextEdit();
    editSelInfo->setParent(this);
    editSelInfo->setVisible(false);
    editSelInfo->setReadOnly(true);

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

    PotaQuery query(*model->db);

    qInfo() << "Open " << TableName; +" ("+model->RealTableName()+")";

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);//OnFieldChange

    //Primary Key
    query.ExecShowErr("PRAGMA table_xinfo("+model->RealTableName()+")");
    while (query.next()){
        if (query.value(5).toInt()==1) {
            model->sPrimaryKey=query.value(1).toString();
            qDebug() << "sPrimaryKey : "+model->sPrimaryKey;
            break;
        }
    }

    //FK
    query.ExecShowErr("PRAGMA foreign_key_list("+model->RealTableName()+");");
    while (query.next()) {
        QString referencedTable = query.value("table").toString();
        QString localColumn = query.value("from").toString();
        QString referencedClumn = query.value("to").toString();
        int localColumnIndex = model->fieldIndex(localColumn);

        if (localColumnIndex>-1){
            model->setRelation(localColumnIndex, QSqlRelation(referencedTable, referencedClumn, referencedClumn));//Issue #2
            model->relationModel(localColumnIndex)->setFilter(FkFilter(model->db,model->RealTableName(),localColumn,model->index(0,0)));
        }
    }

    //Generated columns
    query.clear();
    query.ExecShowErr("PRAGMA table_xinfo("+TableName+")");
    while (query.next()){
        if (query.value(6).toInt()==2)
            model->generatedColumns.insert(query.value(1).toString());
        model->dataTypes.append(DataType(model->db,TableName,query.value(1).toString()));
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

        pbInsertRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
        sbInsertRows->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
        pbDeleteRow->setEnabled(bAllowDelete and (model->rowsToInsert.count()==0));

        if (cur.row()>-1)
            lRowSummary->setText(RowSummary(model->tableName(),model->record(cur.row())));
        else
            lRowSummary->setText("...");

        QString FieldName=model->headerData(cur.column(),Qt::Horizontal,Qt::DisplayRole).toString();

        if (filterFrame->isVisible()) {
            if (!pbFilter->isChecked()) {
                //Filtering
                sFieldNameFilter=FieldName;
                sDataTypeFilter=model->dataTypes[cur.column()];
                sDataFilter=model->index(cur.row(),cur.column()).data(Qt::DisplayRole).toString();

                lFilterOn->setText(sFieldNameFilter);

                if(!pbCommit->isEnabled())
                    pbFilter->setEnabled(true);//Don't enable filtering if there is pending modifications, they would be lost.

                SetFilterTypeCombo(sDataTypeFilter);
                SetLeFilterWith(sFieldNameFilter,sDataTypeFilter,sDataFilter);
            }
        }


        editNotes->setReadOnly(true);
        mEditNotes->setChecked(!editNotes->isReadOnly());

        if (AcceptReturns(FieldName)){
            QString text = model->data(cur,Qt::DisplayRole).toString();
            if (text.contains("\n")){
                SetVisibleEditNotes(true,model->nonEditableColumns.contains(cur.column()));
            } else {
                // QVariant fontVariant = tv->model()->data(cur, Qt::FontRole);
                // QFont font = fontVariant.isValid() ? fontVariant.value<QFont>() : tv->font();
                // QFontMetrics fontMetrics(font);
                // QString elidedText = fontMetrics.elidedText(text, Qt::ElideRight, tv->columnWidth(cur.column())+65);
                // SetVisibleEditNotes(elidedText != text,model->nonEditableColumns.contains(cur.column()));

                QFontMetrics fm(editNotes->font());

                SetVisibleEditNotes(fm.horizontalAdvance(text)>tv->columnWidth(cur.column())-3,model->nonEditableColumns.contains(cur.column()));
            }
        } else {
            SetVisibleEditNotes(false,false);
        }
        //dbSuspend(model->db,true,pbEdit->isChecked(),model->label);//Normaly not necessary but could correct a case where suspend is wrongly OFF.

        editSelInfo->setVisible(false);
    }
}

void PotaWidget::selChanged() {
    if (bUserCurrChanged){
        //Count, sum, etc about selection.
        QModelIndexList selectedIndexes = tv->selectionModel()->selectedIndexes();
        int nbSelected=tv->selectionModel()->selectedIndexes().count();
        int nbOkSelected=0;
        int nbEmptySelected=0;
        if (nbSelected>1){
            float sumSelected=0;
            float minSelected=0;
            float maxSelected=0;
            QStringList sl;
            bool bOk;
            //for (const QModelIndex &index : selectedIndexes) {
            for (int i=0;i<nbSelected;i++) {
                //QModelIndex &index : selectedIndexes) {
                sumSelected+=selectedIndexes[i].data().toFloat(&bOk);
                if (bOk){
                    if (nbOkSelected==0) {
                        minSelected=selectedIndexes[i].data().toFloat();
                        maxSelected=selectedIndexes[i].data().toFloat();
                    } else {
                        minSelected=min(minSelected,selectedIndexes[i].data().toFloat());
                        maxSelected=fmax(maxSelected,selectedIndexes[i].data().toFloat());
                    }
                    nbOkSelected++;
                }
                if (selectedIndexes[i].data().toString().isEmpty()) {
                    nbEmptySelected++;
                } else {
                    if (!sl.contains(selectedIndexes[i].data().toString()))
                        sl.append(selectedIndexes[i].data().toString());
                }
            }
            lSelect->setText(tr("Sélection: ")+str(nbSelected)+
                             iif(nbOkSelected>1," - "+
                                 iif(nbOkSelected<nbSelected,tr("num: ")+str(nbOkSelected)+" - ","").toString()+
                                 tr("somme: ")+str(sumSelected),"").toString());
            if (nbSelected>1) {
                QString s;
                s="\n";
                for (int i=0;i<sl.count();i++)
                    s+="\n- "+sl[i];
                //lSelect->setToolTip(tr("Cliquez pour avoir d'autres infos."));
                editSelInfo->setMarkdown(tr("Nb de cellules sélectionées: ")+str(nbSelected)+"\n\n"+
                                         iif(nbEmptySelected>0,
                                             tr("- Vides: ")+str(nbEmptySelected)+"\n\n"+
                                             tr("- Non vides: ")+str(nbSelected-nbEmptySelected)+"\n\n","").toString()+
                                         tr("Nb de cellules numériques: ")+str(nbOkSelected)+"\n\n"+
                                         iif(nbOkSelected>0,
                                             tr("- **Somme: ")+str(sumSelected)+"**\n"+
                                             tr("- Moyenne: ")+str(sumSelected/nbOkSelected)+"\n"+
                                             tr("- Mini: ")+str(minSelected)+"\n"+
                                             tr("- Maxi: ")+str(maxSelected)+"\n","").toString()+"\n"+
                                         tr("Nb de valeurs différentes: ")+str(sl.count())+s);
                if (isDarkTheme())
                    lSelect->setText(lSelect->text()+" <a style=\"color: #7785ff\" href='#'>plus...</a>");
                else
                    lSelect->setText(lSelect->text()+" <a href='#'>plus...</a>");
                //lSelect->setCursor(Qt::PointingHandCursor);
            } else {
                //lSelect->setToolTip("");
                editSelInfo->clear();
            }
            lSelect->setVisible(true);
        } else {
            lSelect->setVisible(false);
        }
    }
}

void PotaWidget::showSelInfo() {
    editSelInfo->setGeometry(lSelect->geometry().left(),
                             lSelect->geometry().top(),
                             400,200);
    editSelInfo->setVisible(true);
}

void PotaWidget::SetVisibleEditNotes(bool bVisible, bool autoSize){
    if (bVisible){
        int x = tv->columnViewportPosition(tv->currentIndex().column())+5;
        int y = tv->rowViewportPosition(tv->currentIndex().row())+
                tv->horizontalHeader()->height()+
                tv->rowHeight(tv->currentIndex().row());
        int EditNotesWidth = 400;
        int EditNotesHeight = 200;

        if (editNotes->isReadOnly()) {
            editNotes->setMarkdown(model->data(tv->currentIndex(),Qt::DisplayRole).toString());
        } else {
            editNotes->setPlainText(model->data(tv->currentIndex(),Qt::DisplayRole).toString());

        }

        if (autoSize) {
            //editNotes->document()->setTextWidth(QWIDGETSIZE_MAX);
            QFontMetrics fm(editNotes->font());
            int maxWidth = 0;
            int returns=0;
            QStringList lines = editNotes->toPlainText().split('\n');

            for (const QString &line : lines) {
                int lineWidth = fm.horizontalAdvance(line);
                if (lineWidth > EditNotesWidth)
                    returns+=floor(lineWidth/EditNotesWidth);
                if (lineWidth > maxWidth)
                    maxWidth = lineWidth;
            }
            // editNotes->setFixedSize(min(maxWidth+50,400),
            //                         min(lines.count()*22+5,200));
            EditNotesWidth = min(maxWidth+50,400);
            EditNotesHeight = min((lines.count()+returns)*22+5,200);
        }
        x=min(x,tv->width()-EditNotesWidth-20);
        editNotes->setGeometry(x,y,EditNotesWidth,EditNotesHeight);

        editNotes->setVisible(true);
    } else {
        editNotes->setVisible(false);
    }
}

void PotaWidget::PositionSave() {
    //iPositionCol=tv->selectionModel()->currentIndex().column();
    iPositionCol=tv->currentIndex().column();

    //Normal row to retreive.
    sPositionRow="";
    int iStart=iif(tv->isColumnHidden(0),1,0).toInt();
    for (int i=iStart;i<iStart+3;i++)
        sPositionRow+=model->index(tv->selectionModel()->currentIndex().row(),i).data(Qt::DisplayRole).toString();

    //Alternative row to retreive if normal row desapear.
    sPositionRow2="";
    for (int i=iStart;i<iStart+3;i++)
        sPositionRow2+=model->index(tv->selectionModel()->currentIndex().row()+1,i).data(Qt::DisplayRole).toString();
    qDebug() << "PositionSave: " << sPositionRow;
}

void PotaWidget::PositionRestore() {
    int iStart=iif(tv->isColumnHidden(0),1,0).toInt();
    int row2=-1;
    for (int row=0;row<model->rowCount();row++) {
        if (sPositionRow==model->index(row,iStart+0).data(Qt::DisplayRole).toString()+
                          model->index(row,iStart+1).data(Qt::DisplayRole).toString()+
                          model->index(row,iStart+2).data(Qt::DisplayRole).toString()) {
            //Normal row retreived.
            //tv->selectionModel()->setCurrentIndex(model->index(row,iPositionCol),QItemSelectionModel::Current);
            tv->setCurrentIndex(model->index(row,iPositionCol));
            row2=-1;
            qDebug() << "PositionRestore: normal";
            break;
        }
        if (sPositionRow2==model->index(row,iStart+0).data(Qt::DisplayRole).toString()+
                           model->index(row,iStart+1).data(Qt::DisplayRole).toString()+
                           model->index(row,iStart+2).data(Qt::DisplayRole).toString()) {
            row2=row;
        }
    }
    if (row2>-1) {
        //Normal row not in the the new row set.
        tv->setCurrentIndex(model->index(row2,iPositionCol));
        qDebug() << "PositionRestore: alternative";
    }
    lRowSummary->setText(RowSummary(model->tableName(),model->record(tv->currentIndex().row())));
    lSelect->setText("");
}

void PotaWidget::SetLeFilterWith(QString sFieldName, QString sDataType, QString sData){
    //Filtering
    //QString sDataType = DataType(model->db, model->tableName(),sDataName);
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
        // if (cbFilterType->currentText()==tr("est de l'année"))
        //     leFilter->setText(StrFirst(sData,4));
        // else if (cbFilterType->currentText()==tr("est du mois"))
        //     leFilter->setText(StrFirst(sData,7));
        // else
            leFilter->setText(StrFirst(sData,10));
    } else if (sDataType=="REAL" or sDataType.startsWith("INT")) {
        leFilter->setText(sData);
        if(cbFilterType->currentText().startsWith(StrFirst(tr("proche de (%1)").arg(" 5%"),10))){
            float proche=StrFirst(StrLast(cbFilterType->currentText(),4),2).toInt();
            leFilter->setToolTip(tr("'%1' est compris entre %2 et %3.")
                                     .arg(sFieldName)
                                     .arg(str(leFilter->text().toFloat()*(1-proche/100)))
                                     .arg(str(leFilter->text().toFloat()*(1+proche/100))));
        }
    }

    leFilter->setText(leFilter->text().trimmed()); // A space at the end is invisible and make the filter fail.

    if (leFilter->text()==""){
        if (cbFilterType->currentText()==tr("ne contient pas") or
            cbFilterType->currentText()==tr("différent de"))
            leFilter->setToolTip(tr("'%1' n'est pas vide.")
                                     .arg(sFieldName));
        else
            leFilter->setToolTip(tr("'%1' est vide.")
                                     .arg(sFieldName));
    }
    if (!pbFilter->isChecked())
        leFilter->setToolTip(leFilter->toolTip());
    // leFilter->setToolTip(tr("Filtrer les lignes:")+"\n"+
    //                      leFilter->toolTip());
}

void PotaWidget::SetFilterTypeCombo(QString sDataType){
    //Filtering
    //QString sDataType = DataType(model->db, model->tableName(),sDataName);
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
        cbFilterType->addItem(tr("sup. ou = à"));
        cbFilterType->addItem(tr("inf. ou = à"));
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
            if (ReadOnly(model->db, model->tableName(),model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString()))
                model->nonEditableColumns.insert(i);
        }
        pbEdit->setIcon(QIcon(":/images/editOn.svg"));

        PotaQuery query(*model->db);
        if (query.Selec0ShowErr("SELECT Valeur='Oui' FROM Params WHERE Paramètre='Montrer_modifs'").toBool()) {
            model->tempTableName="Temp"+QDateTime::currentDateTime().toString("hhmmss")+model->tableName();
            query.ExecShowErr("CREATE TEMP TABLE "+model->tempTableName+" AS SELECT * FROM "+model->tableName());
        }

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
            twParent->setTabIcon(twParent->currentIndex(),QIcon(""));
            model->tempTableName="";
        }
    }

    //*userDataEditing=true;
   //dbSuspend(model->db,false,true,model->label);
    pbRefreshClick();
    tv->setFocus();//This force columns redraw
    //pbEdit->setFocus();
}

void PotaWidget::pbCommitClick()
{
    bUserCurrChanged=false;
    PositionSave();
    if (model->SubmitAllShowErr())
        PositionRestore();
    pbInsertRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    sbInsertRows->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    pbDeleteRow->setEnabled(bAllowDelete and (model->rowsToInsert.count()==0));
    bUserCurrChanged=true;
}

void PotaWidget::pbRollbackClick()
{
    bUserCurrChanged=false;
    PositionSave();
    if (model->RevertAllShowErr()){
        //model->modifiedCells.clear();
        PositionRestore();
    }
    pbInsertRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    sbInsertRows->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    pbDeleteRow->setEnabled(bAllowDelete and (model->rowsToInsert.count()==0));
    bUserCurrChanged=true;
}

void PotaWidget::pbInsertRowClick()
{
    model->InsertRowShowErr();
    pbDeleteRow->setEnabled(bAllowDelete and (model->rowsToInsert.count()==0));

    tv->setFocus();
}

void PotaWidget::pbDeleteRowClick()
{
    model->DeleteRowShowErr();
    pbInsertRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    sbInsertRows->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    tv->setFocus();
}

void PotaWidget::pbFilterClick(bool checked)
{
    //Filtering
    QString filter="";
    if (checked) {
        //Filter
        //QModelIndex index = tv->selectionModel()->currentIndex();

        QString sDataType = sDataTypeFilter;
        if (sDataType=="" or sDataType=="TEXT" or sDataType.startsWith("BOOL")) {
            if (leFilter->text()==""){
                if (cbFilterType->currentText()==tr("ne contient pas") or
                    cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" NOTNULL";
                else
                    filter=lFilterOn->text()+" ISNULL";
            } else {
                if (cbFilterType->currentText()==tr("contient"))
                    filter=lFilterOn->text()+" LIKE '%"+StrReplace(leFilter->text(),"'","''")+"%'";
                else if (cbFilterType->currentText()==tr("ne contient pas"))
                    filter=lFilterOn->text()+" NOT LIKE '%"+StrReplace(leFilter->text(),"'","''")+"%'";
                else if (cbFilterType->currentText()==tr("égal à"))
                    filter=lFilterOn->text()+" = '"+StrReplace(leFilter->text(),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" != '"+StrReplace(leFilter->text(),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("fini par"))
                    filter=lFilterOn->text()+" LIKE '%"+StrReplace(leFilter->text(),"'","''")+"'";
                else//commence par
                    filter=lFilterOn->text()+" LIKE '"+StrReplace(leFilter->text(),"'","''")+"%'";
            }
        } else if (sDataType=="DATE") {
            if (leFilter->text()==""){
                if (cbFilterType->currentText()==tr("sup. ou = à") or
                    cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" NOTNULL";
                else
                    filter=lFilterOn->text()+" ISNULL";
            } else {
                QDate date = QDate::fromString(leFilter->text(), "dd/MM/yyyy");
                QString dateString = date.toString("yyyy-MM-dd");
                if (cbFilterType->currentText()==tr("égal à"))
                    filter=lFilterOn->text()+" = '"+dateString+"'";
                else if (cbFilterType->currentText()==tr("sup. ou = à"))
                    filter=lFilterOn->text()+" >= '"+dateString+"'";
                else if (cbFilterType->currentText()==tr("inf. ou = à"))
                    filter=lFilterOn->text()+" <= '"+dateString+"'";
                else if (cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" != '"+dateString+"'";
                else if (cbFilterType->currentText()==tr("est du mois"))
                    filter=lFilterOn->text()+" LIKE '"+SubString(dateString,0,7)+"%'";
                else
                    filter=lFilterOn->text()+" LIKE '"+SubString(dateString,0,4)+"%'";
            }
        } else if (sDataType=="REAL" or sDataType.startsWith("INT")) {
            if (leFilter->text()==""){
                if (cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" NOTNULL";
                else
                    filter=lFilterOn->text()+" ISNULL";
            } else {
                if (cbFilterType->currentText()==tr("supérieur à"))
                    filter=lFilterOn->text()+" > '"+StrReplace(leFilter->text(),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("sup. ou = à"))
                    filter=lFilterOn->text()+" >= '"+StrReplace(leFilter->text(),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("inférieur à"))
                    filter=lFilterOn->text()+" < '"+StrReplace(leFilter->text(),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("inf. ou = à"))
                    filter=lFilterOn->text()+" <= '"+StrReplace(leFilter->text(),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" != '"+StrReplace(leFilter->text(),"'","''")+"'";
                else if (cbFilterType->currentText().startsWith(StrFirst(tr("proche de (%1)").arg(" 5%"),10))){
                    float proche=StrFirst(StrLast(cbFilterType->currentText(),4),2).toInt();
                    filter=lFilterOn->text()+" BETWEEN "+str(leFilter->text().toFloat()*(1-proche/100))+" AND "+
                                                        str(leFilter->text().toFloat()*(1+proche/100));
                } else//égal à
                    filter=lFilterOn->text()+" = '"+StrReplace(leFilter->text(),"'","''")+"'";
            }
        }
        delegate->FilterCol=model->fieldIndex(sFieldNameFilter);
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

    tv->setFocus();

    bUserCurrChanged=false;
    PositionSave();
    model->setFilter(filter);
    PositionRestore();
    bUserCurrChanged=true;

    lFilterResult->setText(str(model->rowCount())+" "+tr("lignes"));


}

void PotaWidget::cbFilterTypeChanged(int i){
    if (!bSetType) {//User selection
        //Filtering
        QModelIndex index = tv->selectionModel()->currentIndex();
        if (index.isValid()){
            QString sDataType = model->dataTypes[index.column()];
            if (sDataType=="" or sDataType=="TEXT") {
                iTypeText=i;
            } else if (sDataType=="DATE") {
                iTypeDate=i;
            } else if (sDataType=="REAL" or sDataType.startsWith("INT")){
                iTypeReal=i;
            }

            // qDebug() << "headerdata: "+model->headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString();
            // qDebug() << "sFieldNameFilter: "+sFieldNameFilter;
            // qDebug() << "data: "+index.data(Qt::DisplayRole).toString();
            // qDebug() << "sDataFilter: "+sDataFilter;
            // if(model->headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString()!=sFieldNameFilter){//sDataTypeFilter todo
            //     sDataFilter=index.data(Qt::DisplayRole).toString();
            // }

            SetLeFilterWith(sFieldNameFilter,sDataTypeFilter,leFilter->text());//Update the ToolTip.
            if (pbFilter->isChecked())
                pbFilterClick(true);
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
    tv->setFocus();
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
    return -1;
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

    //dbSuspend(db,false,true,label);

    AppBusy(true,progressBar,0,tableName()+" %p%");
    //modifiedCells.clear();
    commitedCells.clear();
    copiedCells.clear();

    QString sQuery="SELECT * FROM "+tableName();

    if (filter().toStdString()!="") {//Add filter
        sQuery+=" WHERE "+filter();
    }

    if (sOrderByClause.toStdString()!="")//Add order by
        sQuery+=" "+sOrderByClause;

    qInfo() << sQuery;

    PotaQuery query(*db);
    query.ExecShowErr("SELECT COUNT(*) FROM "+tableName()+
                           iif(filter().toStdString()!=""," WHERE "+filter(),"").toString());
    int totalRows = 0;
    if (query.next())
        totalRows = query.value(0).toInt();
    progressBar->setMaximum(totalRows);
    qDebug() << "totalRows " << totalRows;

    // QTimer *timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, this, &PotaTableModel::selectTimer);
    // timer->start(1000);

    setLastError(QSqlError());

    QSqlRelationalTableModel::select();//Avoids duplicate display of inserted lines
    qDebug() << rowCount() << "(select)";
    progressBar->setValue(1);
    setQuery(sQuery);
    progressBar->setValue(rowCount());
    qDebug() << rowCount() << "(setQuery)";

    while (canFetchMore()) {
        fetchMore();
        progressBar->setValue(rowCount());
        qDebug() << rowCount();
    }

    // timer->stop();
    // timer->deleteLater();
    AppBusy(false,progressBar);
    bool result=(lastError().type() == QSqlError::NoError);
    //dbSuspend(db,true,!wasSuspended,label);

    if (!tempTableName.isEmpty()){
        //Show modified cells
        //qDebug() << "culture ligne 1" << query.Selec0ShowErr("SELECT Culture FROM temp."+tempTableName+" WHERE Culture=1700");
        AppBusy(true,progressBar,rowCount(),"Show modified cells %p%");
        //QString sPrimaryKey=PrimaryKeyFieldName(db,RealTableName());
        int jPrimaryKey=FieldIndex(sPrimaryKey);
        query.prepare("SELECT * "
                      "FROM temp."+tempTableName+" "+
                      "WHERE "+sPrimaryKey+"=:pk");
        for (int i=0;i<rowCount();i++) {
            progressBar->setValue(i);
            query.bindValue(":pk",StrReplace(data(index(i,jPrimaryKey)).toString(),"'","''"));
            query.exec();
            query.next();
            //if (i==0)
            //    qDebug() << query.executedQuery() << data(index(i,jPrimaryKey)).toString();
            //break;
            for (int j=0;j<columnCount();j++) {
                //if (i+j==0)
                //    qDebug() << data(index(i,j),Qt::EditRole).toString() << query.value(j).toString();
                if (data(index(i,j),Qt::EditRole).toString()!=query.value(j).toString())
                    // query.Selec0ShowErr("SELECT "+FieldName(j)+" "+
                    //                      "FROM temp."+tempTableName+" "+
                    //                      "WHERE "+sPrimaryKey+"='"+StrReplace(data(index(i,jPrimaryKey)).toString(),"'","''")+"'").toString())
                    commitedCells.insert(index(i,j));
            }
        }
        AppBusy(false,progressBar);
    }

    return result;
}

// void PotaTableModel::selectTimer() {
//     progressBar->setValue(rowCount());
//     qDebug() << rowCount() << " timer";
// };


bool PotaTableModel::SelectShowErr()
{
    int i;
    for (i=0;i<columnCount();i++) {
        if (relationModel(i)){
            //relationModel(i)->select();//FkNull
            relationModel(i)->setQuery("SELECT * FROM "+relationModel(i)->tableName()+
                                       iif(relationModel(i)->filter().toStdString()!=""," WHERE "+relationModel(i)->filter(),"").toString());
        }
    }

    setLastError(QSqlError());

    select();

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

bool PotaTableModel::SubmitAllShowErr() {
    PotaWidget *pw = dynamic_cast<PotaWidget*>(parent());
    setLastError(QSqlError());
    //dbSuspend(db,false,true,label);
    submitAll();
    if (lastError().type() == QSqlError::NoError) {
        SetColoredText(pw->lErr,tableName()+": "+tr("modifications enregistrées."),"Ok");
        pw->isCommittingError=false;
        pw->pbCommit->setEnabled(false);
        pw->pbRollback->setEnabled(false);
        pw->pbFilter->setEnabled(true);
        pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(""));
        //pw->lTabTitle->setStyleSheet(pw->lTabTitle->styleSheet().replace("color: red;", ""));
        //dbSuspend(db,true,true,label);
        return true;
    } else {
        SetColoredText(pw->lErr,lastError().text(),"Err");
        qDebug() <<  lastError().text();
        pw->isCommittingError=true;
        //dbSuspend(db,true,true,label);
        return false;
    }
}

bool PotaTableModel::RevertAllShowErr() {
    PotaWidget *pw = dynamic_cast<PotaWidget*>(parent());
    setLastError(QSqlError());
    //dbSuspend(db,false,true,label);
    revertAll();
    if (lastError().type() == QSqlError::NoError) {
        SetColoredText(pw->lErr,tableName()+": "+tr("modifications abandonnées."),"Info");
        pw->isCommittingError=false;
        pw->pbCommit->setEnabled(false);
        pw->pbRollback->setEnabled(false);
        pw->pbFilter->setEnabled(true);
        pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(""));
        //pw->lTabTitle->setStyleSheet(pw->lTabTitle->styleSheet().replace("color: red;", ""));
        //dbSuspend(db,true,true,label);
        return true;
    } else {
        SetColoredText(pw->lErr,lastError().text(),"Err");
        pw->isCommittingError=true;
        //dbSuspend(db,true,true,label);
        return false;
    }
}

bool PotaTableModel::InsertRowShowErr()
{
    PotaWidget *pw = dynamic_cast<PotaWidget*>(parent());
    if (pw->sbInsertRows->value()==0){//Duplicate selected row
        int sourcerow=pw->tv->currentIndex().row();
        if (insertRows(sourcerow+1,1)) {
            for (int col = 0; col < pw->model->columnCount(); ++col) {
                QModelIndex sourceIndex=pw->model->index(sourcerow, col);
                QModelIndex destIndex=pw->model->index(sourcerow+1, col);
                pw->model->setData(destIndex, pw->model->data(sourceIndex));
            }
            rowsToInsert.insert(sourcerow+1);
            pw->tv->setCurrentIndex(index(sourcerow+1,iif(pw->tv->isColumnHidden(0),1,0).toInt()));
            pw->pbCommit->setEnabled(true);
            pw->pbRollback->setEnabled(true);
            pw->pbFilter->setEnabled(false);
            pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(":/images/toCommit.svg"));
        } else {
            SetColoredText(pw->lErr,"insertRows(x,y)","Err");
            return false;
        }
    } else if (insertRows(pw->tv->currentIndex().row()+1,//Create blank new rows
                   pw->sbInsertRows->value())) {
        for (int i=0;i<pw->sbInsertRows->value();i++)
            rowsToInsert.insert(i+pw->tv->currentIndex().row()+1);
        pw->tv->setCurrentIndex(index(pw->tv->currentIndex().row()+1,iif(pw->tv->isColumnHidden(0),1,0).toInt()));
        pw->pbCommit->setEnabled(true);
        pw->pbRollback->setEnabled(true);
        pw->pbFilter->setEnabled(false);
        pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(":/images/toCommit.svg"));
    } else {
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
            pw->PositionSave();
            //PotaTableModel *pm=dynamic_cast<PotaTableModel*>(model());

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
            pw->PositionRestore();

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


    if (!index.data(Qt::EditRole).isNull() and
        (index.data(Qt::EditRole) == "")) {
        //Hightlight not null empty value. They are not normal, it causes SQL failure.
        c=Qt::red;
        c.setAlpha(150);
    } else if (RowColorCol>-1) {
        c=RowColor(index.model()->index(index.row(),RowColorCol).data(Qt::DisplayRole).toString());
    }
    if (!c.isValid()) {//Table color.
        c=cColColors[index.column()];
        if (!c.isValid() and index.column()!=TempoCol)
            c=cTableColor;
        if (isDarkTheme())
            c.setAlpha(30);
        else
            c.setAlpha(60);
    }
    if (!pw->model->rowsToInsert.contains(index.row()) and c.isValid()) {
        b.setColor(c);
        painter->fillRect(option.rect,b);
    }

    if (index.column()==FilterCol or //Filtering column.
        (!FindText.isEmpty() and index.data(Qt::DisplayRole).toString().toLower().contains(FindText))) { //Find
        if (!isDarkTheme())
            cFiltered=QColor("#000000");
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

    //Paint data
    if (index.column()==TempoCol)
         paintTempo(painter,option,index);
    else {
        if(!index.data(Qt::EditRole).toDate().isNull() and index.data(Qt::EditRole).toDate()>QDate::currentDate()){//todo
            //Write red date in future
            QStyleOptionViewItem opt = option;
            if (isDarkTheme())
                opt.palette.setColor(QPalette::Text, QColor("#ffadad"));//light red
            else
                opt.palette.setColor(QPalette::Text, QColor("#b70000"));//dark red
            QStyledItemDelegate::paint(painter, opt, index);
        } else {
            QStyledItemDelegate::paint(painter, option, index);
        }
    }


    if (pw->model && pw->model->rowsToRemove.contains(index.row())) {
        //Rows to remove.
        painter->save();
        painter->setPen(QColor("#ff0000"));//red
        painter->drawRect(option.rect.x(), option.rect.y()+option.rect.height()/2, option.rect.width()-1, 1);
        painter->restore();
    //} else if (pw->model && pw->model->modifiedCells.contains(index)) {
    } else if (pw->model->isDirty(index)) {
        cModified=QColor("#0086ff");//blue
        cModifiedError=QColor("#ff0000");//red
        painter->save();
        painter->setPen(pw->isCommittingError ? cModifiedError : cModified);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
        painter->restore();
    } else if (pw->model && pw->model->commitedCells.contains(index)) {
        cModified=QColor("#0086ff");//blue
        cModified.setAlpha(150);
        painter->save();
        painter->setPen(cModified);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
        painter->restore();
    }

    if (pw->model && pw->model->copiedCells.contains(index)) {
        cCopied=QColor("#00ab00");//green
        painter->save();
        painter->setPen(cCopied);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
        painter->restore();
    }}

void PotaItemDelegate::paintTempo(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    double const coef=1;
    QBrush b;
    b.setStyle(Qt::SolidPattern);
    QColor c;
    QRect r;
    int left=0;

    //Vert bar for month
    c=QColor(128,128,128);
    b.setColor(c);
    r.setBottom(option.rect.bottom());
    r.setTop(option.rect.top());
    int nbJ=0;
    for(int i=1;i<24;i++){
        if(QSet<int>{1,3,5,7,8,10,12,13,15,17,19,20,22,24}.contains(i))
            nbJ+=31;
        else if (QSet<int>{2,14}.contains(i))
            nbJ+=28;
        else
            nbJ+=30;
        left=option.rect.left()+round(nbJ*coef);
        if(left+2>=option.rect.right())
            break;
        r.setLeft(left);
        r.setWidth(2);
        painter->fillRect(r,b);
    }

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
        if(plant>0) c=cSousAbris;
        else c=cEnPlace;
        c.setAlpha(255);
        b.setColor(c);
        r.setTop(r.bottom()-4);
        r.setLeft(option.rect.left()+attente);
        r.setRight(option.rect.left()+attente+fmax(semis,4));
        painter->fillRect(r,b);
    }
    if (semisF>0){
        //Semis fait, attente plantation
        if(plant>0) c=cSousAbris;
        else c=cEnPlace;
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
        r.setRight(option.rect.left()+attente+semis+semisF+fmax(plant,4));
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
        c=cRecolte;
        c.setAlpha(255);
        b.setColor(c);
        r.setTop(r.bottom()-12);
        r.setBottom(r.bottom()-6);
        r.setLeft(option.rect.left()+attente+semis+semisF+plant+plantF);
        r.setRight(option.rect.left()+attente+semis+semisF+plant+plantF+fmax(recolte,4));
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
    QString sDataType=model->dataTypes[index.column()];
    if (model->relation(index.column()).isValid()) {
        //Create QComboBox for relational columns
        QComboBox *comboBox = new QComboBox(parent);
        QSqlTableModel *relationModel = model->relationModel(index.column());
        int relationIndex = relationModel->fieldIndex(model->relation(index.column()).displayColumn());

        QString filter=FkFilter(model->db,model->RealTableName(),sFieldName,index);
        if (filter!="") {
            //dbSuspend(model->db,false,true,model->label);
            model->relationModel(index.column())->setFilter(filter);
            //dbSuspend(model->db,true,true,model->label);
        }
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


        return comboBox;
    } else if (sDataType=="REAL"){
        return new QLineEdit(parent);
    } else if (sDataType.startsWith("BOOL")){
        return new QLineEdit(parent);
    } else if (sDataType=="DATE"){
        QDateEdit *dateEdit = new QDateEdit(parent);
        dateEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
        //dateEdit->setDisplayFormat("yyyy-MM-dd");
        dateEdit->setDate(QDate::currentDate());
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
