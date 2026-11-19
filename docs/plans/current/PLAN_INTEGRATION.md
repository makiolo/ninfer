# PLAN_INTEGRATION — Runtime y validación end-to-end

Sprint: SPRINT-01 · Grupo: INTEGRATION · Fuente: `backlog/PLAN_ROCM_R9700.md` (etiqueta `archive/2026-11-16/sprint-01-sources`) §F4–F5.
Depende de: HOST (R97-05..08) y OPS (R97-09..15, R97-29..31). Bloquea: PERF.

## 1. Contexto y estado de partida

- El runtime usa CUDA Graph (28 ficheros con `cudaGraph`), streams, eventos, memoria pinned (`cudaMallocHost`/
  `cudaHostRegister`) y consultas de memoria para el planificador de capacidad.
- Windows: TDR mata kernels de más de ~2 s por defecto; WDDM reserva parte de la VRAM.
- Artefacto: `neroued/Qwen3.8-27B-NInfer`, `qwen3_8_27b.ninfer`, 20,437,521,664 bytes (19.03 GiB).

## 2. Decisiones fijadas

- Primer hito: texto + MTP + KV FP8, una GPU, sin Vision ni DFlash (usuario, 2026-11-16).
- El artefacto se descarga a `out/` solo en R97-17 (usuario, 2026-11-16).

## 3. Tareas

### R97-16 — Runtime en HIP
- Nivel L6 · Riesgo alto (ciclo de vida y concurrencia) · Autor: strong · Revisor: strong de otro proveedor.
- hipGraph para la captura y el replay del decode, streams y eventos, memoria pinned, consultas de memoria del
  planificador de capacidad (contando la reserva de WDDM). Documentar en el cierre el límite de TDR observado.
- Aceptación: tests de runtime y de capacidad en verde en `r9700`; test de captura y replay de graph.
- Heredado de R97-08 y R97-31: reactivar `openai_schema_test` y `http_error_handler_test` (necesitan el motor);
  primer enlace real del motor contra `ninfer_ops` (el inventario de símbolos de R97-31 casa por nombre, no por
  sobrecarga); y verificar ISSUE-002 con una configuración `--fresh` **sin** `NINFER_R9700_VCPKG_INSTALLED_DIR` en el
  entorno, para ejercitar la ruta por defecto de `cmake/NinferPlatform.cmake:32`.

### R97-17 — Carga del modelo y generación
- Nivel L5 · Riesgo medio · Autor: strong · Revisor: otro proveedor.
- Descargar `qwen3_8_27b.ninfer` a `out/` (comprobar tamaño exacto) y cargarlo; generación con `apps/ninfer`.
- Aceptación: `ninfer out/qwen3_8_27b.ninfer --prompt ... --max-context 8192 --max-tokens 256` produce texto
  coherente; tests de modelo Qwen3.5 (carga y generación) en verde con el artefacto explícito.

### R97-18 — Corrección numérica end-to-end y MTP/KV FP8
- Nivel L6 · silent-numeric · Autor: strong · Revisor: strong de otro proveedor.
- `ninfer-perplexity` en un corpus fijo, comparado con la referencia del model card o una ejecución CUDA
  documentada; generación con MTP y `--kv-dtype fp8`; tasa de aceptación MTP registrada.
- Entrada de la revisión de R97-08 (547): `ninfer-perplexity` se compila en `r9700`, pero
  `apps/perplexity/report.cpp:30` usa `gmtime_r`, que no existe en Windows; portarlo es parte de esta tarea (y
  reactivar el test host `perplexity_evaluation`, excluido hoy en `r9700` por ese motivo).
- Aceptación: perplejidad dentro de la tolerancia acordada en la tarjeta (el oráculo la fija antes de ejecutar);
  tests MTP de `tests/models/qwen3_5` en verde.

### R97-28 — Documentación de producto de la diana R9700
- Nivel L3 · Riesgo bajo · Autor: Claude Sonnet 5.5 high · Revisor: el director (contraste con el alcance del sprint).
- Añadida por el usuario (2026-11-17): «En todos los textos del repo que dice: RTX 5090 and DGX Spark, ahora añade la
  R9700 de amd, tambien en los ejemplos pon el preset de la r9700 [...] para ir preparando si nos pueden aceptar un
  futuro PR» y «escribelo, asumiendo que el sprint esta implementado, forma parte del sprint».
