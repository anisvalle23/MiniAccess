#ifndef PROJECTSTORAGEQT_H
#define PROJECTSTORAGEQT_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QList>
#include <optional>

struct ProjectPathsQt {
    QString root;    // <repo_root>/data/projects/<Proyecto>
    QString tables;  // .../tables
    QString indexes; // .../indexes
    QString logs;    // .../logs
    QString meta;    // .../project.meta.json
};

// Estructura para representar información de un proyecto existente
struct ProjectInfoQt {
    QString name;           // Nombre del proyecto
    QString displayName;    // Nombre para mostrar (del JSON o del directorio)
    QString path;           // Ruta completa al directorio del proyecto
    QString metaPath;       // Ruta al archivo project.meta.json
    QDateTime created;      // Fecha de creación
    QDateTime modified;     // Fecha de última modificación
    QString description;    // Descripción (del JSON)
    int version;           // Versión del proyecto
    bool isValid;          // Si el proyecto tiene estructura válida
    int tableCount;        // Número de tablas (si está disponible)
};

class ProjectStorageQt {
public:
    explicit ProjectStorageQt(const QString& projectName);

    // Crea (si no existe) la estructura de carpetas y project.meta.json
    // Retorna rutas listas para usar. std::nullopt si no se encontró la raíz del repo.
    std::optional<ProjectPathsQt> create();

    // Métodos para listar proyectos existentes
    static QList<ProjectInfoQt> listExistingProjects();
    static std::optional<ProjectInfoQt> getProjectInfo(const QString& projectName);
    static bool isValidProject(const QString& projectPath);

    // (Opcional) Cambiar los marcadores que identifican la raíz del repo
    static void setRootMarkers(const QStringList& markers);

private:
    static std::optional<QString> findRepoRoot();
    static std::optional<QString> climbForMarker(QString start);

private:
    QString m_projectName;
    static QStringList s_markers; // orden de preferencia
};

#endif // PROJECTSTORAGEQT_H
