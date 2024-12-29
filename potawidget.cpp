#include "potawidget.h"
#include "qheaderview.h"
#include "qlineedit.h"
#include "qsqlerror.h"
#include <QSqlQuery>
#include "PotaUtils.h"
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QList>
#include <Qt>
#include "PotaUtils.h"

PotaWidget::PotaWidget(QWidget *parent) : QWidget(parent)
{
    twParent = dynamic_cast<QTabWidget*>(parent);
    model = new PotaTableModel();
    model->setParent(this);
    delegate = new PotaItemDelegate();
    delegate->setParent(this);
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
    connect(pbRefresh, &QToolButton::released, this, &PotaWidget::pbRefreshClick);

    pbCommit = new QToolButton(this);
    pbCommit->setIcon(QIcon(":/images/commit.svg"));
    SetButtonSize(pbCommit);
    pbCommit->setEnabled(false);
    connect(pbCommit, &QToolButton::released, this, &PotaWidget::pbCommitClick);

    pbRollback = new QToolButton(this);
    pbRollback->setIcon(QIcon(":/images/rollback.svg"));
    SetButtonSize(pbRollback);
    pbRollback->setEnabled(false);
    connect(pbRollback, &QToolButton::released, this, &PotaWidget::pbRollbackClick);

    //Add delete rows
    sbInsertRows = new QSpinBox(this);
    sbInsertRows->setMinimum(1);

    pbInsertRow = new QToolButton(this);
    pbInsertRow->setIcon(QIcon(":/images/insert_row.svg"));
    SetButtonSize(pbInsertRow);
    connect(pbInsertRow, &QToolButton::released, this, &PotaWidget::pbInsertRowClick);

    pbDeleteRow = new QToolButton(this);
    pbDeleteRow->setIcon(QIcon(":/images/delete_row.svg"));
    SetButtonSize(pbDeleteRow);
    pbDeleteRow->setEnabled(false);
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
    QSqlQuery query("PRAGMA foreign_key_list("+TableName+");");
    while (query.next()) {
        QString referencedTable = query.value("table").toString();
        QString localColumn = query.value("from").toString();
        QString referencedClumn = query.value("to").toString();
        int localColumnIndex = model->fieldIndex(localColumn);
        model->setRelation(localColumnIndex, QSqlRelation(referencedTable, referencedClumn, referencedClumn));//Issue #2
    }

    tv->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    tv->verticalHeader()->setDefaultSectionSize(0);//Mini.
    tv->verticalHeader()->setDefaultAlignment(Qt::AlignTop);
}

void PotaWidget::curChanged(const QModelIndex cur, const QModelIndex pre)
{
    tv->clearSpans();//Pour forcer le dessin de toute la grille, pour que la ligne sélectionnée soit visible.

    bool b=(tv->selectionModel()->selectedRows().count()>0);
    pbDeleteRow->setEnabled(!isView and b);//b wrong true if user click a cell to deselect a header clicked line.

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
        //isPendingModifs=true;
        pbCommit->setEnabled(true);
        twParent->setTabIcon(twParent->currentIndex(),QIcon(":/images/yellowbullet.svg"));
        pbRollback->setEnabled(true);
        cbFilter->setEnabled(false);
    }

    qDebug() << topLeft.row() << " - " << topLeft.column();
    qDebug() << "UserRole" << topLeft.data(Qt::ItemDataRole::UserRole).toString();
    model->index(topLeft.row(),topLeft.column()).data(Qt::ItemDataRole::UserRole).setValue("IsModified"); //todo.  Issue #3
    qDebug() << "UserRole" << topLeft.data(Qt::ItemDataRole::UserRole).toString();
}

