#include "rdgui_main.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QLockFile>
#include <QDir>
#include <QMessageBox>

#define RFILES_UI_LOCK_FILE "RFilesUI.lock"

int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath(u8R"(..\libs\qt\plugins)"); // Point to the $(gb_root)\libs\qt\plugins folder.

    QApplication a(argc, argv);

    // Ensure only one instance is running
    //// Choose a unique lock file name in the temp directory
    QString lockPath = QDir::temp()
                    .absoluteFilePath(QStringLiteral(RFILES_UI_LOCK_FILE));
    QLockFile lockFile(lockPath);
    //// Prevent a stale lock from blocking forever
    lockFile.setStaleLockTime(0);

    if (!lockFile.tryLock(100)) {
        //// Another instance is running
        QMessageBox::warning(nullptr,
                            QObject::tr("GreenBox RFiles"),
                            QObject::tr("Another copy of this application is already running."));
        return 0;
    }

    // load and set window icon
    QString iconPath = QCoreApplication::applicationDirPath() + "/icon.png";
    if (QFile::exists(iconPath))
        a.setWindowIcon(QIcon(iconPath));

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "RFiles_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    RDGUIMain w;
    w.show();
    return a.exec();
}
