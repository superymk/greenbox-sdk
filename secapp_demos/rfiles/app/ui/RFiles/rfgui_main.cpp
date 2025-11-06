// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "rfgui_main.h"
#include "ui_rfgui_main.h"

#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QStyle>
#include <QComboBox>
#include <QLayout>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QCoreApplication>
#include <QIcon>
#include <QTimer>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// // --- Initialize static Regex members ---
// const QRegularExpression RFGUIMain::m_ipv4Regex(
//     QStringLiteral("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
//                    "(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"));

// const QRegularExpression RFGUIMain::m_ipv6Regex(
//     QStringLiteral("^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|"
//                    "([0-9a-fA-F]{1,4}:){1,7}:|"
//                    "([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|"
//                    "([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|"
//                    "([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|"
//                    "([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|"
//                    "([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|"
//                    "[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|"
//                    ":((:[0-9a-fA-F]{1,4}){1,7}|:)|"
//                    "fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|"
//                    "::(ffff(:0{1,4}){0,1}:){0,1}"
//                    "((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}"
//                    "(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|"
//                    "([0-9a-fA-F]{1,4}:){1,4}:"
//                    "((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}"
//                    "(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$"));

// const QRegularExpression RFGUIMain::m_hostnameRegex(
//     QStringLiteral("^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*"
//                    "([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$"));

RFGUIMain::RFGUIMain(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::RFGUIMain)
{
    ui->setupUi(this);
    setFixedWidth(550);

    settings = new QSettings(QSettings::IniFormat,
                             QSettings::UserScope,
                             QCoreApplication::organizationName(),
                             QCoreApplication::applicationName(),
                             this);

    // Load histories
    loadHistory();        // kept for compatibility with Clear History section
    loadHomeDirHistory(); // home folder history

    // Wire validation for localDir (enable/disable Run)
    auto *le = ui->localDirComboBox->lineEdit();
    if (le)
    {
        connect(le, &QLineEdit::textChanged,
                this, &RFGUIMain::onLocalDirTextChanged);
        connect(ui->localDirComboBox,
                &QComboBox::currentTextChanged,
                this, &RFGUIMain::onLocalDirTextChanged);
        connect(le, &QLineEdit::returnPressed,
                this, &RFGUIMain::onRunClicked);
        onLocalDirTextChanged(ui->localDirComboBox->currentText());
    }
    else
    {
        ui->runButton->setEnabled(false);
    }

    // Clear connection history button (existing)
    connect(ui->clearHistoryButton, &QPushButton::clicked,
            this, &RFGUIMain::onClearHistoryClicked);

    // Home folder browse button & editing acceptance
    connect(ui->localDirBrowseButton, &QPushButton::clicked,
            this, &RFGUIMain::onLocalDirBrowseClicked);

    if (auto *homeLe = ui->localDirComboBox->lineEdit())
    {
        connect(homeLe, &QLineEdit::returnPressed,
                this, &RFGUIMain::onLocalDirTextAccepted);
    }
    connect(ui->localDirComboBox, &QComboBox::currentTextChanged,
            this, &RFGUIMain::onLocalDirTextAccepted);

    initializeOptionsArea(); // keeps the tab/animation only; no settings use

    // standard buttons
    connect(ui->runButton, &QPushButton::clicked,
            this, &RFGUIMain::onRunClicked);
    connect(ui->optionsButton, &QPushButton::clicked,
            this, &RFGUIMain::onOptionsClicked);
    connect(ui->helpButton, &QPushButton::clicked,
            this, &RFGUIMain::onHelpClicked);

    // banner image
    {
        QString bannerPath = QCoreApplication::applicationDirPath() + "/banner.png";
        if (QFile::exists(bannerPath))
        {
            QPixmap banner(bannerPath);
            ui->headerLabel->setPixmap(
                banner.scaled(width(), banner.height(),
                              Qt::KeepAspectRatio,
                              Qt::SmoothTransformation));
        }
    }

    // animation
    m_animation = new QPropertyAnimation(this, "geometry", this);
    m_animation->setDuration(300);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_animation, &QPropertyAnimation::finished,
            this, &RFGUIMain::onAnimationFinished);

    ui->localDirComboBox->setFocus();

    // Initialize the internal RFiles console process.
    rd_proc_init();
}

RFGUIMain::~RFGUIMain()
{
    rd_proc_stop();
    rd_proc_fini();
    delete ui;
}

/* =========================
 *  Histories: remote host (kept, UI-bound to Clear History)
 * ========================= */
