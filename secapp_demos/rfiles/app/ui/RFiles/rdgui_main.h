#ifndef RDGUI_MAIN_H
#define RDGUI_MAIN_H

#include <QMainWindow>
#include <QStringList>
#include <QProcess>
#include <QPushButton>

class QSettings;
class QPropertyAnimation;
class QRegularExpression;

QT_BEGIN_NAMESPACE
namespace Ui
{
    class RDGUIMain;
}
QT_END_NAMESPACE

class RDGUIMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit RDGUIMain(QWidget *parent = nullptr);
    ~RDGUIMain() override;

private slots:
    void onConnectClicked();
    void onOptionsClicked();
    void onHelpClicked();
    void onRemoteHostTextChanged(const QString &text);
    void onAnimationFinished();
    void onClearHistoryClicked(); // <-- new slot

private:
    Ui::RDGUIMain *ui;

    // History & settings
    void loadHistory();
    void saveHistory();
    void updateHistory(const QString &host);

    QSettings *settings;
    QStringList connectionHistory;
    const int MAX_HISTORY_ITEMS = 10;
    const QString HISTORY_KEY = "ConnectionHistory/RecentHosts";
    const QString LAST_HOST_KEY = "ConnectionHistory/LastHost";
    const QString BANDWIDTH_KEY = "Experience/BandwidthProfile";

    // Validation & connectivity
    bool isValidHostFormat(const QString &hostInput);

    /// @brief Return true iff the host:port is reachable.
    /// @param host 
    /// @param port 
    /// @return 
    bool checkHostReachable(const QString &host, quint16 port);

    static const QRegularExpression m_ipv4Regex;
    static const QRegularExpression m_ipv6Regex;
    static const QRegularExpression m_hostnameRegex;

    // Options‐area animation
    void initializeOptionsArea();
    void calculateHeights();
    void updateOptionsButton(bool showOptions);

    bool m_optionsVisible = false;
    int m_collapsedHeight = 0;
    int m_expandedHeight = 0;
    QPropertyAnimation *m_animation = nullptr;

private:
    QProcess* _rd_proc = NULL;

    void disableButtonForSeconds(QPushButton* button, int seconds);
    void rd_proc_init();
    void rd_proc_fini();
    int rd_proc_run();
    void rd_proc_stop();
};

#endif // RDGUI_MAIN_H
