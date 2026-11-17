# EXECUTION — SPRINT-01 (ninfer)

Registro de ejecución del director. Una línea por hecho relevante; los cierres por tarea van en el registro de cada plan.

- 2026-11-16 21:32 UTC · director `ninfer-32` (Opus 5.5 high) arranca; deps READY; cuota Claude 96 %, Fable 100 %,
  Codex 94 %. ASUS (HIP 0) libre: 31,71/31,86 GB.
- 2026-11-16 · R97-01 cerrada (`92958cf5`, `d3302962`; registro `222b306d`).
- 2026-11-16 21:55 UTC (22:55 CET) · R97-03 lanzada en Codex `gpt-6.1-sol` high (intento 487).
- 2026-11-16 22:58 CET (21:58 UTC) · **Congelación por anuncio del usuario** (INBOX, vía `quantdesk-backend-a3`):
  «Vamos a congelarlos el desarrollo, hasta el reinicio de Claude, y luego me recuerdas hacer el reset de codex».
  Aplicación aquí: no se lanza trabajo nuevo ni hijos hasta el reinicio semanal de Claude (2026-11-16 23:59 UTC);
  R97-03 ya estaba en curso antes del anuncio y se deja terminar, pero su revisión y el oráculo de R97-02 esperan al
  reinicio. El recordatorio del reset de Codex lo hace la raíz de quantdesk-backend.
- 2026-11-16 22:05 UTC · **Aclaración del usuario en esta sesión:** «tu continua el sprint, ese anuncio era para todos,
  menos para ti, y para ayudarte a conservar la cuota, hasta que se reinicie». La congelación no aplica a ninfer: el
  sprint continúa (oráculo de R97-02 y revisiones incluidos).
- 2026-11-16 · R97-03 intento 1 BLOCKED (`95e6402e`, dependencias host resueltas con vcpkg; FFmpeg instalado). Hueco
  del plan: faltan `r9700/sources.cmake` en las Ops (incluidos por `${NINFER_TARGET}`) y los targets `CUDA::*`. Decisión
  del director: R97-03 (sube a L4) crea manifiestos vacíos y mapea `CUDA::*` en `Dependencies.cmake`; las tareas de
  OPS rellenan cada manifiesto. Intento 2 lanzado.
- 2026-11-16 ~23:00 UTC · R97-02 en revisión (492); R97-05 lanzada (Codex, HOST empieza: depende solo de R97-01/03).
- 2026-11-16 · R97-02 revisión 492: ACCEPT WITH CONDITIONS (numérica OK en host y ASUS, oráculo OK; el target CMake no
  enlaza por builtins de compiler-rt → intento 2). Nota para PLAN_OPS: sin mapear en la cabecera
  `cudaLaunchKernelEx`, `cudaLaunchAttribute*`, `cudaLaunchConfig_t`, `cudaGridDependencySynchronize`,
  `cudaTriggerProgrammaticLaunchCompletion` (PDL, `src/core/pdl.cuh`), `cudaOccupancyMaxActiveClusters`
  (`mhc/dgx_spark`) y `cudaRoundNearest` (FP4); `normalization` usa máscara de 32 bits en `__shfl_down_sync`;
  `sampling` necesita `cub/block/block_merge_sort.cuh` (hipCUB). Inventario: `.agent-tmp/R97-02/ops-compile.md`.
