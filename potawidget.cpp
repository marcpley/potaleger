#include "potawidget.h"
#include "Dialogs.h"
#include "data/Data.h"
#include "qapplication.h"
#include "qheaderview.h"
#include "qlineedit.h"
#include "qmenu.h"
#include "qsettings.h"
#include "qshortcut.h"
#include "qsqlerror.h"
#include "PotaUtils.h"
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QList>
#include <Qt>
#include <QLabel>
#include "potagraph.h"
#include "qtimer.h"
#include <QFileDialog>
#include <QWidgetAction>
#include <QScrollBar>

PotaWidget::PotaWidget(QWidget *parent) : QWidget(parent)
{
    twParent=dynamic_cast<QTabWidget*>(parent);
    model=new PotaTableModel();
    model->setParent(this);

    delegate=new PotaItemDelegate();
    delegate->setParent(this);
    //query=new PotaQuery(nullptr);
    lTabTitle=new QLabel();
    lTabTitle->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    //Toolbar
    toolbar=new QWidget(this);

    pbRefresh=new QToolButton(this);
    pbRefresh->setIcon(QIcon(":/images/reload.svg"));
    pbRefresh->setShortcut( QKeySequence(Qt::Key_F5));
    pbRefresh->setToolTip(tr("Recharger les données depuis le fichier.")+"\n"+
                          tr("Les modifications en cours seront automatiquement enregistrées")+"\n"+
                          "F5");
    connect(pbRefresh, &QToolButton::released, this, &PotaWidget::pbRefreshClick);

    pbEdit=new QToolButton(this);
    pbEdit->setIcon(QIcon(":/images/edit.svg"));
    pbEdit->setCheckable(true);
    pbEdit->setChecked(false);
    pbEdit->setShortcut( QKeySequence(Qt::Key_F2));
    pbEdit->setToolTip(tr("Basculer le mode édition.")+"\n"+
                          "F2");
    connect(pbEdit, &QToolButton::released, this, &PotaWidget::pbEditClick);

    pbCommit=new QToolButton(this);
    pbCommit->setIcon(QIcon(":/images/commit.svg"));
    // Associer plusieurs raccourcis
    aCommit=new QAction(pbCommit);
    aCommit->setShortcuts({QKeySequence(Qt::CTRL | Qt::Key_Enter), QKeySequence(Qt::CTRL | Qt::Key_Return)});
    QObject::connect(aCommit, &QAction::triggered, pbCommit, &QToolButton::click);
    pbCommit->addAction(aCommit);
    pbCommit->setToolTip(tr("Enregistrer les modifications en cours dans la BDD.")+"\n"+
                            "Ctrl + Enter");
    pbCommit->setEnabled(false);
    pbCommit->setVisible(false);
    connect(pbCommit, &QToolButton::released, this, &PotaWidget::pbCommitClick);

    pbRollback=new QToolButton(this);
    pbRollback->setIcon(QIcon(":/images/rollback.svg"));
    pbRollback->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Escape));
    pbRollback->setToolTip(tr("Abandonner les modifications en cours.")+"\n"+
                         "Ctrl + Escape");
    pbRollback->setEnabled(false);
    pbRollback->setVisible(false);
    connect(pbRollback, &QToolButton::released, this, &PotaWidget::pbRollbackClick);

    //Add delete rows
    sbInsertRows=new QSpinBox(this);
    sbInsertRows->setMinimum(1);
    sbInsertRows->setValue(1);
    sbInsertRows->setEnabled(false);
    sbInsertRows->setVisible(false);

    pbInsertRow=new QToolButton(this);
    pbInsertRow->setIcon(QIcon(":/images/insert_row.svg"));
    pbInsertRow->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Insert));
    pbInsertRow->setToolTip(tr("Ajouter des lignes.")+"\n"+
                               "Ctrl + Insert");
    connect(pbInsertRow, &QToolButton::released, this, &PotaWidget::pbInsertRowClick);
    pbInsertRow->setEnabled(false);
    pbInsertRow->setVisible(false);

    pbDuplicRow=new QToolButton(this);
    pbDuplicRow->setIcon(QIcon(":/images/duplicate_row.svg"));
    pbDuplicRow->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_D));
    pbDuplicRow->setToolTip(tr("Dupliquer la ligne courante.")+"\n"+
                               "Ctrl + D");
    connect(pbDuplicRow, &QToolButton::released, this, &PotaWidget::pbDuplicRowClick);
    pbDuplicRow->setEnabled(false);
    pbDuplicRow->setVisible(false);


    pbDeleteRow=new QToolButton(this);
    pbDeleteRow->setIcon(QIcon(":/images/delete_row.svg"));
    pbDeleteRow->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Delete));
    pbDeleteRow->setToolTip(tr("Supprimer des lignes.")+"\n"+
                            "Ctrl + Delete (Suppr)");
    connect(pbDeleteRow, &QToolButton::released, this, &PotaWidget::pbDeleteRowClick);
    pbDeleteRow->setEnabled(false);
    pbDeleteRow->setVisible(false);
    lToDelete=new QLabel(this);
    lToDelete->setVisible(false);


    //Filter tool
    filterFrame=new QFrame(this);
    filterFrame->setFrameShape(QFrame::NoFrame);
    filterFrame->setBackgroundRole(QPalette::Midlight);
    filterFrame->setAutoFillBackground(true);
    lFilterOn=new QLabel(this);
    lFilterOn->setText("...");
    cbFilterType=new QComboBox(this);
    connect(cbFilterType, &QComboBox::currentIndexChanged,this, &PotaWidget::cbFilterTypeChanged);
    leFilter=new QLineEdit(this);
    connect(leFilter, &QLineEdit::returnPressed, this, &PotaWidget::leFilterReturnPressed);
    pbFilter=new QPushButton(this);
    pbFilter->setText(tr("Filtrer"));
    pbFilter->setCheckable(true);
    pbFilter->setEnabled(false);
    pbFilter->setShortcut(QKeySequence(Qt::Key_F6));
    pbFilter->setToolTip("F6");
    connect(pbFilter, &QPushButton::clicked,this,&PotaWidget::pbFilterClick);
    lFilterResult=new QLabel(this);

    //Find tool
    findFrame=new QFrame(this);
    findFrame->setFrameShape(QFrame::NoFrame);
    findFrame->setBackgroundRole(QPalette::Midlight);
    findFrame->setAutoFillBackground(true);
    lFind=new QLabel(this);
    lFind->setText(tr("Rechercher"));
    leFind=new QLineEdit(this);
    connect(leFind, &QLineEdit::textEdited, this, &PotaWidget::leFindTextEdited);
    connect(leFind, &QLineEdit::returnPressed, this, &PotaWidget::leFindReturnPressed);
    pbFindFirst=new QPushButton(this);
    pbFindFirst->setText(tr("1er"));
    pbFindFirst->setEnabled(false);
    pbFindFirst->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_F3));
    pbFindFirst->setToolTip("Ctrl + F3");
    connect(pbFindFirst, &QPushButton::clicked,this,&PotaWidget::pbFindFirstClick);
    pbFindNext=new QPushButton(this);
    pbFindNext->setText(tr("Suivant"));
    pbFindNext->setEnabled(false);
    pbFindNext->setShortcut( QKeySequence(Qt::Key_F3));
    pbFindNext->setToolTip("F3");
    connect(pbFindNext, &QPushButton::clicked,this,&PotaWidget::pbFindNextClick);
    pbFindPrev=new QPushButton(this);
    pbFindPrev->setText(tr("Précédent"));
    pbFindPrev->setEnabled(false);
    pbFindPrev->setShortcut( QKeySequence(Qt::Key_F4));
    pbFindPrev->setToolTip("Ctrl + F4");
    connect(pbFindPrev, &QPushButton::clicked,this,&PotaWidget::pbFindPrevClick);

    //Page filter tool
    pageFilterFrame=new QFrame(this);
    pageFilterFrame->setFrameShape(QFrame::NoFrame);
    pageFilterFrame->setBackgroundRole(QPalette::Midlight);
    pageFilterFrame->setAutoFillBackground(true);
    pageFilterFrame->setVisible(false);
    lPageFilter=new QLabel(this);
    lPageFilter->setText("...");
    cbPageFilter=new QComboBox(this);
    connect(cbPageFilter, &QComboBox::currentIndexChanged,this, &PotaWidget::cbPageFilterChanged);
    //cbPageFilter->setFixedHeight(26);


    ffFrame=new QFrame(this);
    ffFrame->setVisible(true);

    lRowSummary=new QLabel(this);
    lRowSummary->setMinimumSize(375,lRowSummary->height());
    lRowSummary->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    lRowSummary->setTextInteractionFlags(Qt::TextSelectableByMouse);
    //lRowSummary->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    lSelect=new QLabel(this);
    lSelect->setText("");
    lSelect->setTextInteractionFlags(Qt::TextBrowserInteraction);//Qt::TextSelectableByMouse &
    lSelect->setOpenExternalLinks(false);
    SetFontWeight(lSelect,QFont::Weight::Bold);

    connect(lSelect, &QLabel::linkActivated, this, &PotaWidget::showSelInfo);

    tv=new PotaTableView();
    tv->setParent(this);
    tv->setModel(model);
    tv->setItemDelegate(delegate);
    auto *header=new PotaHeaderView( Qt::Horizontal, tv);
    //header->setParent(tv);
    tv->setHorizontalHeader(header);
    //tv->setLocale(QLocale::C ); ???
    //tv->setGridStyle()

    connect(tv->selectionModel(), &QItemSelectionModel::currentChanged, this, &PotaWidget::curChanged);
    connect(tv->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PotaWidget::selChanged);
    connect(model, &PotaTableModel::dataChanged, this, &PotaWidget::dataChanged);
    connect(tv->verticalHeader(), &QHeaderView::sectionClicked,this, &PotaWidget::headerRowClicked);
    //connect(tv->verticalScrollBar(), &QScrollBar::valueChanged, this, &PotaWidget::onVBarValueChanged, Qt::UniqueConnection);

    editNotes=new QTextEdit();
    editNotes->setParent(tv);
    editNotes->setVisible(false);
    editNotes->setReadOnly(true);
    QPalette palette=editNotes->palette();
    //palette.setColor(QPalette::Base, delegate->cTableColor);
    palette.setColor(QPalette::Base, QApplication::palette().color(QPalette::ToolTipBase));
    palette.setColor(QPalette::Text, QApplication::palette().color(QPalette::ToolTipText));
    editNotes->setPalette(palette);

    aSaveNotes=new QAction(editNotes);
    aSaveNotes->setShortcuts({QKeySequence(Qt::CTRL | Qt::Key_Enter), QKeySequence(Qt::CTRL | Qt::Key_Return)});
    connect(aSaveNotes, &QAction::triggered, [this]() {
        toogleReadOnlyEditNotes(tv->currentIndex());
    });
    aSaveNotes->setEnabled(false);
    tv->addAction(aSaveNotes);

    QAction* aEscapeNotes=new QAction(editNotes);
    aEscapeNotes->setShortcut(Qt::Key_Escape);
    connect(aEscapeNotes, &QAction::triggered, [this]() {
        if (editNotes->isVisible() and !editNotes->isReadOnly())
            toogleReadOnlyEditNotes(tv->currentIndex(),true);
    });
    tv->addAction(aEscapeNotes);

    editSelInfo=new QTextEdit();
    editSelInfo->setParent(this);
    editSelInfo->setVisible(false);
    editSelInfo->setReadOnly(true);

    toolbarCV=new QWidget(this);
    toolbarCV->setVisible(false);

    pbRefreshCV=new QToolButton(this);
    pbRefreshCV->setIcon(QIcon(":/images/reload.svg"));
    pbRefreshCV->setShortcut( QKeySequence(Qt::Key_F5));
    pbRefreshCV->setToolTip(pbRefresh->toolTip());
    connect(pbRefreshCV, &QToolButton::released, this, &PotaWidget::pbRefreshClick);

    pbCloseCV=new QPushButton(this);
    pbCloseCV->setText(tr("Fermer"));
    //pbCloseCV->setEnabled(false);
    pbCloseCV->setShortcut( QKeySequence(Qt::Key_Escape));
    pbCloseCV->setToolTip("Echap");
    connect(pbCloseCV, &QPushButton::clicked,[this]() {
        toolbarCV->setVisible(false);
        chartView->setVisible(false);
        //if (graph)

        //graph->removeAllSeries();
        // QChart* chart=chartView->chart();
        // chart->removeAllSeries();
        // for (auto axis : chart->axes()) {
        //     chart->removeAxis(axis);
        //     delete axis;
        // }
        //chartView->setChart(nullptr); No : QChart bug.
        //delete currentChart;

        toolbar->setVisible(true);
        ffFrame->setVisible(true);
        tv->setVisible(true);

        graph->deleteLater();
        graph=nullptr;
        lw->removeWidget(chartView);
        chartView->deleteLater();
        chartView=nullptr;
    });

    helpCV=new QToolButton(this);
    helpCV->setIcon(QIcon(":/images/help.svg"));
    helpCV->setToolTip(tr("Zoom : roulette de la souris\n"
                          "Déplacement horizontal : roulette + ctrl"));


    //Toolbar layout
    ltb=new QHBoxLayout(this);
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
    ltb->addWidget(pbDuplicRow);
    ltb->addSpacing(5);
    ltb->addWidget(pbDeleteRow);
    ltb->addWidget(lToDelete);
    ltb->addSpacing(10);
    ltb->addWidget(lRowSummary);
    ltb->addSpacing(10);
    ltb->addWidget(lSelect);
    toolbar->setLayout(ltb);

    //Filter layout
    filterLayout=new QHBoxLayout(this);
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
    findLayout=new QHBoxLayout(this);
    findLayout->setSizeConstraint(QLayout::SetFixedSize);
    findLayout->setContentsMargins(5,3,5,3);
    findLayout->setSpacing(5);
    findLayout->addWidget(lFind);
    findLayout->addWidget(leFind);
    findLayout->addWidget(pbFindFirst);
    findLayout->addWidget(pbFindNext);
    findLayout->addWidget(pbFindPrev);
    findFrame->setLayout(findLayout);

    //PageFilter layout
    pageFilterLayout=new QHBoxLayout(this);
    pageFilterLayout->setSizeConstraint(QLayout::SetFixedSize);
    pageFilterLayout->setContentsMargins(5,3,5,3);
    pageFilterLayout->setSpacing(5);
    pageFilterLayout->addWidget(lPageFilter);
    pageFilterLayout->addWidget(cbPageFilter);
    pageFilterFrame->setLayout(pageFilterLayout);

    //Filter find layout
    ffLayout=new QHBoxLayout(this);
    ffLayout->setSizeConstraint(QLayout::SetFixedSize);
    ffLayout->setContentsMargins(0,0,0,0);
    ffLayout->addWidget(filterFrame);
    ffLayout->addSpacing(5);
    ffLayout->addWidget(findFrame);
    ffLayout->addSpacing(5);
    ffLayout->addWidget(pageFilterFrame);
    ffFrame->setLayout(ffLayout);

    //Chart toolbar layout
    ltbCV=new QHBoxLayout(this);
    ltbCV->setSizeConstraint(QLayout::SetMaximumSize);
    ltbCV->setContentsMargins(2,2,2,2);
    ltbCV->setSpacing(0);
    ltbCV->addWidget(pbRefreshCV);
    ltbCV->addSpacing(10);
    ltbCV->addWidget(pbCloseCV);
    ltbCV->addSpacing(10);
    ltbCV->addStretch();
    ltbCV->addWidget(helpCV);
    toolbarCV->setLayout(ltbCV);

    //Main layout
    lw=new QVBoxLayout(this);
    lw->setContentsMargins(2,2,2,2);
    lw->setSpacing(2);
    lw->addWidget(toolbar);
    lw->addWidget(ffFrame);
    lw->addWidget(tv);
    lw->addWidget(toolbarCV);
    // lw->addWidget(chartView);
    setLayout(lw);
}