void RFGUIMain::loadHistory()
{
    // Keep data only; do not modify the Home Folder combo here.
    connectionHistory = settings->value(HISTORY_KEY).toStringList();
    // (No UI updates; localDirComboBox is for home directories.)
}

void RFGUIMain::saveHistory()
{
    settings->setValue(HISTORY_KEY, connectionHistory);
    if (!ui->localDirComboBox->currentText().isEmpty())
        settings->setValue(LAST_HOST_KEY,
                           ui->localDirComboBox->currentText());
    settings->sync();
}

void RFGUIMain::updateHistory(const QString &host)
{
    if (host.isEmpty())
        return;
    connectionHistory.removeAll(host);
    connectionHistory.prepend(host);
    while (connectionHistory.size() > MAX_HISTORY_ITEMS)
        connectionHistory.removeLast();

    ui->localDirComboBox->blockSignals(true);
    ui->localDirComboBox->clear();
    ui->localDirComboBox->addItems(connectionHistory);
    ui->localDirComboBox->setCurrentText(host);
    ui->localDirComboBox->blockSignals(false);

    saveHistory();
}

/* =========================
 *  Histories: home folder
 * ========================= */
void RFGUIMain::loadHomeDirHistory()
{
    homeDirHistory = settings->value(HOME_HISTORY_KEY).toStringList();

    if (homeDirHistory.isEmpty())
    {
#ifdef Q_OS_WIN
        homeDirHistory << "C:\\";
#else
        homeDirHistory << QDir::homePath();
#endif
    }

    ui->localDirComboBox->clear();
    ui->localDirComboBox->addItems(homeDirHistory);

    QString last = settings->value(LAST_HOME_DIR_KEY).toString();
    if (!last.isEmpty())
    {
        ui->localDirComboBox->setCurrentText(last);
    }
    else
    {
        ui->localDirComboBox->setCurrentIndex(0);
    }
}

void RFGUIMain::saveHomeDirHistory()
{
    settings->setValue(HOME_HISTORY_KEY, homeDirHistory);
    if (!ui->localDirComboBox->currentText().isEmpty())
        settings->setValue(LAST_HOME_DIR_KEY,
                           ui->localDirComboBox->currentText());
    settings->sync();
}

void RFGUIMain::updateHomeDirHistory(const QString &dirAbs)
{
    if (dirAbs.isEmpty())
        return;

    QString norm = QDir(dirAbs).absolutePath();
    homeDirHistory.removeAll(norm);
    homeDirHistory.prepend(norm);
    while (homeDirHistory.size() > MAX_HISTORY_ITEMS)
        homeDirHistory.removeLast();

    ui->localDirComboBox->blockSignals(true);
    ui->localDirComboBox->clear();
    ui->localDirComboBox->addItems(homeDirHistory);
    ui->localDirComboBox->setCurrentText(norm);
    ui->localDirComboBox->blockSignals(false);

    saveHomeDirHistory();
}

/* =========================
 *  Options & animation
 * ========================= */
void RFGUIMain::initializeOptionsArea()
{
    QStringList opts = {"Low-speed network (<1 Mbps)", "WAN (>1 Mbps - 100 Mbps)", "LAN (>100 Mbps)"};
    ui->bandwidthComboBox->clear();
    ui->bandwidthComboBox->addItems(opts);

    // Keep the Experience tab visible/toggleable, but do not load/save or use its values.
    // (No settings references here.)
    ui->optionsTabWidget->setVisible(false);
    updateOptionsButton(/*showOptions=*/true);
    calculateHeights();
    resize(550, m_collapsedHeight);
}

void RFGUIMain::calculateHeights()
{
    ui->centralwidget->layout()->invalidate();
    ui->centralwidget->layout()->activate();

    ui->optionsTabWidget->setVisible(false);
    int c = sizeHint().height();

    ui->optionsTabWidget->setVisible(true);
    int e = sizeHint().height();

    m_collapsedHeight = c;
    m_expandedHeight = e;
    ui->optionsTabWidget->setVisible(m_optionsVisible);
    qDebug() << "Heights: collapsed =" << c << ", expanded =" << e;
}

void RFGUIMain::updateOptionsButton(bool showOptions)
{
    ui->optionsButton->setText(showOptions ? "Show Options" : "Hide Options");
    auto icon = showOptions ? QStyle::SP_ArrowDown : QStyle::SP_ArrowUp;
    ui->optionsButton->setIcon(style()->standardIcon(icon));
}

