#!/bin/bash
cd /Volumes/anis/MiniAccess/build
echo "Iniciando aplicación y filtrando mensajes de debug..."
./MiniAccess.app/Contents/MacOS/MiniAccess 2>&1 | grep -E "(RefreshTables|tablas disponibles|RelationshipsView|setTableEditor|No hay tablas)" --line-buffered