void PotaWidget::Init(QString TableName)
{
    //AppBusy(true,model->progressBar,16,"Init");

    model->setTable(TableName);

    PotaQuery query(*model->db);
    PotaQuery query2(*model->db);


    qInfo() << "Open "+TableName+iif(TableName!=model->RealTableName()," ("+model->RealTableName()+")","").toString();

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);//OnFieldChange

    model->sRowSummary=RowSummaryModel(model->db,TableName);

    model->progressBar->setValue(model->progressBar->value()+1);

    //Primary Key
    query.ExecShowErr("PRAGMA table_xinfo("+model->RealTableName()+")");
    while (query.next()){
        if (query.value(5).toInt()==1) {
            model->sPrimaryKey=query.value(1).toString();
            break;
        }
    }

    model->progressBar->setValue(model->progressBar->value()+1);

    //FK
    query.ExecShowErr("PRAGMA foreign_key_list("+model->RealTableName()+");");
    while (query.next()) {
        QString referencedTable=query.value("table").toString();
        QString localColumn=query.value("from").toString();
        QString referencedClumn=query.value("to").toString();
        int localColumnIndex=model->fieldIndex(localColumn);

        if (localColumnIndex>-1){
            model->setRelation(localColumnIndex, QSqlRelation(referencedTable, referencedClumn, referencedClumn));//Issue #2
            model->relationModel(localColumnIndex)->setFilter(FkFilter(model->db,model->RealTableName(),localColumn,model->index(0,0)));
            model->relationModel(localColumnIndex)->setSort(model->relationModel(localColumnIndex)->fieldIndex(FkSortCol(model->db,model->RealTableName(),localColumn)),Qt::SortOrder::AscendingOrder);
        }
    }

    model->progressBar->setValue(model->progressBar->value()+1);

    //Generated columns

    query.clear();
    query.ExecShowErr("PRAGMA table_xinfo("+TableName+")");
    while (query.next()){
        if (query.value(6).toInt()==2)
            model->generatedColumns.insert(query.value(1).toString());
        model->dataTypes.append(DataType(model->db,TableName,query.value(1).toString()));
        model->baseDataFields.append(query2.Select0ShowErr("SELECT base_data FROM fda_f_schema "
                                                           "WHERE (name='"+model->RealTableName()+"')AND"
                                                                 "(field_name='"+query.value(1).toString()+"')").toString());
        model->progressBar->setValue(model->progressBar->value()+1);
    }

    model->setOrderBy(-1,Qt::SortOrder::AscendingOrder);

    tv->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    tv->verticalHeader()->setDefaultAlignment(Qt::AlignTop);
    tv->verticalHeader()->hide();
    tv->setTabKeyNavigation(false);
    // QString natSortCols=NaturalSortCol(model->db,TableName);
    // PotaHeaderView *phv=dynamic_cast<PotaHeaderView*>(tv->horizontalHeader());
    // phv->iSortCol=model->FieldIndex(natSortCols.split(',')[0]);
    // for (int i=0;i<natSortCols.split(',').count();i++)
    //     phv->model()->setHeaderData(model->FieldIndex(natSortCols.split(',')[i]), Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_BlueDown.svg")), Qt::DecorationRole);

    tv->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tv, &QTableView::customContextMenuRequested, this, &PotaWidget::showContextMenu);

    model->progressBar->setValue(model->progressBar->value()+1);

    //widget width according to font size
    SetSizes();
    //AppBusy(false,model->progressBar);
}

void PotaWidget::curChanged(const QModelIndex cur, const QModelIndex pre)
{
    if (bUserCurrChanged){

        tv->viewport()->update();
        //tv->clearSpans();//Force redraw of grid, for selected ligne visibility.

        // sbInsertRows->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
        // pbInsertRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
        // pbDuplicRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
        // pbDeleteRow->setEnabled(bAllowDelete and (model->rowsToInsert.count()==0));

        if (cur.row()>-1)
            lRowSummary->setText(RowSummary(model->db,model->tableName(),model->sRowSummary,model,cur.row()));
        else
            lRowSummary->setText("...");

        QString sFieldName=model->headerData(cur.column(),Qt::Horizontal,Qt::EditRole).toString();

        if (filterFrame->isVisible()) {
            if (!pbFilter->isChecked()) {
                //Filtering
                sFieldNameFilter=sFieldName;
                sDataTypeFilter=model->dataTypes[cur.column()];
                if (sDataTypeFilter=="DATE")
                    sDataFilter=model->index(cur.row(),cur.column()).data(Qt::DisplayRole).toString();
                else
                    sDataFilter=model->index(cur.row(),cur.column()).data(Qt::EditRole).toString();

                lFilterOn->setText(sFieldNameFilter);

                if(!pbCommit->isEnabled())
                    pbFilter->setEnabled(true);//Don't enable filtering if there is pending modifications, they would be lost.

                SetFilterTypeCombo(sDataTypeFilter);
                SetLeFilterWith(sFieldNameFilter,sDataTypeFilter,sDataFilter);
            }
        }


        if (pbEdit->isChecked() and !editNotes->isReadOnly()) {
            toogleReadOnlyEditNotes(pre);
        }
        editNotes->setReadOnly(true);
        aSaveNotes->setEnabled(false);
        aCommit->setEnabled(true);

        // if (AcceptReturns(model->db,model->tableName(),sFieldName)){
        if (model->dataTypes[cur.column()]=="TEXT") {
            QString text=model->data(cur,Qt::DisplayRole).toString();
            if (text.contains("\n")){
                SetVisibleEditNotes(true,model->nonEditableColumns.contains(cur.column()));
            } else {
                QFontMetrics fm(editNotes->font());
                SetVisibleEditNotes(fm.horizontalAdvance(text)>tv->columnWidth(cur.column())-3,model->nonEditableColumns.contains(cur.column()));
            }
        } else {
            SetVisibleEditNotes(false,false);
        }

        editSelInfo->setVisible(false);

        //Keep visible rows above and below the current line.
        int autoScroll = 0;
        if (tv->rowViewportPosition(cur.row())>tv->height()-7*tv->rowHeight(cur.row())) {
            autoScroll=1; //tv->verticalScrollBar()->value()+
        } else if (tv->rowViewportPosition(cur.row())<3*tv->rowHeight(cur.row())) {
            autoScroll=-1; //tv->verticalScrollBar()->value()
        }
        if (autoScroll!=0 and
            (autoScroll+tv->verticalScrollBar()->value()<=tv->verticalScrollBar()->maximum())and
            (autoScroll+tv->verticalScrollBar()->value()>=0)) { //and !editNotes->isVisible()
            if (editNotes->isVisible()) {
                //qDebug() << model->record(cur.row()).value(0).toString() << autoScroll << tv->rowHeight(cur.row());
                editNotes->move(editNotes->x(),editNotes->y()-autoScroll*tv->rowHeight(cur.row()));
            }
            autoScroll+=tv->verticalScrollBar()->value();

            QScrollBar *vbar = tv->verticalScrollBar();
            QTimer::singleShot(0, this, [vbar, autoScroll, this]() {
            vbar->setValue(autoScroll);
            tv->viewport()->update();
            });
        }
    }
}

int PotaWidget::exportToFile(QString sFileName, QString format, QString baseDataName) {
    QString sTableName=model->RealTableName();
    if (sTableName=="Temp_UserSQL") {
        QFileInfo fi(sFileName);
        sTableName=fi.baseName();
    }

    PotaTableModel *exportModel;
    if (!baseDataName.isEmpty()) { //Program export for reset base data.
        if (format!="csv")
            return -1;
        exportModel=new PotaTableModel();
        exportModel->setTable(baseDataName);
        //exportModel->setHeaderData()
        exportModel->select();
        qDebug() << "exportModel->rowCount()" << exportModel->rowCount();
        for (int col=0; col < exportModel->columnCount(); ++col) {
            exportModel->setHeaderData( col,Qt::Horizontal,exportModel->headerData(col,Qt::Horizontal,Qt::DisplayRole).toString(),Qt::EditRole);
        }
    } else {
        exportModel=model;
    }

    QByteArray data;
    QStringList dataTypes;
    PotaQuery pQuery(*model->db);
    QString decimalSep;
    QString header="";
    decimalSep=pQuery.Select0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='Export_sep_decim'").toString();
    if (decimalSep.isEmpty()) {
        QLocale locale;
        decimalSep=QString(locale.decimalPoint());
    }
    QString ColSep;
    if (format=="csv") {
        ColSep=pQuery.Select0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='Export_sep_col'").toString();
        if(ColSep.isEmpty())
            ColSep=";";
    } else {
        ColSep=",";
    }


    //Header export
    QModelIndexList selectedIndexes=tv->selectionModel()->selectedIndexes();
    QStringList selectedColumns;
    bool updateAllColumns=false;
    if (format=="INSERT") header.append("INSERT INTO "+sTableName+" (");
    else if (format=="UPDATE") {
        header.append("UPDATE "+sTableName+" SET ");
        for (const QModelIndex &index : selectedIndexes) { //todo: exporter les colonnes visibles.
            selectedColumns.append(model->headerData(index.column(),Qt::Horizontal,Qt::EditRole).toString());
        }
        updateAllColumns=(selectedColumns.count()==0 or
                          (selectedColumns.count()==1 and selectedColumns[0]==model->sPrimaryKey));
    }
    bool firstCol=true;
    for (int col=0; col < exportModel->columnCount(); ++col) {
        if (format=="csv") {
            if (!firstCol) header.append(ColSep);
            header.append(exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString());
            dataTypes.append(DataType(model->db,sTableName,exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString()));
            firstCol=false;
        } else if (!ReadOnly(model->db,sTableName,exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString())) {
            if (format=="INSERT") { //All not readonly columns
                if (!firstCol) header.append(ColSep);
                header.append(exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString());
                firstCol=false;
                dataTypes.append(DataType(model->db,sTableName,exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString()));
            } else {
                if ((updateAllColumns or selectedColumns.contains(exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString()))and
                    exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString()!=model->sPrimaryKey) //Not PK column
                    dataTypes.append(DataType(model->db,sTableName,exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString()));
                else
                    dataTypes.append(""); //Not export this column
            }
        } else {
            dataTypes.append(""); //Not export this column
        }
    }
    if (format=="csv")
        data.append(header.toUtf8()+"\n");
    else if (format=="INSERT")
        header.append(") VALUES (");

    int exportedRow=-1;
    int totalRow=exportModel->rowCount();
    QFile FileExport(sFileName);
    FileExport.open(QIODevice::WriteOnly);
    if (FileExport.write(data)!=-1) {
        data.clear();

        AppBusy(true,model->progressBar,totalRow,0,lTabTitle->text().trimmed()+" %p%");

        //Data export
        exportedRow=0;
        qDebug() << "exportModel->rowCount()" << exportModel->rowCount();
        for (int row=0; row < exportModel->rowCount(); ++row) {
            QString line="";
            QString pkValue;
            bool firstCol=true;
            if (format!="csv") line.append(header);
            for (int col=0; col < exportModel->columnCount(); ++col) {
                if (format=="csv") {
                    if (!firstCol) line.append(ColSep);
                    if (dataTypes[col]=="REAL")
                        line.append(EscapeCSV(StrReplace(exportModel->data(exportModel->index(row, col),Qt::EditRole).toString(),".",decimalSep),ColSep));
                    else
                        line.append(EscapeCSV(exportModel->data(exportModel->index(row, col),Qt::EditRole).toString(),ColSep));
                    firstCol=false;
                } else if (dataTypes[col]!="") {
                    if (!firstCol) line.append(ColSep);
                    if (exportModel->data(exportModel->index(row, col),Qt::EditRole).isNull())
                        line.append(iif(format=="UPDATE",exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString()+"=","").toString()+
                                    "NULL");
                    else if (dataTypes[col]=="REAL" or dataTypes[col].startsWith("INT"))
                        line.append(iif(format=="UPDATE",exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString()+"=","").toString()+
                                    exportModel->data(exportModel->index(row, col),Qt::EditRole).toString());
                    else
                        line.append(iif(format=="UPDATE",exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString()+"=","").toString()+
                                    EscapeSQL(exportModel->data(exportModel->index(row, col),Qt::EditRole).toString()));
                    firstCol=false;
                }
                if (format=="UPDATE" and model->sPrimaryKey==exportModel->headerData(col,Qt::Horizontal,Qt::EditRole).toString()) {
                    if (dataTypes[col]=="REAL" or dataTypes[col].startsWith("INT"))
                        pkValue=exportModel->data(exportModel->index(row, col),Qt::EditRole).toString();
                    else
                        pkValue=EscapeSQL(exportModel->data(exportModel->index(row, col),Qt::EditRole).toString());
                }
            }
            if (format=="INSERT") line.append(");");
            else if (format=="UPDATE") line.append(" WHERE "+model->sPrimaryKey+"="+pkValue+";");
            data.append(line.toUtf8()+"\n");
            if (FileExport.write(data)!=-1)
                exportedRow+=1;
            data.clear();
            model->progressBar->setValue(row);
        }

        FileExport.close();
        AppBusy(false,model->progressBar);
    }
    if (!baseDataName.isEmpty()) { //Program export for reset base data.
        exportModel->clear();
        delete exportModel;
        exportModel = nullptr;
    }

    return exportedRow;
}

void PotaWidget::exportData() {
    QSettings settings;
    QString PathExport=settings.value("PathExport").toString();
    if (PathExport.isEmpty())
        PathExport=settings.value("PathImport").toString();

    QString selectedFilter;
    QString sFileName=QFileDialog::getSaveFileName(this, tr("Exporter les données dans un fichier %1").arg("CSV"),
                                                     PathExport+lTabTitle->text().trimmed(),  "*.csv;;SQL INSERT (*.sql);;SQL UPDATE (*.sql)",
                                                     &selectedFilter,QFileDialog::DontConfirmOverwrite);
    //Check filename.
    if (sFileName.endsWith("."))
        sFileName.removeLast();
    if (sFileName.isEmpty()) return;
    QString format;
    if (selectedFilter=="*.csv") {
        if (StrLast(sFileName.toLower(),4)!=".csv") sFileName+=".csv";
        format="csv";
    } else {
        if (StrLast(sFileName.toLower(),4)!=".sql") sFileName+=".sql";
        if (selectedFilter=="SQL INSERT (*.sql)")
            format="INSERT";
        else
            format="UPDATE";
    }

    QFileInfo FileInfoVerif;
    FileInfoVerif.setFile(sFileName);
    if (!FileInfoVerif.exists() or
        OkCancelDialog("Potaléger",tr("Le fichier existe déjà")+"\n"+
                           sFileName+"\n"+
                           FileInfoVerif.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfoVerif.size()/1000)+" ko\n\n"+
                           tr("Remplacer ?"),QStyle::SP_MessageBoxWarning,600)) {
        QFile FileInfo2;
        if (FileInfoVerif.exists()) {
            FileInfo2.setFileName(sFileName);
            if (!FileInfo2.remove()) {
                MessageDlg("Potaléger",tr("Impossible de supprimer le fichier")+"\n"+
                                  sFileName,"",QStyle::SP_MessageBoxCritical);
                return;
            }
        }

        PathExport=FileInfoVerif.absolutePath()+QDir::separator();

        //Export
        QFile FileExport(sFileName);
        if (!FileExport.open(QIODevice::WriteOnly)) {// | QIODevice::Text
            MessageDlg("Potaléger",tr("Impossible de créer le fichier")+"\n"+
                              sFileName,"",QStyle::SP_MessageBoxCritical);
            return;
        }

        int totalRow=model->rowCount();
        int exportedRow=exportToFile(sFileName,format);

        settings.setValue("PathExport",PathExport);

        if (exportedRow==totalRow) {
            MessageDlg("Potaléger",tr("%1 lignes exportées vers le fichier").arg(str(totalRow))+"\n"+
                              sFileName,"",QStyle::SP_MessageBoxInformation);
        } else
            MessageDlg("Potaléger",tr("%1 sur %2 lignes exportées vers le fichier").arg(str(exportedRow)).arg(str(totalRow))+"\n"+
                              sFileName,"",QStyle::SP_MessageBoxWarning);
    }
}