- README, docs de producto, ejemplo `examples/configs/r9700-qwen3_8-27b-groupwise.toml` y model card de Qwen3.8-27B
  con el alcance del sprint (Qwen3.8-27B `groupwise-int`, texto + MTP + KV FP8, una GPU, Windows/HIP); sin cifras de
  rendimiento inventadas (comentarios para R97-19..21). `docs/maintainer/build-system.md` lo completa R97-08.
- Aceptación: ninguna afirmación fuera de alcance; antes de cerrar el sprint, el director comprueba cada afirmación
  contra R97-17/18 (TOML de ejemplo arrancando con el modelo) y R97-19..21 rellena las cifras.

### R97-32 — `claude-local` con Qwen3.8 de ninfer en la ASUS
- Nivel L4 · Riesgo medio (herramienta compartida por todos los proyectos) · Autor: Codex · Revisor: otro modelo.
- Añadida por el usuario (2026-11-19): «cuando terminemos cambia claude-local para poder usarlo desde allí, pudiendo
  elegir entre 2 modelos, el gwen 3.8 de la asus, y el de la shappire, y comandos para cargar y descargar el modelo de
  la gpu».
- Fuera del repo: la fuente única es `~/.claude/skills/sprint/tools/claude_local.py` (los envoltorios de
  `S:/Apps/bin` no se editan); copia de seguridad antes de tocarlo. Hoy los grados van a LM Studio (`opus` → Qwen 3.8
  en la ASUS; `sonnet`/`haiku` → Qwen 3.8 en la Sapphire). Precisión del usuario (2026-11-19): «tanto asus como
  shappire usarian ninfer, que esperamos que sea mas rapido que lm studio. Y desde code, elegimos el modelo (o la gpu
  mas bien)». El cambio: **las dos GPU sirven Qwen 3.8 con ninfer** (`out/qwen3_8_27b.ninfer`), una instancia de
  `ninfer-serve` por GPU (ASUS por bus PCI 6, Sapphire por bus 3; el índice HIP no es estable, `claude_local.py` ya los
  reconoce por bus), con el protocolo Anthropic del propio servidor; LM Studio deja de servir esos grados. Desde Claude
  Code se elige la GPU eligiendo el modelo (un identificador por GPU, p. ej. `qwen3.8-asus` / `qwen3.8-sapphire`, que
  `/model` y `Agent` puedan pedir). Órdenes nuevas para cargar y descargar el modelo de cada GPU (arrancar y parar su
  servidor), sin romper las ranuras (`~/.claude-local/slots`) que usan los demás proyectos.
- La Sapphire lleva el escritorio (WDDM reserva más VRAM) y la comparten otros proyectos: comprobar que el modelo
  con su contexto cabe allí y fijar el contexto de cada GPU por medida, no por suposición.
- Aceptación: una sesión `claude-local` contra cada GPU responde y usa herramientas; cargar y descargar dejan la VRAM
  de esa GPU como estaba (medido); los demás proyectos siguen funcionando con sus órdenes. **Sin benchmarks ni
  comparaciones con LM Studio** (usuario, 2026-11-19: «no pierdas tiempos, haciendo benchmarks ni comparaciones hasta
  que optimices primero»): se miden después de R97-19..21.
- Depende de R97-18 (corrección numérica comprobada). Rendimiento con prompts largos de Claude Code: prefill
  limitado por ISSUE-004 (TDR) y sin optimizar hasta R97-19..21 (SPRINT-02); se mide y se informa, no se arregla aquí.

## 4. Orden y paralelismo

R97-16 → R97-17 → R97-18 → R97-32, en serie (GPU y artefacto compartidos). R97-28 en paralelo (solo documentación);
su verificación final va tras R97-18.

## 5. Registro de cierre

| ID | Commit | Tests | Desviaciones |
|---|---|---|---|
| R97-28 (parcial: escrita y contrastada con el alcance; falta la verificación final tras R97-18) | `68f00ba3` + corrección del director en `README.md:21` | No aplica (documentación y un TOML); enlaces relativos comprobados por el autor | Pendiente de comprobar con el producto: `cuda_graph = true` (hipGraph, R97-16), TOML arrancando con el modelo (R97-17/18), nombres `*.exe`, conversión `groupwise-int` con `--device cpu` (leída en `tools/convert`, no ejecutada), y si la fila de 260 096 tokens del perfil cabe en la R9700 (el TOML limita a 131 072) |
