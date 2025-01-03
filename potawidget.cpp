#include "potawidget.h"
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


PotaWidget::PotaWidget(QWidget *parent) : QWidget(parent)
{
    twParent = dynamic_cast<QTabWidget*>(parent);
    model = new PotaTableModel();
    model->setParent(this);
    delegate = new PotaItemDelegate2();
    delegate->setParent(this);
    //delegateFK = new PotaItemDelegateFK();
    //delegateFK->setParent(this);
    query = new PotaQuery();
    tv = new PotaTableView();
    tv->setParent(this);
    tv->setModel(model);
    tv->setItemDelegate(delegate);
    connect(tv->selectionModel(), SIGNAL(currentChanged(const QModelIndex,const QModelIndex)),
            this, SLOT(curChanged(const QModelIndex,const QModelIndex)));
    connect(model, SIGNAL(dataChanged(const QModelIndex,const QModelIndex,const QList<int>)),
            this, SLOT(dataChanged(const QModelIndex,const QModelIndex,const QList<int>)));
    connect(tv->horizontalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(headerColClicked(int)));
    connect(tv->verticalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(headerRowClicked()));//int

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

    pbInsertRow = new QToolButton(this);
    pbInsertRow->setIcon(QIcon(":/images/insert_row.svg"));
    SetButtonSize(pbInsertRow);
    pbInsertRow->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Insert));
    pbInsertRow->setToolTip(tr("Ajouter des lignes.")+"\n"+
                            "Ctrl + Insert");
    connect(pbInsertRow, &QToolButton::released, this, &PotaWidget::pbInsertRowClick);

    pbDeleteRow = new QToolButton(this);
    pbDeleteRow->setIcon(QIcon(":/images/delete_row.svg"));
    SetButtonSize(pbDeleteRow);
    pbDeleteRow->setShortcut( QKeySequence(Qt::CTRL | Qt::Key_Delete));
    pbDeleteRow->setToolTip(tr("Supprimer des lignes.")+"\n"+
                            "Ctrl + Delete (Suppr)");
    connect(pbDeleteRow, &QToolButton::released, this, &PotaWidget::pbDeleteRowClick);

    //Filtering
    fFilter = new QFrame(this);
    fFilter->setFrameShape(QFrame::NoFrame);
    cbFilter = new QCheckBox(this);
    cbFilter->setLayoutDirection(Qt::RightToLeft);
    cbFilter->setFixedWidth(150);
    cbFilter->setEnabled(false);
    cbFilter->setText(tr("Filtrer"));
    leFilter = new QLineEdit(this);
    connect(leFilter, &QLineEdit::returnPressed, this, &PotaWidget::leFilterReturnPressed);
    sbFilter = new QSpinBox(this);
    sbFilter->setMinimum(-1);
    sbFilter->setValue(5);
    connect(cbFilter, &QCheckBox::checkStateChanged, this, &PotaWidget::cbFilterClick);
    lFilter = new QLabel(this);
    lFilter->setFixedWidth(150);


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
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);//OnFieldChange

    //FK
    query->ExecShowErr("PRAGMA foreign_key_list("+TableName+");");
    while (query->next()) {
        QString referencedTable = query->value("table").toString();
        QString localColumn = query->value("from").toString();
        QString referencedClumn = query->value("to").toString();
        int localColumnIndex = model->fieldIndex(localColumn);
        model->setRelation(localColumnIndex, QSqlRelation(referencedTable, referencedClumn, referencedClumn));//Issue #2
    }

    tv->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    tv->verticalHeader()->setDefaultSectionSize(0);//Mini.
    tv->verticalHeader()->setDefaultAlignment(Qt::AlignTop);
}

void PotaWidget::curChanged(const QModelIndex cur, const QModelIndex pre)
{
    tv->clearSpans();//Force redraw of grid, for selected ligne visibility.

    qDebug() << cur.row() << " " << cur.column();

    if (!cbFilter->isChecked())
    {
        //cbFilter->setText(model->FieldName(cur.column()));
        cbFilter->setText(model->headerData(cur.column(),Qt::Horizontal,Qt::DisplayRole).toString());

        cbFilter->setEnabled(true);

        if ((sbFilter->value()==-1)or
            (sbFilter->value()>model->index(cur.row(),cur.column()).data(Qt::DisplayRole).toString().length()))
            leFilter->setText(model->index(cur.row(),cur.column()).data(Qt::DisplayRole).toString());
        else
            leFilter->setText(model->index(cur.row(),cur.column()).data(Qt::DisplayRole).toString().first(sbFilter->value()));
    }

}