void PotaWidget::importData() {
    if (pbCommit->isEnabled()) {
        if (YesNoDialog("Potaléger",lTabTitle->text().trimmed()+"\n\n"+
                        tr("Valider les modifications en cours ?"))) {
            if (!model->SubmitAllShowErr())
                return;
        } else {
            if (!model->RevertAllShowErr())
                return;
        }
    } else if (!pbEdit->isChecked()){
        return;
    }

    QSettings settings;
    QString PathImport=settings.value("PathImport").toString();

    if (PathImport.isEmpty())
        PathImport=settings.value("PathExport").toString();;

    QString sFileName=QFileDialog::getOpenFileName(this, tr("Importer des données"),
                                                     PathImport+lTabTitle->text().trimmed(), "*.csv");

    //Check filename.
    if (sFileName.isEmpty()) return;

    importCSV(sFileName);
}

void PotaWidget::importCSV(QString sFileName, QString enableFields, bool enableResetTable, bool baseData) {
    QFileInfo FileInfoVerif;
    FileInfoVerif.setFile(sFileName);
    QString PathImport=FileInfoVerif.absolutePath()+QDir::separator();

    QFile FileImport(sFileName);
    if (!FileImport.open(QIODevice::ReadOnly)) {
        MessageDlg("Potaléger",tr("Impossible d'ouvrir le fichier")+"\n"+
                   sFileName,"",QStyle::SP_MessageBoxCritical);
        return;
    }

    QByteArray ba = FileImport.readAll();

    QString data,info,info2;
    QStringList lines,linesToImport,fieldNames,dataTypes,valuesToImport;
    QList<int> fieldindexes;
    data = QString::fromUtf8(ba);
    //data.append(FileImport.readAll().toString());
    lines=data.split("\n");
    qDebug() << "data" << data.count();
    qDebug() << "lines" << lines.count();

    //Header check
    fieldNames=lines[0].split(";");
    int nbColImport=fieldNames.count();
    int primaryFieldImport=-1;
    info="";
    for (int col=0; col < nbColImport; ++col) {
        fieldindexes.append(model->FieldIndex(fieldNames[col]));
        if (fieldindexes[col]==-1 or//Column don't exists in the table.
            model->nonEditableColumns.contains(fieldindexes[col]) or//Column is readonly
            (!enableFields.isEmpty() and !enableFields.contains(fieldNames[col]))) {//Column must not be imported (not base data) .
            dataTypes.append("");
            fieldindexes[col]=-1;
        } else {
            dataTypes.append(DataType(model->db, model->tableName(),fieldNames[col]));
            info+=iif(info.isEmpty(),"",", ").toString()+fieldNames[col]+" ("+dataTypes[col]+")";
        }
        if(fieldNames[col]==model->sPrimaryKey) {
            PotaQuery query(*model->db);
            if (query.Select0ShowErr("SELECT field_type FROM fda_f_schema "
                                     "WHERE (name='"+model->RealTableName()+"')AND"
                                           "(field_name='"+fieldNames[col]+"')").toString()!="AUTOINCREMENT")
                primaryFieldImport=col;
        }
    }

    QSet<int> disabledChoices;
    QSettings settings;
    int TypeImport=0;
    TypeImport=settings.value("TypeImport").toInt();
    if (primaryFieldImport==-1) {
        TypeImport=4; //Append only
        disabledChoices.insert(0);
        disabledChoices.insert(1);
        disabledChoices.insert(2);
        disabledChoices.insert(3);
    }
    if (!enableResetTable) {
        disabledChoices.insert(5);
        disabledChoices.insert(6);
    }

    int choice=-1;
    if (info.isEmpty()) {
        MessageDlg("Potaléger",QObject::tr("Aucun champ dans le fichier %1 n'est modifiable dans l'onglet %2.")
                          .arg(FileInfoVerif.fileName())
                          .arg(lTabTitle->text().trimmed()),"",QStyle::SP_MessageBoxWarning);
        return;
    } else if (lines.count()<2) {
        MessageDlg("Potaléger",QObject::tr("Aucune ligne à importer dans le fichier %1.").arg(FileInfoVerif.fileName()),"",QStyle::SP_MessageBoxWarning);
        return;
    }
    //Concat lines for each records.
    parseCSV(data,"\n",linesToImport);
    linesToImport.removeFirst();//Remove header
    info2="";
    for (int i=0;i<4;i++){
        if (i<linesToImport.count())
            info2+="<br>"+
                     StrFirst(linesToImport[i],60)+iif(linesToImport[i].length()>60,"...","").toString();
    }
    if(linesToImport.count()>4)
        info2+="<br>...";

    choice=RadiobuttonDialog("Potaléger",lTabTitle->text().trimmed()+"<br><br>"+
                                   iif(baseData,
                                       tr("Réinitialiser les données de base."),
                                       "<b>"+tr("Importer des données depuis un fichier %1.").arg("CSV")+"</b><br>"+
                                       sFileName).toString()+"<br><br>"+
                                   "<b>"+tr("Les champs suivants vont être peuvent être mis à jour:")+"</b><br>"+info+"<br><br>"+
                                   "<b>"+tr("%1 lignes à importer:").arg(linesToImport.count()-1)+"</b>"+info2+"<br>"+
                                   tr("<u>Fusionner</u>: les lignes déjà présentes seront mises à jour, les autres seront créées.")+"<br>"+
                                   tr("<u>Mettre à jour</u>: seules les lignes visibles seront mises à jour, aucune nouvelle ligne ne sera créée.")+"<br>"+
                                   tr("<u>Supprimer</u>: les lignes visibles seront supprimées si elles ne sont pas utilisées ailleurs)."),
                               {tr("Fusionner, priorité aux données importées"),                               //0
                                tr("Fusionner, priorité aux données déjà présentes"),                          //1
                                tr("Mettre à jour, priorité aux données importées"),                           //2
                                tr("Mettre à jour, priorité aux données déjà présentes"),                      //3
                                tr("Ajouter les lignes absentes, ne pas modifier les lignes déjà présentes"),  //4
                                tr("Supprimer puis importer, priorité aux données importées"),                 //5
                                tr("Supprimer puis importer, priorité aux données déjà présentes")},TypeImport,disabledChoices,true);//6
    if (choice==-1) return;

    TypeImport=choice;

    if (primaryFieldImport==-1 and TypeImport!=4) {
        MessageDlg("Potaléger",QObject::tr("Champ %1 non trouvée dans le fichier %2.\nSeul l'ajout de ligne est éventuellement possible.").arg(model->sPrimaryKey).arg(FileInfoVerif.fileName()),"",QStyle::SP_MessageBoxWarning);
        return;
    }

    int TypeValid=0;
    TypeValid=settings.value("TypeImportValid").toInt();

    choice=RadiobuttonDialog("Potaléger",lTabTitle->text().trimmed()+"<br><br>"+
                                   iif(baseData,
                                       tr("Réinitialiser les données de base."),
                                       "<b>"+tr("Importer des données depuis un fichier %1.").arg("CSV")+"</b><br>"+
                                       sFileName).toString()+"<br><br>"+
                                   //"<b>"+tr("Les champs suivants vont être importés:")+"</b><br>"+info+"<br><br>"+
                                   //"<b>"+tr("%1 lignes à importer:").arg(linesToImport.count()-1)+"</b>"+info2+"<br>"+
                                   tr("<u>Enregistrer au fur et à mesure</u>: permet d'importer une partie des données malgrè des erreurs.")+"<br>"+
                                   tr("<u>Ne rien enregistrer</u>: permet de visualiser les changements avant de les valider."),
                               {tr("Enregistrer les modifications valides au fur et à mesure"),                           //0
                                tr("Ne rien enregistrer, je tenterais une validation globale à la fin")},TypeValid);//1
    if (choice==-1) return;
    TypeValid=choice;

    int nbDeletedRows=0;
    int nbCreatedRows=0;
    int nbModifiedRows=0;
    int nbErrors=0;
    int nbUndeletebaleRows=0;
    bool bSubmit=(TypeValid==0);
    QLocale locale;
    QString decimalSep=QString(locale.decimalPoint());
    if (TypeImport==5 or TypeImport==6) {//Delete selected lines
        if(model->rowCount()>1 and
            !OkCancelDialog("Potaléger",tr("Attention, %1 lignes sont susceptibles d'être supprimées!").arg(model->rowCount()),QStyle::SP_MessageBoxWarning,600)) {
           //dbSuspend(&db,true,userDataEditing,ui->lDBErr);
            return;
        }
        AppBusy(true,model->progressBar,model->rowCount(),0,"Delete %p%");
        for (int i=model->rowCount()-1;i>=0;i--) {
            model->progressBar->setValue(model->progressBar->value()+1);
            if(model->childCount(i)==0) {
                if(model->removeRow(i) and (!bSubmit or model->submitAll()))
                    nbDeletedRows++;
                else {
                    qInfo() << "Import (delete) "+model->data(model->index(i,primaryFieldImport)).toString()+" : "+model->lastError().text();
                    model->revertAll();
                    nbErrors++;
                }
            } else {
                    nbUndeletebaleRows++;
            }
        }
        AppBusy(false,model->progressBar);
    }

    //Remove filter
    if (TypeImport!=2 and TypeImport!=3 and pbFilter->isChecked()) {
        pbFilterClick(false);
        pbFilter->setChecked(false);
    }
    //Edit mode
    if (!pbEdit->isChecked()) {
        pbEdit->setChecked(true);
        pbEditClick();
    }


    //Import
    AppBusy(true,model->progressBar,linesToImport.count(),0,FileInfoVerif.fileName()+" %p%");

    model->bBatch=true;

    bool bModified;

    for(int i=0;i<linesToImport.count();i++) {
        model->progressBar->setValue(i);
        if (!linesToImport[i].isEmpty()) {
            parseCSV(linesToImport[i],";",valuesToImport);

            int recordToUpdate=-1;
            if(primaryFieldImport>-1 and valuesToImport.count()>primaryFieldImport and !valuesToImport[primaryFieldImport].isEmpty()){
                //Search existing record
                for (int i=0;i<model->rowCount();i++) {
                    if (model->data(model->index(i,0),Qt::EditRole).toString()==valuesToImport[primaryFieldImport]){
                        recordToUpdate=i;
                        break;
                    }
                }
            }
            if(recordToUpdate>-1){
                if(TypeImport!=4){
                    //Update existing record.
                    bModified=false;
                    for (int col=0; col < valuesToImport.count(); col++) {
                        if (fieldindexes[col]>-1){//Col exists in table.
                            if(dataTypes[col]=="REAL")
                                valuesToImport[col]=StrReplace(valuesToImport[col],decimalSep,".");
                            if (TypeImport==0 or TypeImport==2 or TypeImport==5 or//Priority to imported data
                                model->data(model->index(recordToUpdate,fieldindexes[col]),Qt::EditRole).toString()=="") {
                                if (model->data(model->index(recordToUpdate,fieldindexes[col]),Qt::EditRole).toString()!=valuesToImport[col]){
                                    model->setData(model->index(recordToUpdate,fieldindexes[col]),valuesToImport[col]);
                                    bModified=true;
                                }
                            }
                        }
                    }

                    if (bModified) {
                        if(!bSubmit or model->submitAll()) {
                            nbModifiedRows++;
                        } else {
                            qInfo() << "Import (update) "+linesToImport[i]+" : "+model->lastError().text();
                            model->revertAll();
                            nbErrors++;
                        }
                    }
                }
            } else {
                if(TypeImport!=2 and TypeImport!=3){
                    //Create new record.
                    int row=model->rowCount();
                    if (model->insertRow(row)){
                        for (int col=0; col < valuesToImport.count(); col++) {
                            if (fieldindexes[col]>-1){//Col exists in table.
                                if(dataTypes[col]=="REAL")
                                    model->setData(model->index(row,fieldindexes[col]),StrReplace(valuesToImport[col],decimalSep,"."));
                                else
                                    model->setData(model->index(row,fieldindexes[col]),valuesToImport[col]);

                            }
                        }
                        if (!bSubmit or model->submitAll()) {
                            nbCreatedRows++;
                        } else {
                            qInfo() << "Import (create) "+linesToImport[i]+" : "+model->lastError().text();
                            model->revertAll();
                            nbErrors++;
                        }
                    } else {
                        qInfo() << "Import (insert) "+linesToImport[i]+" : "+model->lastError().text();
                        nbErrors++;
                    }
                }
            }
        }
    }
    model->bBatch=false;
    AppBusy(false,model->progressBar);

    if (bSubmit)
        model->SubmitAllShowErr();//To deactivate commit and rollback buttons, and show modified cells.

    settings.setValue("PathImport",PathImport);
    settings.setValue("TypeImport",TypeImport);
    settings.setValue("TypeImportValid",TypeValid);

    MessageDlg("Potaléger",QObject::tr("%1 lignes supprimées").arg(nbDeletedRows)+"\n"+
                           QObject::tr("%1 lignes non supprimables").arg(nbUndeletebaleRows)+"\n"+
                           QObject::tr("%1 lignes créées").arg(nbCreatedRows)+"\n"+
                           QObject::tr("%1 lignes modifiées").arg(nbModifiedRows)+"\n"+
                           QObject::tr("%1 erreurs").arg(nbErrors));
}

void PotaWidget::resetBaseData() {
    if (pbCommit->isEnabled()) {
        if (YesNoDialog("Potaléger",lTabTitle->text().trimmed()+"\n\n"+
                        tr("Valider les modifications en cours ?"))) {
            if (!model->SubmitAllShowErr())
                return;
        } else {
            if (!model->RevertAllShowErr())
                return;
        }
    } else if (!pbEdit->isChecked()){
        return;
    }

    //Read fda schema
    PotaQuery query(*model->db);
    query.exec("SELECT name,field_name FROM fda_f_schema WHERE (name='"+model->RealTableName()+"')AND(base_data='x')"); //,field_type,base_data
    bool bResetTable=false;
    QString fieldNames="";
    while (query.next()){
        if (query.value(1).isNull()) bResetTable=true;
        else fieldNames+=query.value(1).toString()+";";
    }

    //Extract SQL statements for base data
    int exportedRow=-2;
    QString sBaseData=loadSQLFromResource("CreateBaseData");
    QString sQuerys;
    QString tempBaseData="Base_data_"+QDateTime::currentDateTime().toString("hhmmss");
    sQuerys="CREATE TEMP TABLE "+tempBaseData+" AS SELECT * FROM "+model->RealTableName()+" WHERE false;";
    QStringList QuerysList;
    QuerysList=sBaseData.split("\n");
    int iBaseData=0;
    for(int i=0;i<QuerysList.count();i++) {
        if (QuerysList[i].startsWith("INSERT INTO "+model->RealTableName())) {
            sQuerys+=QuerysList[i].replace("INSERT INTO "+model->RealTableName()+" ","INSERT INTO temp."+tempBaseData+" ");
            iBaseData++;
        }
    }

    //Create temp CSV file with base data.
    QFileInfo file(model->db->databaseName());
    QString fileName=file.absolutePath()+QDir::toNativeSeparators("/Temp_Base_data.csv");
    QFile csvFile;
    if (query.ExecMultiShowErr(sQuerys,";",model->progressBar)) {
        csvFile.setFileName(fileName);
        csvFile.remove();

        int iBaseData2=query.Select0ShowErr("SELECT count() FROM "+tempBaseData).toInt();

        if (iBaseData2!=iBaseData) {
            MessageDlg("Potaléger",tr("Erreur %1 lors de la collecte des données de base.").arg("RowCount")+"\n"+
                        QString::number(iBaseData)+" in CreateBaseData.sql\n"+
                        QString::number(iBaseData2)+" ready to import\n"+
                        fileName,"",QStyle::SP_MessageBoxCritical);
            return;
        }

        exportedRow=exportToFile(fileName,"csv",tempBaseData);
    }

    if (exportedRow==-2) {
        MessageDlg("Potaléger",tr("Erreur %1 lors de la collecte des données de base.").arg("SQL"),"",QStyle::SP_MessageBoxCritical);
        return;
    } else if (exportedRow==-1) {
        MessageDlg("Potaléger",tr("Erreur %1 lors de la collecte des données de base.").arg("CSV")+"\n"+
                    fileName,"",QStyle::SP_MessageBoxCritical);
        return;
    }

    //Import CSV dialog
    importCSV(fileName,fieldNames,bResetTable,true);

    csvFile.remove(fileName);
}

