#ifndef PROJECTSTORAGEQT_H
#define PROJECTSTORAGEQT_H

#include <QString>
#include <QStringList>
#include <optional>

struct ProjectPathsQt {
    QString root;    // <repo_root>/data/projects/<Proyecto>
    QString tables;  // .../tables
    QString indexes; // .../indexes
    QString logs;    // .../logs
    QString meta;    // .../project.meta.json
};

class ProjectStorageQt {
public:
    explicit ProjectStorageQt(const QString& projectName);

    // Crea (si no existe) la estructura de carpetas y project.meta.json
    // Retorna rutas listas para usar. std::nullopt si no se encontró la raíz del repo.
    std::optional<ProjectPathsQt> create();

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
