#include "bookwindow.h"
#include <QtSql>
#include <QModelIndex>
#include <QPixmap>
#include <QSize>
#include <QSqlRelationalDelegate>
#include "resourceloader.h"

using namespace fl_common;
void addBook(QSqlQuery &q, const QString &title, int year, const QVariant &authorId,
             const QVariant &genreId, int rating)
{
    q.addBindValue(title);
    q.addBindValue(year);
    q.addBindValue(authorId);
    q.addBindValue(genreId);
    q.addBindValue(rating);
    q.exec();
}

QVariant addGenre(QSqlQuery &q, const QString &name)
{
    q.addBindValue(name);
    q.exec();
    return q.lastInsertId();
}

QVariant addAuthor(QSqlQuery &q, const QString &name, const QDate &birthdate)
{
    q.addBindValue(name);
    q.addBindValue(birthdate);
    q.exec();
    return q.lastInsertId();
}

QSqlError initDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");

    if (!db.open())
        return db.lastError();

    QStringList tables = db.tables();
    if (tables.contains("books", Qt::CaseInsensitive)
        && tables.contains("authors", Qt::CaseInsensitive))
        return QSqlError();

    QSqlQuery q;
    if (!q.exec(QLatin1String("create table books(id integer primary key, title varchar, author integer, genre integer, year integer, rating integer)")))
        return q.lastError();
    if (!q.exec(QLatin1String("create table authors(id integer primary key, name varchar, birthdate date)")))
        return q.lastError();
    if (!q.exec(QLatin1String("create table genres(id integer primary key, name varchar)")))
        return q.lastError();

    if (!q.prepare(QLatin1String("insert into authors(name, birthdate) values(?, ?)")))
        return q.lastError();
    QVariant asimovId = addAuthor(q, QLatin1String("Isaac Asimov"), QDate(1920, 2, 1));
    QVariant greeneId = addAuthor(q, QLatin1String("Graham Greene"), QDate(1904, 10, 2));
    QVariant pratchettId = addAuthor(q, QLatin1String("Terry Pratchett"), QDate(1948, 4, 28));

    if (!q.prepare(QLatin1String("insert into genres(name) values(?)")))
        return q.lastError();
    QVariant sfiction = addGenre(q, QLatin1String("Science Fiction"));
    QVariant fiction = addGenre(q, QLatin1String("Fiction"));
    QVariant fantasy = addGenre(q, QLatin1String("Fantasy"));

    if (!q.prepare(QLatin1String("insert into books(title, year, author, genre, rating) values(?, ?, ?, ?, ?)")))
        return q.lastError();
    addBook(q, QLatin1String("Foundation"), 1951, asimovId, sfiction, 3);
    addBook(q, QLatin1String("Foundation and Empire"), 1952, asimovId, sfiction, 4);
    addBook(q, QLatin1String("Second Foundation"), 1953, asimovId, sfiction, 3);
    addBook(q, QLatin1String("Foundation's Edge"), 1982, asimovId, sfiction, 3);
    addBook(q, QLatin1String("Foundation and Earth"), 1986, asimovId, sfiction, 4);
    addBook(q, QLatin1String("Prelude to Foundation"), 1988, asimovId, sfiction, 3);
    addBook(q, QLatin1String("Forward the Foundation"), 1993, asimovId, sfiction, 3);
    addBook(q, QLatin1String("The Power and the Glory"), 1940, greeneId, fiction, 4);
    addBook(q, QLatin1String("The Third Man"), 1950, greeneId, fiction, 5);
    addBook(q, QLatin1String("Our Man in Havana"), 1958, greeneId, fiction, 4);
    addBook(q, QLatin1String("Guards! Guards!"), 1989, pratchettId, fantasy, 3);
    addBook(q, QLatin1String("Night Watch"), 2002, pratchettId, fantasy, 3);
    addBook(q, QLatin1String("Going Postal"), 2004, pratchettId, fantasy, 3);

    return QSqlError();
}


class BookDelegate : public QSqlRelationalDelegate
{
public:
    BookDelegate(QObject *parent)
        : QSqlRelationalDelegate(parent), star(QPixmap(ResourceLoader::instacne()->rc("books/star"))){}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (index.column() != 5) {
            QStyleOptionViewItem opt = option;
            // Since we draw the grid ourselves:
            opt.rect.adjust(0, 0, -1, -1);
            QSqlRelationalDelegate::paint(painter, opt, index);
        } else {
            const QAbstractItemModel *model = index.model();
            QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ?
                (option.state & QStyle::State_Active) ?
                            QPalette::Normal :
                            QPalette::Inactive :
                            QPalette::Disabled;

            if (option.state & QStyle::State_Selected)
                painter->fillRect(
                            option.rect,
                            option.palette.color(cg, QPalette::Highlight));

            int rating = model->data(index, Qt::DisplayRole).toInt();
            int width = star.width();
            int height = star.height();
            int x = option.rect.x();
            int y = option.rect.y() + (option.rect.height() / 2) - (height / 2);
            for (int i = 0; i < rating; ++i) {
                painter->drawPixmap(x, y, star);
                x += width;
            }
            // Since we draw the grid ourselves:
            drawFocus(painter, option, option.rect.adjusted(0, 0, -1, -1));
        }