void PotaWidget::selChanged() {
    if (bUserCurrChanged){
        //Count, sum, etc about selection.
        QModelIndexList selectedIndexes=tv->selectionModel()->selectedIndexes();
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
                sumSelected+=selectedIndexes[i].data(Qt::EditRole).toFloat(&bOk);
                if (bOk){
                    if (nbOkSelected==0) {
                        minSelected=selectedIndexes[i].data(Qt::EditRole).toFloat();
                        maxSelected=selectedIndexes[i].data(Qt::EditRole).toFloat();
                    } else {
                        minSelected=min(minSelected,selectedIndexes[i].data(Qt::EditRole).toFloat());
                        maxSelected=fmax(maxSelected,selectedIndexes[i].data(Qt::EditRole).toFloat());
                    }
                    nbOkSelected++;
                }
                if (selectedIndexes[i].data(Qt::EditRole).toString().isEmpty()) {
                    nbEmptySelected++;
                } else {
                    if (!sl.contains(selectedIndexes[i].data(Qt::EditRole).toString()))
                        sl.append(selectedIndexes[i].data(Qt::EditRole).toString());
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

void PotaWidget::showContextMenu(const QPoint& pos) {
    QMenu contextMenu("Context menu", this);

    QAction mDefColWidth(QIcon::fromTheme("object-flip-horizontal"),tr("Largeurs de colonnes par défaut"), this);
    //QAction mEditNotes(tr("Editer texte muti-lignes"), this);
    QAction mGraficView(QIcon::fromTheme("utilities-system-monitor"),tr("Graphique..."), this);
    QAction mExport(tr("Exporter les données..."), this);
    QAction mImport(tr("Importer des données..."), this);
    QAction mResetBaseData(QIcon::fromTheme("system-reboot"),tr("Réinitialiser les données de base (🔺️)..."), this);

    QModelIndex index=tv->indexAt(pos);
    QString sFieldName=model->headerData(index.column(),Qt::Horizontal,Qt::EditRole).toString();

    if (!index.isValid()) {
        mDefColWidth.setEnabled(false);
    }

    mExport.setIcon(QIcon(TablePixmap(model->db,model->tableName(),"  >>")));
    mImport.setIcon(QIcon(TablePixmap(model->db,model->tableName(),">>  ")));
    mImport.setEnabled(pbInsertRow->isEnabled() and pbEdit->isChecked());
    mResetBaseData.setEnabled(pbEdit->isChecked());
    PotaQuery query(*model->db);
    mResetBaseData.setVisible((query.Select0ShowErr("SELECT count() FROM fda_f_schema "
                                                    "WHERE (name='"+model->RealTableName()+"')AND(base_data='x')").toInt()>0)and
                              (query.Select0ShowErr("SELECT count() FROM fda_t_schema "
                                                    "WHERE (name='"+model->tableName()+"')AND(tbl_type IN('Table','View as table'))").toInt()>0));

    connect(&mDefColWidth, &QAction::triggered, this, &PotaWidget::hDefColWidth);
    connect(&mGraficView, &QAction::triggered, this, &PotaWidget::showGraphDialog);
    connect(&mExport, &QAction::triggered, this, &PotaWidget::exportData);
    connect(&mImport, &QAction::triggered, this, &PotaWidget::importData);
    connect(&mResetBaseData, &QAction::triggered, this, &PotaWidget::resetBaseData);

    //Vivible columns menu entries.
    QMenu *VisibleColMenu = contextMenu.addMenu(QIcon::fromTheme("preferences-system"), tr("Colonnes visibles"));
    if (true) {
        QWidgetAction *allWa = new QWidgetAction(VisibleColMenu);
        QCheckBox *allCb = new QCheckBox(tr("Toutes"), VisibleColMenu);
        allWa->setDefaultWidget(allCb);
        VisibleColMenu->addAction(allWa);
        VisibleColMenu->addSeparator();

        bool allVisible = true;
        for (int col = 0; col < model->columnCount(); ++col) {
            QWidgetAction *wa = new QWidgetAction(VisibleColMenu);
            QCheckBox *cb = new QCheckBox(model->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString(), VisibleColMenu);
            cb->setChecked(!tv->isColumnHidden(col));
            if (tv->isColumnHidden(col)) allVisible = false;
            connect(cb, &QCheckBox::toggled, this, [this, col, VisibleColMenu, allCb, allWa](bool checked){
                PotaWidget::setVisibleColumn(col, checked);
                // Check if all columns are checked
                bool every = true;
                for (QAction *a : VisibleColMenu->actions()) {
                    if (a == allWa) continue;
                    QWidgetAction *w = qobject_cast<QWidgetAction*>(a);
                    if (!w) continue;
                    QCheckBox *c = qobject_cast<QCheckBox*>(w->defaultWidget());
                    if (c && !c->isChecked()) { every = false; break; }
                }
                allCb->blockSignals(true);
                allCb->setChecked(every);
                allCb->blockSignals(false);
            });

            wa->setDefaultWidget(cb);
            VisibleColMenu->addAction(wa);
        }

        connect(allCb, &QCheckBox::toggled, this, [VisibleColMenu, allWa](bool checked){
            for (QAction *a : VisibleColMenu->actions()) {
                if (a == allWa) continue;
                QWidgetAction *w = qobject_cast<QWidgetAction*>(a);
                if (!w) continue;
                QCheckBox *c = qobject_cast<QCheckBox*>(w->defaultWidget());
                if (!c) continue;
                c->setChecked(checked); // Fire the menu action.
            }
        });
        allCb->setChecked(allVisible);
    }

    contextMenu.addAction(&mDefColWidth);
    contextMenu.addMenu(VisibleColMenu);
    contextMenu.addAction(&mGraficView);
    contextMenu.addAction(&mExport);
    contextMenu.addAction(&mImport);
    contextMenu.addAction(&mResetBaseData);

    QFont font=contextMenu.font();
    font.setPointSize(cbFontSize->currentText().toInt());
    contextMenu.setFont(font);

    contextMenu.exec(tv->viewport()->mapToGlobal(pos));
}

void PotaWidget::hDefColWidth() {
    for (int i=0; i<model->columnCount();i++) {
        int iWidth=DefColWidth(model->db, model->tableName(),model->headerData(i,Qt::Horizontal,Qt::EditRole).toString());
        if (iWidth<=0 or iWidth>500)
            tv->resizeColumnToContents(i);
        else
            tv->setColumnWidth(i,iWidth);
    }
}

void PotaWidget::toogleReadOnlyEditNotes(const QModelIndex index, bool discard) {
    if (pbEdit->isChecked() and
        !model->nonEditableColumns.contains(index.column())) {
        if (editNotes->isReadOnly()) { //Go to notes edit mode
            //Move to keep unvalidated user input
            QModelIndex mi=tv->currentIndex();
            if (tv->currentIndex().column()+1<model->columnCount())
                tv->setCurrentIndex(model->index(tv->currentIndex().row(),tv->currentIndex().column()+1));
            else if (tv->currentIndex().column()>0)
                tv->setCurrentIndex(model->index(tv->currentIndex().row(),tv->currentIndex().column()-1));
            tv->setCurrentIndex(mi);

            editNotes->setReadOnly(false);
            aSaveNotes->setEnabled(true);
            aCommit->setEnabled(false);
        } else if (discard) {//Discard changes and return to notes read mode
            editNotes->setReadOnly(true);
            tv->setFocus();
            aSaveNotes->setEnabled(false);
            aCommit->setEnabled(true);
        } else { //Save data and return to notes read mode
            editNotes->setReadOnly(true);
            aSaveNotes->setEnabled(false);
            aCommit->setEnabled(true);
            QString save=editNotes->toPlainText().trimmed().replace("\n\n","\n").replace("\n","\n\n");
            int i=editNotes->toPlainText().count("<");
            editNotes->setMarkdown(editNotes->toPlainText().trimmed());
            if (i != editNotes->toMarkdown().count("<")) {
                //qDebug() << save;
                editNotes->setPlainText(save);
                editNotes->setReadOnly(false);
                aSaveNotes->setEnabled(true);
                aCommit->setEnabled(false);
                MessageDlg(tr("Editeur"),tr("Les balises HTML ne sont pas accéptées."));
            } else {
                if (save!=model->data(index).toString())
                    model->setData(index,save);
                tv->setFocus();
            }
        }
        SetVisibleEditNotes(true,false);
    }
}

void PotaWidget::setVisibleColumn(int col, bool visible) {
    tv->setColumnHidden(col,!visible);
    //qDebug() << col << visible;
}

void PotaWidget::showGraphDialog()
{
    QStringList columns,dataTypes;
    bool user=false;
    for (int i=0; i < model->columnCount(); ++i) {
        columns.append(model->headerData(i, Qt::Horizontal,Qt::DisplayRole).toString());
        QString sDataType=DataType(model->db, model->tableName(),model->headerData(i, Qt::Horizontal,Qt::EditRole).toString());
        dataTypes.append(sDataType);
    }

    if (sGraph.count()==0) {
        sGraph=GraphDialog(tr("Graphique sur '%1'").arg(lTabTitle->text()),
                                       tr("Indiquez quels champs utiliser pour les abscisses et les ordonnées.")+"\n\n"+
                                       tr("Ce graphique sera enregistré et utilisable sur cet ordinateur uniquement."),
                                       columns,dataTypes);
        user=true;
    }

    if (sGraph.count()==23 and sGraph[0].toInt()>=0 and sGraph[0].toInt()<dataTypes.count() and
        !graph and !chartView) {
        //chartView->setChart(new PotaGraph(model, sGraph[0].toInt(),model->dataTypes[sGraph[0].toInt()] ,sGraph[1].toInt(),sGraph[2].toInt(),sGraph[3].toInt()));
        chartView=new PotaChartView();
        chartView->setParent(this);
        chartView->setVisible(false);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setContentsMargins(0,0,0,0);
        lw->addWidget(chartView);
        graph=new PotaGraph();
        graph->setParent(chartView);
        chartView->setChart(graph);

        graph->m_xAxisFieldNum=sGraph[0].toInt();
        graph->m_xAxisDataType=model->dataTypes[graph->m_xAxisFieldNum];
        graph->m_xAxisGroup=sGraph[1].toInt();
        graph->m_xAxisYearSeries=(sGraph[2].toInt()==1);
        const int start=3;
        for (int i=0;i<graph->seriesCount;i++) {

            graph->m_yAxisFieldNum[i]=sGraph[i*5+start].toInt();
            // if (graph->m_yAxisFieldNum[i]>=0 and graph->m_yAxisFieldNum[i]<dataTypes.count())
            //     graph->m_yAxisDataType[i]=model->dataTypes[graph->m_yAxisFieldNum[i]];
            graph->m_yAxisCalc[i]=sGraph[i*5+start+1].toInt();
            graph->m_yAxisType[i]=sGraph[i*5+start+2].toInt();
            qDebug() << "111";
            qDebug() << sGraph[i*5+start+2];
            qDebug() << sGraph[i*5+start+3];
            qDebug() << sGraph[i*5+start+4];
            qDebug() << QColor(sGraph[i*5+start+3]);
            if (QColor(sGraph[i*5+start+3]).isValid())
                graph->m_yColor[i]=QColor(sGraph[i*5+start+3]);
            graph->m_yRightAxis[i]=(sGraph[i*5+start+4].toInt()==1);

            if (graph->m_xAxisYearSeries) break;
        }

        graph->createSeries(model);
        graph->fillSeries(model);

        if(isDarkTheme())
            chartView->chart()->setTheme(QChart::ChartThemeDark);
        else
            chartView->chart()->setTheme(QChart::ChartThemeLight);

        toolbar->setVisible(false);
        ffFrame->setVisible(false);
        tv->setVisible(false);
        pbCloseCV->setVisible(user);
        toolbarCV->setVisible(true);
        chartView->setVisible(true);

        sGraph.clear();
    }
}

void PotaWidget::showSelInfo() {
    editSelInfo->setGeometry(lSelect->geometry().left(),
                             lSelect->geometry().top(),
                             400,200);
    editSelInfo->setVisible(true);
}

void PotaWidget::SetVisibleEditNotes(bool bVisible, bool autoSize){
    QColor tc=delegate->cTableColor;
    if (bVisible){
        int EditNotesWidth=fmax(tv->columnWidth(tv->currentIndex().column()),400);
        int EditNotesHeight=200;

        if (editNotes->isReadOnly()) {
            editNotes->setMarkdown(model->data(tv->currentIndex(),Qt::DisplayRole).toString());
            tc.setAlpha(80);
        } else {
            editNotes->setPlainText(model->data(tv->currentIndex(),Qt::DisplayRole).toString().replace("\n\n","\n"));
            tc.setAlpha(0);
            editNotes->setFocus();
        }
        QPalette palette=editNotes->palette();
        palette.setColor(QPalette::Base, blendColors(tv->palette().color(QPalette::Base),tc));
        palette.setColor(QPalette::Text, tv->palette().color(QPalette::Text));
        editNotes->setPalette(palette);

        if (editNotes->isReadOnly()) { //auto size
            //editNotes->document()->setTextWidth(QWIDGETSIZE_MAX);
            QFontMetrics fm(editNotes->font());
            int maxWidth=0;
            int returns=0;
            QStringList lines=editNotes->toPlainText().split('\n');

            for (const QString &line : lines) {
                int lineWidth=fm.horizontalAdvance(line);
                if (lineWidth > EditNotesWidth)
                    returns+=floor(lineWidth/EditNotesWidth);
                if (lineWidth > maxWidth)
                    maxWidth=lineWidth;
            }
            // editNotes->setFixedSize(min(maxWidth+50,400),
            //                         min(lines.count()*22+5,200));
            EditNotesWidth=min((maxWidth*1.2)+50,EditNotesWidth);
            EditNotesHeight=min((lines.count()+returns)*22/10*cbFontSize->currentText().toInt()+7,200);
            //qDebug() << "autosize";
        }
        int x=tv->columnViewportPosition(tv->currentIndex().column())+5;
        int y=tv->rowViewportPosition(tv->currentIndex().row())+
              tv->horizontalHeader()->height()+
              tv->rowHeight(tv->currentIndex().row());
        x=min(x,tv->width()-EditNotesWidth-20);

        //EditNotesHeight=EditNotesHeight/10*cbFontSize->currentText().toInt();
        if (y+EditNotesHeight>tv->height()) { //Not enought room below
            if (tv->height()-y>100) {
                EditNotesHeight=tv->height()-y;
            } else {
                //Move EditNotes above curent line.
                y=y-EditNotesHeight-tv->rowHeight(tv->currentIndex().row());
                if (y<0) {//Not enought room above
                    EditNotesHeight=EditNotesHeight+y;
                    y=0;
                }
            }
        }
        editNotes->setGeometry(x,y,EditNotesWidth,EditNotesHeight);

        editNotes->setVisible(true);
    } else {
        editNotes->setVisible(false);
        tv->setFocus();
    }
}

void PotaWidget::PositionSave() {
    //iPositionCol=tv->selectionModel()->currentIndex().column();
    iPositionCol=tv->currentIndex().column();

    //Normal row to retreive.
    sPositionRow="";
    sPositionRowPrev="";
    sPositionRowNext="";
    int iStart=iif(tv->isColumnHidden(0),1,0).toInt();
    for (int i=iStart;i<iStart+3;i++) {
        sPositionRow+=model->index(tv->selectionModel()->currentIndex().row(),i).data(Qt::DisplayRole).toString();
        if (tv->selectionModel()->currentIndex().row()>0)
            sPositionRowPrev+=model->index(tv->selectionModel()->currentIndex().row()-1,i).data(Qt::DisplayRole).toString();
        if (tv->selectionModel()->currentIndex().row()<tv->model()->rowCount()-1)
            sPositionRowNext+=model->index(tv->selectionModel()->currentIndex().row()+1,i).data(Qt::DisplayRole).toString();
    }

    qDebug() << "PositionSave: " << sPositionRow;
}

void PotaWidget::PositionRestore() {
    int iStart=iif(tv->isColumnHidden(0),1,0).toInt();
    int rowRestore=-1;
    for (int row=0;row<model->rowCount();row++) {
        if (sPositionRow==model->index(row,iStart+0).data(Qt::DisplayRole).toString()+
                          model->index(row,iStart+1).data(Qt::DisplayRole).toString()+
                          model->index(row,iStart+2).data(Qt::DisplayRole).toString()) {
            //Normal row retreived with 3 1st columns
            rowRestore=row;
            break;
        } else if (sPositionRowNext==model->index(row,iStart+0).data(Qt::DisplayRole).toString()+
                                     model->index(row,iStart+1).data(Qt::DisplayRole).toString()+
                                     model->index(row,iStart+2).data(Qt::DisplayRole).toString()) {
            rowRestore=row;
            //break;
        } else if (sPositionRowPrev==model->index(row,iStart+0).data(Qt::DisplayRole).toString()+
                                     model->index(row,iStart+1).data(Qt::DisplayRole).toString()+
                                     model->index(row,iStart+2).data(Qt::DisplayRole).toString()) {
            rowRestore=row;
        } else if (sPositionRow.contains(model->index(row,iStart+0).data(Qt::DisplayRole).toString()+
                                         model->index(row,iStart+1).data(Qt::DisplayRole).toString())) {
            rowRestore=row;
        }
    }
    if (rowRestore>-1)
        tv->setCurrentIndex(model->index(rowRestore,iPositionCol));
    else if (lastRow(model->db,model->tableName()))
        tv->setCurrentIndex(model->index(model->rowCount()-1,1));

    lRowSummary->setText(RowSummary(model->db,model->tableName(),model->sRowSummary,model,tv->currentIndex().row()));
    lSelect->setText("");
}

void PotaWidget::RefreshHorizontalHeader() {
    PotaHeaderView *phv=dynamic_cast<PotaHeaderView*>(tv->horizontalHeader());
    //delegate->PaintedCols.clear();
    delegate->PaintedColsTitles.clear();
    delegate->PaintedColsTypes.clear();
    PotaQuery query(*model->db);
    int saison=query.Select0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='Année_culture'").toInt();
    for (int i=0; i<model->columnCount();i++)             {
        delegate->PaintedColsTitles.append("");
        delegate->PaintedColsTypes.append("");
        if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString().startsWith("TEMPO")){
            //delegate->PaintedCols.insert(i);
            delegate->PaintedColsTypes[i]="Tempo";
            if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()!="TEMPO"){//Multiple TEMPO columns
                if (StrLast(model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),2)=="NP") {
                    delegate->PaintedColsTitles[i]=str(saison-1);
                    if (QDate::currentDate().toString("yyyy").toInt()==saison-1)
                        delegate->PaintedColsTypes[i]="TempoNow";
                } else if (StrLast(model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),2)=="NN") {
                    delegate->PaintedColsTitles[i]=str(saison+1);
                    if (QDate::currentDate().toString("yyyy").toInt()==saison+1)
                        delegate->PaintedColsTypes[i]="TempoNow";
                } else {
                    delegate->PaintedColsTitles[i]=str(saison);
                    if (QDate::currentDate().toString("yyyy").toInt()==saison)
                        delegate->PaintedColsTypes[i]="TempoNow";
                }
                if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()!="TEMPO")
                    phv->setSectionResizeMode(i, QHeaderView::Fixed);
            }
        // } else if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString().startsWith("Prod_N")) {
        //     delegate->PaintedColsTypes[i]="TitleRed";
        //     if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Prod_Nm1")
        //         delegate->PaintedColsTitles[i]=str(saison-1);
        //     else if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Prod_N")
        //         delegate->PaintedColsTitles[i]=str(saison);
        //     else if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Prod_Np1")
        //         delegate->PaintedColsTitles[i]=str(saison+1);
        } else if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString().startsWith("Fert_N")) {
            delegate->PaintedColsTypes[i]="TitleRed";
            if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Fert_Nm1_pc")
                delegate->PaintedColsTitles[i]=str(saison-1);
            else if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Fert_Nm2_pc")
                delegate->PaintedColsTitles[i]=str(saison-2);
            else if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Fert_Nm3_pc")
                delegate->PaintedColsTitles[i]=str(saison-3);
            else
                delegate->PaintedColsTitles[i]=str(saison);
        // } else if (model->headerData(i,Qt::Horizontal,Qt::EditRole).toString().startsWith("Couv_N")) {
        //     delegate->PaintedColsTypes[i]="Title";
        //     delegate->PaintedColsTitles[i]="Couv";
        }
    }
}