void PotaWidget::headerColClicked(int logicalIndex)
{
    if (iSortCol==-1)//Pas de tri pour le moment.
    {
        model->sort(logicalIndex,Qt::SortOrder::AscendingOrder);
        model->setHeaderData(logicalIndex, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_GreenDown.svg")), Qt::DecorationRole);
        iSortCol=logicalIndex;
        bSortDes=false;
    }
    else if (iSortCol==logicalIndex)//Tri actuel sur cette colonne.
    {
        if (!bSortDes)
        {
            model->sort(logicalIndex,Qt::SortOrder::DescendingOrder);
            model->setHeaderData(logicalIndex, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_RedUp.svg")), Qt::DecorationRole);
            bSortDes=true;
        }
        else//Sort on 0 col. Best would be reset sorting.
        {
            model->setHeaderData(iSortCol, Qt::Horizontal, 0, Qt::DecorationRole);
            model->sort(0,Qt::SortOrder::AscendingOrder);
            model->setHeaderData(0, Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_GreenDown.svg")), Qt::DecorationRole);
            iSortCol=0;
            bSortDes=false;
        }
    }
    else
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
    pbDeleteRow->setEnabled(true);
}

void PotaWidget::pbRefreshClick()
{
    int i=tv->selectionModel()->currentIndex().row();
    int j=tv->selectionModel()->currentIndex().column();
    if (pbCommit->isEnabled())
        model->SubmitAllShowErr();
    model->SelectShowErr();
    tv->selectionModel()->setCurrentIndex(model->index(i,j),QItemSelectionModel::Current);//todo: retreive the reccord, not the line
}

void PotaWidget::pbCommitClick()
{
    model->SubmitAllShowErr();
}

void PotaWidget::pbRollbackClick()
{
    model->RevertAllShowErr();
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
        model->setFilter(cbFilter->text()+" LIKE '"+leFilter->text()+"%'");//Issue #4
        fFilter->setFrameShape(QFrame::Box);

    }
    else
    {
        //Reset sort
        model->setFilter("TRUE");
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
    QSqlQuery query("PRAGMA table_xinfo("+tableName()+");");
    if (query.seek(index))
        return query.value("name").toString();
    else
        return "";
}

bool PotaTableModel::SelectShowErr()
{
    select();
    //Combo FK are not updated and could out of date. A recall off setRelation() don't fix that.
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
            //pw->isPendingModifs=false;
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
            //pw->isPendingModifs=false;/
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
        QItemSelectionModel *sel = pw->tv->selectionModel();
        int rr=0;
        for (int i = 0 ; i<rowCount() ; i++)
        {
            if (sel->isRowSelected(i))
            {
                if (removeRow(i))
                    rr++;
                else
                    SetColoredText(pw->lErr,"removeRows("+str(i)+")","Err");
            }
        }
        if (rr>0)
        {
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
//                                 PotaQueryModel
//////////////////////////////////////////////////////////////////////////////////////////////////

QString PotaQueryModel::FieldName(int index)
{
    if (TableName=="")
        return "";
    else
    {
        if (index < columnCount())
            return headerData(index,Qt::Horizontal).toString();
        else
            return "";
    }
}

bool PotaQueryModel::setQueryShowErr(QString query)
{
    setQuery(query);
    if (lastError().type() != QSqlError::NoError)
    {
        if (lErr!=nullptr)
            SetColoredText(lErr,lastError().text(),"Err");
        return false;

    }
    return true;
}

bool PotaQueryModel::setMultiQueryShowErr(QString querys)
{
    QStringList QueryList = querys.split(";");
    QString s;
    s=lErr->text();
    if ((s.length()>11)and(s.last(11)==" statements"))
        s=s.first(s.length()-11)+"+"+str(QueryList.count())+" statements";
    else
        s=str(QueryList.count())+" statements";
    SetColoredText(lErr,s,"Info");
    for (int i=0;i<QueryList.count();i++)
    {
        if (QueryList[i].trimmed().isEmpty())
            continue;
        setQuery(QueryList[i]);
        if (lastError().type() != QSqlError::NoError)
        {
            if (lErr!=nullptr)
                SetColoredText(lErr,lastError().text(),"Err");
            return false;

        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                                  PotaItemDelegate
//////////////////////////////////////////////////////////////////////////////////////////////////

//QWidget *PotaItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
//    return QStyledItemDelegate::createEditor(parent,option,index);
//}

void PotaItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    QBrush b;
    b.setStyle(Qt::SolidPattern);
    QColor c;
    QItemSelectionModel *selection = dynamic_cast<PotaWidget*>(parent())->tv->selectionModel();

    if (index.row() == selection->currentIndex().row())//Line selected
    {
        c=Qt::blue;
        c.setAlpha(70);
    }
    else //Row data color
    {
        // if (index.data(Qt::DisplayRole) == "1") todo
        // {
        //     c=Qt::green;
        //     c.setAlpha(50);
        // }
    }
    if (!c.isValid())//Table color.
    {
        c=cColColors[index.column()];//FK
        if (!c.isValid())
            c=cTableColor;
        c.setAlpha(30);
    }
    if (c.isValid())
    {
        b.setColor(c);
        painter->fillRect(option.rect,b);
    }

    QSqlRelationalDelegate::paint(painter, option, index);

    if (index.data(Qt::ItemDataRole::UserRole).toString()=="isModified")//Don't work. todo. Issue #3
    {
        painter->save();
        painter->setPen(dynamic_cast<PotaWidget*>(parent())->isCommittingError ? QColor(Qt::red) : QColor(Qt::blue));
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
        painter->restore();
    }
}

//void PotaItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
//    QStyledItemDelegate::setEditorData(editor,index);
//}
