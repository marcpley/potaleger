#include "maelia.h"
#include "qdir.h"

maelia::maelia() {}

#include <QProcess>
#include <QDebug>

void runMaeliaSimulation() {
    QProcess process;
    process.start("python3", {"run_maelia.py", "scenario.xml"});

    if (!process.waitForFinished()) {
        qDebug() << "Erreur lors de l'exécution du script Maelia";
        return;
    }

    QByteArray output = process.readAllStandardOutput();
    qDebug() << "Sortie Maelia : " << output;

    // Lire le fichier de résultats (ex: results.csv)
    QFile file("results.csv");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            // Traiter chaque ligne de résultats
            qDebug() << "Résultat : " << line;
        }
        file.close();
    }
}