void PotaWidget::SetSizes() {
    int UserFont=cbFontSize->currentText().toInt();
    int ButtonSize=24*UserFont/10;
    pbRefresh->setFixedSize(ButtonSize,ButtonSize);
    pbRefresh->setIconSize(QSize(ButtonSize,ButtonSize));
    pbEdit->setFixedSize(ButtonSize,ButtonSize);
    pbEdit->setIconSize(QSize(ButtonSize,ButtonSize));
    pbCommit->setFixedSize(ButtonSize,ButtonSize);
    pbCommit->setIconSize(QSize(ButtonSize,ButtonSize));
    pbRollback->setFixedSize(ButtonSize,ButtonSize);
    pbRollback->setIconSize(QSize(ButtonSize,ButtonSize));
    sbInsertRows->setFixedSize(1.5*ButtonSize,1.1*ButtonSize);
    //sbInsertRows->setIconSize(QSize(ButtonSize,ButtonSize));
    pbInsertRow->setFixedSize(ButtonSize,ButtonSize);
    pbInsertRow->setIconSize(QSize(ButtonSize,ButtonSize));
    pbDuplicRow->setFixedSize(ButtonSize,ButtonSize);
    pbDuplicRow->setIconSize(QSize(ButtonSize,ButtonSize));
    pbDeleteRow->setFixedSize(ButtonSize,ButtonSize);
    pbDeleteRow->setIconSize(QSize(ButtonSize,ButtonSize));

    lFilterOn->setFixedWidth(80*UserFont/10);
    cbFilterType->setFixedWidth(125*UserFont/10);
    leFilter->setFixedWidth(85*UserFont/10);
    pbFilter->setFixedWidth(70*UserFont/10);
    lFilterResult->setFixedWidth(80*UserFont/10);
    leFind->setFixedWidth(80*UserFont/10);
    pbFindFirst->setFixedWidth(40*UserFont/10);
    pbFindNext->setFixedWidth(70*UserFont/10);
    pbFindPrev->setFixedWidth(70*UserFont/10);
    cbPageFilter->setFixedWidth(120*UserFont/10);
    tv->horizontalHeader()->setMinimumHeight(24*UserFont/10);
    tv->horizontalHeader()->setMaximumHeight(24*UserFont/10);
    tv->verticalHeader()->setDefaultSectionSize(0);//Mini.

    pbRefreshCV->setFixedSize(ButtonSize,ButtonSize);
    pbRefreshCV->setIconSize(QSize(ButtonSize,ButtonSize));
    helpCV->setFixedSize(ButtonSize,ButtonSize);
    helpCV->setIconSize(QSize(ButtonSize,ButtonSize));

    QColor c=delegate->cTableColor;
    if (model->tableName().startsWith("Cultures"))
        c=cCulture;
    lTabTitle->setStyleSheet(QString(
                             "background-color: rgba(%1, %2, %3, %4);"
                             "font-weight: bold;"
                             "font-size: "+str(UserFont)+";"
                             )
                             .arg(c.red())
                             .arg(c.green())
                             .arg(c.blue())
                             .arg(60));
}

void PotaWidget::SetLeFilterWith(QString sFieldName, QString sDataType, QString sData){
    //Filtering
    //QString sDataType=DataType(model->db, model->tableName(),sDataName);
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
    //QString sDataType=DataType(model->db, model->tableName(),sDataName);
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
        cbFilterType->addItem(tr("sup. ou=à"));
        cbFilterType->addItem(tr("inf. ou=à"));
        cbFilterType->addItem(tr("différent de"));
        cbFilterType->addItem(tr("est du mois"));
        cbFilterType->addItem(tr("est de l'année"));
        cbFilterType->setCurrentIndex(iTypeDate);
    } else if (sDataType=="REAL" or sDataType.startsWith("INT")) {
        cbFilterType->addItem(tr("égal à"));
        cbFilterType->addItem(tr("supérieur à"));
        cbFilterType->addItem(tr("sup. ou=à"));
        cbFilterType->addItem(tr("inférieur à"));
        cbFilterType->addItem(tr("inf. ou=à"));
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
    cbPageFilter->setEnabled(false);
    twParent->setTabIcon(twParent->indexOf(this),QIcon(":/images/toCommit.svg"));

    if (!topLeft.data(Qt::EditRole).isNull() and
        (topLeft.data(Qt::EditRole)==""))
        model->setData(topLeft,QVariant(),Qt::EditRole);//To avoid empty non null values.
}

// void PotaWidget::onVBarValueChanged(int value) {
//     // if (bUserCurrChanged and editNotes->isVisible())
//     //     SetVisibleEditNotes(false,false);
// }

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

    bool PaintedCols=false;
    for (int i=0;i<delegate->PaintedColsTypes.count();i++){
        if (delegate->PaintedColsTypes[i]!="") {
            PaintedCols=true;
            break;
        }
    }
    if (PaintedCols) {
    //if(!delegate->PaintedCols.isEmpty()) {
        RefreshHorizontalHeader();
        emit model->headerDataChanged(Qt::Horizontal, 0, model->columnCount() - 1);
        tv->horizontalHeader()->update();
    }
    PositionRestore();
    bUserCurrChanged=true;

    PotaQuery query(*model->db);
    if (query.Select0ShowErr("SELECT Valeur='Oui' FROM Params WHERE Paramètre='Montrer_modifs'").toBool()) {
        if (!model->tempTableName.isEmpty())
            query.ExecShowErr("DROP TABLE IF EXISTS temp."+model->tempTableName+";");
        model->tempTableName="Temp_"+QDateTime::currentDateTime().toString("hhmmss")+model->tableName();
        query.ExecShowErr("CREATE TEMP TABLE "+model->tempTableName+" AS SELECT * FROM "+model->tableName());
        model->commitedCells.clear();
    }

    if (chartView and chartView->isVisible()){
        dynamic_cast<PotaGraph*>(chartView->chart())->createSeries(model);
        dynamic_cast<PotaGraph*>(chartView->chart())->fillSeries(model);
    }
}

