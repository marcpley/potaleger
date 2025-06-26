#include "potawidget.h"
#include "data/Data.h"
#include "qapplication.h"
#include "qheaderview.h"
#include "qlineedit.h"
#include "qmenu.h"
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
    lTabTitle->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    //Toolbar
    toolbar = new QWidget(this);

    pbRefresh = new QToolButton(this);
    pbRefresh->setIcon(QIcon(":/images/reload.svg"));
    pbRefresh->setShortcut( QKeySequence(Qt::Key_F5));
    pbRefresh->setToolTip(tr("Recharger les données depuis le fichier.")+"\n"+
                          tr("Les modifications en cours seront automatiquement enregistrées")+"\n"+
                          "F5");
    connect(pbRefresh, &QToolButton::released, this, &PotaWidget::pbRefreshClick);

    pbEdit = new QToolButton(this);
    pbEdit->setIcon(QIcon(":/images/edit.svg"));
    pbEdit->setCheckable(true);
    pbEdit->setChecked(false);
    pbEdit->setShortcut( QKeySequence(Qt::Key_F2));
    pbEdit->setToolTip(tr("Basculer le mode édition.")+"\n"+
                          "F2");
    connect(pbEdit, &QToolButton::released, this, &PotaWidget::pbEditClick);

    pbCommit = new QToolButton(this);
    pbCommit->setIcon(QIcon(":/images/commit.svg"));
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
    pbInsertRow->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Insert));
    pbInsertRow->setToolTip(tr("Ajouter des lignes.\nSi le nombre de ligne à ajouter en mis à 0, l'enregistrement courant sera dupliqué.")+"\n"+
                               "Ctrl + Insert");
    connect(pbInsertRow, &QToolButton::released, this, &PotaWidget::pbInsertRowClick);
    pbInsertRow->setEnabled(false);
    pbInsertRow->setVisible(false);

    pbDeleteRow = new QToolButton(this);
    pbDeleteRow->setIcon(QIcon(":/images/delete_row.svg"));
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
    lFilterOn->setText("...");
    cbFilterType = new QComboBox(this);
    connect(cbFilterType, &QComboBox::currentIndexChanged,this, &PotaWidget::cbFilterTypeChanged);
    leFilter = new QLineEdit(this);
    connect(leFilter, &QLineEdit::returnPressed, this, &PotaWidget::leFilterReturnPressed);
    pbFilter = new QPushButton(this);
    pbFilter->setText(tr("Filtrer"));
    pbFilter->setCheckable(true);
    pbFilter->setEnabled(false);
    pbFilter->setShortcut(QKeySequence(Qt::Key_F6));
    pbFilter->setToolTip("F6");
    connect(pbFilter, &QPushButton::clicked,this,&PotaWidget::pbFilterClick);
    lFilterResult = new QLabel(this);

    //Find tool
    findFrame = new QFrame(this);
    findFrame->setFrameShape(QFrame::NoFrame);
    findFrame->setBackgroundRole(QPalette::Midlight);
    findFrame->setAutoFillBackground(true);
    lFind = new QLabel(this);
    lFind->setText(tr("Rechercher"));
    leFind = new QLineEdit(this);
    connect(leFind, &QLineEdit::textEdited, this, &PotaWidget::leFindTextEdited);
    connect(leFind, &QLineEdit::returnPressed, this, &PotaWidget::leFindReturnPressed);
    pbFindFirst = new QPushButton(this);
    pbFindFirst->setText(tr("1er"));
    pbFindFirst->setEnabled(false);
    pbFindFirst->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_F3));
    pbFindFirst->setToolTip("Ctrl + F3");
    connect(pbFindFirst, &QPushButton::clicked,this,&PotaWidget::pbFindFirstClick);
    pbFindNext = new QPushButton(this);
    pbFindNext->setText(tr("Suivant"));
    pbFindNext->setEnabled(false);
    pbFindNext->setShortcut( QKeySequence(Qt::Key_F3));
    pbFindNext->setToolTip("F3");
    connect(pbFindNext, &QPushButton::clicked,this,&PotaWidget::pbFindNextClick);
    pbFindPrev = new QPushButton(this);
    pbFindPrev->setText(tr("Précédent"));
    pbFindPrev->setEnabled(false);
    pbFindPrev->setShortcut( QKeySequence(Qt::Key_F4));
    pbFindPrev->setToolTip("Ctrl + F4");
    connect(pbFindPrev, &QPushButton::clicked,this,&PotaWidget::pbFindPrevClick);

    //Page filter tool
    pageFilterFrame = new QFrame(this);
    pageFilterFrame->setFrameShape(QFrame::NoFrame);
    pageFilterFrame->setBackgroundRole(QPalette::Midlight);
    pageFilterFrame->setAutoFillBackground(true);
    pageFilterFrame->setVisible(false);
    lPageFilter = new QLabel(this);
    lPageFilter->setText("...");
    cbPageFilter = new QComboBox(this);
    connect(cbPageFilter, &QComboBox::currentIndexChanged,this, &PotaWidget::cbPageFilterChanged);
    //cbPageFilter->setFixedHeight(26);


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
    //header->setParent(tv);
    tv->setHorizontalHeader(header);
    //tv->setLocale(QLocale::C ); ???

    connect(tv->selectionModel(), &QItemSelectionModel::currentChanged, this, &PotaWidget::curChanged);
    connect(tv->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PotaWidget::selChanged);
    connect(model, &PotaTableModel::dataChanged, this, &PotaWidget::dataChanged);
    connect(tv->verticalHeader(), &QHeaderView::sectionClicked,this, &PotaWidget::headerRowClicked);

    editNotes = new QTextEdit();
    editNotes->setParent(tv);
    editNotes->setVisible(false);
    editNotes->setReadOnly(true);
    QPalette palette = editNotes->palette();
    //palette.setColor(QPalette::Base, delegate->cTableColor);
    palette.setColor(QPalette::Base, QApplication::palette().color(QPalette::ToolTipBase));
    palette.setColor(QPalette::Text, QApplication::palette().color(QPalette::ToolTipText));
    editNotes->setPalette(palette);

    QAction* aEditNotes = new QAction(tv);
    aEditNotes->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    connect(aEditNotes, &QAction::triggered, [this]() {
        QModelIndex currentIndex = tv->currentIndex();
        hEditNotes(currentIndex);
    });
    //connect(aEditNotes, &QAction::triggered, this, &PotaWidget::hEditNotes);
    tv->addAction(aEditNotes);

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

    //PageFilter layout
    pageFilterLayout = new QHBoxLayout(this);
    pageFilterLayout->setSizeConstraint(QLayout::SetFixedSize);
    pageFilterLayout->setContentsMargins(5,3,5,3);
    pageFilterLayout->setSpacing(5);
    pageFilterLayout->addWidget(lPageFilter);
    pageFilterLayout->addWidget(cbPageFilter);
    pageFilterFrame->setLayout(pageFilterLayout);

    //Filter find layout
    ffLayout = new QHBoxLayout(this);
    ffLayout->setSizeConstraint(QLayout::SetFixedSize);
    ffLayout->setContentsMargins(0,0,0,0);
    ffLayout->addWidget(filterFrame);
    ffLayout->addSpacing(5);
    ffLayout->addWidget(findFrame);
    ffLayout->addSpacing(5);
    ffLayout->addWidget(pageFilterFrame);
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
            model->relationModel(localColumnIndex)->setFilter(FkFilter(model->db,model->RealTableName(),localColumn,"",model->index(0,0)));
            model->relationModel(localColumnIndex)->setSort(model->relationModel(localColumnIndex)->fieldIndex(FkSortCol(model->RealTableName(),localColumn)),Qt::SortOrder::AscendingOrder);
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
    tv->verticalHeader()->setDefaultAlignment(Qt::AlignTop);
    tv->verticalHeader()->hide();
    tv->setTabKeyNavigation(false);
    dynamic_cast<PotaHeaderView*>(tv->horizontalHeader())->iSortCol=NaturalSortCol(TableName);
    dynamic_cast<PotaHeaderView*>(tv->horizontalHeader())->model()->setHeaderData(NaturalSortCol(TableName), Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_BlueDown.svg")), Qt::DecorationRole);

    tv->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tv, &QTableView::customContextMenuRequested, this, &PotaWidget::showContextMenu);

    //widget width according to font size
    SetSizes();
}