- 2026-11-16 23:0x UTC · R97-02 cerrada (`36f36b4a`, registro `a718ec76`). R97-04 lanzada (smoke con fuentes de dispositivo de core; enlace de `ninfer_core` completo pasa a R97-08).
- 2026-11-16 23:1x UTC · R97-04 revisión 496: ACCEPT WITH CONDITIONS (escaneo de módulos C++ y reenvío de `<cuda_*.h>` deben ser globales en `r9700`) → intento 2.
- 2026-11-16 23:2x UTC · R97-04 cerrada (`de5cce05`): **hito 1 cumplido** (r9700-ops configura, smoke en la ASUS). PLAN_BUILD completo. R97-05 entregada (`6b075a37`), a revisión.
- 2026-11-16 23:2x UTC · R97-05 revisión 498: ACCEPT WITH CONDITIONS (raíz de tests de io en el árbol fuente en todas las dianas) → intento 2 (500). R97-06 en curso (499).
- 2026-11-16 23:3x UTC · R97-05 cerrada (`168398d7`). R97-06 entregada (`761d4440`), a revisión.
- 2026-11-16 23:4x UTC · R97-06 entregada (`761d4440`, `a0ade27f`, `92ae1c50`), revisión 501 lanzada.
- 2026-11-16 23:4x UTC · Usuario: «no, lanzalo con opus 5.5» → el relevo 1 sale en claude-opus-5-5 high (no Fable).
- 2026-11-16 23:5x UTC · Relevo 1 aceptado (sesión 58ebdb7d). R97-06 revisión 501: ACCEPT WITH CONDITIONS; condiciones = documentar (sin intento 2): tensor test bloqueado por nvtx3 (ISSUE-001), señales en logging y compuerta host_system como deuda. **R97-06 cerrada.** JPEG ausente en el árbol vcpkg offline (`r9700-dev` no configura): va a la ficha de R97-07 (`media_acquire` es su consumidor).
- 2026-11-16 23:5x UTC · R97-07 lanzada (Codex gpt-6.1-sol high, intento 502; ficha pre-revisada READY). Oráculo de R97-09 preparado (Sonnet 5.5 high, READY), se lanza tras el reinicio de Claude (23:59 UTC). Codex al 96 %: si se agota, parar y preguntar.
- 2026-11-17 00:02 UTC · Cuota de Claude reiniciada (0 %). Oráculo de R97-09 lanzado (Sonnet 5.5 high, intento 503). R97-07 sigue en Codex (502). Usuario: ninfer no hace revisión de alertas.
- 2026-11-17 00:06 UTC · Recuperación en frío (sesión 58ebdb7d caída, sin hijos perdidos). R97-07 entregada por Codex
  (`535693be`); revisión Sonnet 5.5 high lanzada (intento 504). Oráculo de R97-09 (503) sigue vivo. Relevo 2.
- 2026-11-17 00:10 UTC · Incidente del director: un heredoc con comillas invertidas hizo que la herramienta Bash
  ejecutara como scripts ficheros del repo (eventos de hijos, Markdown, fuentes). Parado a los ~2 min; daños: un fichero
  vacío basura (borrado) y `build/r9700-dev` reconfigurado (ignorado por git); sin commits, sin procesos sueltos, hijos
  intactos. Regla: texto Markdown solo con Write/Edit.
- 2026-11-17 00:2x UTC · Oráculo de R97-09 entregado (`040f2a3a`; contraste C++/Python 0 discrepancias en 1154 casos).
  El `.cu` no lo compiló el oráculo (no encontró `hipcc`); el director lo analizó con `clang++ -x hip`: para en
  `memory.cuh:51` y `reduce.cuh:4` (trabajo del autor). Decisiones: PINNED debe pasar; `float4 atomicAdd`, `mma`,
  `pipeline` y afines fuera. Autor Codex lanzado (intento 505; ficha pre-revisada READY). Codex al 96 %.
- 2026-11-17 00:3x UTC · R97-07 revisión 504: ACCEPT WITH CONDITIONS (documentar). **R97-07 cerrada.** Nuevos
  ISSUE-002 (árbol vcpkg de `r9700` en `.agent-tmp/R97-03`, dueño R97-08) e ISSUE-003 (endurecimiento de
  `media_acquire`, preexistente, backlog). R97-08 desbloqueada (R97-05..07 cerradas).
- 2026-11-17 00:45 UTC · Usuario: «Pide a los otros agentes que necesitas una GPU reservada, creo que el proyecto
  reinforment_learning pretende usar la gpu». Aviso de reserva exclusiva de la ASUS (HIP 0, bus 6; Q-009) añadido al
  INBOX de los 8 proyectos restantes y enviado en directo a reinforcement-learning-f5/-0e y deeplearning-dev-8b; la
  Sapphire (HIP 1) sigue compartida. hipInfo: ambas GPU libres.
- 2026-11-17 00:5x UTC · R97-09 intento 1 (505) BLOCKED con `ff6599b3`: en la ASUS EXACT 823 027 / 4 fallos, BOUND
  5 688 / 0, PINNED 46 082 / 0. Los 4 fallos son un defecto de entrada del oráculo (`threadIdx.x % 9 - 4` sin signo en
  CUDA y HIP; la referencia usa `int`). Decisión del director: autorizado el cast con signo en el test (no cambia
  expectativas). Intento 2 lanzado (506, `card-fix.md`).
- 2026-11-17 01:xx UTC · R97-09 intento 2 (`80aaf2c5`): ASUS 0 fallos en EXACT/BOUND/PINNED. Revisión 507 (Sonnet 5.5
  high): ACCEPT WITH CONDITIONS; reejecutado en la ASUS 874 797 checks / 0 fallos; oráculo íntegro (solo `with_pow2` y
  el cast autorizado). Condición: sin guarda de wave32 en compilación (wave64 truncaría `__ballot_sync` en silencio) →
  intento 3 (508, `card-fix2.md`). Deuda baja a registrar al cierre: `__reduce_{and,or,xor}_sync` con bucle de
  shuffles también con máscara completa (rendimiento); `HIP_VISIBLE_DEVICES=0` fijo en `tests/cmake/HipCompatTests.cmake:9`.
