#include <QtDebug>
#include <QVector>

#include "analysertimbre.h"
#include "track/timbre_preferences.h"


AnalyserTimbre::AnalyserTimbre(ConfigObject<ConfigValue> *pConfig)
    : m_pConfig(pConfig),
      m_pVamp(NULL),
      m_iSampleRate(0),
      m_iTotalSamples(0),
      m_bShouldAnalyze(false),
      m_bPreferencesFastAnalysis(false) {
}
AnalyserTimbre::~AnalyserTimbre() {
}

bool AnalyserTimbre::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    m_bShouldAnalyze = false;
    if (totalSamples == 0) {
        return false;
    }
    bool bPreferencesTimbralAnalysisEnabled = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(TIMBRE_CONFIG_KEY, TIMBRE_ANALYSIS_ENABLED)).toInt());
    if (!bPreferencesTimbralAnalysisEnabled) {
        qDebug() << "Timbral analysis is deactivated";
        m_bShouldAnalyze = false;
        return false;
    }
    QString library = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_TIMBRE_LIBRARY));
    QString pluginID = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_TIMBRE_PLUGIN_ID));

    m_pluginId = pluginID;
    m_iSampleRate = sampleRate;
    m_iTotalSamples = totalSamples;

    qDebug() << "Timbral analysis started with plugin" << pluginID;

    m_pVamp = new VampAnalyser(m_pConfig);
    m_bShouldAnalyze = m_pVamp->Init(library, pluginID, m_iSampleRate, totalSamples,
                                     m_bPreferencesFastAnalysis);
    if (!m_bShouldAnalyze) {
        delete m_pVamp;
        m_pVamp = NULL;
    }
    return m_bShouldAnalyze;
}

void AnalyserTimbre::process(const CSAMPLE *pIn, const int iLen) {
    if (!m_bShouldAnalyze || m_pVamp == NULL)
        return;
    m_bShouldAnalyze = m_pVamp->Process(pIn, iLen);
    if (!m_bShouldAnalyze) {
        delete m_pVamp;
        m_pVamp = NULL;
    }
}

void AnalyserTimbre::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
    delete m_pVamp;
    m_pVamp = NULL;
}
void AnalyserTimbre::finalise(TrackPointer tio) {
    if (!m_bShouldAnalyze || m_pVamp == NULL) {
        return;
    }

    // Call End() here, because the number of total samples may have been
    // estimated incorrectly.
    bool success = m_pVamp->End();
    qDebug() << "Timbral analysis " << (success ? "complete" : "failed");

    QVector<double> beats = m_pVamp->GetInitFramesVector();
    delete m_pVamp;
    m_pVamp = NULL;
}