void PotaWidget::dataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight,const QList<int> &roles)
{
    if (!isView)
    {
        pbCommit->setEnabled(true);
        pbRollback->setEnabled(true);
        cbFilter->setEnabled(false);
        twParent->setTabIcon(twParent->currentIndex(),QIcon(":/images/yellowbullet.svg"));

        QVariant variant;
        if (!topLeft.data(Qt::EditRole).isNull() and
            (topLeft.data(Qt::EditRole) == ""))
            model->setData(topLeft,variant,Qt::EditRole);//To avoid empty non null values.
    }
}

void PotaWidget::headerColClicked(int logicalIndex)
{
    if (iSortCol==logicalIndex)//Sort on this column.
    {
        if (!bSortDes)
        {
            model->sort(logicalIndex,Qt::SortOrder::DescendingOrder);
            model->setHeaderData(logicalIndex, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_RedUp.svg")), Qt::DecorationRole);
            bSortDes=true;
        }
        else//Reset sorting.
        {
            model->setHeaderData(iSortCol, Qt::Horizontal, 0, Qt::DecorationRole);
            model->sort( -1,Qt::SortOrder::AscendingOrder);
            //model->setHeaderData(0, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_GreenDown.svg")), Qt::DecorationRole);
            iSortCol=0;
            bSortDes=false;
        }
    }
    else//Actual sort on another column.
    {
        model->setHeaderData(iSortCol, Qt::Horizontal, 0, Qt::DecorationRole);
        model->sort(logicalIndex,Qt::SortOrder::AscendingOrder);
        model->setHeaderData(logicalIndex, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_GreenDown.svg")), Qt::DecorationRole);
        iSortCol=logicalIndex;
        bSortDes=false;
    }
}

void PotaWidget::headerRowClicked() //int logicalIndex
{
    //pbDeleteRow->setEnabled(true);
}

void PotaWidget::pbRefreshClick()
{
    int i=tv->selectionModel()->currentIndex().row();
    int j=tv->selectionModel()->currentIndex().column();
    if (pbCommit->isEnabled())
        model->SubmitAllShowErr();
    model->clearCellCopied();
    model->SelectShowErr();
    tv->selectionModel()->setCurrentIndex(model->index(i,j),QItemSelectionModel::Current);//todo: retreive the reccord, not the line
}

void PotaWidget::pbCommitClick()
{
    tv->setFocus();
    model->SubmitAllShowErr();
}

void PotaWidget::pbRollbackClick()
{
    model->clearCellCopied();
    model->RevertAllShowErr();
    tv->setFocus();
}

void PotaWidget::pbInsertRowClick()
{
    model->InsertRowShowErr();
}

void PotaWidget::pbDeleteRowClick()
{
    model->DeleteRowShowErr();
}

void PotaWidget::cbFilterClick(Qt::CheckState state)
{
    if (state==Qt::CheckState::Checked)
    {
        //Sort
        if (leFilter->text()=="")
            model->setFilter("\""+cbFilter->text()+"\" ISNULL");
        else
            model->setFilter("\""+cbFilter->text()+"\" LIKE '"+leFilter->text()+"%'");//todo : escape ' caracter in leFilter->text()
        fFilter->setFrameShape(QFrame::Box);

    }
    else
    {
        //Reset sort
        model->setFilter("");
        fFilter->setFrameShape(QFrame::NoFrame);
    }
    if (isView)
        lFilter->setText(str(model->rowCount())+" "+tr("lignes"));
    else
        lFilter->setText(str(model->rowCount())+" "+model->tableName().toLower());
    qDebug() << model->filter();
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
    select();

    //Combo FK are not updated and could be out of date. A recall off setRelation() don't fix that.
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
    if (!pw->isView)
    {
        submitAll();
        if (lastError().type() == QSqlError::NoError)
        {
            SetColoredText(pw->lErr,tableName()+": "+tr("modifications enregistrées."),"Ok");
            pw->isCommittingError=false;
            pw->pbCommit->setEnabled(false);
            pw->pbRollback->setEnabled(false);
            pw->cbFilter->setEnabled(true);
            pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(""));

        }
        else
        {
            SetColoredText(pw->lErr,lastError().text(),"Err");
            pw->isCommittingError=true;
            return false;
        }
        return true;
    }
    else
        return false;
}

bool PotaTableModel::RevertAllShowErr()
{
    PotaWidget *pw = dynamic_cast<PotaWidget*>(parent());
    if (!pw->isView)
    {
        revertAll();
        if (lastError().type() == QSqlError::NoError)
        {
            SetColoredText(pw->lErr,tableName()+": "+tr("modifications abandonnées."),"Info");
            pw->isCommittingError=false;
            pw->pbCommit->setEnabled(false);
            pw->pbRollback->setEnabled(false);
            pw->cbFilter->setEnabled(true);
            pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(""));

        }
        else
        {
            SetColoredText(pw->lErr,lastError().text(),"Err");
            pw->isCommittingError=true;
            return false;
        }
        return true;
    }
    else
        return false;
}

