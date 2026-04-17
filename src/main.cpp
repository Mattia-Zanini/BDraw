#include "mainwindow.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h> // Necessario per i log colorati

#include <QApplication>
#include <QLocale>
#include <QTranslator>

// Funzione di callback che intercetta i messaggi di Qt
// (qDebug, qWarning, ecc.) e li ridirige verso spdlog.
void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    // Crea un logger per la console che supporta i colori
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);

    // Configura il pattern di spdlog: %^ e %$ racchiudono la parte del messaggio che deve 
    // essere colorata in base al livello (es. INFO in verde, WARN in giallo, ERROR in rosso).
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    // Installa il gestore dei messaggi personalizzato
    qInstallMessageHandler(qtMessageHandler);

    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "BDraw_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();

    return QCoreApplication::exec();
}

void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    std::string message = msg.toStdString();

    switch (type)
    {
    case QtDebugMsg:
        spdlog::debug(message);
        break;
    case QtInfoMsg:
        spdlog::info(message);
        break;
    case QtWarningMsg:
        spdlog::warn(message);
        break;
    case QtCriticalMsg:
        spdlog::error(message);
        break;
    case QtFatalMsg:
        spdlog::critical(message);
        abort();
    }
}