void RFGUIMain::onOptionsClicked()
{
    m_optionsVisible = !m_optionsVisible;
    m_animation->stop();
    updateOptionsButton(!m_optionsVisible);
    calculateHeights();

    if (m_optionsVisible)
        ui->optionsTabWidget->setVisible(false);

    QRect start = geometry();
    QRect end = start;
    end.setHeight(m_optionsVisible ? m_expandedHeight
                                   : m_collapsedHeight);

    m_animation->setStartValue(start);
    m_animation->setEndValue(end);
    m_animation->start();
}

void RFGUIMain::onAnimationFinished()
{
    ui->optionsTabWidget->setVisible(m_optionsVisible);
    resize(550, m_optionsVisible ? m_expandedHeight
                                 : m_collapsedHeight);
}

// /* =========================
//  *  Remote host validation (kept but unused)
//  * ========================= */
// bool RFGUIMain::isValidHostFormat(const QString &input)
// {
//     QString s = input.trimmed();
//     if (s.isEmpty())
//         return false;

//     int pos = s.lastIndexOf(':');
//     if (pos != -1)
//     {
//         bool ok;
//         int p = s.mid(pos + 1).toInt(&ok);
//         if (!ok || p <= 0 || p > 65535)
//             return false;
//         s = s.left(pos);
//     }
//     if (s.isEmpty())
//         return false;

//     return m_ipv4Regex.match(s).hasMatch() || m_ipv6Regex.match(s).hasMatch() || m_hostnameRegex.match(s).hasMatch();
// }

// bool RFGUIMain::checkHostReachable(const QString &host, quint16 /*port*/)
// {
//     QTcpSocket sock;
//     sock.connectToHost(host, 0);
//     if (!sock.waitForConnected(2000))
//         return false;
//     sock.disconnectFromHost();
//     return true;
// }

/* =========================
 *  Local dir validation on text change
 * ========================= */
void RFGUIMain::onLocalDirTextChanged(const QString &text)
{
    ui->runButton->setEnabled(isValidAbsoluteDir(text));
}

/* =========================
 *  Run click
 * ========================= */
void RFGUIMain::onRunClicked()
{
    // Normalize/format the path first
    const QString normalized = normalizeDirPath(ui->localDirComboBox->currentText());
    ui->localDirComboBox->setCurrentText(normalized);

    // Validate again after normalization
    if (!isValidAbsoluteDir(normalized))
    {
        QMessageBox::warning(this, tr("Home Folder Error"),
                             tr("The selected Home Folder does not exist or is not a directory:\n%1")
                                 .arg(normalized));
        ui->localDirComboBox->setFocus();
        ui->runButton->setEnabled(false);
        return;
    }

    // Update history with the normalized absolute path
    updateHomeDirHistory(normalized);

    // Construct rfiles_conf.json
    QJsonObject localFSObj;
    localFSObj["certs_filepath"] = QStringLiteral("./ca-certificates.with-extras.crt");
    localFSObj["certs_filepath_comment"] = QStringLiteral("Absolute file path of the trusted certificates.");
    localFSObj["home_dir"] = normalized;
    localFSObj["home_dir_comment"] = QStringLiteral("Absolute file path of the home directory; e.g., /home/superymk/ for Linux, and c:/ for Windows");

    QJsonObject root;
    root["local_fs"] = localFSObj;

    QString fp = QCoreApplication::applicationDirPath() + "/rfiles_conf.json";
    QFile f(fp);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        f.close();
    }
    else
    {
        QMessageBox::warning(this, "File Error",
                             QString("Failed to write configuration file:\n%1").arg(fp));
    }

    // Throttle repeated runs
    this->disableButtonForSeconds(ui->runButton, 5);

    // Start the console application
    const int ret = rf_proc_run();
    qDebug() << "Run rfiles console process: ret:" << ret;

    // Minimize the current window
    this->showMinimized();
}

/* =========================
 *  Clear connection history
 * ========================= */
void RFGUIMain::onClearHistoryClicked()
{
    // --- Legacy: clear remote-connection history (kept for compatibility) ---
    connectionHistory.clear();
    settings->remove(HISTORY_KEY);
    settings->remove(LAST_HOST_KEY);
    for (const QString &k : settings->allKeys())
    {
        if (k.startsWith("PerHost/"))
            settings->remove(k);
    }

    // --- Current: clear Home Folder history (this drives the combo) ---
    homeDirHistory.clear();
    settings->remove(HOME_HISTORY_KEY);
    settings->remove(LAST_HOME_DIR_KEY);

    // Reflect in UI
    ui->localDirComboBox->clear();
#ifdef Q_OS_WIN
    ui->localDirComboBox->addItem(QStringLiteral("C:\\"));
#else
    ui->localDirComboBox->addItem(QDir::homePath());
#endif
    ui->localDirComboBox->setCurrentIndex(0);

    settings->sync();
}

