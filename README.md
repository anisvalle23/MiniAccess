# 📘 Documento de Requisitos del Producto (PRD): MiniAccess

## 1. Introducción

### 1.1. Propósito
Este documento detalla los requisitos funcionales y no funcionales para el desarrollo de **MiniAccess**, un gestor de base de datos educativo y funcional.  
El objetivo es servir como guía clara para el equipo de desarrollo, asegurando que el producto final cumpla con los objetivos establecidos en torno a la comprensión de **estructuras de datos avanzadas** y la **gestión de bases de datos**.

### 1.2. Audiencia
Este documento está dirigido a:
- Desarrolladores
- Arquitectos de software
- Testers
- Stakeholders del proyecto MiniAccess

### 1.3. Contexto y Visión General
Los sistemas de bases de datos son fundamentales en aplicaciones académicas, empresariales y de gestión.  
**MiniAccess** busca recrear funcionalidades de **Microsoft Access** utilizando **C++ y Java**, integrando estructuras como **árboles B, B+, B\*** y una **Avail List** para optimizar búsquedas, almacenamiento y reutilización de espacio.  
El sistema contará además con una **interfaz gráfica intuitiva** para facilitar la interacción del usuario.

---

## 2. Objetivos del Proyecto

### 2.1. Objetivo General
Desarrollar un **gestor de base de datos en C++/Java** que permita:
- Crear y manejar múltiples tablas con distintos tipos de datos.  
- Establecer relaciones entre ellas.  
- Usar índices eficientes para búsquedas exactas o por rango.  
- Gestionar espacio libre mediante **Avail List**.  
- Integrar una **interfaz gráfica** para el usuario.

### 2.2. Objetivos Específicos
- Crear, modificar y eliminar tablas con campos y tipos definidos.
- Implementar relaciones **1:1**, **1:N** y **N:M**.
- Incorporar **índices con árboles B, B+, B\***.
- Gestionar espacio libre mediante una **Avail List**.
- Desarrollar GUI para creación de tablas, inserción de registros y consultas.
- Permitir consultas básicas con filtros, ordenamientos y joins limitados.

### 2.3. Beneficios
- Comprender la gestión de registros y tablas en archivos binarios.
- Aprender el funcionamiento de índices avanzados (B, B+, B\*).
- Manejar relaciones entre tablas y claves foráneas.
- Desarrollar habilidades en GUI y visualización de datos.

---

## 3. Funcionalidades (Módulos y Características)

### 3.1. Gestión de Tablas
- Crear, modificar y eliminar tablas.  
- Definición de campos con tipos de datos soportados:  
  - **int**: IDs, claves primarias, edades.  
  - **float**: notas, precios, promedios.  
  - **bool**: estados activos/inactivos.  
  - **char[N]**: nombres, descripciones cortas.  
  - **string**: direcciones, observaciones.  
- Propiedades de campos:  
  - Texto (máx. 255 caracteres).  
  - Número (entero, decimal, doble, byte).  
  - Moneda (Lps, Dollar, Euro, Millares).  
  - Fecha (DD-MM-YY, DD/MM/YY, DD/MESTEXTO/YYYY).  
- **Metadatos** en archivos `.meta` para reconstrucción de tablas.

---

### 3.2. Gestión de Registros
- **Inserción**: Validación de tipos, búsqueda en Avail List, actualización de índices.  
- **Eliminación**: Marcaje lógico, actualización de Avail List, limpieza de índices.  
- **Actualización**: Modificación de campos, integridad referencial, actualización de índices.  
- **Consultas**: Por clave, por rango o secuenciales usando índices.  
- **Persistencia**: Archivos binarios `.mad`.

---

### 3.3. Relaciones entre Tablas
- Soporte para relaciones:  
  - **1:1** (Empleado ↔️ Credencial).  
  - **1:N** (Profesor ↔️ Curso).  
  - **N:M** (Estudiante ↔️ Curso con tabla intermedia).  
- **Integridad referencial**: Validación de claves foráneas y eliminación en cascada opcional.

---

### 3.4. Sistema de Índices
- **Objetivo**: Acelerar búsquedas en disco, reduciendo accesos.  
- Tipos de índices:  
  - **Árbol B**: búsquedas exactas y actualizaciones frecuentes.  
  - **Árbol B+**: hojas enlazadas para recorridos secuenciales y rangos.  
  - **Árbol B\***: redistribución de claves antes de dividir nodos.  
- **Integración CRUD**: Cada operación actualiza índices de forma automática.  

---

### 3.5. Gestión de Espacio (Avail List)
- Lista enlazada de registros eliminados.  
- Inserción rápida de espacios disponibles (O(1)).  
- Estrategias: **First-fit, Best-fit, Worst-fit**.  
- Compactación periódica para reducir fragmentación.

---

### 3.6. Interfaz Gráfica (GUI)
- Visualización de tablas y registros.  
- Formularios para agregar, modificar y eliminar registros.  
- Navegación entre registros y relaciones.  
- Consultas con filtros y ordenamiento visual.  
- Soporte para **SQL básico**: SELECT, INSERT, DELETE.  
- **Tecnologías sugeridas**: Qt, SFML, SDL.  

---

## 4. Flujo de Trabajo del Usuario
1. Crear tablas (ej: Estudiantes, Profesores, Cursos).  
2. Definir campos y tipos de datos.  
3. Establecer relaciones (1:1, 1:N, N:M).  
4. Insertar registros usando la GUI.  
5. Ejecutar consultas rápidas con índices B/B+/B\*.  
6. Eliminar registros → reutilización de espacio con Avail List.  
7. Visualizar relaciones y generar reportes simples.

---

## 5. Aplicaciones del Proyecto
- **Académico**: Gestión de estudiantes, cursos, profesores, calificaciones.  
- **Inventario**: Control de productos, proveedores y stock.  
- **Bibliotecario**: Gestión de libros, préstamos y usuarios.

---

## 6. Requisitos Técnicos
- **Lenguaje**: C++ / Java.  
- **Almacenamiento**: Archivos binarios `.mad` (datos) y `.meta` (metadatos).  
- **Estructuras**: Árboles B, B+, B\* y Avail List.  
- **GUI**: Qt, SFML, SDL.  

---
