#include "rdgui_main.h"
#include "ui_rdgui_main.h"

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

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// --- Initialize static Regex members ---
const QRegularExpression RDGUIMain::m_ipv4Regex(
    QStringLiteral("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
                   "(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"));

const QRegularExpression RDGUIMain::m_ipv6Regex(
    QStringLiteral("^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|"
                   "([0-9a-fA-F]{1,4}:){1,7}:|"
                   "([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|"
                   "([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|"
                   "([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|"
                   "([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|"
                   "([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|"
                   "[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|"
                   ":((:[0-9a-fA-F]{1,4}){1,7}|:)|"
                   "fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|"
                   "::(ffff(:0{1,4}){0,1}:){0,1}"
                   "((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}"
                   "(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|"
                   "([0-9a-fA-F]{1,4}:){1,4}:"
                   "((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}"
                   "(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$"));

const QRegularExpression RDGUIMain::m_hostnameRegex(
    QStringLiteral("^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*"
                   "([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$"));

RDGUIMain::RDGUIMain(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::RDGUIMain)
{
    ui->setupUi(this);
    setFixedWidth(550);

    settings = new QSettings(QSettings::IniFormat,
                             QSettings::UserScope,
                             QCoreApplication::organizationName(),
                             QCoreApplication::applicationName(),
                             this);

    // Load history
    loadHistory();

    // Hook up validation & history‐restore
    auto *le = ui->remoteHostComboBox->lineEdit();
    if (le)
    {
        connect(le, &QLineEdit::textChanged,
                this, &RDGUIMain::onRemoteHostTextChanged);
        connect(ui->remoteHostComboBox,
                &QComboBox::currentTextChanged,
                this, &RDGUIMain::onRemoteHostTextChanged);
        connect(le, &QLineEdit::returnPressed,
                this, &RDGUIMain::onConnectClicked);
        onRemoteHostTextChanged(ui->remoteHostComboBox->currentText());
    }
    else
    {
        ui->connectButton->setEnabled(false);
    }

    // New: clear history button
    connect(ui->clearHistoryButton, &QPushButton::clicked,
            this, &RDGUIMain::onClearHistoryClicked);

    initializeOptionsArea();

    // standard buttons
    connect(ui->connectButton, &QPushButton::clicked,
            this, &RDGUIMain::onConnectClicked);
    connect(ui->optionsButton, &QPushButton::clicked,
            this, &RDGUIMain::onOptionsClicked);
    connect(ui->helpButton, &QPushButton::clicked,
            this, &RDGUIMain::onHelpClicked);

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
            this, &RDGUIMain::onAnimationFinished);

    ui->remoteHostComboBox->setFocus();

    // Initialize the internal RDesktop console process.
    rd_proc_init();
}

RDGUIMain::~RDGUIMain()
{
    // Stop running RDesktop console process, if any.
    rd_proc_stop();

    // Finalize the internal RDesktop console process.
    rd_proc_fini();

    delete ui;
}

void RDGUIMain::loadHistory()
{
    connectionHistory = settings->value(HISTORY_KEY).toStringList();
    ui->remoteHostComboBox->clear();
    ui->remoteHostComboBox->addItems(connectionHistory);
    QString last = settings->value(LAST_HOST_KEY).toString();
    if (!last.isEmpty())
    {
        ui->remoteHostComboBox->setCurrentText(last);
    }
    else if (!connectionHistory.isEmpty())
    {
        ui->remoteHostComboBox->setCurrentIndex(0);
    }
}

void RDGUIMain::saveHistory()
{
    settings->setValue(HISTORY_KEY, connectionHistory);
    if (!ui->remoteHostComboBox->currentText().isEmpty())
        settings->setValue(LAST_HOST_KEY,
                           ui->remoteHostComboBox->currentText());
    settings->sync();
}

void RDGUIMain::updateHistory(const QString &host)
{
    if (host.isEmpty())
        return;
    connectionHistory.removeAll(host);
    connectionHistory.prepend(host);
    while (connectionHistory.size() > MAX_HISTORY_ITEMS)
        connectionHistory.removeLast();

    ui->remoteHostComboBox->blockSignals(true);
    ui->remoteHostComboBox->clear();
    ui->remoteHostComboBox->addItems(connectionHistory);
    ui->remoteHostComboBox->setCurrentText(host);
    ui->remoteHostComboBox->blockSignals(false);

    saveHistory();
}