/* =========================
 *  Help
 * ========================= */
void RFGUIMain::onHelpClicked()
{
    QMessageBox::information(this, "Help", QStringLiteral("GreenBox Remote Files Help\n\n"
                                                          "- Use 'Home Folder' → Browse... to choose a local workspace directory.\n"
                                                          "- You can also type a full absolute path; the Run button enables when it exists.\n"
                                                          "- Click 'Show Options' to toggle the Experience tab (kept for UI only).\n"
                                                          "- In System → Connection History you can clear connection history."));
}

/* =========================
 *  Utility
 * ========================= */
void RFGUIMain::disableButtonForSeconds(QPushButton *button, int seconds)
{
    button->setEnabled(false);
    QTimer::singleShot(seconds * 1000, [button]()
                       { button->setEnabled(true); });
}

/* =========================
 *  RFiles console process
 * ========================= */
void RFGUIMain::rd_proc_init(void)
{
    _rd_proc = new QProcess(this);

#ifdef Q_OS_WIN
    _rd_proc->setCreateProcessArgumentsModifier(
        [](QProcess::CreateProcessArguments *args)
        {
            args->flags |= CREATE_NO_WINDOW;
        });
#endif
}

void RFGUIMain::rd_proc_fini(void)
{
    if (_rd_proc)
    {
        delete _rd_proc;
        _rd_proc = NULL;
    }
}

int RFGUIMain::rf_proc_run(void)
{
    _rd_proc->start("./sl_rfiles.exe");
    if (!_rd_proc->waitForStarted())
    {
        qDebug() << "Failed to start sl_rfiles.exe process!";
        return 1;
    }
    return 0;
}

void RFGUIMain::rd_proc_stop(void)
{
    if (_rd_proc && _rd_proc->state() == QProcess::Running)
    {
        _rd_proc->write("\r\n");
        _rd_proc->waitForBytesWritten();
        _rd_proc->closeWriteChannel();
        _rd_proc->waitForFinished();
    }
}

/* =========================
 *  Home Folder picker logic
 * ========================= */
static QString rf_defaultStartDir()
{
#ifdef Q_OS_WIN
    return QStringLiteral("C:\\");
#else
    return QDir::homePath();
#endif
}

void RFGUIMain::onLocalDirBrowseClicked()
{
    QString fromCombo = ui->localDirComboBox->currentText().trimmed();
    QString startDir = QDir(fromCombo).exists()
                           ? QDir(fromCombo).absolutePath()
                           : rf_defaultStartDir();

    QFileDialog dlg(this, tr("Select Home Folder"), startDir);
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setOption(QFileDialog::ShowDirsOnly, true);
    dlg.setOption(QFileDialog::DontResolveSymlinks, false);

    if (dlg.exec() == QDialog::Accepted)
    {
        QString chosen = dlg.selectedFiles().value(0);
        if (!chosen.isEmpty())
        {
            QString abs = QDir(chosen).absolutePath();
#ifdef Q_OS_WIN
            abs = QDir::toNativeSeparators(abs);
#endif
            ui->localDirComboBox->setCurrentText(abs);
            updateHomeDirHistory(abs);
        }
    }
}

void RFGUIMain::onLocalDirTextAccepted()
{
    QString typed = ui->localDirComboBox->currentText().trimmed();
    if (typed.isEmpty())
        return;

    QString abs = QDir(typed).absolutePath();
#ifdef Q_OS_WIN
    abs = QDir::toNativeSeparators(abs);
#endif
    ui->localDirComboBox->setCurrentText(abs);
}

/* =========================
 *  Utilities
 * ========================= */
QString RFGUIMain::normalizeDirPath(const QString &input) const
{
    QString abs = QDir(input).absolutePath();
    abs = QDir::fromNativeSeparators(abs);
    QFileInfo fi(abs);
    if (fi.exists() && fi.isDir() && !abs.endsWith('/'))
        abs.append('/');
    return abs;
}

bool RFGUIMain::isValidAbsoluteDir(const QString &text) const
{
    if (text.trimmed().isEmpty())
        return false;
    QFileInfo fi(QDir(text).absolutePath());
    return fi.exists() && fi.isDir();
}