void PotaWidget::pbEditClick(){
    if (pbEdit->isChecked()){
        pbCommit->setVisible(true);
        pbRollback->setVisible(true);
        sbInsertRows->setVisible(sbInsertRows->isEnabled());
        pbInsertRow->setVisible(pbInsertRow->isEnabled());
        pbDuplicRow->setVisible(pbDuplicRow->isEnabled());
        pbDeleteRow->setVisible(pbDeleteRow->isEnabled());
        model->nonEditableColumns.clear();
        for (int i=0; i<model->columnCount();i++) {
            if (ReadOnly(model->db, model->tableName(),model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()))
                model->nonEditableColumns.insert(i);
        }
        pbEdit->setIcon(QIcon(":/images/editOn.svg"));
        //mImport->setEnabled(pbInsertRow->isEnabled());
    } else {
        if (!pbCommit->isEnabled() or
            model->SubmitAllShowErr()){
            pbCommit->setVisible(false);
            pbRollback->setVisible(false);
            sbInsertRows->setVisible(false);
            pbInsertRow->setVisible(false);
            pbDuplicRow->setVisible(false);
            pbDeleteRow->setVisible(false);
            model->nonEditableColumns.clear();
            for (int i=0; i<model->columnCount();i++){
                model->nonEditableColumns.insert(i);
            }
            pbEdit->setIcon(QIcon(":/images/edit.svg"));
            twParent->setTabIcon(twParent->indexOf(this),QIcon(""));
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
    AppBusy(true);

    //Move to keep unvalidated user input
    QModelIndex mi=tv->currentIndex();
    if (tv->currentIndex().column()+1<model->columnCount())
        tv->setCurrentIndex(model->index(tv->currentIndex().row(),tv->currentIndex().column()+1));
    else if (tv->currentIndex().column()>0)
        tv->setCurrentIndex(model->index(tv->currentIndex().row(),tv->currentIndex().column()-1));
    tv->setCurrentIndex(mi);

    PositionSave();
    if (model->SubmitAllShowErr())
        PositionRestore();
    // sbInsertRows->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    // pbInsertRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    // pbDuplicRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    // pbDeleteRow->setEnabled(bAllowDelete and (model->rowsToInsert.count()==0));
    AppBusy(false);
    bUserCurrChanged=true;
}

void PotaWidget::pbRollbackClick()
{
    bUserCurrChanged=false;
    SetVisibleEditNotes(false,false);
    PositionSave();
    if (model->RevertAllShowErr()){
        //model->modifiedCells.clear();
        PositionRestore();
    }
    // sbInsertRows->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    // pbInsertRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    // pbDuplicRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    // pbDeleteRow->setEnabled(bAllowDelete and (model->rowsToInsert.count()==0));
    bUserCurrChanged=true;
}

void PotaWidget::pbInsertRowClick()
{
    model->InsertRowShowErr();
    //pbDeleteRow->setEnabled(bAllowDelete and (model->rowsToInsert.count()==0));

    tv->setFocus();
}

void PotaWidget::pbDuplicRowClick()
{
    model->DuplicRowShowErr();
    //pbDeleteRow->setEnabled(bAllowDelete and (model->rowsToInsert.count()==0));

    tv->setFocus();
}

void PotaWidget::pbDeleteRowClick()
{
    model->DeleteRowShowErr();
    // pbInsertRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    // sbInsertRows->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    // pbDuplicRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    tv->setFocus();
}

void PotaWidget::pbFilterClick(bool checked)
{
    //Filtering
    QString filter="";
    if (checked) {
        //Filter
        //QModelIndex index=tv->selectionModel()->currentIndex();

        QString sDataType=sDataTypeFilter;
        if (sDataType=="" or sDataType=="TEXT" or sDataType.startsWith("BOOL")) {
            if (leFilter->text()==""){
                if (cbFilterType->currentText()==tr("ne contient pas") or
                    cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" NOTNULL";
                else
                    filter=lFilterOn->text()+" ISNULL";
            } else {
                // QString raField="remove_accents("+lFilterOn->text()+")";
                QString raField=lFilterOn->text();
                if (cbFilterType->currentText()==tr("contient"))
                    filter=raField+" LIKE '%"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"%'";
                else if (cbFilterType->currentText()==tr("ne contient pas"))
                    filter="coalesce("+raField+",'') NOT LIKE '%"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"%'";
                else if (cbFilterType->currentText()==tr("égal à"))
                    filter=raField+" LIKE '"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("différent de"))
                    filter="coalesce("+raField+",'') NOT LIKE '"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("fini par"))
                    filter=raField+" LIKE '%"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"'";
                else//commence par
                    filter=raField+" LIKE '"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"%'";
            }
        } else if (sDataType=="DATE") {
            if (leFilter->text()==""){
                if (cbFilterType->currentText()==tr("sup. ou=à") or
                    cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" NOTNULL";
                else
                    filter=lFilterOn->text()+" ISNULL";
            } else {
                QDate date=QDate::fromString(leFilter->text(), "dd/MM/yyyy");
                QString dateString=date.toString("yyyy-MM-dd");
                if (cbFilterType->currentText()==tr("égal à"))
                    filter=lFilterOn->text()+"='"+dateString+"'";
                else if (cbFilterType->currentText()==tr("sup. ou=à"))
                    filter=lFilterOn->text()+" >= '"+dateString+"'";
                else if (cbFilterType->currentText()==tr("inf. ou=à"))
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
                else if (cbFilterType->currentText()==tr("sup. ou=à"))
                    filter=lFilterOn->text()+" >= '"+StrReplace(leFilter->text(),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("inférieur à"))
                    filter=lFilterOn->text()+" < '"+StrReplace(leFilter->text(),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("inf. ou=à"))
                    filter=lFilterOn->text()+" <= '"+StrReplace(leFilter->text(),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("différent de"))
                    filter=lFilterOn->text()+" != '"+StrReplace(leFilter->text(),"'","''")+"'";
                else if (cbFilterType->currentText().startsWith(StrFirst(tr("proche de (%1)").arg(" 5%"),10))){
                    float proche=StrFirst(StrLast(cbFilterType->currentText(),4),2).toInt();
                    filter=lFilterOn->text()+" BETWEEN "+str(leFilter->text().toFloat()*(1-proche/100))+" AND "+
                                                        str(leFilter->text().toFloat()*(1+proche/100));
                } else//égal à
                    filter=lFilterOn->text()+"='"+StrReplace(leFilter->text(),"'","''")+"'";
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

    if (pageFilterFrame->isVisible()) {
        QString pageFilter;
        //pageFilter=pageFilterField+"='"+cbPageFilter->currentText()+"'";
        pageFilter=pageFilterFilters[fmax(cbPageFilter->currentIndex(),0)];
        if (filter.isEmpty())
            filter=pageFilter;
        else
            filter="("+filter+")AND("+pageFilter+")";
    }

    tv->setFocus();

    bUserCurrChanged=false;
    PositionSave();
    model->setFilter(filter);
    PositionRestore();
    bUserCurrChanged=true;

    //lFilterResult->setText(str(model->rowCount())+" "+tr("lignes"));


}

void PotaWidget::cbFilterTypeChanged(int i){
    if (!bSetType) {//User selection
        //Filtering
        QModelIndex index=tv->selectionModel()->currentIndex();
        if (index.isValid()){
            QString sDataType=model->dataTypes[index.column()];
            if (sDataType=="" or sDataType=="TEXT") {
                iTypeText=i;
            } else if (sDataType=="DATE") {
                iTypeDate=i;
            } else if (sDataType=="REAL" or sDataType.startsWith("INT")){
                iTypeReal=i;
            }

            // qDebug() << "headerdata: "+model->headerData(index.column(),Qt::Horizontal,Qt::EditRole).toString();
            // qDebug() << "sFieldNameFilter: "+sFieldNameFilter;
            // qDebug() << "data: "+index.data(Qt::DisplayRole).toString();
            // qDebug() << "sDataFilter: "+sDataFilter;
            // if(model->headerData(index.column(),Qt::Horizontal,Qt::EditRole).toString()!=sFieldNameFilter){//sDataTypeFilter
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
    delegate->FindText=RemoveAccents(text);
    delegate->FindTextchanged=true;
}

void PotaWidget::leFindReturnPressed() {
    if (delegate->FindTextchanged) {
        if (pbFindFirst->isEnabled()) {
            pbFindFirstClick();
            delegate->FindTextchanged=false;
        }
    } else {
        if (pbFindNext->isEnabled())
            pbFindNextClick();
    }
}

void PotaWidget::FindFrom(int row, int column, bool Backward){
    SetFontColor(leFind,QColor());
    if (!Backward){
        for (int i=row;i<model->rowCount();i++) {
            for (int j=column;j<model->columnCount();j++) {
                if (!tv->isColumnHidden(j) and
                    RemoveAccents(model->index(i,j).data(Qt::DisplayRole).toString()).contains(RemoveAccents(leFind->text()),Qt::CaseInsensitive)){
                    tv->setCurrentIndex(model->index(i,j));
                    return;
                }
            }
            column=0;//Find in all columns since 2nd row
        }
    } else {
        for (int i=row;i>=0;i--) {
            for (int j=column;j>=0;j--) {
                if (!tv->isColumnHidden(j) and
                    RemoveAccents(model->index(i,j).data(Qt::DisplayRole).toString()).contains(RemoveAccents(leFind->text()),Qt::CaseInsensitive)){
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

void PotaWidget::cbPageFilterChanged(){
    if (pageFilterFrame->isVisible())
        pbFilterClick(pbFilter->isChecked());
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                           PotaTableModel
//////////////////////////////////////////////////////////////////////////////////////////////////

int PotaTableModel::FieldIndex(QString sFieldName){
    for (int i=0;i<columnCount();i++){
        if (headerData(i,Qt::Horizontal,Qt::EditRole).toString()==sFieldName)
            return i;
    }
    return -1;
}


QString PotaTableModel::FieldName(int index)
{
    QString sFieldName=headerData(index,Qt::Horizontal,Qt::EditRole).toString();
    if (sFieldName!="")
        return sFieldName;
    else {
        PotaQuery q(*db);
        q.ExecShowErr("PRAGMA table_xinfo("+tableName()+");");

        if (q.seek(index))
            return q.value("name").toString();
        else
            return "";
    }
}

int PotaTableModel::childCount(int row,QString childTable) {
    PotaQuery childTables(*db);
    PotaQuery query(*db);
    childTables.exec("SELECT name,field_name,master_field FROM fda_f_schema "
               "WHERE (tbl_type='Table')AND(master_table='"+RealTableName()+"')"+
               iif(!childTable.isEmpty(),"AND(name='"+childTable+"')","").toString());
    int result=0;
    while (childTables.next()) {
        const QString childTable = childTables.value(0).toString();
        const QString childField = childTables.value(1).toString();
        const QString masterField = childTables.value(2).toString();
        const int masterCol=FieldIndex(sPrimaryKey);
        result+=query.Select0ShowErr("SELECT count() FROM "+childTable+" WHERE "+childField+"='"+index(row,masterCol).data(Qt::EditRole).toString()+"'").toInt();
    }
    return result;
}

bool PotaTableModel::select()  {
    //return QSqlRelationalTableModel::select();
    //If use of QSqlRelationalTableModel select(), the generated columns and null FK value rows are not displayed. #FKNull

    //dbSuspend(db,false,true,label);

    if (!bBatch) {
        AppBusy(true,progressBar,-1,-1,tableName()+" - SELECT...");// +" %p%"
        //modifiedCells.clear();
        commitedCells.clear();
        //copiedCells.clear(); Danger
    }

    QString sQuery="SELECT * FROM "+tableName()+" TN";

    if (filter().toStdString()!="") {//Add filter
        sQuery+=" WHERE "+filter();
    }

    if (sOrderByClause.toStdString()!="")//Add order by
        sQuery+=" "+sOrderByClause;

    qInfo() << sQuery;

    //PotaQuery query(*db);
    //if (!bBatch) {
        // progressBar->setValue(1);
        // query.ExecShowErr("SELECT COUNT(*) FROM "+tableName()+" TN"+
        //                        iif(filter().toStdString()!=""," WHERE "+filter(),"").toString());
        //int totalRows=0;
        //if (query.next())
        //    totalRows=query.value(0).toInt();
        //progressBar->setMaximum(totalRows);
        //qDebug() << "totalRows " << totalRows;
    //}


    setLastError(QSqlError());

    QSqlRelationalTableModel::select();//Avoids duplicate display of inserted lines
    // if (!bBatch)
    //     progressBar->setValue(1);
    setQuery(sQuery);
    // if (!bBatch)
    //     progressBar->setValue(rowCount());

    while (canFetchMore()) {
        fetchMore();
        // if (!bBatch)
        //     progressBar->setValue(rowCount());
    }

    if (!bBatch) {
        qInfo() << str(rowCount())+" rows";//+tr("lignes");//Line necessary to make work lFilterResult->setText(...), don't know why!
        PotaWidget *pw=dynamic_cast<PotaWidget*>(parent());
        if (pw)
            pw->lFilterResult->setText(str(rowCount())+" "+tr("lignes"));
        AppBusy(false,progressBar);
    }
    bool result=(lastError().type()==QSqlError::NoError);
    //dbSuspend(db,true,!wasSuspended,label);

    if (!tempTableName.isEmpty() and
        !bBatch) {
        //Show modified cells
        AppBusy(true,progressBar,rowCount(),0,tableName()+" - Commited cells %p%");
        int jPrimaryKey=FieldIndex(sPrimaryKey);
        PotaQuery query(*db);
        query.prepare("SELECT * "
                      "FROM temp."+tempTableName+" "+
                      "WHERE "+sPrimaryKey+"=:pk");
        for (int i=0;i<rowCount();i++) {
            progressBar->setValue(i);
            query.bindValue(":pk",data(index(i,jPrimaryKey),Qt::EditRole).toString());
            query.exec();
            query.next();
            for (int j=0;j<columnCount();j++) {
                if (data(index(i,j),Qt::EditRole).toString()!=query.value(j).toString()){
                    commitedCells.insert(index(i,j));
                }
            }
        }
        AppBusy(false,progressBar);
    }

    return result;
}

void PotaTableModel::setOrderBy(int column,Qt::SortOrder so)  {
    QStringList noAccentFields={"Analyse","Destination","Espèce","Famille","Fertilisant","Fournisseur","IT_plante","Variété"};

    if (column<0) {//Natural sort
        if (noAccentFields.contains(FieldName(0)) and //Col 0 is noAccent
            FieldIndex(NaturalSortCol(db,RealTableName()).split(",")[0])==0)
            column=0; //Explicit sort on column 0 for accent removal
    }

    if (column<0) {
        sOrderByClause="";
        //qDebug() << "sOrderByClause vide" << FieldName(0);
    } else {
        QString sFieldName=FieldName(column);
        QList<QStringList> accentList={{"à","a"},{"â","a"},{"ä","a"},
                                       {"è","e"},{"é","e"},{"ê","e"},{"ë","e"},
                                       {"î","i"},{"ï","i"},
                                       {"ô","o"},{"ö","o"},
                                       {"ù","u"},{"û","u"},{"ü","u"},
                                       {"ç","c"}};
        if (noAccentFields.contains(sFieldName)) {//Sort on no accent field.
            for (int i=0;i<accentList.count();i++)
                sFieldName="replace("+sFieldName+",'"+accentList[i][0]+"','"+accentList[i][1]+"')";
        }

        sOrderByClause="ORDER BY "+sFieldName+" COLLATE NOCASE";

        if (column>0 and noAccentFields.contains(FieldName(0))) {
            sFieldName=FieldName(0);
            for (int i=0;i<accentList.count();i++)
                sFieldName="replace("+sFieldName+",'"+accentList[i][0]+"','"+accentList[i][1]+"')";
            sOrderByClause=sOrderByClause+","+sFieldName;
        }

        if (so==Qt::SortOrder::DescendingOrder)
            sOrderByClause+=" DESC";
        else
            sOrderByClause+=" ASC";
    }
    //select();
}

bool PotaTableModel::SelectShowErr()
{
    for (int i=0;i<columnCount();i++) {
        if (relationModel(i)){
            //relationModel(i)->select();//FkNull
            relationModel(i)->setQuery("SELECT * FROM "+relationModel(i)->tableName()+
                                       iif(relationModel(i)->filter().toStdString()!=""," WHERE "+relationModel(i)->filter(),"").toString());
            while (relationModel(i)->canFetchMore())
                relationModel(i)->fetchMore();
        }
    }


    setLastError(QSqlError());

    select();

    if ((lastError().type()==QSqlError::NoError)and(parent()->objectName().startsWith("PW"))) {
        SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,tableName(),"Ok");
        return true;
    } else {
        SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,tableName()+", "+lastError().text(),"Err");
        return false;
    }
}

bool PotaTableModel::SubmitAllShowErr() {
    PotaWidget *pw=dynamic_cast<PotaWidget*>(parent());
    setLastError(QSqlError());
    //dbSuspend(db,false,true,label);
    submitAll();
    if (lastError().type()==QSqlError::NoError) {
        SetColoredText(pw->lErr,tableName()+": "+tr("modifications enregistrées."),"Ok");
        pw->isCommittingError=false;
        pw->pbCommit->setEnabled(false);
        pw->pbRollback->setEnabled(false);
        pw->lToDelete->setVisible(false);
        pw->pbFilter->setEnabled(true);
        pw->cbPageFilter->setEnabled(true);
        pw->twParent->setTabIcon(pw->twParent->indexOf(pw),QIcon(""));
        //dbSuspend(db,true,true,label);
        return true;
    } else {
        SetColoredText(pw->lErr,lastError().text(),"Err");
        qDebug() <<  "SubmitAllShowErr" << lastError().text();
        pw->isCommittingError=true;
        //dbSuspend(db,true,true,label);
        return false;
    }
}

bool PotaTableModel::RevertAllShowErr() {
    PotaWidget *pw=dynamic_cast<PotaWidget*>(parent());
    setLastError(QSqlError());
    //dbSuspend(db,false,true,label);
    //pw->model->revert();
    revertAll();
    if (lastError().type()==QSqlError::NoError) {
        SetColoredText(pw->lErr,tableName()+": "+tr("modifications abandonnées."),"Info");
        pw->isCommittingError=false;
        pw->pbCommit->setEnabled(false);
        pw->pbRollback->setEnabled(false);
        pw->lToDelete->setVisible(false);
        pw->pbFilter->setEnabled(true);
        pw->cbPageFilter->setEnabled(true);
        pw->twParent->setTabIcon(pw->twParent->indexOf(pw),QIcon(""));
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
    PotaWidget *pw=dynamic_cast<PotaWidget*>(parent());
    if (insertRows(pw->tv->currentIndex().row()+1,//Create blank new rows
                   pw->sbInsertRows->value())) {
        for (int i=0;i<pw->sbInsertRows->value();i++) {
            int newRow=i+pw->tv->currentIndex().row()+1;
            for (int k = 0; k < rowsToInsert.size(); ++k) {
                if (rowsToInsert[k] >= newRow)
                    rowsToInsert[k]++;
            }
            for (int k = 0; k < rowsToRemove.size(); ++k) {
                if (rowsToRemove[k] >= newRow)
                    rowsToRemove[k]++;
            }
            rowsToInsert.append(newRow);
        }
        pw->tv->setCurrentIndex(index(pw->tv->currentIndex().row()+1,iif(pw->tv->isColumnHidden(0),1,0).toInt()));
        pw->pbCommit->setEnabled(true);
        pw->pbRollback->setEnabled(true);
        pw->pbFilter->setEnabled(false);
        pw->cbPageFilter->setEnabled(false);
        pw->twParent->setTabIcon(pw->twParent->indexOf(pw),QIcon(":/images/toCommit.svg"));
        if (pw->sbInsertRows->value()>5)
            pw->sbInsertRows->setValue(1);
    } else {
        SetColoredText(pw->lErr,"insertRows(x,y)","Err");
        return false;
    }
    return true;
}

bool PotaTableModel::DuplicRowShowErr()
{
    PotaWidget *pw=dynamic_cast<PotaWidget*>(parent());
    int sourceRow=pw->tv->currentIndex().row();
    if (insertRows(sourceRow+1,pw->sbInsertRows->value())) {
        for (int i=0;i<pw->sbInsertRows->value();i++) {
            for (int col=0; col < pw->model->columnCount(); ++col) {
                QModelIndex sourceIndex=index(sourceRow, col);
                QModelIndex destIndex=index(sourceRow+1+i, col);
                setData(destIndex, data(sourceIndex,Qt::EditRole));
            }
            int newRow=sourceRow+1;
            for (int k = 0; k < rowsToInsert.size(); ++k) {
                if (rowsToInsert[k] >= newRow)
                    rowsToInsert[k]++;
            }
            for (int k = 0; k < rowsToRemove.size(); ++k) {
                if (rowsToRemove[k] >= newRow)
                    rowsToRemove[k]++;
            }
            rowsToInsert.append(newRow);
        }
        pw->tv->setCurrentIndex(index(sourceRow+1,iif(pw->tv->isColumnHidden(0),1,0).toInt()));
        pw->pbCommit->setEnabled(true);
        pw->pbRollback->setEnabled(true);
        pw->pbFilter->setEnabled(false);
        pw->cbPageFilter->setEnabled(false);
        pw->twParent->setTabIcon(pw->twParent->indexOf(pw),QIcon(":/images/toCommit.svg"));
        if (pw->sbInsertRows->value()>5)
            pw->sbInsertRows->setValue(1);
    } else {
        SetColoredText(pw->lErr,"insertRows(x,y)","Err");
        return false;
    }
    return true;
}

bool PotaTableModel::DeleteRowShowErr()
{
    PotaWidget *pw=dynamic_cast<PotaWidget*>(parent());
    QModelIndexList selectedIndexes=pw->tv->selectionModel()->selectedIndexes();
    //int rr=0;
    for (const QModelIndex &index : selectedIndexes) {
        if (rowsToRemove.contains(index.row())) {
            revertRow(index.row());
            //rr--;
        } else if (childCount(index.row())==0) {
            if (!removeRow(index.row()))
            //     rr++;
            // else
                SetColoredText(pw->lErr,"removeRow("+str(index.row())+")","Err");
        }
    }
    pw->lToDelete->setText(QString::number(rowsToRemove.count()));
    pw->lToDelete->setVisible(rowsToRemove.count()>0);
    if (rowsToInsert.count()>0 or rowsToRemove.count()>0) {
        pw->pbCommit->setEnabled(true);
        pw->pbRollback->setEnabled(true);
        pw->pbFilter->setEnabled(false);
        pw->cbPageFilter->setEnabled(false);
        pw->twParent->setTabIcon(pw->twParent->indexOf(pw),QIcon(":/images/toCommit.svg"));
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                                  PotaTableView
//////////////////////////////////////////////////////////////////////////////////////////////////

void PotaTableView::keyPressEvent(QKeyEvent *event) {
    QModelIndex currentIndex=selectionModel()->currentIndex();

    if ((event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) && !(event->modifiers()==Qt::ControlModifier)) {
        if (currentIndex.isValid()) {
            PotaWidget *pw=dynamic_cast<PotaWidget*>(parent());
            if (pw->editNotes->isVisible() and pw->editNotes->isReadOnly())
                pw->toogleReadOnlyEditNotes(currentIndex);
            else if (!pw->editNotes->isVisible() and AcceptReturns(pw->model->db,pw->model->tableName(),currentIndex.model()->headerData(currentIndex.column(),Qt::Horizontal,Qt::EditRole).toString()))
                pw->toogleReadOnlyEditNotes(currentIndex);
            else
                edit(currentIndex);
        }
    } else if (event->key()==Qt::Key_Delete) {
        clearSelectionData();
    } else if (event->matches(QKeySequence::Copy)) {
        copySelectionToClipboard();
    } else if (event->matches(QKeySequence::Cut)) {
        cutSelectionToClipboard();
    } else if (event->matches(QKeySequence::Paste)) {
        pasteFromClipboard();
    } else if ((event->modifiers() & Qt::ControlModifier) &&
               (event->modifiers() & Qt::ShiftModifier) &&
               (event->key() == Qt::Key_V)) {
        pasteFromClipboard(true,false);
    } else if ((event->modifiers() & Qt::ControlModifier) &&
               (event->modifiers() & Qt::ALT) &&
               (event->key() == Qt::Key_V)) {
        pasteFromClipboard(true,true);
    } else if (event->key()==Qt::Key_Left or event->key()==Qt::Key_Right or event->key()==Qt::Key_Up or event->key()==Qt::Key_Down or
               event->key()==Qt::Key_PageUp or event->key()==Qt::Key_PageDown or event->key()==Qt::Key_Back or event->key()==Qt::Key_End) {
        QTableView::keyPressEvent(event);
    } else { //Do not modify data directly in tv if editNotes is Visible.
        PotaWidget *pw=dynamic_cast<PotaWidget*>(parent());
        if (!pw->editNotes->isVisible())
            QTableView::keyPressEvent(event);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                                  PotaHeaderView
//////////////////////////////////////////////////////////////////////////////////////////////////

void PotaHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const {
    if (inPaintSection) return;
    PotaWidget *pw=dynamic_cast<PotaWidget*>(parent()->parent());
    QString title=pw->delegate->PaintedColsTitles[logicalIndex];
    if (pw->delegate->PaintedColsTypes[logicalIndex].startsWith("Tempo")) {
    //if (pw->delegate->PaintedCols.contains(logicalIndex)) {
        //painter->save();
        int xOffset=-22;
        int yOffset=rect.height()-5;
        painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(1).left(3));
        painter->drawText(rect.left() + (xOffset+=28), rect.top() + yOffset, locale().monthName(2).left(3));
        painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(3).left(3));
        painter->drawText(rect.left() + (xOffset+=30), rect.top() + yOffset, locale().monthName(4).left(3));
        painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(5).left(3));
        if(title.isEmpty()) {
            painter->drawText(rect.left() + (xOffset+=30), rect.top() + yOffset, locale().monthName(6).left(3));
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(7).left(3));
        } else {
            QBrush b;
            b.setStyle(Qt::SolidPattern);
            QColor c("red");
            c.setAlpha(60);
            b.setColor(c);
            xOffset+=37;
            painter->fillRect(rect.left()+xOffset,rect.top()+3, 36, rect.height()-6,b);
            painter->drawText(rect.left()+xOffset+3, rect.top() + yOffset, title);
            xOffset+=24;
        }
        painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(8).left(3));
        painter->drawText(rect.left() + (xOffset+=30), rect.top() + yOffset, locale().monthName(9).left(3));
        painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(10).left(3));
        painter->drawText(rect.left() + (xOffset+=30), rect.top() + yOffset, locale().monthName(11).left(3));
        painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(12).left(3));

        if(title.isEmpty()) {
            //Year 2
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(1).left(3));
            painter->drawText(rect.left() + (xOffset+=28), rect.top() + yOffset, locale().monthName(2).left(3));
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(3).left(3));
            painter->drawText(rect.left() + (xOffset+=30), rect.top() + yOffset, locale().monthName(4).left(3));
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(5).left(3));
            painter->drawText(rect.left() + (xOffset+=30), rect.top() + yOffset, locale().monthName(6).left(3));
            painter->drawText(rect.left() + (xOffset+=31), rect.top() + yOffset, locale().monthName(7).left(3));
        }

        //painter->restore();
    } else if (pw->delegate->PaintedColsTypes[logicalIndex]=="TitleRed") {
        if(title!="") {
            QBrush b;
            b.setStyle(Qt::SolidPattern);
            QColor c("red");
            c.setAlpha(60);
            b.setColor(c);
            painter->fillRect(rect.left()+6,rect.top()+3, 36, rect.height()-6,b);
            painter->drawText(rect.left()+9,rect.top() + rect.height()-5, title);
        }
    } else if (pw->delegate->PaintedColsTypes[logicalIndex]=="Title") {
        if(title!="") {
            painter->drawText(rect.left()+9,rect.top() + rect.height()-5, title);
        }
    } else {
        // QString FieldName=pw->model->headerData(logicalIndex,Qt::Horizontal,Qt::EditRole).toString();
        // QString DisplayName=FieldName;
        // if (DisplayName.endsWith("_pc"))
        //     DisplayName=StrReplace(DisplayName,"_pc","%");
        // DisplayName=StrReplace(DisplayName,"_"," ");
        // //painter->drawText(rect.left()+9,rect.top() + rect.height()-5, DisplayName);

        // inPaintSection=true;
        // pw->model->setHeaderData(logicalIndex, Qt::Horizontal,DisplayName);
        // QCoreApplication::processEvents(); cause a crash
        // inPaintSection=false;

        QHeaderView::paintSection(painter, rect, logicalIndex);

        // inPaintSection=true;
        // pw->model->setHeaderData(logicalIndex, Qt::Horizontal,FieldName);
        // QCoreApplication::processEvents();
        // inPaintSection=false;

    }
}

void PotaHeaderView::mouseDoubleClickEvent(QMouseEvent *event)  {
    int logicalIndex=logicalIndexAt(event->pos());
    if (logicalIndex != -1) {
        PotaWidget *pw=dynamic_cast<PotaWidget*>(parent()->parent());
        if (!pw->pbCommit->isEnabled()){
            pw->PositionSave();
            //PotaTableModel *pm=dynamic_cast<PotaTableModel*>(model());

            //Change sort
            if (iSortCol==logicalIndex) {//Already sorted on this column.
                if (!bSortDes) {
                    pw->model->setOrderBy(logicalIndex,Qt::SortOrder::DescendingOrder);
                    for (int col=0; col < pw->model->columnCount(); ++col)
                        pw->model->setHeaderData(col, Qt::Horizontal, 0, Qt::DecorationRole);
                    pw->model->setHeaderData(logicalIndex, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_RedUp.svg")), Qt::DecorationRole);
                    bSortDes=true;
                } else {//Reset sorting.
                    pw->model->setOrderBy( -1,Qt::SortOrder::AscendingOrder);
                    //pw->model->setHeaderData(iSortCol, Qt::Horizontal, 0, Qt::DecorationRole);

                    QStringList natSortCols=NaturalSortCol(pw->model->db,pw->model->tableName()).split(",");
                    iSortCol=pw->model->FieldIndex(natSortCols[0]);
                    for (int col=0; col < pw->model->columnCount(); ++col)
                        pw->model->setHeaderData(col, Qt::Horizontal, 0, Qt::DecorationRole);
                    for (int i=0;i<natSortCols.count();i++)
                        pw->model->setHeaderData(pw->model->FieldIndex(natSortCols[i]), Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_BlueDown"+str(i)+".svg")), Qt::DecorationRole);

                    bSortDes=false;
                }
            } else {//Actual sort on another column.
                pw->model->setOrderBy(logicalIndex,Qt::SortOrder::AscendingOrder);
                for (int col=0; col < pw->model->columnCount(); ++col)
                    pw->model->setHeaderData(col, Qt::Horizontal, 0, Qt::DecorationRole);
                pw->model->setHeaderData(logicalIndex, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_GreenDown.svg")), Qt::DecorationRole);
                iSortCol=logicalIndex;
                bSortDes=false;
            }
            pw->model->select();
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
    PotaWidget* pw=dynamic_cast<PotaWidget*>(parent());

    QBrush b;
    b.setStyle(Qt::SolidPattern);
    QColor c,cFiltered,cCopied,cModified,cModifiedError;
    QItemSelectionModel *selection=pw->tv->selectionModel();
    QString sFieldName=pw->model->headerData(index.column(),Qt::Horizontal,Qt::EditRole).toString();
    QString sTableName=pw->model->tableName();
    QLocale locale;
    QString decimalSep=QString(locale.decimalPoint());


    if (!index.data(Qt::EditRole).isNull() and
        (index.data(Qt::EditRole)=="")) {
        //Hightlight not null empty value. They are not normal, it causes SQL failure.
        c=Qt::red;
        c.setAlpha(150);
    } else if (decimalSep!="." and
               pw->model->dataTypes[index.column()]=="REAL" and
               index.data(Qt::EditRole).toString().contains(decimalSep)) {
        //Hightlight wrond decimal value.
        c=Qt::red;
        c.setAlpha(150);
    } else if (index.data(Qt::EditRole).toString().endsWith("!") or
               index.data(Qt::DisplayRole).toString().endsWith("!")) {
        c=Qt::red;
        c.setAlpha(150);
    } else if (index.data(Qt::EditRole).toString().startsWith(".") and index.data(Qt::EditRole).toString().length()>1) {//Invisible data
        c=Qt::gray;
        c.setAlpha(50);
    } else if (index.data(Qt::EditRole).toString().length()>7 and
               index.data(Qt::EditRole).toString().contains("#") and
               QColor(index.data(Qt::EditRole).toString().last(7)).isValid()) { //Special color cell.
        c=QColor(index.data(Qt::EditRole).toString().last(7));
        c.setAlpha(50);
    } else if (RowColorCol>-1) {
        //c=RowColor(index.model()->index(index.row(),RowColorCol).data(Qt::DisplayRole).toString(),pw->model->tableName());
        c=QColor(index.model()->index(index.row(),RowColorCol).data(Qt::DisplayRole).toString());
        int alpha;
        if (isDarkTheme())
            alpha=60;
        else
            alpha=80;
        c.setAlpha(alpha);
    }
    if (!c.isValid()) {//Table color.
        c=cColColors[index.column()];
        // if (!c.isValid() and !TempoCols.contains(index.column()))
        //     c=cTableColor;
        if (isDarkTheme())
            c.setAlpha(30);
        else
            c.setAlpha(60);
    }
    if (!pw->model->rowsToInsert.contains(index.row()) and c.isValid()) {
        b.setColor(c);
        painter->fillRect(option.rect,b);
    }

    // Stronger horizontal line.
    if (isDarkTheme())
        c=QColor("#2A2A2A");
    else
        c=QColor(Qt::white);
    c.setAlpha(255);
    b.setColor(c);
    QRect r;
    r.setTop(option.rect.bottom());
    r.setHeight(1);
    r.setLeft(option.rect.left());
    r.setRight(option.rect.right());
    painter->fillRect(r,b);

    if (index.column()==FilterCol or //Filtering column and search match.
        (!FindText.isEmpty() and RemoveAccents(index.data(Qt::DisplayRole).toString()).contains(FindText,Qt::CaseInsensitive))) { //Find
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

    if (index.row()==selection->currentIndex().row()) {//Line selected
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

    //PotaHeaderView* phv=dynamic_cast<PotaHeaderView*>(pw->tv->horizontalHeader());
    //Paint data
    if (PaintedColsTypes[index.column()].startsWith("Tempo")) {
    //if (PaintedCols.contains(index.column()))
         paintTempo(painter,option,index);
    } else { //Green or red value.
        QString ColoText="";
        if(!index.data(Qt::EditRole).toDate().isNull()){
            if(index.data(Qt::EditRole).toDate()>QDate::currentDate())//Date in future
                ColoText="G";
        } else if(sFieldName=="pH"){
            if(index.data(Qt::EditRole).toDouble()>=8 or index.data(Qt::EditRole).toDouble()<=6) //todo : code métier à mettre ailleurs
                ColoText="R";
        } else if(sFieldName.startsWith("★")){
            if(index.data(Qt::EditRole).toString()=="Elevé") //todo : code métier à mettre ailleurs
                ColoText="R";
            else if (index.data(Qt::EditRole).toString()=="Faible")
                ColoText="G";
        } else if(sFieldName.startsWith("☆")){
            if(index.data(Qt::EditRole).toString()=="Elevé") //todo : code métier à mettre ailleurs
                ColoText="G";
            else if (index.data(Qt::EditRole).toString()=="Faible")
                ColoText="R";
        } else if(sFieldName.endsWith("_manq")){
            if(index.data(Qt::EditRole).toDouble()>0) //todo : code métier à mettre ailleurs
                ColoText="R";
            else if (index.data(Qt::EditRole).toDouble()<0)
                ColoText="G";
        } else if(sFieldName=="Couv_réc_pc" or sFieldName=="Couv_obj_pc"){
            if(index.data(Qt::EditRole).toDouble()<80) //todo : code métier à mettre ailleurs
                ColoText="R";
            else if (index.data(Qt::EditRole).toDouble()>120)
                ColoText="G";
        } else if(sFieldName=="Export_pc"){
            if(index.data(Qt::EditRole).toDouble()>100) //todo : code métier à mettre ailleurs
                ColoText="R";
        } else if(sTableName=="Analyses_de_sol"){
            if(sFieldName=="MO") {
                if(index.data(Qt::EditRole).toDouble()<=15) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>=20) ColoText="G";
            } else if(sFieldName=="IB") {
                if(index.data(Qt::EditRole).toDouble()>=1.4) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()<=0.7) ColoText="G";
            } else if(sFieldName=="CEC") {
                if(index.data(Qt::EditRole).toDouble()<10) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>15) ColoText="G";
            } else if(sFieldName=="N") {
                if(index.data(Qt::EditRole).toDouble()<=0.9) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>=1.1) ColoText="G";
            } else if(sFieldName=="P") {
                if(index.data(Qt::EditRole).toDouble()<=0.08) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>=0.12) ColoText="G";
            } else if(sFieldName=="K") {
                if(index.data(Qt::EditRole).toDouble()<=0.12) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>=0.15) ColoText="G";
            } else if(sFieldName=="C") {
                if(index.data(Qt::EditRole).toDouble()<=9) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>=11) ColoText="G";
            } else if(sFieldName=="CN") {
                if(index.data(Qt::EditRole).toDouble()<=8) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>=11) ColoText="G";
            } else if(sFieldName=="Ca") {
                if(index.data(Qt::EditRole).toDouble()<=3.7) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>=3.9) ColoText="G";
            } else if(sFieldName=="Mg") {
                if(index.data(Qt::EditRole).toDouble()<=0.09) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>=0.14) ColoText="G";
            } else if(sFieldName=="Na") {
                if(index.data(Qt::EditRole).toDouble()>=0.24) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()<=0.02) ColoText="G";
            }
        } else if(sTableName=="Fertilisants"){
            if(sFieldName=="N_disp_pc") {
                if(index.data(Qt::EditRole).toDouble()<0.2) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>1) ColoText="G";
            } else if(sFieldName=="P_disp_pc") {
                if(index.data(Qt::EditRole).toDouble()<0.1) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>0.5) ColoText="G";
            } else if(sFieldName=="K_disp_pc") {
                if(index.data(Qt::EditRole).toDouble()<0.4) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>2) ColoText="G";
            } else if(sFieldName=="Ca_disp_pc") {
                if(index.data(Qt::EditRole).toDouble()<0.4) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>2) ColoText="G";
            } else if(sFieldName=="Fe_disp_pc") {
                if(index.data(Qt::EditRole).toDouble()<0.02) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>0.1) ColoText="G";
            } else if(sFieldName=="Mg_disp_pc") {
                if(index.data(Qt::EditRole).toDouble()<0.2) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>1) ColoText="G";
            } else if(sFieldName=="Na_disp_pc") {
                if(index.data(Qt::EditRole).toDouble()<0.04) ColoText="G";
                else if(index.data(Qt::EditRole).toDouble()>0.2) ColoText="R";
            } else if(sFieldName=="S_disp_pc") {
                if(index.data(Qt::EditRole).toDouble()<0.04) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>0.2) ColoText="G";
            } else if(sFieldName=="Si_disp_pc") {
                if(index.data(Qt::EditRole).toDouble()<0.1) ColoText="R";
                else if(index.data(Qt::EditRole).toDouble()>0.5) ColoText="G";
            }
        }

        if (ColoText=="G") {
            QStyleOptionViewItem opt=option;
            if (isDarkTheme())
                opt.palette.setColor(QPalette::Text, QColor("#77ff77"));//light green
            else
                opt.palette.setColor(QPalette::Text, QColor("#009e00"));//dark green
            QStyledItemDelegate::paint(painter, opt, index);
        } else if (ColoText=="R") {
            QStyleOptionViewItem opt=option;
            if (isDarkTheme())
                opt.palette.setColor(QPalette::Text, QColor("#ff7171"));//light red
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
    QLinearGradient gradient;
    b.setStyle(Qt::SolidPattern);
    QColor c;
    QRect r;
    int left=0;

    //Vertical bars for month
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
        if(left>option.rect.right())
            break;
        r.setLeft(left);
        r.setWidth(1);
        painter->fillRect(r,b);
    }

    QStringList ql=index.data(Qt::DisplayRole).toString().split(":");
    QString t;
    int xt=0;
    if (ql.count()>=6) {
        int const DebSemis=ql[0].toInt()*coef;
        int const FinSemis=ql[1].toInt()*coef;
        int DebPlant=ql[2].toInt()*coef;
        int FinPlant=ql[3].toInt()*coef;
        int DebRecolte=ql[4].toInt()*coef;
        int FinRecolte=ql[5].toInt()*coef;

        if(DebRecolte==0 and FinRecolte>0){//Affichage d'une culture sans récolte (Début_récolte est null au lieu de Fin_récolte)
            DebRecolte=FinRecolte;
            FinRecolte=0;
        }

        while (DebPlant>0 and DebPlant<DebSemis)
            DebPlant=DebPlant+365*coef;
        while (FinPlant>0 and FinPlant<DebPlant)
            FinPlant=FinPlant+365*coef;
        while (DebRecolte>0 and (DebRecolte<DebSemis or DebRecolte<DebPlant))
            DebRecolte=DebRecolte+365*coef;
        while (FinRecolte>0 and FinRecolte<DebRecolte)
            FinRecolte=FinRecolte+365*coef;

        if (ql.count()>6) {
            t=ql[6];
            if (FinPlant==0)
                xt=option.rect.left()+FinSemis+3;
            else
                xt=option.rect.left()+FinPlant+3;
        }
        bool bSemis=true;
        bool bPlant=true;
        bool bDRec=true;
        bool bFRec=true;
        bool bTer=false;
        bool bDeperisementPep=false;
        bool bDeperisementEP=false;
        bool bAffSemNeg=(index.model()->headerData(index.column(),Qt::Horizontal,Qt::EditRole)=="TEMPO_N")or
                        (index.model()->headerData(index.column(),Qt::Horizontal,Qt::EditRole)=="TEMPO_NN");//Afficher la partie de la période de semis négative (à gauche du 1er janvier).
        bool bAffPlant=true;
        bool bAffRec=true;
        if (ql.count()>10) {
            bSemis=!ql[7].isEmpty();
            bPlant=!ql[8].isEmpty();
            bDRec=!ql[9].isEmpty();
            bFRec=ql[9].startsWith('x');
            bTer=!ql[10].isEmpty();
            if(bTer and FinRecolte>0){
                //détection des échecs de culture.
                if(!bDRec and !bFRec) bAffRec=false; //Ne pas afficher la récolte.
                if(FinSemis>0 and FinPlant>0) { //Semis pépinière
                    if(bSemis and !bPlant) bAffPlant=false; //Ne pas afficher la plantation.
                    if(bSemis and !bPlant and !bDRec and !bFRec) bDeperisementPep=true;
                    if(bSemis and bPlant and !bDRec and !bFRec) bDeperisementEP=true;
                    //if(!bSemis) DebPlant=0;
                } else if (FinSemis>0) { //Semis en place
                    // if(!bSemis) DebRecolte=0;
                    // else
                    if(bSemis and !bDRec and !bFRec) bDeperisementEP=true;
                } else if (FinPlant>0) { //Plant
                    // if(!bPlant) FinPlant=0;
                    // else
                    if(bPlant and !bDRec and !bFRec) bDeperisementEP=true;
                }
            }
        }

        if(DebRecolte==0 and FinRecolte==0){//Aucune date de récolte ou de destruction de culture
            DebRecolte=365;
            bDeperisementEP=true;
        }

        if (FinSemis>0 or (bAffSemNeg and FinSemis<0)){
            //Période de semis
            if (FinPlant>0) c=cPepiniere; else c=cEnPlace;
            if (bSemis) c.setAlpha(255); else  c.setAlpha(100);
            b.setColor(c);
            r.setBottom(option.rect.bottom()-0);
            r.setTop(r.bottom()-4);
            r.setLeft(option.rect.left()+DebSemis);
            r.setRight(option.rect.left()+FinSemis);
            painter->fillRect(r,b);
        }
        if (DebPlant>FinSemis and FinSemis!=0){ // FinSemis peut être négatif dans l'affichage des rotations.
            //Semis fait, attente plantation
            if(FinPlant!=0) c=cPepiniere; else c=cEnPlace;
            r.setBottom(option.rect.bottom()-0);
            r.setTop(r.bottom()-4);
            r.setLeft(option.rect.left()+iif(bAffSemNeg,FinSemis,fmax(FinSemis,0)).toInt());
            r.setRight(option.rect.left()+DebPlant);
            if (bDeperisementPep) {
                //Gradient pour marquer le dépérissement du semis.
                gradient.setStart(r.left(), r.top());
                gradient.setFinalStop(r.right(), r.top());
                c.setAlpha(100);
                gradient.setColorAt(0, c); // couleur de départ
                c.setAlpha(0);
                gradient.setColorAt(1, c);
                painter->fillRect(r,gradient);
                //FinPlant=0;
            } else {
                c.setAlpha(100);
                b.setColor(c);
                painter->fillRect(r,b);
            }
        }
        if (FinPlant>0 and bAffPlant) {
            //Période de plantation
            c=cEnPlace;
            if (bPlant) c.setAlpha(255); else  c.setAlpha(150);
            b.setColor(c);
            r.setBottom(option.rect.bottom()-3);
            r.setTop(r.bottom()-10);
            r.setLeft(option.rect.left()+DebPlant);
            r.setRight(option.rect.left()+FinPlant);
            painter->fillRect(r,b);
        }
        if ((DebRecolte>FinPlant and FinPlant>0 and bAffPlant)or
            (DebRecolte>FinSemis and FinSemis>0 and FinPlant==0)){
            //Semis en place ou plantation faite, attente récolte
            c=cEnPlace;
            r.setBottom(option.rect.bottom()-3);
            r.setTop(r.bottom()-10);
            r.setLeft(option.rect.left()+fmax(FinPlant,FinSemis));
            r.setRight(option.rect.left()+DebRecolte);
            if (bDeperisementEP) {
                //Gradient pour marquer le dépérissement de la culture
                gradient.setStart(r.left(), r.top());
                gradient.setFinalStop(r.right(), r.top());
                c.setAlpha(150);
                gradient.setColorAt(0, c); // couleur de départ
                c.setAlpha(0);
                gradient.setColorAt(1, c);
                painter->fillRect(r,gradient);
            } else {
                c.setAlpha(150);
                b.setColor(c);
                painter->fillRect(r,b);
            }
        }
        if (FinRecolte>DebRecolte and bAffRec){
            //Période de récolte
            c=cRecolte;         
            r.setBottom(option.rect.bottom()-8);
            r.setTop(r.bottom()-6);
            r.setLeft(option.rect.left()+DebRecolte);
            r.setRight(option.rect.left()+FinRecolte);

            if (bDRec and bFRec) {
                c.setAlpha(255);
                b.setColor(c);
                painter->fillRect(r,b);
            } else if (bDRec) {
                gradient.setStart(r.left(), r.top());
                gradient.setFinalStop(r.right(), r.top());
                c.setAlpha(255);
                gradient.setColorAt(0, c);
                c.setAlpha(100);
                gradient.setColorAt(1, c);
                painter->fillRect(r,gradient);
            } else {
                c.setAlpha(100);
                b.setColor(c);
                painter->fillRect(r,b);
            }
        }
    }

    if (PaintedColsTypes[index.column()]=="TempoNow") {//Vertical bar for now

        c=QColor("red");
        left=option.rect.left()+round(QDate::currentDate().dayOfYear()*coef);
        int r1w=1;

        if (ql.count()>11) {//Blue indicator for water.
            QString tDrop=ql[11];
            if (!tDrop.isEmpty()) {
                painter->save();
                QColor waterDropColor(0, 122, 255);
                c=waterDropColor;
                r1w=3;
                //Draw drop
                //painter->setRenderHint(QPainter::Antialiasing, true);
                // painter->setBrush(waterDropColor);
                // painter->setPen(Qt::NoPen);
                int dropX=left;
                int dropY=option.rect.top();
                // QRectF ellipseRect( dropX-3, dropY+3, 6, 6);
                // painter->drawEllipse(ellipseRect);
                // QPolygonF dropPolygon;
                // dropPolygon << QPointF(dropX, dropY) // Pointe
                //             << QPointF(dropX-2, dropY + 4)  // Coin gauche
                //             << QPointF(dropX+2, dropY + 4); // Coin droit
                // painter->drawPolygon(dropPolygon);

                if (tDrop.toLower()!="x") {
                    //Irrig text
                    painter->setPen(Qt::SolidLine);
                    painter->setPen(waterDropColor);
                    QFont font=painter->font();
                    font.setBold(true);
                    int fontSize=font.pointSize();
                    font.setPointSize(fontSize-2);
                    painter->setFont(font);
                    QFontMetrics metrics(font);
                    int textX;
                    if (t.isEmpty() or dropX>xt+30)
                        textX=dropX+5;
                    else
                        textX=xt+30+5;
                    int textY=dropY+10;
                    QRectF textRect(textX, textY-metrics.height()+3, metrics.horizontalAdvance(tDrop)+1, metrics.height()-3);
                    QColor backgroundColor(180, 180, 180);
                    painter->setBrush(backgroundColor);
                    painter->setPen(Qt::NoPen);
                    painter->drawRect(textRect);
                    painter->setPen(Qt::SolidLine);
                    painter->setPen(waterDropColor);
                    painter->drawText(textX, textY, tDrop);
                }

                painter->restore();
            }
        }

        b.setColor(c);
        r.setBottom(option.rect.bottom());
        r.setTop(option.rect.top());
        r.setLeft(left-iif(r1w>1,1,0).toInt());
        r.setWidth(r1w);
        painter->fillRect(r,b);
    }
    if (!t.isEmpty())
        painter->drawText(xt, option.rect.bottom()-2, t);
}

