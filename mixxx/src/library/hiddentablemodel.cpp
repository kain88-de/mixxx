#include "library/hiddentablemodel.h"

HiddenTableModel::HiddenTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection,
                                     QStringList availableDirs)
        : BaseSqlTableModel(parent, pTrackCollection,
                            NULL, availableDirs,
                            "mixxx.db.model.missing") {
    setTableModel(0,QString());
}

HiddenTableModel::~HiddenTableModel() {
}

void HiddenTableModel::setTableModel(int id,QString name){
    Q_UNUSED(id);
    QSqlQuery query;
    QString tableName("hidden_songs_"+name);

    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID;

    QString filter("mixxx_deleted=1");

    query.prepare("CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName + " AS "
                  "SELECT "
                  + columns.join(",") +
                  " FROM library "
                  "INNER JOIN track_locations "
                  "ON library.location=track_locations.id "
                  "WHERE " + filter +" AND track_locations.dir in (\""+m_availableDirs.join("\",\"")+"\")");
    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
    }

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << __FILE__ << __LINE__ << query.lastError();
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    setTable(tableName, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource("default"));

    initHeaderData();
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
    setSearch("");

}

void HiddenTableModel::purgeTracks(const QModelIndexList& indices) {
    QList<int> trackIds;

    foreach (QModelIndex index, indices) {
        int trackId = getTrackId(index);
        trackIds.append(trackId);
    }

   m_trackDAO.purgeTracks(trackIds);

    // TODO(rryan) : do not select, instead route event to BTC and notify from
    // there.
    select(); //Repopulate the data model.
}

void HiddenTableModel::purgeTracks(const int dirId){
    QSqlQuery query;
    query.prepare("SELECT library.id FROM library INNER JOIN track_locations "
                  "ON library.location=track_locations.id "
                  "WHERE dir_id="+QString::number(dirId));

    if (!query.exec()) {
        qDebug() << "could not purge tracks from libraryPath "<<dirId;
    }

    QList<int> trackIds;
    while (query.next()) {
        trackIds.append(query.value(query.record().indexOf("id")).toInt());
    }
    qDebug() << "starting to purge Tracks " << trackIds;
   m_trackDAO.purgeTracks(trackIds);

    // TODO(rryan) : do not select, instead route event to BTC and notify from
    // there.
    select();
}

void HiddenTableModel::unhideTracks(const QModelIndexList& indices) {
    QList<int> trackIds;

    foreach (QModelIndex index, indices) {
        int trackId = getTrackId(index);
        trackIds.append(trackId);
    }

   m_trackDAO.unhideTracks(trackIds);

    // TODO(rryan) : do not select, instead route event to BTC and notify from
    // there.
    select(); //Repopulate the data model.
}

bool HiddenTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_PLAYED) ||
        column == fieldIndex(LIBRARYTABLE_BPM_LOCK) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED) ||
        column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED)) {
        return true;
    }
    return false;
}
bool HiddenTableModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(LIBRARYTABLE_KEY)) {
        return true;
    }
    return false;
}

// Override flags from BaseSqlModel since we don't want edit this model
Qt::ItemFlags HiddenTableModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

TrackModel::CapabilitiesFlags HiddenTableModel::getCapabilities() const {
    return TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_PURGE
            | TRACKMODELCAPS_UNHIDE
            | TRACKMODELCAPS_RELOCATE;
}
