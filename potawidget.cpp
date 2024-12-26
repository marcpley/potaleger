#include "potawidget.h"
#include "qheaderview.h"
#include "qpushbutton.h"
#include "qsqlerror.h"
#include <QSqlQuery>
#include "PotaUtils.h"
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include "data/Data.cpp"
#include <QList>
#include <Qt>
#include "PotaUtils.h"

PotaWidget::PotaWidget(QWidget *parent) : QWidget(parent)
{
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
            this, SLOT(headerRowClicked(int)));

    //Toolbar
    toolbar = new QWidget(this);

    pbRefresh = new QPushButton(this);
    pbRefresh->setIcon(QIcon(":/images/reload.svg"));
    SetButtonSize(pbRefresh);
    connect(pbRefresh, &QPushButton::released, this, &PotaWidget::pbRefreshClick);

    pbCommit = new QPushButton(this);
    pbCommit->setIcon(QIcon(":/images/commit.svg"));
    SetButtonSize(pbCommit);
    pbCommit->setEnabled(false);
    connect(pbCommit, &QPushButton::released, this, &PotaWidget::pbCommitClick);

    pbRollback = new QPushButton(this);
    pbRollback->setIcon(QIcon(":/images/rollback.svg"));
    SetButtonSize(pbRollback);
    pbRollback->setEnabled(false);
    connect(pbRollback, &QPushButton::released, this, &PotaWidget::pbRollbackClick);

    //Add delete rows
    sbInsertRows = new QSpinBox(this);
    sbInsertRows->setMinimum(1);

    pbInsertRow = new QPushButton(this);
    pbInsertRow->setIcon(QIcon(":/images/insert_row.svg"));
    SetButtonSize(pbInsertRow);
    connect(pbInsertRow, &QPushButton::released, this, &PotaWidget::pbInsertRowClick);

    pbDeleteRow = new QPushButton(this);
    pbDeleteRow->setIcon(QIcon(":/images/delete_row.svg"));
    SetButtonSize(pbDeleteRow);
    pbDeleteRow->setEnabled(false);
    connect(pbDeleteRow, &QPushButton::released, this, &PotaWidget::pbDeleteRowClick);

    //Filtering
    fSort = new QFrame(this);
    fSort->setFrameShape(QFrame::NoFrame);
    cbSort = new QCheckBox(this);
    cbSort->setLayoutDirection(Qt::RightToLeft);
    cbSort->setFixedWidth(150);
    cbSort->setEnabled(false);
    cbSort->setText(tr("Filtrer"));
    sbSort = new QSpinBox(this);
    sbSort->setMinimum(-1);
    sbSort->setValue(10);
    connect(cbSort, &QCheckBox::checkStateChanged, this, &PotaWidget::cbSortClick);

    leSort = new QLineEdit(this);
    //connect(leSort, &QLineEdit::returnPressed, this, &PotaWidget::leSortReturnPressed); A faire.

    //Sort layout
    ls = new QHBoxLayout(this);
    ls->setSizeConstraint(QLayout::SetFixedSize);
    ls->setContentsMargins(0,0,0,0);
    ls->setSpacing(2);
    ls->addWidget(cbSort);
    ls->addWidget(leSort);
    ls->addWidget(sbSort);
    fSort->setLayout(ls);

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
    ltb->addWidget(fSort);
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
        model->setRelation(localColumnIndex, QSqlRelation(referencedTable, referencedClumn, referencedClumn));
        delegate->CouleursColonnes[localColumnIndex]=CouleurTable(query.value("table").toString());//Couleur de la colonne FK de la couleur de la table parente.
    }
    delegate->Couleur=CouleurTable(TableName);

    tv->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    tv->verticalHeader()->setDefaultSectionSize(0);//Mini.
    tv->verticalHeader()->setDefaultAlignment(Qt::AlignTop);
    //tv->verticalHeader()->hide();//Rend impossible d'augmenter la hauteur des lignes.

    //connect(w->tv,QAccessibleTableModelChangeEvent::DataChanged,SLOT());
}

void PotaWidget::curChanged(const QModelIndex cur, const QModelIndex pre)
{
    tv->clearSpans();//Pour forcer le dessin de toute la grille, pour que la ligne sélectionnée soit visible.

    bool b=(tv->selectionModel()->selectedRows().count()>0);
    pbDeleteRow->setEnabled(b);//b wrong true if click a cell to deselect a header clicked line.

    if (!cbSort->isChecked())
    {
        cbSort->setText(model->FieldName(cur.column()));
        cbSort->setEnabled(true);

        if ((sbSort->value()==-1)or
            (sbSort->value()>model->index(cur.row(),cur.column()).data(Qt::DisplayRole).toString().length()))
            leSort->setText(model->index(cur.row(),cur.column()).data(Qt::DisplayRole).toString());
        else
            leSort->setText(model->index(cur.row(),cur.column()).data(Qt::DisplayRole).toString().first(sbSort->value()));
    }

}

