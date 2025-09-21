#!/bin/bash
cd /Volumes/anis/MiniAccess/build
export QT_LOGGING_RULES="*=true"
export QT_FORCE_STDERR_LOGGING=1
/Volumes/anis/MiniAccess/build/MiniAccess.app/Contents/MacOS/MiniAccess 2>&1 | grep -E "(DEBUG|loadTables|RefreshTables|RelationshipsView)" --line-buffered