        QPen pen = painter->pen();
        painter->setPen(option.palette.color(QPalette::Mid));
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
        painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
        painter->setPen(pen);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (index.column() == 5)
            return QSize(5 * star.width(), star.height()) + QSize(1, 1);
        // Since we draw the grid ourselves:
        return QSqlRelationalDelegate::sizeHint(option, index) + QSize(1, 1);
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,const QModelIndex &index)
    {
        if (index.column() != 5)
            return QSqlRelationalDelegate::editorEvent(event, model, option, index);

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            int stars = qBound(0, int(0.7 + qreal(mouseEvent->pos().x()
                - option.rect.x()) / star.width()), 5);
            model->setData(index, QVariant(stars));
            // So that the selection can change:
            return false;
        }

        return true;
    }

    QWidget *createEditor(QWidget *parent,const QStyleOptionViewItem &option,const QModelIndex &index) const
    {
        if (index.column() != 4)
            return QSqlRelationalDelegate::createEditor(parent, option, index);

        // For editing the year, return a spinbox with a range from -1000 to 2100.
        QSpinBox *sb = new QSpinBox(parent);
        sb->setFrame(false);
        sb->setMaximum(2100);
        sb->setMinimum(-1000);

        return sb;
    }

private:
    QPixmap star;
};

BookWindow::BookWindow()
{
    ui.setupUi(this);

   // qDebug() << ResourceLoader::instacne()->rc("star");

    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        QMessageBox::critical(
                    this,
                    "Unable to load database",
                    "This demo needs the SQLITE driver"
                    );

    // Initialize the database:
    QSqlError err = initDb();
    if (err.type() != QSqlError::NoError) {
       showError( err);
        return;
    }

    ui.bookTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // Create the data model:
    model = new QSqlRelationalTableModel(ui.bookTable);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setTable("books");

    // Remember the indexes of the columns:
    authorIdx = model->fieldIndex("author");
    genreIdx = model->fieldIndex("genre");

    // Set the relations to the other database tables:
    model->setRelation(authorIdx, QSqlRelation("authors", "id", "name"));
    model->setRelation(genreIdx, QSqlRelation("genres", "id", "name"));

    // Set the localized header captions:
    model->setHeaderData(authorIdx, Qt::Horizontal, tr("Author Name"));
    model->setHeaderData(genreIdx, Qt::Horizontal, tr("Genre"));
    model->setHeaderData(model->fieldIndex("title"),
                         Qt::Horizontal, tr("Title"));
    model->setHeaderData(model->fieldIndex("year"), Qt::Horizontal, tr("Year"));
    model->setHeaderData(model->fieldIndex("rating"),
                         Qt::Horizontal, tr("Rating"));

    // Populate the model:
    if (!model->select()) {
        showError(model->lastError());
        return;
    }

    // Set the model and hide the ID column:
    ui.bookTable->setModel(model);
    ui.bookTable->setItemDelegate(new BookDelegate(ui.bookTable));
    ui.bookTable->setColumnHidden(model->fieldIndex("id"), true);
    ui.bookTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Initialize the Author combo box:
    ui.authorEdit->setModel(model->relationModel(authorIdx));
    ui.authorEdit->setModelColumn(
                model->relationModel(authorIdx)->fieldIndex("name"));

    ui.genreEdit->setModel(model->relationModel(genreIdx));
    ui.genreEdit->setModelColumn(
                model->relationModel(genreIdx)->fieldIndex("name"));

    // Lock and prohibit resizing of the width of the rating column:
    ui.bookTable->horizontalHeader()->setSectionResizeMode(
                model->fieldIndex("rating"),
                QHeaderView::ResizeToContents);

    QDataWidgetMapper *mapper = new QDataWidgetMapper(this);
    mapper->setModel(model);
    mapper->setItemDelegate(new BookDelegate(this));
    mapper->addMapping(ui.titleEdit, model->fieldIndex("title"));
    mapper->addMapping(ui.yearEdit, model->fieldIndex("year"));
    mapper->addMapping(ui.authorEdit, authorIdx);
    mapper->addMapping(ui.genreEdit, genreIdx);
    mapper->addMapping(ui.ratingEdit, model->fieldIndex("rating"));

    connect(ui.bookTable->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            mapper,
            &QDataWidgetMapper::setCurrentModelIndex
            );

    ui.bookTable->setCurrentIndex(model->index(0, 0));
    this->setContentsMargins(0,0,0,0);
}


void BookWindow::showError(const QSqlError &err)
{
    QMessageBox::critical(this, "Unable to initialize Database",
                "Error initializing database: " + err.text());
}