void RDGUIMain::initializeOptionsArea()
{
    QStringList opts = {
        "Low-speed network (<1 Mbps)",
        "WAN (>1 Mbps - 100 Mbps)",
        "LAN (>100 Mbps)"};
    ui->bandwidthComboBox->clear();
    ui->bandwidthComboBox->addItems(opts);

    QString saved = settings->value(BANDWIDTH_KEY, opts.first()).toString();
    int idx = ui->bandwidthComboBox->findText(saved);
    ui->bandwidthComboBox->setCurrentIndex(idx != -1 ? idx : 0);

    ui->optionsTabWidget->setVisible(false);
    updateOptionsButton(/*showOptions=*/true);
    calculateHeights();
    resize(550, m_collapsedHeight);
}

void RDGUIMain::calculateHeights()
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

void RDGUIMain::updateOptionsButton(bool showOptions)
{
    ui->optionsButton->setText(showOptions ? "Show Options" : "Hide Options");
    auto icon = showOptions ? QStyle::SP_ArrowDown : QStyle::SP_ArrowUp;
    ui->optionsButton->setIcon(style()->standardIcon(icon));
}

void RDGUIMain::onOptionsClicked()
{
    m_optionsVisible = !m_optionsVisible;
    m_animation->stop();
    updateOptionsButton(!m_optionsVisible);
    calculateHeights();

    // defer showing the options‐tab until AFTER the window expands
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

void RDGUIMain::onAnimationFinished()
{
    // once the animation completes, show or hide the tab
    ui->optionsTabWidget->setVisible(m_optionsVisible);
    // and force the final size
    resize(550, m_optionsVisible ? m_expandedHeight
                                 : m_collapsedHeight);
}

bool RDGUIMain::isValidHostFormat(const QString &input)
{
    QString s = input.trimmed();
    if (s.isEmpty())
        return false;

    int pos = s.lastIndexOf(':');
    if (pos != -1)
    {
        bool ok;
        int p = s.mid(pos + 1).toInt(&ok);
        if (!ok || p <= 0 || p > 65535)
            return false;
        s = s.left(pos);
    }
    if (s.isEmpty())
        return false;

    return m_ipv4Regex.match(s).hasMatch() || m_ipv6Regex.match(s).hasMatch() || m_hostnameRegex.match(s).hasMatch();
}

bool RDGUIMain::checkHostReachable(const QString &host, quint16 port)
{
    QTcpSocket sock;
    sock.connectToHost(host, port);
    if (!sock.waitForConnected(2000))
        return false;
    sock.disconnectFromHost();
    return true;
}

void RDGUIMain::onRemoteHostTextChanged(const QString &text)
{
    bool valid = isValidHostFormat(text);
    ui->connectButton->setEnabled(valid);

    if (connectionHistory.contains(text))
    {
        QString key = QString("PerHost/%1/BandwidthProfile").arg(text);
        QString saved = settings->value(key).toString();
        int idx = ui->bandwidthComboBox->findText(saved);
        if (idx != -1)
            ui->bandwidthComboBox->setCurrentIndex(idx);
    }
    else if (!connectionHistory.isEmpty())
    {
        QString recent = connectionHistory.first();
        QString key = QString("PerHost/%1/BandwidthProfile").arg(recent);
        QString saved = settings->value(key,
                                        ui->bandwidthComboBox->currentText())
                            .toString();
        int idx = ui->bandwidthComboBox->findText(saved);
        if (idx != -1)
            ui->bandwidthComboBox->setCurrentIndex(idx);
    }
}

void RDGUIMain::onConnectClicked()
{
    int ret = 0;
    bool result = false;

    QString hostPort = ui->remoteHostComboBox->currentText().trimmed();
    if (!isValidHostFormat(hostPort))
    {
        QMessageBox::warning(this, "Connection Error",
                             "The entered remote host is not valid.");
        ui->remoteHostComboBox->lineEdit()->setFocus();
        return;
    }

    QString host = hostPort;
    quint16 port = 0;
    int pos = hostPort.lastIndexOf(':');
    if (pos != -1)
    {
        host = hostPort.left(pos);
        bool ok = false;
        port = hostPort.mid(pos + 1).toUInt(&ok);
        if (!ok)
        {
            QMessageBox::warning(this, "Connection Error",
                                 "Invalid port number.");
            return;
        }
    }

    if (!checkHostReachable(host, port))
    {
        QMessageBox::critical(this, "Connection Error",
                              QString("GreenBox RDesktop cannot find the computer '%1'.\n"
                                      "This might mean that '%1' does not belong to the specified network.\n"
                                      "Verify the name and the port of the destination host.")
                                  .arg(hostPort));
        return;
    }

    // save per‐host bandwidth
    QString bw = ui->bandwidthComboBox->currentText();
    settings->setValue(QString("PerHost/%1/BandwidthProfile").arg(hostPort), bw);

    updateHistory(hostPort);
    settings->setValue(BANDWIDTH_KEY, bw);
    settings->sync();

    int compress = 8, quality = 5;
    if (bw.startsWith("Low-speed"))
    {
        compress = 9;
        quality = 9;
    }
    else if (bw.startsWith("WAN"))
    {
        compress = 8;
        quality = 5;
    }
    else if (bw.startsWith("LAN"))
    {
        compress = 3;
        quality = 3;
    }

    QJsonObject logonObj;
    logonObj["url"] = hostPort;
    logonObj["url_comment"] = QStringLiteral(
        "hostname:port of the VNC server; e.g., host.example.com:5900");
    QJsonObject displayObj;
    displayObj["compress"] = compress;
    displayObj["compress_comment"] = QStringLiteral(
        "Compression level. Ranges from 0 (lowest) to 9 (highest)");
    displayObj["quality"] = quality;
    displayObj["quality_comment"] = QStringLiteral(
        "Quality level. Ranges from 0 (lowest) to 9 (highest)");
    QJsonObject root;
    root["logon"] = logonObj;
    root["display"] = displayObj;

    QString fp = QCoreApplication::applicationDirPath() + "/rdesktop_conf.json";
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

    // Run the RDesktop console application to connect to the VNC server.
    statusBar()->showMessage("Connecting to " + hostPort + "..." + " Please wait for 10 seconds.", 10000);
    qDebug() << "Connecting to" << hostPort;

    //// Disable the button for several seconds to avoid frequent re-connect.
    this->disableButtonForSeconds(ui->connectButton, 10);

    //// Start the RDesktop console application.
    ret = rd_proc_run();
    qDebug() << "Run rdesktop console process: ret:" << ret;

    // Minimize the current window so the loader does not block the view of GreenBox DesktopMgr window.
    this->showMinimized();
}

void RDGUIMain::onClearHistoryClicked()
{
    connectionHistory.clear();
    ui->remoteHostComboBox->clear();
    settings->remove(HISTORY_KEY);
    for (const QString &k : settings->allKeys())
    {
        if (k.startsWith("PerHost/"))
            settings->remove(k);
    }
    settings->sync();
}

void RDGUIMain::onHelpClicked()
{
    QMessageBox::information(this, "Help", QStringLiteral("GreenBox RDesktop Help\n\n"
                                                          "- Enter a valid hostname or IP (e.g., server.domain.com or 192.168.1.100).\n"
                                                          "- To specify a port, append it with a single colon, e.g., mypc:5902.\n"
                                                          "- Click 'Show Options' to adjust bandwidth settings.\n"
                                                          "- Click 'Connect' to start (only enabled when valid).\n"
                                                          "- In System → Connection History you can clear connection history."));
}

void RDGUIMain::disableButtonForSeconds(QPushButton *button, int seconds)
{
    button->setEnabled(false);
    QTimer::singleShot(seconds * 1000, [button]()
                       { button->setEnabled(true); });
}

void RDGUIMain::rd_proc_init(void)
{
    // set up the process
    _rd_proc = new QProcess(this);

    // tell CreateProcess to hide the console window
    _rd_proc->setCreateProcessArgumentsModifier(
        [](QProcess::CreateProcessArguments *args)
        {
            // option A: completely no window
            args->flags |= CREATE_NO_WINDOW;

            // -- or, option B: use STARTUPINFO to hide it --
            // args->startupInfo->dwFlags    |= STARTF_USESHOWWINDOW;
            // args->startupInfo->wShowWindow = SW_HIDE;
        });
}

void RDGUIMain::rd_proc_fini(void)
{
    if (_rd_proc)
    {
        delete _rd_proc;
        _rd_proc = NULL;
    }
}

int RDGUIMain::rd_proc_run(void)
{
    _rd_proc->start("./sl_rdesktop.exe");
    if (!_rd_proc->waitForStarted())
    {
        qDebug() << "Failed to start sl_rdesktop.exe process!";
        return 1;
    }

    // On success
    return 0;
}

void RDGUIMain::rd_proc_stop(void)
{
    if (_rd_proc->state() == QProcess::Running)
    {
        // send an "Enter" character to its stdin
        _rd_proc->write("\r\n");
        _rd_proc->waitForBytesWritten();

        _rd_proc->closeWriteChannel(); // Signal end of input if needed
        _rd_proc->waitForFinished();
    }
}