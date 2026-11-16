# PLAN_HOST — Lado CPU en Windows nativo

Sprint: SPRINT-01 · Grupo: HOST · Fuente: `backlog/PLAN_ROCM_R9700.md` §F2.
Depende de: R97-01, R97-03. Bloquea: INTEGRATION.

## 1. Contexto y estado de partida

Ficheros con APIs exclusivas de Linux (sin contar `src/distributed/`, que queda OFF):
`src/core/io/file.cpp`, `src/core/io/read_queue.{h,cpp}` (io_uring, `O_DIRECT`), `src/core/cpu/system.cpp` (hwloc),
`src/product/logging/{logging,startup_log}.cpp`, `src/product/media_acquire/acquire.cpp`,
`src/serve/{http_transport,observations,request_log}.cpp`, `apps/serve/main.cpp` (señales),
`src/runtime/engine/context_cache/context_cost.cpp`.

## 2. Decisiones fijadas

- Windows nativo (usuario, 2026-11-16). Compilador host: el `clang++` de ROCm (ABI MSVC).
- Cada módulo conserva su contrato público; solo cambia la implementación de plataforma dentro del módulo dueño.

## 3. Tareas

### R97-05 — Lectura de pesos en Windows (`core/io`)
- Nivel L5 · Riesgo medio (lectura de 19 GiB, alineación) · Autor: strong · Revisor: otro proveedor.
- `read_queue` sobre `ReadFile` overlapped/IOCP con `FILE_FLAG_NO_BUFFERING` (alineación de sector) detrás del mismo
  contrato; `file.cpp` con `CreateFileW`. Primero corrección; rendimiento medido en R97-19.
- Aceptación: tests existentes de `core/io` y de artifact en verde en Windows; lectura de un fichero > 4 GiB con
  hash idéntico al de referencia.

### R97-06 — Sistema, señales y logging
- Nivel L4 · Riesgo bajo · Autor: medium–strong · Revisor: medium de otro proveedor.
- `cpu/system.cpp` (hwloc de vcpkg o `GetLogicalProcessorInformationEx`), señales → `SetConsoleCtrlHandler`,
  logging y startup log sin `<unistd.h>`/`<sys/*>`.
- Aceptación: compila en `r9700`; Ctrl+C detiene `ninfer` limpiamente (test manual documentado en el cierre).

### R97-07 — Servidor HTTP y adquisición de medios
- Nivel L4 · Riesgo bajo · Autor: medium–strong · Revisor: medium de otro proveedor.
- `serve/*` y `apps/serve/main.cpp` con Winsock (cpp-httplib ya es portable); `media_acquire` compila aunque Vision
  quede fuera del hito.
- Aceptación: `ninfer-serve` arranca y responde a `/v1/models` con un Engine falso o el test de protocolo existente.

### R97-08 — Tests host en Windows
- Nivel L4 · Riesgo bajo · Autor: medium · Revisor: medium de otro proveedor.
- Activar en `r9700-dev` los tests que no usan GPU (artifact, frontend, tokenizer, protocolos OpenAI/Anthropic,
  logging) y arreglar los fallos de plataforma.
- Aceptación: `ctest --test-dir build/r9700-dev -L host` (o el filtro equivalente) en verde; lista de tests excluidos
  y motivo.

## 4. Orden y paralelismo

R97-05, R97-06 y R97-07 en paralelo (ficheros disjuntos); R97-08 al final. Puede avanzar en paralelo con PLAN_OPS.

## 5. Registro de cierre

| ID | Commit | Tests | Desviaciones |
|---|---|---|---|