- 2026-11-17 01:xx UTC · R97-09 intento 3 (508, `dec8d874`): guarda `static_assert(__has_builtin(__builtin_amdgcn_ballot_w32))`
  en la pasada de dispositivo `r9700`; el director comprobó las dos ramas (wave32 compila, `-mwavefrontsize64` falla con
  el mensaje). Test en la ASUS 874 797 / 0. **R97-09 cerrada** (registro en PLAN_OPS). Siguientes: R97-10..15
  (OPS) y R97-08 (HOST, con ISSUE-002).
- 2026-11-17 01:xx UTC · Codex al 97 % (R97-09 costó ~1 punto). Decisión del director: las suites de Ops ya usan
  referencia FP64 independiente (op-development.md), así que los oráculos de R97-10..15 auditan esa referencia,
  contrastan con Python y añaden casos de riesgo `r9700` (solo tests nuevos). Lanzados los oráculos de R97-10 (Opus
  5.5 high, 509) y R97-11 (Sonnet 5.5 high, 510); fichas pre-revisadas READY. Claude al 15 %.
- 2026-11-17 01:xx UTC · Usuario: «Lanza con Codex, quiero gastarlo». R97-08 lanzada en Codex `gpt-6.1-sol` high
  (intento 511; ficha READY) con ISSUE-002 (árbol vcpkg estable) y la parte nvtx3 de ISSUE-001 asignadas. Cupo lleno
  (2 oráculos + 1 autor). Los autores de R97-10/11 salen en Codex en cuanto entreguen sus oráculos.
- 2026-11-17 01:xx UTC · Oráculo de R97-11 (510) parcial por contexto: auditoría (rmsnorm, rope, argmax, Q8, elementwise
  con oráculos FP64 independientes; criterio de embedding Q8 laxo, 2^-8), contraste 61 casos / 0 discrepancias y
  `778d8840` (argmax_edge). Decisiones del director: |x|>1,8e19 en RMSNorm y NaN en argmax fuera de contrato;
  `gated_residual` fuera (0 usos en qwen3_5); caso Q8 bit a bit nuevo. Intento 2 lanzado (512, `oracle-card-2.md`).
- 2026-11-17 01:5x UTC · Oráculo de R97-10 (509) aceptado: `0ab0acdc` (referencia de `tests/ops/quantized_weight.h`
  independiente; contraste 129 casos / 0; casos de frontera row-split y rechazo de 3 formas solo Spark/5090). Autor
  lanzado en Codex `gpt-6-astra` xhigh (513), con el shim `cudaErrorNoDevice`/`InsufficientDriver` de
  `op_tester.h:65` como primer commit.
- 2026-11-17 01:5x UTC · Oráculo de R97-11 (512) aceptado: `3c796e59` (RMSNorm, RoPE 2^22, embedding Q8 bit a bit) +
  `778d8840`; alcance fijado por uso real en el camino de texto de Qwen3.8-27B. `linear/swiglu` sale de R97-11: se
  asigna cuando cierre R97-10. Autor lanzado en Codex `gpt-6.1-sol` high (514). Cupo lleno (3 autores Codex).
- 2026-11-17 01:54 UTC · Usuario (anuncio vía concertino-e6): «Aunque este codex al 3%, no dejeis de usarlo, asi
  cuando llegemos a 1% lo reinicio, pero hay que gastar». Aplicado: Codex sigue como autor; ya no se para por cuota
  escasa, solo si se agota de verdad (entonces PushNotification).
- 2026-11-17 02:0x UTC · Usuario: «He creado un fork, ves subiendo todo aqui https://github.com/makiolo/ninfer puedes
  subir la rama, o subirlo a master directamente, es un fork mio». Remoto `fork` = `git@github.com:makiolo/ninfer.git`
  (SSH; HTTPS se queda esperando al Git Credential Manager). Subida la rama `fork_R9700` (seguimiento
  `fork/fork_R9700`); `master` del fork se deja igual que el original para sincronizarlo. El director sube tras cada
  commit de registro; los hijos siguen sin push.
