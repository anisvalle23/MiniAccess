#include "projectpathsqt.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

QStringList ProjectStorageQt::s_markers{
    ".miniaccess_root",
    "CMakeLists.txt",
    ".git"
};

ProjectStorageQt::ProjectStorageQt(const QString& projectName)
    : m_projectName(projectName) {}

void ProjectStorageQt::setRootMarkers(const QStringList& markers) {
    s_markers = markers;
}

std::optional<ProjectPathsQt> ProjectStorageQt::create() {
    auto repoOpt = findRepoRoot();
    if (!repoOpt.has_value()) {
        return std::nullopt;
    }

    const QString repo = repoOpt.value();
    const QString projectRoot = QDir(repo).filePath(QStringLiteral("proyectos/%1").arg(m_projectName));
    const QString tables  = QDir(projectRoot).filePath("tables");
    const QString indexes = QDir(projectRoot).filePath("indexes");
    const QString logs    = QDir(projectRoot).filePath("logs");
    const QString meta    = QDir(projectRoot).filePath("project.meta.json");

    QDir().mkpath(tables);
    QDir().mkpath(indexes);
    QDir().mkpath(logs);

    if (!QFile::exists(meta)) {
        QJsonObject obj{
            {"magic", "MINIACCESS_PROJECT"},
            {"version", 1},
            {"project", m_projectName},
            {"paths", QJsonObject{
                          {"tables",  "tables"},
                          {"indexes", "indexes"},
                          {"logs",    "logs"}
                      }}
        };
        QFile f(meta);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            f.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
            f.close();
        }
    }

    return ProjectPathsQt{ projectRoot, tables, indexes, logs, meta };
}

std::optional<QString> ProjectStorageQt::findRepoRoot() {
    const QString appDir  = QDir::cleanPath(QCoreApplication::applicationDirPath());
    QStringList starts;
    starts << QDir::currentPath();                // donde se ejecuta (IDE/terminal)
    starts << appDir;                             // carpeta del binario
    starts << QDir(appDir).absoluteFilePath(".."); // por si el binario está en ./build/<cfg>

    for (const QString& st : starts) {
        if (auto p = climbForMarker(st); p.has_value())
            return p;
    }
    return std::nullopt;
}

std::optional<QString> ProjectStorageQt::climbForMarker(QString start) {
    QDir cur(start);

    // Canonicaliza si es posible (sigue enlaces simbólicos, etc.)
    const QString canon = QFileInfo(cur.absolutePath()).canonicalFilePath();
    if (!canon.isEmpty())
        cur.setPath(canon);
    else
        cur.setPath(QDir::cleanPath(cur.absolutePath()));

    while (true) {
        for (const QString& m : s_markers) {
            if (QFileInfo(cur.filePath(m)).exists()) {
                return cur.absolutePath();
            }
        }
        if (!cur.cdUp()) break;
    }
    return std::nullopt;
}