void PotaWidget::dataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight,const QList<int> &roles)
{
    //isPendingModifs=true;
    pbCommit->setEnabled(true);
    pbRollback->setEnabled(true);
    cbSort->setEnabled(false);

    qDebug() << topLeft.row() << " - " << topLeft.column();
    qDebug() << "UserRole" << topLeft.data(Qt::ItemDataRole::UserRole).toString();
    model->index(topLeft.row(),topLeft.column()).data(Qt::ItemDataRole::UserRole).setValue("test");
    qDebug() << "UserRole" << topLeft.data(Qt::ItemDataRole::UserRole).toString();

    //palette().window() A faire, mettre le titre de l'onglet en rouge.
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

void PotaWidget::headerRowClicked(int logicalIndex)
{
    pbDeleteRow->setEnabled(true);
}

void PotaWidget::pbRefreshClick()
{
    model->SubmitAllShowErr();
    model->SelectShowErr();
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

void PotaWidget::cbSortClick(Qt::CheckState state)
{
    qDebug() << model->filter();
    if (state==Qt::CheckState::Checked)
    {
        //Sort
        model->setFilter(cbSort->text()+" LIKE '"+leSort->text()+"%'");
        fSort->setFrameShape(QFrame::Box);

    }
    else
    {
        //Reset sort
        model->setFilter("TRUE");
        fSort->setFrameShape(QFrame::NoFrame);
        //model->SelectShowErr();
    }
    qDebug() << model->filter();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//PotaTableModel::PotaTableModel(){}

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
    //Il faudrait refaire les FK pour que les listes déroulantes soient à jour.
    //Mais appeler setRelation() ne suffit pas.
    if ((lastError().type() != QSqlError::NoError)and(parent()->objectName().startsWith("PW")))
    {
        SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,lastError().text(),"Err");
        return false;
    }
    return true;
}

bool PotaTableModel::SubmitAllShowErr()
{
    submitAll();
    if (lastError().type() == QSqlError::NoError)
    {
        SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,tableName()+": "+tr("modifications enregistrées."),"Ok");
        //dynamic_cast<PotaWidget*>(parent())->isPendingModifs=false;
        dynamic_cast<PotaWidget*>(parent())->isCommittingError=false;
        dynamic_cast<PotaWidget*>(parent())->pbCommit->setEnabled(false);
        dynamic_cast<PotaWidget*>(parent())->pbRollback->setEnabled(false);
        dynamic_cast<PotaWidget*>(parent())->cbSort->setEnabled(true);

    }
    else
    {
        SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,lastError().text(),"Err");
        dynamic_cast<PotaWidget*>(parent())->isCommittingError=true;
        return false;
    }
    return true;
}

bool PotaTableModel::RevertAllShowErr()
{
    revertAll();
    if (lastError().type() == QSqlError::NoError)
    {
        SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,tableName()+": "+tr("modifications abandonnées."),"Info");
        //dynamic_cast<PotaWidget*>(parent())->isPendingModifs=false;/
        dynamic_cast<PotaWidget*>(parent())->isCommittingError=false;
        dynamic_cast<PotaWidget*>(parent())->pbCommit->setEnabled(false);
        dynamic_cast<PotaWidget*>(parent())->pbRollback->setEnabled(false);
        dynamic_cast<PotaWidget*>(parent())->cbSort->setEnabled(true);
    }
    else
    {
        SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,lastError().text(),"Err");
        dynamic_cast<PotaWidget*>(parent())->isCommittingError=true;
        return false;
    }
    return true;
}

bool PotaTableModel::InsertRowShowErr()
{

    if (insertRows(dynamic_cast<PotaWidget*>(parent())->tv->currentIndex().row()+1,
                   dynamic_cast<PotaWidget*>(parent())->sbInsertRows->value()))
    {
        dynamic_cast<PotaWidget*>(parent())->pbCommit->setEnabled(true);
        dynamic_cast<PotaWidget*>(parent())->pbRollback->setEnabled(true);
        dynamic_cast<PotaWidget*>(parent())->cbSort->setEnabled(false);
    }
    else
    {
        SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,"insertRows(x,y)","Err");
        return false;
    }
    return true;
}

bool PotaTableModel::DeleteRowShowErr()
{
    QItemSelectionModel *sel = dynamic_cast<PotaWidget*>(parent())->tv->selectionModel();
    int rr=0;
    for (int i = 0 ; i<rowCount() ; i++)
    {
        if (sel->isRowSelected(i))
        {
            if (removeRow(i))
                rr++;
            else
                SetColoredText(dynamic_cast<PotaWidget*>(parent())->lErr,"removeRows("+str(i)+")","Err");
        }
    }
    if (rr>0)
    {
        dynamic_cast<PotaWidget*>(parent())->pbCommit->setEnabled(true);
        dynamic_cast<PotaWidget*>(parent())->pbRollback->setEnabled(true);
        dynamic_cast<PotaWidget*>(parent())->cbSort->setEnabled(false);
        return true;
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//PotaQueryModel::PotaQueryModel(){}

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
//PotaItemDelegate::PotaItemDelegate(QObject *parent) : QSqlRelationalDelegate(parent) {}

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
        if (index.data(Qt::DisplayRole) == "1")
        {
            c=Qt::green;
            c.setAlpha(50);
        }
    }
    if (!c.isValid())//Table color.
    {
        c=CouleursColonnes[index.column()];//FK
        if (!c.isValid())
            c=Couleur;
        c.setAlpha(30);
    }
    if (c.isValid())
    {
        b.setColor(c);
        painter->fillRect(option.rect,b);
    }

    QSqlRelationalDelegate::paint(painter, option, index);

    if (index.data(Qt::ItemDataRole::UserRole).toString()=="isModified")//A faire, marche pas
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
