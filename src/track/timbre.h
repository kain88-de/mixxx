#ifndef TIMBRE_H
#define TIMBRE_H

class Timbre {
  public:
    explicit Timbre(const QByteArray* pByteArray=NULL);
    QByteArray* toByteArray() const;
    QString getVersion() const;
    QString getSubVersion() const;
    void setSubVersion(QString subVersion);
  private:
    void readByteArray(const QByteArray* pByteArray);

    mutable QMutex m_mutex;
    QString m_subVersion;
    mixxx::track::io::timbre::Timbre m_timbreModel;

    // For private constructor access.
    friend class TimbreFactory;
};

#endif // TIMBRE_H