void PotaWidget::curChanged(const QModelIndex cur, const QModelIndex pre)
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

        QString FieldName=model->headerData(cur.column(),Qt::Horizontal,Qt::EditRole).toString();

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


        if (pbEdit->isChecked() and !editNotes->isReadOnly()) {
            hEditNotes(pre);
        }
        editNotes->setReadOnly(true);

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
    QMenu contextMenu(tr("Context menu"), this);

    QAction mDefColWidth(QIcon::fromTheme("object-flip-horizontal"),tr("Largeurs de colonnes par défaut"), this);
    QAction mEditNotes(tr("Editer"), this);

    QModelIndex index = tv->indexAt(pos);
    QString FieldName = model->headerData(index.column(),Qt::Horizontal,Qt::EditRole).toString();

    mEditNotes.setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    mEditNotes.setCheckable(true);
    mEditNotes.setChecked(!editNotes->isReadOnly());
    if (index.isValid()) {
        mEditNotes.setEnabled(pbEdit->isChecked() and
                              !model->nonEditableColumns.contains(index.column()) and AcceptReturns(FieldName));
    } else {
        mDefColWidth.setEnabled(false);
        mEditNotes.setEnabled(false);
    }

    connect(&mDefColWidth, &QAction::triggered, this, &PotaWidget::hDefColWidth);
    connect(&mEditNotes, &QAction::triggered, [this]() {
        QModelIndex currentIndex = tv->currentIndex();
        hEditNotes(currentIndex);
    });
    //connect(&mEditNotes, &QAction::triggered, this, &PotaWidget::hEditNotes);

    contextMenu.addAction(&mDefColWidth);
    contextMenu.addAction(&mEditNotes);

    QFont font = contextMenu.font();
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

