// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef RFGUI_MAIN_H
#define RFGUI_MAIN_H

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
    class RFGUIMain;
}
QT_END_NAMESPACE

class RFGUIMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit RFGUIMain(QWidget *parent = nullptr);
    ~RFGUIMain() override;

private slots:
    void onRunClicked();
    void onOptionsClicked();
    void onHelpClicked();
    void onLocalDirTextChanged(const QString &text);
    void onAnimationFinished();
    void onClearHistoryClicked(); // existing

    // workspace/home-dir interactions
    void onLocalDirBrowseClicked();
    void onLocalDirTextAccepted();

private:
    Ui::RFGUIMain *ui;

    // Remote host history & settings (kept for compatibility with Clear History button)
    void loadHistory();
    void saveHistory();
    void updateHistory(const QString &host);

    QSettings *settings;
    QStringList connectionHistory;
    const int MAX_HISTORY_ITEMS = 10;
    const QString HISTORY_KEY = "ConnectionHistory/RecentHosts";
    const QString LAST_HOST_KEY = "ConnectionHistory/LastHost";

    // Home folder history & settings
    void loadHomeDirHistory();
    void saveHomeDirHistory();
    void updateHomeDirHistory(const QString &dirAbs);

    QStringList homeDirHistory;
    const QString HOME_HISTORY_KEY = "Workspace/RecentHomeDirs";
    const QString LAST_HOME_DIR_KEY = "Workspace/LastHomeDir";

    // Validation & connectivity (host funcs kept but not used)
    bool isValidHostFormat(const QString &hostInput);
    bool checkHostReachable(const QString &host, quint16 port);

    static const QRegularExpression m_ipv4Regex;
    static const QRegularExpression m_ipv6Regex;
    static const QRegularExpression m_hostnameRegex;

    // Options-area animation (Experience tab is kept but values unused)
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
    int rf_proc_run();
    void rd_proc_stop();

    // Utilities
    /// Normalize a directory path: make absolute, use '/', and ensure trailing '/' if it exists as a directory.
    QString normalizeDirPath(const QString& input) const;
    /// Check that the given text is an absolute, existing directory.
    bool isValidAbsoluteDir(const QString& text) const;
};

#endif // RFGUI_MAIN_H
