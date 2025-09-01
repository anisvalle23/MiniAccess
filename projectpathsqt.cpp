#include "projectpathsqt.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <QJsonArray>
#include <algorithm>

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

// Implementación de métodos para listar proyectos existentes
QList<ProjectInfoQt> ProjectStorageQt::listExistingProjects()
{
    QList<ProjectInfoQt> projects;
    
    auto repoOpt = findRepoRoot();
    if (!repoOpt.has_value()) {
        return projects; // Lista vacía si no se encuentra la raíz
    }
    
    const QString repo = repoOpt.value();
    const QString proyectosDir = QDir(repo).filePath("proyectos");
    
    QDir dir(proyectosDir);
    if (!dir.exists()) {
        return projects; // No existe la carpeta proyectos
    }
    
    // Obtener todas las subcarpetas en proyectos/
    QStringList projectDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const QString& projectName : projectDirs) {
        auto projectInfo = getProjectInfo(projectName);
        if (projectInfo.has_value()) {
            projects.append(projectInfo.value());
        }
    }
    
    // Ordenar por fecha de modificación (más reciente primero)
    std::sort(projects.begin(), projects.end(), 
              [](const ProjectInfoQt& a, const ProjectInfoQt& b) {
                  return a.modified > b.modified;
              });
    
    return projects;
}

std::optional<ProjectInfoQt> ProjectStorageQt::getProjectInfo(const QString& projectName)
{
    auto repoOpt = findRepoRoot();
    if (!repoOpt.has_value()) {
        return std::nullopt;
    }
    
    const QString repo = repoOpt.value();
    const QString projectPath = QDir(repo).filePath(QStringLiteral("proyectos/%1").arg(projectName));
    const QString metaPath = QDir(projectPath).filePath("project.meta.json");
    
    QDir projectDir(projectPath);
    if (!projectDir.exists()) {
        return std::nullopt;
    }
    
    ProjectInfoQt info;
    info.name = projectName;
    info.path = projectPath;
    info.metaPath = metaPath;
    info.isValid = isValidProject(projectPath);
    
    // Obtener fechas del directorio
    QFileInfo dirInfo(projectPath);
    info.created = dirInfo.birthTime(); // Fecha de creación (si está disponible)
    info.modified = dirInfo.lastModified();
    
    // Valores por defecto
    info.displayName = projectName;
    info.description = "";
    info.version = 1;
    info.tableCount = 0;
    
    // Leer metadatos del archivo JSON si existe
    if (QFile::exists(metaPath)) {
        QFile metaFile(metaPath);
        if (metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = metaFile.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                
                // Leer información del JSON
                if (obj.contains("project")) {
                    info.displayName = obj["project"].toString();
                }
                if (obj.contains("description")) {
                    info.description = obj["description"].toString();
                }
                if (obj.contains("version")) {
                    info.version = obj["version"].toInt(1);
                }
                if (obj.contains("created")) {
                    info.created = QDateTime::fromString(obj["created"].toString(), Qt::ISODate);
                }
                if (obj.contains("last_modified")) {
                    info.modified = QDateTime::fromString(obj["last_modified"].toString(), Qt::ISODate);
                }
                
                // Contar tablas si están listadas en el JSON
                if (obj.contains("tables") && obj["tables"].isArray()) {
                    info.tableCount = obj["tables"].toArray().size();
                }
            }
        }
    }
    
    // Si no se pudo obtener la fecha de creación del JSON, usar la del directorio
    if (!info.created.isValid()) {
        info.created = info.modified;
    }
    
    // Contar archivos .mad en la carpeta tables si no se obtuvo del JSON
    if (info.tableCount == 0) {
        QString tablesPath = QDir(projectPath).filePath("tables");
        QDir tablesDir(tablesPath);
        if (tablesDir.exists()) {
            QStringList madFiles = tablesDir.entryList(QStringList() << "*.mad", QDir::Files);
            info.tableCount = madFiles.size();
        }
    }
    
    return info;
}

bool ProjectStorageQt::isValidProject(const QString& projectPath)
{
    QDir projectDir(projectPath);
    if (!projectDir.exists()) {
        return false;
    }
    
    // Verificar que existan las carpetas principales
    QStringList requiredDirs = {"tables", "indexes", "logs"};
    for (const QString& dirName : requiredDirs) {
        if (!projectDir.exists(dirName)) {
            return false;
        }
    }
    
    // Verificar que exista el archivo de metadatos
    QString metaPath = QDir(projectPath).filePath("project.meta.json");
    if (!QFile::exists(metaPath)) {
        return false;
    }
    
    // Verificar que el JSON sea válido
    QFile metaFile(metaPath);
    if (metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray data = metaFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            // Verificar que tenga los campos mínimos necesarios
            return obj.contains("magic") && 
                   obj.contains("project") &&
                   obj["magic"].toString() == "MINIACCESS_PROJECT";
        }
    }
    
    return false;
}