void PotaWidget::hEditNotes(const QModelIndex index) {
    if (pbEdit->isChecked() and
        !model->nonEditableColumns.contains(index.column())) {
        // qDebug() << "toMarkdown: " << w->editNotes->toMarkdown();
        // qDebug() << "toPlainText: " << w->editNotes->toPlainText();
        if (editNotes->isReadOnly()) { //Go to notes edit mode
            editNotes->setReadOnly(false);
            editNotes->setPlainText(editNotes->toMarkdown().trimmed());
            editNotes->setFocus();
        } else { //Save data and return to notes read mode
            editNotes->setReadOnly(true);
            QString save = editNotes->toPlainText().trimmed();
            int i = editNotes->toPlainText().count("<");
            editNotes->setMarkdown(editNotes->toPlainText().trimmed());
            if (i != editNotes->toMarkdown().count("<")) {
                qDebug() << save;
                editNotes->setPlainText(save);
                editNotes->setReadOnly(false);
                //MessageDialog(tr("Les balises HTML (<b>, <br>, etc) ne sont pas accéptées.")); todo: reactivate this.
            } else {
                if (save!=model->data(index).toString())
                    model->setData(index,save);
                tv->setFocus();
            }
        }
        SetVisibleEditNotes(true,false);
    }
}

void PotaWidget::showSelInfo() {
    editSelInfo->setGeometry(lSelect->geometry().left(),
                             lSelect->geometry().top(),
                             400,200);
    editSelInfo->setVisible(true);
}

