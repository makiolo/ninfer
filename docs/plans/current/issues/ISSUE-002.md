# [ISSUE-002] Las dependencias host de `r9700` viven en un árbol vcpkg temporal fuera del repo

> **Type:** Defect\
> **Status:** Open\
> **Detected:** 2026-11-17\
> **Origin:** SPRINT-01 / PLAN_HOST / R97-07 (revisión 504, Sonnet 5.5 high, hallazgo 1)\
> **Affected component:** resolución de dependencias de `r9700` (`cmake/Dependencies.cmake`, `CMakePresets.json`, vcpkg)\
> **Reference specification:** AGENTS.md («Local environment»: `.agent-tmp/` es temporal y se borra al acabar la tarea)\
> **Severity:** Medium\
> **Destination:** Current sprint\
> **Priority and reason:** cualquier limpieza de `.agent-tmp/R97-03/` deja sin configurar todas las dianas `r9700`\
> **Owner:** R97-08 (cierre de HOST; tests host y entorno de compilación en Windows)\
> **Follow-up:** incluir en la ficha de R97-08\

---

## 1. Context and observed behaviour
- `cmake --preset r9700-dev` solo configura con
  `-DVCPKG_MANIFEST_INSTALL=OFF -DVCPKG_INSTALLED_DIR=S:/Projects/ninfer/.agent-tmp/R97-03/r9700-dev/vcpkg_installed`,
  su `bin` en `PATH`, `VCPKG_ROOT=G:\vcpkg_root`, `HIP_PATH` y el entorno MSVC. Nada de eso está en el repo ni documentado.
- El árbol lo creó R97-03 (instalación offline de vcpkg) dentro de `.agent-tmp/`, ignorado por git.
- Desde un checkout limpio el preset intentaría instalar el manifiesto (red o caché binaria).

## 2. Divergence from the specification
- AGENTS.md: `.agent-tmp/` es para temporales de agente; un prerrequisito de compilación de todas las dianas `r9700`
  no puede vivir allí ni depender de variables de la sesión del autor.

## 3. Minimal reproducible case
Configurar `r9700-dev` en un directorio nuevo sin esas variables: falla en `find_package(JPEG)`.

## 4. Expected resolution
Ubicación estable del árbol de dependencias (fuera de `.agent-tmp/`, p. ej. en el `VCPKG_ROOT` o una ruta documentada)
y un preset o fichero de entorno `r9700` versionado que lo seleccione; procedimiento documentado donde corresponda
según `docs/maintainer/build-system.md`.
