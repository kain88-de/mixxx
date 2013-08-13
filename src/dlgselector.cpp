#include <QSqlTableModel>

#include "configobject.h"
#include "library/selector/selectorlibrarytablemodel.h"
#include "library/selector/selector_preferences.h"
#include "library/trackcollection.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableview.h"
#include "widget/wwidget.h"

#include "dlgselector.h"

DlgSelector::DlgSelector(QWidget* parent,
                         ConfigObject<ConfigValue>* pConfig,
                         TrackCollection* pTrackCollection,
                         MixxxKeyboard* pKeyboard)
        : QWidget(parent), Ui::DlgSelector(),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_pTrackTableView(
              new WTrackTableView(this, pConfig, m_pTrackCollection, true)) {
    setupUi(this);

    m_pTrackTableView->installEventFilter(pKeyboard);
    connect(m_pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pTrackTablePlaceholder);
    m_pTrackTablePlaceholder->hide();
    box->insertWidget(2, m_pTrackTableView);

    m_pSelectorLibraryTableModel =
            new SelectorLibraryTableModel(this, pConfig, pTrackCollection);
    m_pTrackTableView->loadTrackModel(m_pSelectorLibraryTableModel);

    connect(checkBoxGenre, SIGNAL(toggled(bool)),
            this, SLOT(filterByGenre(bool)));
    connect(checkBoxBpm, SIGNAL(toggled(bool)),
            this, SLOT(filterByBpm(bool)));
    connect(checkBoxKey, SIGNAL(toggled(bool)),
            this, SLOT(filterByKey(bool)));
    connect(checkBoxKey4th, SIGNAL(toggled(bool)),
            this, SLOT(filterByKey4th(bool)));
    connect(checkBoxKey5th, SIGNAL(toggled(bool)),
            this, SLOT(filterByKey5th(bool)));
    connect(checkBoxKeyRelative, SIGNAL(toggled(bool)),
            this, SLOT(filterByKeyRelative(bool)));
    connect(horizontalSliderBpmRange, SIGNAL(valueChanged(int)),
            this, SLOT(bpmRangeChanged(int)));
    connect(m_pSelectorLibraryTableModel, SIGNAL(filtersChanged()),
            this, SLOT(slotFiltersChanged()));
    connect(m_pSelectorLibraryTableModel, SIGNAL(resetFilters()),
            this, SLOT(loadStoredFilterSettings()));
    connect(m_pSelectorLibraryTableModel, SIGNAL(seedTrackInfoChanged()),
           this, SLOT(slotSeedTrackInfoChanged()));

    loadStoredFilterSettings();

}

DlgSelector::~DlgSelector() {
}

void DlgSelector::onShow()
{
    qDebug() << "DlgSelector::onShow()";
    m_pSelectorLibraryTableModel->active(true);
    slotSeedTrackInfoChanged();
}

void DlgSelector::onHide() {
    qDebug() << "DlgSelector::onHide()";
    m_pSelectorLibraryTableModel->active(false);
}

void DlgSelector::onSearch(const QString& text) {
    m_pSelectorLibraryTableModel->search(text);
}


void DlgSelector::slotFiltersChanged() {
    int count = m_pSelectorLibraryTableModel->rowCount();
    QString pluralize = ((count > 1 || count == 0) ? QString("s") : QString(""));
    QString labelMatchText = QString(tr("%1 Track%2 Found ")).arg(count).arg(pluralize);
    labelMatchCount->setText(labelMatchText);
}

void DlgSelector::slotSeedTrackInfoChanged() {
    QString seedTrackInfo = m_pSelectorLibraryTableModel->getSeedTrackInfo();
    QString labelSeedTrackText = QString(tr("Matches for: %1")).arg(seedTrackInfo);
    labelSeedTrackInfo->setText(labelSeedTrackText);

    // check which filters to activate
    checkBoxGenre->setEnabled(m_pSelectorLibraryTableModel->seedTrackGenreExists());
    checkBoxBpm->setEnabled(m_pSelectorLibraryTableModel->seedTrackBpmExists());
    horizontalSliderBpmRange->setEnabled(m_pSelectorLibraryTableModel->seedTrackBpmExists());
    bool hasKey = m_pSelectorLibraryTableModel->seedTrackKeyExists();
    checkBoxKey->setEnabled(hasKey);
    checkBoxKey4th->setEnabled(hasKey);
    checkBoxKey5th->setEnabled(hasKey);
    checkBoxKeyRelative->setEnabled(hasKey);
}

void DlgSelector::setSeedTrack(TrackPointer pSeedTrack) {
    m_pSelectorLibraryTableModel->setSeedTrack(pSeedTrack);
}

void DlgSelector::calculateAllSimilarities(const QString& filename) {
    m_pSelectorLibraryTableModel->calculateAllSimilarities(filename);
}

void DlgSelector::loadSelectedTrack() {
    m_pTrackTableView->loadSelectedTrack();
}

void DlgSelector::loadSelectedTrackToGroup(QString group) {
    m_pTrackTableView->loadSelectedTrackToGroup(group, false);
}

void DlgSelector::moveSelection(int delta) {
    m_pTrackTableView->moveSelection(delta);
}

void DlgSelector::filterByGenre(bool checked) {
    m_pSelectorLibraryTableModel->filterByGenre(checked);
}

void DlgSelector::filterByBpm(bool checked) {
    int range = horizontalSliderBpmRange->value();
    m_pSelectorLibraryTableModel->filterByBpm(checked, range);
}

void DlgSelector::bpmRangeChanged(int value) {
    Q_UNUSED(value);
    filterByBpm(checkBoxBpm->isChecked());
}

void DlgSelector::filterByKey(bool checked) {
    m_pSelectorLibraryTableModel->filterByKey(checked);
}

void DlgSelector::filterByKey4th(bool checked) {
    m_pSelectorLibraryTableModel->filterByKey4th(checked);
}

void DlgSelector::filterByKey5th(bool checked) {
    m_pSelectorLibraryTableModel->filterByKey5th(checked);
}

void DlgSelector::filterByKeyRelative(bool checked) {
    m_pSelectorLibraryTableModel->filterByKeyRelative(checked);
}

void DlgSelector::installEventFilter(QObject* pFilter) {
    QWidget::installEventFilter(pFilter);
    m_pTrackTableView->installEventFilter(pFilter);
}

void DlgSelector::on_buttonCalcSimilarity_clicked() {
    m_pSelectorLibraryTableModel->calculateSimilarity();
}

void DlgSelector::loadStoredFilterSettings() {
    bool bFilterGenre = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_GENRE)).toInt());
    bool bFilterBpm = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_BPM)).toInt());
    bool iFilterBpmRange = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_BPM_RANGE)).toInt();
    bool bFilterKey = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY)).toInt());
    bool bFilterKey4th = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_4TH)).toInt());
    bool bFilterKey5th = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_5TH)).toInt());
    bool bFilterKeyRelative = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_RELATIVE)).toInt());

    checkBoxGenre->setChecked(bFilterGenre);
    checkBoxBpm->setChecked(bFilterBpm);
    horizontalSliderBpmRange->setValue(iFilterBpmRange);
    checkBoxKey->setChecked(bFilterKey);
    checkBoxKey4th->setChecked(bFilterKey4th);
    checkBoxKey5th->setChecked(bFilterKey5th);
    checkBoxKeyRelative->setChecked(bFilterKeyRelative);
}
