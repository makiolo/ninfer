# [ISSUE-002] Las dependencias host de `r9700` viven en un árbol vcpkg temporal fuera del repo

> **Type:** Defect\
> **Status:** Closed\
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

## 5. Estado (2026-11-17, director relevo 6)
- Ubicación estable versionada: `cmake/NinferPlatform.cmake` (desde `b8fd7fe2`, R97-08) toma por defecto
  `$VCPKG_ROOT/installed-ninfer-r9700` (aquí `G:\vcpkg_root\installed-ninfer-r9700`, 420 ficheros verificados), con
  `NINFER_R9700_VCPKG_INSTALLED_DIR` como alternativa.
- Árbol temporal de `.agent-tmp/R97-03/` borrado tras cerrar R97-08 (`buildtrees`, `packages`, `try2` y la copia de
  `vcpkg_installed`, 1,4 GiB); quedan 8 MB de logs de evidencia de R97-03.
- Falta verificar: configurar `r9700-dev` en un directorio nuevo sin variables de sesión (la caché actual de
  `build/r9700-dev` aún guarda el `VCPKG_INSTALLED_DIR` de su primera configuración). Lo comprueba la primera
  reconfiguración limpia (R97-16 o R97-31); el procedimiento en `docs/maintainer/build-system.md` va con la revisión
  de documentación que programa el usuario (AGENTS.md).

## 6. Cierre (2026-11-19, director relevo 7)
- Verificado en R97-16: `cmake --preset r9700-dev --fresh` con `NINFER_R9700_VCPKG_INSTALLED_DIR` y `VCPKG_INSTALLED_DIR`
  fuera del entorno resolvió `G:/vcpkg_root/installed-ninfer-r9700` (`.agent-tmp/R97-16/configure-fresh.log:1`);
  confirmado por la revisión de R97-16 (650, Codex `gpt-6-astra` xhigh).