- 2026-11-17 02:09 UTC · Usuario (anuncio vía reinforcement-learning-0e): «Acabo de usar el reset de Codex, volvemos a
  usos normales, codex al 100% y claude casi nuevo». Codex 0 % consumido (reinicio 2026-11-24 02:09 UTC), Claude 20 %.
  Aplicado: el reparto vuelve a las reglas de siempre; Codex sigue como autor por el orden Q-010 (Claude orquesta y
  revisa, Codex implementa). Queda superado el anuncio de las 01:54 UTC.
- 2026-11-17 02:4x UTC · Usuario: «Haz que este proyecto pueda lanzar mas subagentes que 3, se decide dinamicamente».
  LAUNCH.md: cupo dinámico (tareas listas con rutas disjuntas, cuota, cola de compilación y GPU; un test de GPU a la
  vez). Observación: la cola tiene un candado por checkout y 2 ranuras globales, así que los autores que compilan en
  ninfer van por turnos (R97-11 lo tenía 39 min); se amplía con hijos que no compilan. Lanzados los oráculos de R97-12
  (515), R97-13 (516) y R97-14 (517), Opus 5.5 high, fichas READY. En curso: 3 autores Codex + 3 oráculos.
  Progreso de los autores: `563e4526` (shim de errores de R97-10), `c666530a` (registro de oráculos de R97-11).
- 2026-11-17 02:5x UTC · Oráculo de R97-13 (516) aceptado: `b5117b4b` (alcance: solo `causal_softmax_attention`, texto
  `text.cpp:175` + MTP `mtp.cpp:210`; cachés bf16 y fp8; contraste 9 casos / 0, logits ±600). Las cabezas 24/4 del 27B
  no salen de un config del repo (se confirman con los pesos en R97-17). Decisión del director: los planes densos
  `bf16`/`fp8` incluyen `ops/kv_cache/kernels/write.h`, así que **R97-13 porta los kernels de escritura de KV** y R97-14
  (append, materialize, Hadamard) los consume y valida. Autor lanzado (Codex `gpt-6-astra` xhigh, 518; ficha READY).
- 2026-11-17 03:0x UTC · Oráculos aceptados: R97-12 (515, `96d31157`: trayectoria prefill→decode de GDN y cadenas de
  conv, contraste 0) y R97-14 (517, `d6fe1304`: fronteras E4M3, 65 536 patrones BF16, Hadamard exacta, contraste 0).
  Decisiones: `gdn_input_proj_conv_*` y `gdn_control` entran en R97-12 (el modelo no corre sin ellos);
  `cudaMemcpyDefault`/`cudaMemcpy2D` los traduce R97-13 (lo necesitan R97-12/13/14); R97-14 consume los kernels de
  escritura de R97-13 y deja fuera `context_materialize` (solo drafter); NaN de V BF16 = cualquier NaN FP16; no finitos
  FP8 fuera de contrato. Autores lanzados en Codex `gpt-6-astra` xhigh: R97-12 (519) y R97-14 (520), fichas READY.
  Cola de compilación: R97-08 con el candado del checkout 46 min y R97-10 esperando 45 min; las fichas nuevas obligan a
  compilar solo los objetivos propios. En curso: 6 autores Codex.
- 2026-11-17 03:1x UTC · Usuario: «En todos los textos del repo que dice: RTX 5090 and DGX Spark, ahora añade la R9700
  de amd, tambien en los ejemplos pon el preset de la r9700 [...] para ir preparando si nos pueden aceptar un futuro
  PR». El director editó README (cabecera, Build, `r9700-release`, `r9700-dev`), `bench/README.md` y
  `tests/README.md` (`8a792933`). Usuario: «escribelo, asumiendo que el sprint esta implementado, forma parte del
  sprint» → nueva tarea **R97-28** en PLAN_INTEGRATION (docs de producto, TOML de ejemplo, model card; sin cifras
  inventadas). Lanzada en Claude Sonnet 5.5 high (521; ficha READY). Su verificación final va tras R97-18.
- 2026-11-17 03:12 UTC · **Relevo 3 del director** aceptado (sesión `8dd8dd4a`, `ninfer-e1`, Fable 5.2 high, contexto
  inicial 25,3 %). Adoptados sin relanzar: los 6 autores Codex (R97-08/10/11/12/13/14, pids vivos) y el vigía
  (pid 37076, `ws://127.0.0.1:8791/`). `deps.py`: READY. Cuota: Claude 27 %, Fable 37 % (con el 1,9 % de la ventana),
  Codex 10 %. R97-28 terminó (exit 0, 140 s, `68f00ba3`): pendiente el contraste del director. Cola de compilación:
  R97-10 tiene el candado del checkout 1 h 05 min; esperan R97-11, 13, 14, 08 y 12. INBOX sin entradas nuevas.
