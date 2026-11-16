# PLAN_HOST — Lado CPU en Windows nativo

Sprint: SPRINT-01 · Grupo: HOST · Fuente: `backlog/PLAN_ROCM_R9700.md` (etiqueta `archive/2026-11-16/sprint-01-sources`) §F2.
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
| R97-05 | `6b075a37`, `168398d7` | Tests de io (original + bordes + >4 GiB) en verde en Windows; fichero de 4 831 838 331 B con 8 lecturas en vuelo cruzando 4 GiB: SHA-256 idéntico (Python, `hashlib` independiente y BCrypt) `18beb34a…454d`; CTest DLL 4/4 (incl. artifact `reader`/`backing`); mutaciones (EOF, `OffsetHigh=0`) detectadas por el revisor. Revisión Sonnet 5.5 high: ACCEPT WITH CONDITIONS (corregidas en `168398d7`). | `test_io_edges` solo en Windows (comprueba IOCP); `wake()` sin coalescencia documentado; alineación leída de `FILE_STORAGE_INFO` (512/4096 aquí). `ninfer_core` completo aún no enlaza (R97-06/08). |
| R97-06 | `761d4440`, `a0ade27f`, `92ae1c50` | CPU 4/4 (`system`, `task_pool`, `x86`, `executor`) en `r9700`; host_system 2/2 (`pretty_logging`, `process_signals` con Ctrl+C, Ctrl+Break y cierre de consola reales, 5/5 repeticiones); Ctrl+C con ejecutable mínimo del mismo módulo (`.agent-tmp/R97-06/ctrl-c.md`), prueba con `ninfer-serve` pendiente de que enlace (R97-08). Revisión Sonnet 5.5 high: ACCEPT WITH CONDITIONS (deuda documentada aquí). | hwloc de vcpkg sin cambios en `system.cpp`; `ninfer_cpu_executor` compila en `r9700` (parte de ISSUE-001 resuelta). Deuda: `ninfer_cpu_tensor_test` registrado en `r9700` no compila hasta portar nvtx3 (ISSUE-001); `ProcessSignals` vive en `ninfer_product_logging` (ubicación a revisar); `tests/host_system` es una compuerta independiente que nadie invoca y la rama POSIX de `ProcessSignals` no tiene test automático; el destructor restaura los manejadores POSIX previos. `r9700-dev` no configura: falta JPEG en el árbol vcpkg offline (R97-07). |