bool PotaTableModel::InsertRowShowErr()
{
    PotaWidget *pw = dynamic_cast<PotaWidget*>(parent());
    if (!pw->isView)
    {
        if (insertRows(pw->tv->currentIndex().row()+1,
                       pw->sbInsertRows->value()))
        {
            pw->pbCommit->setEnabled(true);
            pw->pbRollback->setEnabled(true);
            pw->cbFilter->setEnabled(false);
            pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(":/images/yellowbullet.svg"));
        }
        else
        {
            SetColoredText(pw->lErr,"insertRows(x,y)","Err");
            return false;
        }
        return true;
    }
    else
        return false;
}

bool PotaTableModel::DeleteRowShowErr()
{
    PotaWidget *pw = dynamic_cast<PotaWidget*>(parent());
    if (!pw->isView)
    {
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
            pw->twParent->setTabIcon(pw->twParent->currentIndex(),QIcon(":/images/yellowbullet.svg"));
            return true;
        }
        return false;
    }
    else
        return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//                                  PotaItemDelegate
//////////////////////////////////////////////////////////////////////////////////////////////////


void PotaItemDelegate2::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    PotaWidget* pw = dynamic_cast<PotaWidget*>(parent());
    //if (isDarkTheme()) todo

    QBrush b;
    b.setStyle(Qt::SolidPattern);
    QColor c;
    QItemSelectionModel *selection = pw->tv->selectionModel();

    // if (pw->model && pw->model->isCellCopied(index)) {
    //     c=QApplication::palette().color(QPalette::Highlight);
    //     c.setAlpha(100);
    // } else
    if (pw->model && pw->model->isRowMarkedForRemoval(index.row())) {
        // Surligner les cellules de la ligne supprimée avec un fond gris clair
        QStyleOptionViewItem opt = option;
        opt.backgroundBrush = QBrush(QColor(220, 220, 220)); // Gris clair
        return;
    } else if (index.row() == selection->currentIndex().row()) {//Line selected
        //c=Qt::blue;
        c=QApplication::palette().color(QPalette::Highlight);
        c.setAlpha(70);
    } else {//Row data color
        if (!index.data(Qt::EditRole).isNull() and
            (index.data(Qt::EditRole) == "")) {
            //Not null empty value. Not normal, it causes SQL failure.
            c=Qt::red;
            c.setAlpha(100);
        }
    }
    if (!c.isValid()) {//Table color.
        c=cColColors[index.column()];//FK
        if (!c.isValid())
            c=cTableColor;
        c.setAlpha(30);
    }
    if (c.isValid()) {
        b.setColor(c);
        painter->fillRect(option.rect,b);
    }

    QStyledItemDelegate::paint(painter, option, index);

    if (pw->model && pw->model->isCellCopied(index)) {
        painter->save();
        painter->setPen(QColor(Qt::green));
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
        painter->restore();
    }
    if (pw->model && pw->model->isCellModified(index)) {
        painter->save();
        painter->setPen(pw->isCommittingError ? QColor(Qt::red) : QColor(Qt::blue));
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
        painter->restore();
    }
}