QWidget *PotaItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const   {
    const PotaTableModel *constModel=qobject_cast<const PotaTableModel *>(index.model());
    if (!constModel) {
        return QStyledItemDelegate::createEditor(parent, option, index); // Standard editor
    }

    PotaTableModel *model=const_cast<PotaTableModel *>(constModel);
    QString sFieldName=model->headerData(index.column(),Qt::Horizontal,Qt::EditRole).toString();
    QString sDataType=model->dataTypes[index.column()];
    QString sComboValues=ComboValues(model->db,model->RealTableName(),sFieldName);
    if (model->relation(index.column()).isValid()) {
        //Create QComboBox for relational columns
        PotaWidget* pw=dynamic_cast<PotaWidget*>(this->parent());
        QComboBox *comboBox=new QComboBox(parent);
        QSqlTableModel *relationModel=model->relationModel(index.column());
        int relationIndex=relationModel->fieldIndex(model->relation(index.column()).displayColumn());
        QString filter;
        if (pw->pageFilterFilters.count()>0)
            filter=FkFilter(model->db,model->RealTableName(),sFieldName,index);
        else
            filter=FkFilter(model->db,model->RealTableName(),sFieldName,index);
        model->relationModel(index.column())->setFilter(filter);
        comboBox->addItem("", QVariant()); // Option for setting a NULL
        QString sRelRowSummary=RowSummaryModel(model->db,relationModel->tableName());
        for (int i=0; i < relationModel->rowCount(); ++i) {
            QString value=relationModel->record(i).value(relationIndex).toString();
            QString displayValue;//=relationModel->record(i).value(0).toString();
            // if (!relationModel->record(i).value(1).toString().isEmpty())
            //     displayValue+=" | "+relationModel->record(i).value(1).toString();
            // if (!relationModel->record(i).value(2).toString().isEmpty())
            //     displayValue+=" | "+relationModel->record(i).value(2).toString();
            displayValue=RowSummary(model->db,relationModel->tableName(),sRelRowSummary,relationModel,i);
            comboBox->addItem( displayValue,value);
        }
        return comboBox;
    } else if (!sComboValues.isEmpty()){
        //PotaQuery query(*model->db);
        QStringList slComboValues=sComboValues.split("|");
        if (slComboValues.count()>1) {
            QComboBox *comboBox=new QComboBox(parent);
            comboBox->addItem("", QVariant()); // Option for setting a NULL
            for (int i=0; i < slComboValues.count(); ++i)
                comboBox->addItem(slComboValues[i],slComboValues[i]);
            return comboBox;
        }
    } else if (sDataType=="REAL"){
        return new QLineEdit(parent);
    } else if (sDataType.startsWith("INT")){
        return new QLineEdit(parent);
    } else if (sDataType.startsWith("BOOL")){
        return new QLineEdit(parent);
    } else if (sDataType=="DATE"){
        QDateEdit *dateEdit=new QDateEdit(parent);
        dateEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
        //dateEdit->setDisplayFormat("yyyy-MM-dd");
        if(model->data(index,Qt::EditRole).isNull())
            dateEdit->setDate(QDate::currentDate());
        else
            dateEdit->setDate(model->data(index,Qt::EditRole).toDate());
        return dateEdit;
    }
    return QStyledItemDelegate::createEditor(parent, option, index); // Standard editor
}

void PotaItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const  {
    QComboBox *comboBox=qobject_cast<QComboBox *>(editor);
    if (comboBox) {
        QVariant selectedValue=comboBox->currentData();
        if (!selectedValue.isValid() || selectedValue.toString().isEmpty()) {
            model->setData(index, QVariant(), Qt::EditRole); // Définit à NULL
        } else {
            model->setData(index, selectedValue, Qt::EditRole);
        }
        dynamic_cast<PotaWidget*>(model->parent())->tv->setFocus();
        return;
    }
    QLineEdit *lineEdit=qobject_cast<QLineEdit *>(editor);
    if (lineEdit) {
        if (lineEdit->text().isEmpty()) {
            model->setData(index, QVariant(), Qt::EditRole); // Définit à NULL
        } else {
            model->setData(index, lineEdit->text(), Qt::EditRole);
        }
        return;
    }
    QDateEdit *dateEdit=qobject_cast<QDateEdit *>(editor);
    if (dateEdit) {
        model->setData(index, dateEdit->date(), Qt::EditRole);
        return;
    }

    QStyledItemDelegate::setModelData(editor, model, index); // Éditeur standard
}