void PotaWidget::SetVisibleEditNotes(bool bVisible, bool autoSize){
    QColor tc = delegate->cTableColor;
    if (bVisible){
        int x = tv->columnViewportPosition(tv->currentIndex().column())+5;
        int y = tv->rowViewportPosition(tv->currentIndex().row())+
                tv->horizontalHeader()->height()+
                tv->rowHeight(tv->currentIndex().row());
        int EditNotesWidth = 400;
        int EditNotesHeight = 200;

        if (editNotes->isReadOnly()) {
            editNotes->setMarkdown(model->data(tv->currentIndex(),Qt::DisplayRole).toString());
            // editNotes->setLineWidth(4);
            // editNotes->setFrameShape(QFrame::Panel);
            if (isDarkTheme())
                tc.setAlpha(50);
            else
                tc.setAlpha(80);
        } else {
            editNotes->setPlainText(model->data(tv->currentIndex(),Qt::DisplayRole).toString());
            // editNotes->setLineWidth(1);
            // editNotes->setFrameShape(QFrame::StyledPanel);
            tc.setAlpha(0);
        }
        QPalette palette = editNotes->palette();
        palette.setColor(QPalette::Base, blendColors(tv->palette().color(QPalette::Base),tc));
        palette.setColor(QPalette::Text, tv->palette().color(QPalette::Text));
        editNotes->setPalette(palette);

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
            EditNotesWidth = min((maxWidth*1.2)+50,400);
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
    int row2=-1;
    int row3=-1;
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
            break;
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

    lRowSummary->setText(RowSummary(model->tableName(),model->record(tv->currentIndex().row())));
    lSelect->setText("");
}

void PotaWidget::RefreshHorizontalHeader() {
    PotaHeaderView *phv=dynamic_cast<PotaHeaderView*>(tv->horizontalHeader());
    //delegate->PaintedCols.clear();
    delegate->PaintedColsTitles.clear();
    delegate->PaintedColsTypes.clear();
    PotaQuery query(*model->db);
    int saison=query.Selec0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='Année_culture'").toInt();
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
    sbInsertRows->setFixedSize(1.4*ButtonSize,ButtonSize);
    //sbInsertRows->setIconSize(QSize(ButtonSize,ButtonSize));
    pbInsertRow->setFixedSize(ButtonSize,ButtonSize);
    pbInsertRow->setIconSize(QSize(ButtonSize,ButtonSize));
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
    cbPageFilter->setFixedWidth(80*UserFont/10);
    tv->horizontalHeader()->setMinimumHeight(24*UserFont/10);
    tv->horizontalHeader()->setMaximumHeight(24*UserFont/10);
    tv->verticalHeader()->setDefaultSectionSize(0);//Mini.

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
    cbPageFilter->setEnabled(false);
    twParent->setTabIcon(twParent->currentIndex(),QIcon(":/images/toCommit.svg"));

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

    bool PaintedCols=false;
    for (int i=0;i<delegate->PaintedColsTypes.count();i++){
        if (delegate->PaintedColsTypes[i]!="") {
            PaintedCols=true;
            break;
        }
    }
    if(PaintedCols) {
    //if(!delegate->PaintedCols.isEmpty()) {
        RefreshHorizontalHeader();
        emit model->headerDataChanged(Qt::Horizontal, 0, model->columnCount() - 1);
        tv->horizontalHeader()->update();
    }
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
            if (ReadOnly(model->db, model->tableName(),model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()))
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
    AppBusy(true);
    PositionSave();
    if (model->SubmitAllShowErr())
        PositionRestore();
    pbInsertRow->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    sbInsertRows->setEnabled(bAllowInsert and (model->rowsToRemove.count()==0));
    pbDeleteRow->setEnabled(bAllowDelete and (model->rowsToInsert.count()==0));
    AppBusy(false);
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
                QString raField="remove_accents("+lFilterOn->text()+")";
                if (cbFilterType->currentText()==tr("contient"))
                    filter=raField+" LIKE '%"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"%'";
                else if (cbFilterType->currentText()==tr("ne contient pas"))
                    filter=raField+" NOT LIKE '%"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"%'";
                else if (cbFilterType->currentText()==tr("égal à"))
                    filter=raField+" = '"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("différent de"))
                    filter=raField+" != '"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"'";
                else if (cbFilterType->currentText()==tr("fini par"))
                    filter=raField+" LIKE '%"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"'";
                else//commence par
                    filter=raField+" LIKE '"+StrReplace(RemoveAccents(leFilter->text()),"'","''")+"%'";
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

    if (pageFilterFrame->isVisible()) {
        QString pageFilter;
        //pageFilter=pageFilterField+" = '"+cbPageFilter->currentText()+"'";
        pageFilter=pageFilterFilters[cbPageFilter->currentIndex()];
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
                    RemoveAccents(model->index(i,j).data(Qt::DisplayRole).toString()).contains(RemoveAccents(leFind->text()))){
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
                    RemoveAccents(model->index(i,j).data(Qt::DisplayRole).toString()).contains(RemoveAccents(leFind->text()))){
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

int PotaTableModel::FieldIndex(QString FieldName){
    for (int i=0;i<columnCount();i++){
        if (headerData(i,Qt::Horizontal,Qt::EditRole).toString()==FieldName)
            return i;
    }
    return -1;
}


QString PotaTableModel::FieldName(int index)
{
    return headerData(index,Qt::Horizontal,Qt::EditRole).toString();
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

    if (!bBatch) {
        AppBusy(true,progressBar,0,tableName()+" %p%");
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

    PotaQuery query(*db);
    if (!bBatch) {
        query.ExecShowErr("SELECT COUNT(*) FROM "+tableName()+" TN"+
                               iif(filter().toStdString()!=""," WHERE "+filter(),"").toString());
        int totalRows = 0;
        if (query.next())
            totalRows = query.value(0).toInt();
        progressBar->setMaximum(totalRows);
        qDebug() << "totalRows " << totalRows;
    }

    // QTimer *timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, this, &PotaTableModel::selectTimer);
    // timer->start(1000);

    setLastError(QSqlError());

    QSqlRelationalTableModel::select();//Avoids duplicate display of inserted lines
    // qDebug() << rowCount() << "(select)";
    if (!bBatch)
        progressBar->setValue(1);
    setQuery(sQuery);
    if (!bBatch)
        progressBar->setValue(rowCount());
    // qDebug() << rowCount() << "(setQuery)";

    while (canFetchMore()) {
        fetchMore();
        if (!bBatch)
            progressBar->setValue(rowCount());
        // qDebug() << rowCount();
    }

    // timer->stop();
    // timer->deleteLater();

    if (!bBatch) {
        qInfo() << str(rowCount())+" "+tr("lignes");//Line necessary to make work lFilterResult->setText(...), don't know why!
        dynamic_cast<PotaWidget*>(parent())->lFilterResult->setText(str(rowCount())+" "+tr("lignes"));
        AppBusy(false,progressBar);
    }
    bool result=(lastError().type() == QSqlError::NoError);
    //dbSuspend(db,true,!wasSuspended,label);

    if (!tempTableName.isEmpty() and
        !bBatch) {
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

    if ((lastError().type() == QSqlError::NoError)and(parent()->objectName().startsWith("PW"))) {
        SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,tableName(),"Ok");
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
        pw->cbPageFilter->setEnabled(true);
        pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(""));
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
        pw->cbPageFilter->setEnabled(true);
        pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(""));
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
            pw->cbPageFilter->setEnabled(false);
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
        pw->cbPageFilter->setEnabled(false);
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
        pw->cbPageFilter->setEnabled(false);
        pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(":/images/toCommit.svg"));
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

void PotaHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const {
    if (inPaintSection) return;
    PotaWidget *pw = dynamic_cast<PotaWidget*>(parent()->parent());
    QString title=pw->delegate->PaintedColsTitles[logicalIndex];
    if (pw->delegate->PaintedColsTypes[logicalIndex].startsWith("Tempo")) {
    //if (pw->delegate->PaintedCols.contains(logicalIndex)) {
        //painter->save();
        int xOffset = -22;
        int yOffset = rect.height()-5;
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
    QString sFieldName=pw->model->headerData(index.column(),Qt::Horizontal,Qt::EditRole).toString();
    QString sTableName=pw->model->tableName();
    QLocale locale;
    QString decimalSep = QString(locale.decimalPoint());


    if (!index.data(Qt::EditRole).isNull() and
        (index.data(Qt::EditRole) == "")) {
        //Hightlight not null empty value. They are not normal, it causes SQL failure.
        c=Qt::red;
        c.setAlpha(150);
    } else if (index.data(Qt::EditRole).toString().startsWith("Err") or
               index.data(Qt::DisplayRole).toString().startsWith("Err")) {
        c=Qt::red;
        c.setAlpha(150);
    } else if (decimalSep!="." and
               pw->model->dataTypes[index.column()]=="REAL" and
               index.data(Qt::EditRole).toString().contains(decimalSep)) {
        c=Qt::red;
        c.setAlpha(150);
    } else if (RowColorCol>-1) {
        c=RowColor(index.model()->index(index.row(),RowColorCol).data(Qt::DisplayRole).toString(),pw->model->tableName());
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

    if (index.column()==FilterCol or //Filtering column.
        (!FindText.isEmpty() and RemoveAccents(index.data(Qt::DisplayRole).toString()).contains(FindText))) { //Find
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

    //PotaHeaderView* phv = dynamic_cast<PotaHeaderView*>(pw->tv->horizontalHeader());
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
            if(index.data(Qt::EditRole).toDouble()>=8 or index.data(Qt::EditRole).toDouble()<=6) //Todo : code métier à mettre ailleurs
                ColoText="R";
        } else if(sFieldName.startsWith("★")){
            if(index.data(Qt::EditRole).toString()=="Elevé") //Todo : code métier à mettre ailleurs
                ColoText="R";
            else if (index.data(Qt::EditRole).toString()=="Faible")
                ColoText="G";
        } else if(sFieldName.startsWith("☆")){
            if(index.data(Qt::EditRole).toString()=="Elevé") //Todo : code métier à mettre ailleurs
                ColoText="G";
            else if (index.data(Qt::EditRole).toString()=="Faible")
                ColoText="R";
        } else if(sFieldName.endsWith("_manq")){
            if(index.data(Qt::EditRole).toDouble()>0) //Todo : code métier à mettre ailleurs
                ColoText="R";
            else if (index.data(Qt::EditRole).toDouble()<0)
                ColoText="G";
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
            QStyleOptionViewItem opt = option;
            if (isDarkTheme())
                opt.palette.setColor(QPalette::Text, QColor("#77ff77"));//light green
            else
                opt.palette.setColor(QPalette::Text, QColor("#009e00"));//dark green
            QStyledItemDelegate::paint(painter, opt, index);
        } else if (ColoText=="R") {
            QStyleOptionViewItem opt = option;
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
    b.setStyle(Qt::SolidPattern);
    QColor c;
    QRect r1,r2;
    int left=0;

    //Vertical bars for month
    c=QColor(128,128,128);
    b.setColor(c);
    r1.setBottom(option.rect.bottom());
    r1.setTop(option.rect.top());
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
        r1.setLeft(left);
        r1.setWidth(1);
        painter->fillRect(r1,b);
    }

    QStringList ql = index.data(Qt::DisplayRole).toString().split(":");
    QString t;
    int xt=0;
    if (ql.count()>=6) {
        int const attente=ql[0].toInt()*coef;
        int const semis=ql[1].toInt()*coef;
        int const semisF=ql[2].toInt()*coef;
        int const plant=ql[3].toInt()*coef;
        int const plantF=ql[4].toInt()*coef;
        int const recolte=ql[5].toInt()*coef;
        if (ql.count()>6) {
            t=ql[6];
            xt=option.rect.left()+attente+semis+semisF+plant+3;
        }
        bool bSemis=true;
        bool bPlant=true;
        bool bDRec=true;
        bool bFRec=true;
        if (ql.count()>9) {
            bSemis=!ql[7].isEmpty();
            bPlant=!ql[8].isEmpty();
            bDRec=!ql[9].isEmpty();
            bFRec=ql[9].startsWith('x');
        }

        r2.setBottom(option.rect.bottom()-2);
        r2.setLeft(option.rect.left()+attente);
        r2.setWidth(0);

        if (semis>0){
            //Période de semis
            if (plant>0) c=cPepiniere; else c=cEnPlace;
            if (bSemis) c.setAlpha(255); else  c.setAlpha(100);
            b.setColor(c);
            r2.setTop(r2.bottom()-4);
            r2.setWidth(semis);
            painter->fillRect(r2,b);
        }
        if (semisF>0){
            //Semis fait, attente plantation
            if(plant>0) c=cPepiniere;
            else c=cEnPlace;
            c.setAlpha(100);
            b.setColor(c);
            r2.setTop(r2.bottom()-4);
            r2.setLeft(r2.left()+r2.width());
            r2.setWidth(semisF);
            painter->fillRect(r2,b);
        }
        if (plant>0) {
            //Période de plantation
            c=cEnPlace;
            if (bPlant) c.setAlpha(255); else  c.setAlpha(150);
            b.setColor(c);
            r2.setTop(r2.bottom()-10);
            r2.setLeft(r2.left()+r2.width());
            r2.setWidth(plant);
            painter->fillRect(r2,b);
        }
        if (plantF>0){
            //Plantation faite, attente récolte
            c=cEnPlace;
            c.setAlpha(150);
            b.setColor(c);
            r2.setTop(r2.bottom()-10);
            r2.setLeft(r2.left()+r2.width());
            r2.setWidth(plantF);
            painter->fillRect(r2,b);
        }
        if (recolte>0 or bDRec or bFRec){
            //Période de récolte
            c=cRecolte;
            c.setAlpha(255);
            if (bDRec) c.setAlpha(255); else  c.setAlpha(100);
            b.setColor(c);
            r2.setTop(r2.bottom()-12);
            r2.setBottom(r2.bottom()-6);
            r2.setLeft(r2.left()+r2.width());
            if (bDRec)
                r2.setWidth(fmax(recolte/2,4));
            else
                r2.setWidth(recolte/2);
            painter->fillRect(r2,b);

            if (bFRec) c.setAlpha(255); else  c.setAlpha(100);
            b.setColor(c);
            r2.setLeft(r2.left()+r2.width());
            if (bFRec)
                r2.setWidth(fmax(recolte/2,4));
            else
                r2.setWidth(recolte/2);
            painter->fillRect(r2,b);
        }
    }

    if (PaintedColsTypes[index.column()]=="TempoNow") {

        c=QColor("red");
        left=option.rect.left()+round(QDate::currentDate().dayOfYear()*coef);
        int r1w=1;

        if (ql.count()>10) {
            //Blue indicator for water.
            QString tDrop=ql[10];
            if (!tDrop.isEmpty()) {
                painter->save();
                QColor waterDropColor(0, 122, 255);
                c=waterDropColor;
                r1w=3;
                //Draw drop
                //painter->setRenderHint(QPainter::Antialiasing, true);
                // painter->setBrush(waterDropColor);
                // painter->setPen(Qt::NoPen);
                int dropX = left;
                int dropY = option.rect.top();
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
                    QFont font = painter->font();
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
        //Vertical bar for now
        b.setColor(c);
        r1.setBottom(option.rect.bottom());
        r1.setTop(option.rect.top());
        r1.setLeft(left-iif(r1w>1,1,0).toInt());
        r1.setWidth(r1w);
        painter->fillRect(r1,b);
    }
    if (!t.isEmpty())
        painter->drawText(xt, option.rect.bottom()-2, t);
}

QWidget *PotaItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const   {
    const PotaTableModel *constModel = qobject_cast<const PotaTableModel *>(index.model());
    if (!constModel) {
        return QStyledItemDelegate::createEditor(parent, option, index); // Standard editor
    }

    PotaTableModel *model = const_cast<PotaTableModel *>(constModel);
    QString sFieldName=model->headerData(index.column(),Qt::Horizontal,Qt::EditRole).toString();
    QString sDataType=model->dataTypes[index.column()];
    QString sComboField=ComboField(model->RealTableName(),sFieldName);
    if (model->relation(index.column()).isValid()) {
        //Create QComboBox for relational columns
        PotaWidget* pw = dynamic_cast<PotaWidget*>(this->parent());
        QComboBox *comboBox = new QComboBox(parent);
        QSqlTableModel *relationModel = model->relationModel(index.column());
        int relationIndex = relationModel->fieldIndex(model->relation(index.column()).displayColumn());
        QString filter;
        if (pw->pageFilterFilters.count()>0)
            filter=FkFilter(model->db,model->RealTableName(),sFieldName,pw->pageFilterFilters[pw->cbPageFilter->currentIndex()],index);
        else
            filter=FkFilter(model->db,model->RealTableName(),sFieldName,"",index);
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
    } else if (!sComboField.isEmpty()){
        PotaQuery query(*model->db);
        QStringList TypeValues=query.Selec0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='Combo_"+sComboField+"'").toString().split("|");
        if (TypeValues.count()>1) {
            QComboBox *comboBox = new QComboBox(parent);
            comboBox->addItem("", QVariant()); // Option for setting a NULL
            for (int i = 0; i < TypeValues.count(); ++i)
                comboBox->addItem(TypeValues[i],TypeValues[i]);
            return comboBox;
        }
    } else if (sDataType=="REAL"){
        return new QLineEdit(parent);
    } else if (sDataType=="INTEGER"){
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
