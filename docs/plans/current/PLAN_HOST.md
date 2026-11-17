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
| R97-07 | `535693be` | `r9700-dev` configura (JPEG 62, FFmpeg 61.7, libcurl 8.13) y compilan los 25 objetos de `ninfer_serve`/`ninfer_media_acquire`/`ninfer-serve`; compuerta `tests/host_serve` 3/3 con el código de producción (`/v1/models` con fixture, corte de SSE con cancelación, puerto ocupado, IPv6 literal, protocolo y raíz), reejecutada desde cero por el revisor; el revisor probó además 26 URLs y rutas `..`/unidad/UNC/`\\?\`. Revisión Sonnet 5.5 high: ACCEPT WITH CONDITIONS (documentar, sin intento 2). | La configuración depende del árbol vcpkg de `.agent-tmp/R97-03/` y de variables fuera del repo (ISSUE-002, R97-08). `ninfer-serve` no enlaza todavía (nvtx3, ISSUE-001; Ops de PLAN_OPS); procedimiento real en `.agent-tmp/R97-07/serve.md`. Winsock: httplib en serve, `curl_global_init` en media; listener `SO_EXCLUSIVEADDRUSE` + keepalive solo en Windows; sin `TCP_USER_TIMEOUT` en Windows. Preexistente/backlog: literal IPv6 en Linux, `::a.b.c.d` sin filtrar, tests de ruta (ISSUE-003). |
| R97-08 | `b8fd7fe2`, `e9f25fff`, `69e094d3`, `034bc7f9`, `c05ca4d4`, `35caa77f`, `b29da1e8`, `2a91fb04`, `e1f750de`, `1e4cff7a` | `ctest -L host` en `r9700-dev`: **67 de 67**, 0 omitidos, sin `WILL_FAIL` (`.agent-tmp/R97-08/gate4.log`, tras `1e4cff7a`, que recompila `ninfer_resource_manager_test` con el `static_assert`). 184 tests registrados: 67 host, 117 excluidos con causa (`.agent-tmp/R97-08/excluded.md`: 97 GPU, 10 por enlace con Ops, 4 Engine, 3 inyección ELF, 3 otras). Revisión Sonnet 5.5 high (547): ACCEPT WITH CONDITIONS; el defecto real era la alineación de `RecoveryCostSum` bajo el ABI de MSVC (`e1f750de`, director), fijada con `static_assert` (`1e4cff7a`, condición 1). | Condición 2: `gmtime_r` de `apps/perplexity/report.cpp:30` añadido a R97-18 (PLAN_INTEGRATION §3). Condición 3: inyección de fallos ELF sin equivalente COFF → ISSUE-006 (backlog). Reactivación: `openai_schema_test` y `http_error_handler_test` cuando enlace el motor (R97-16, R97-31); los 97 GPU tras sus Ops; `glm5next_media` cuando FFmpeg de vcpkg traiga PNG (solo medios). Árbol vcpkg estable en `G:\vcpkg_root\installed-ninfer-r9700` (`NINFER_R9700_VCPKG_INSTALLED_DIR`; ISSUE-002). Cinco cambios en tests, solo de plataforma. |
