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
- 2026-11-17 03:2x UTC · R97-28 (521) contrastada por el director contra el alcance: 14 ficheros, ninguna afirmación
  de Vision, DFlash, NVFP4, offload ni multi-GPU en la R9700, sin cifras visibles. Un hallazgo corregido por el
  director: `README.md:21` decía «All rows support text, Vision…» y contradecía la fila nueva. Queda **parcial** hasta
  la verificación final tras R97-18 (lista en PLAN_INTEGRATION §5).
- 2026-11-17 03:3x UTC · R97-10: el autor pidió permiso para tocar `tests/ops/linear/projection/cases_bf16.cpp`
  (fuera de sus rutas): el ejecutable cae con ACCESS_VIOLATION incluso con `--list`. Causa: defecto latente del
  upstream (`91972187`), `Geometry::boundaries` es un `std::initializer_list<int>` que queda colgando al copiarse a
  `std::array` (GCC lo tolera, el `clang++` de ROCm no). **Decisión del director:** autorizado y aplicado por el
  director con una línea (`std::vector<int> boundaries`), que cubre los dos sitios con el defecto; no cambia ningún
  caso ni tolerancia. Sin compilar por el director: lo valida la compilación de R97-10 y lo comprueba su revisión; va
  en el commit de aceptación de R97-10. Nota al autor en `.agent-tmp/R97-10/DIRECTOR-NOTE.md`.
  Observación de cola: R97-10 retuvo el candado 1 h 05 min con un bucle de diagnóstico dentro de `build_queue run`;
  las fichas siguientes lo prohíben (una entrada = una compilación o una tanda de tests).
- 2026-11-17 03:4x UTC · Oráculo de R97-15 lanzado (522, Claude Sonnet 5.5 high; ficha READY): audita
  `tests/ops/sampling` y añade casos de riesgo `r9700`; no compila ni usa la GPU.
- 2026-11-17 03:5x UTC · **R97-11 intento 1 (514): BLOCKED**, 0 suites en GPU. Commits `c666530a` (registro de
  oráculos) y `21cf7c6d`. Causa: tres cabeceras de `src/ops/common` que R97-09 no cubrió y que no tenían dueño:
  `numeric/math.cuh:16,24,31,39` (instrucciones PTX `.approx`), `numeric/convert.cuh:93` y `quant/fp8.cuh:44-45`
  (conversiones BF16/FP16 ambiguas en HIP). R97-14 preguntó por las mismas; R97-13 ya las había arreglado en el árbol.
  **Decisión del director:** `convert.cuh` y `fp8.cuh` → dueña R97-13 (su arreglo: FP32 exacto y un solo redondeo
  RNE, equivalente bit a bit a la conversión directa); `math.cuh` → R97-11 intento 2 (rama `r9700` con `tanhf`,
  `exp2f`, `1/x` y vaciado a cero explícito en las variantes `_ftz`; sonda de aceptación con cotas fijadas por el
  director). Notas a los autores en `.agent-tmp/R97-{12,13,14}/DIRECTOR-NOTE.md` (R97-10 leyó la suya). Lanzado
  R97-11 intento 2 (523, Codex `gpt-6.1-sol` high; pre-revisión: una aclaración sobre `sigmoid_approx`, incorporada).
- 2026-11-17 04:0x UTC · Oráculo de R97-15 (522) aceptado: `e4368ed2` (`tests/ops/sampling/r9700_edges.cpp`, sin
  registrar ni ejecutar; sintaxis HIP limpia). Alcance: `ops::sample` (`decision.cpp:101,177`),
  `speculative_accept_greedy_drafts` (`decision.cpp:93`) e `increment_token_counts[_batch]` (`commit.cpp:73,95,135`).
  Fuera (decisión del director): aceptación dispersa (DFlash2), `candidate_selector_path`, `linear_topk`,
  `accept_vocabulary_*`; sus tests se excluyen de `r9700` sin tocar su contenido. Contraste en Python: 0
  discrepancias en 7 comprobaciones (136 de 4 000 casos aleatorios omitidos por caer a menos de 1e-9 de una frontera
  de top-p, donde la suma depende del orden). Selecciones, empates y aceptación exactos; NaN fuera de contrato; `-inf`
  legal si queda algún token finito. Aviso: `acceptance/greedy_drafts/kernel.cuh:95` usa `asm volatile("trap;")`.
  Autor lanzado ya (524, Codex `gpt-6.1-sol` high; ficha READY): sus rutas son disjuntas de las seis en curso, así que
  no espera a R97-10..14 (cupo dinámico); solo comparte la cola de compilación y la GPU. Cuota a las 04:0x UTC:
  Claude 30 %, Fable 40 % (2,1 % de la ventana), Codex 15 %.
- 2026-11-17 04:4x UTC · **Q-008 respondida** (usuario, en la sesión saliente del relevo 2, relevada por `ninfer-0b`;
  registro `637de1f9`): «Sí, exclusivas (Recomendado)». Las dos R9700 serán exclusivas de ninfer durante R97-22 y
  desde R97-24 (SPRINT-02). Aplicado en `next/SPRINT.md` §4 y `next/PLAN_TP.md` §5. No cambia SPRINT-01: la Sapphire
  (HIP 1) sigue prohibida para los hijos hasta entonces. Sin preguntas abiertas.
- 2026-11-17 04:1x UTC · **R97-08 intento 1 (511): BLOCKED**, `b8fd7fe2`. Árbol vcpkg en
  `G:\vcpkg_root\installed-ninfer-r9700` (420 ficheros verificados; el de `.agent-tmp/R97-03/` se conserva hasta
  aceptar); nvtx con marcadores vacíos en `r9700`; 67 tests host registrados, **0 ejecutados**, 135 excluidos. Pidió
  dos ficheros fuera de sus rutas. **Decisión del director:** autorizados `src/text/CMakeLists.txt`
  (`UTF8PROC_STATIC`, con `if(WIN32)` y no con la diana) y `third_party/llama-jinja/jinja/value.cpp`
  (`localtime_s` bajo `_WIN32`). Intento 2 lanzado (525, Codex `gpt-6.1-sol` high; ficha READY): ejecutar
  `ctest -L host` y agrupar los 135 excluidos por causa.
- 2026-11-17 04:1x UTC · **Cola de compilación, medida:** 10 entradas de ninfer en espera. Las 2 ranuras de la máquina
  las tenían `quantdesk-network` (`S02-cierre`, `cargo test` del workspace desde las 03:22 UTC, límite 240 min) y
  `radiance-win` (`W1-probe`, límite 40 min); R97-14 retenía el candado de ninfer 49 min esperando ranura (la cola da
  primero el candado del checkout y luego la ranura). Entradas recientes de ninfer: 17-28 min de espera para fallar en
  2-30 s por errores de compilación. Mitigación del director (nota en `.agent-tmp/R97-1{0..5}/DIRECTOR-NOTE.md` y en
  las fichas nuevas): pasada de sintaxis `-fsyntax-only` fuera de la cola antes de pedir turno (un proceso a la vez),
  una sola entrada por autor y compilar + ejecutar en la misma entrada. La capacidad (2 ranuras) es del usuario.
- 2026-11-17 04:5x UTC · **R97-12 intento 1 (519): BLOCKED**, `2d40029e`, sin compilación completa ni GPU. Usó la
  pasada de sintaxis fuera de la cola (11 unidades pasan; recurrente y replay fallan) y canceló su entrada antes de
  arrancar. Causa: `src/ops/common/numeric/math.h:31`, `NINFER_KERNEL_HD` solo vale `__host__ __device__` con
  `__CUDACC__`, así que `FastDivisorU31::divide` es de host bajo HIP. **Decisión del director:** `math.h` autorizado
  a R97-12 solo para la anotación (división exacta también en el dispositivo `r9700`; rama CUDA intacta). Intento 2
  lanzado (526, Codex `gpt-6-astra` xhigh; ficha READY), con sonda de `FastDivisorU31` contra el host.
  R97-08 intento 2 avanza: `e9f25fff` (texto en Windows) y `69e094d3`.
- 2026-11-17 04:5x UTC · **R97-10 intento 1 (513): BLOCKED por la cola** (180 min agotados). Commits `563e4526`,
  `7adc091d`, `7f252ce2`, `3402c172` (este incluye el arreglo del director de `cases_bf16.cpp`). Compila 72/72; 28
  comprobaciones de rechazo Q4 en verde; **0 casos numéricos ejecutados**. Intento 2 lanzado (527, Codex
  `gpt-6-astra` xhigh): solo la aceptación numérica `--format q4|q5|q8` en una entrada; si la cola no da turno en
  150 min, deja una orden única para que la lance el director.
  Cola a las 04:57 UTC: ninfer ejecutó una entrada en 55 min (R97-14, 44 min de ejecución); una ranura sigue con el
  `cargo test` de cierre de `quantdesk-network` (1 h 35 min) y la otra rota entre proyectos. Esperas de 45-90 min por
  entrada: es previsible que más intentos agoten su tiempo sin validar.
- 2026-11-17 05:3x UTC · **R97-11 intento 2 (523): BLOCKED por la cola** (114 min sin ranura; retiró su entrada).
  `de3aa33f`: `numeric/math.cuh` portado (cuatro helpers, vaciado a cero explícito con signo). Sonda de 4 137 puntos
  y arneses de las cinco suites preparados, sin compilar ni ejecutar. Desde las 04:55 UTC ninfer no ha tenido ranura
  (`quantdesk` y `quantdesk-network`). **Cambio de método del director:** las validaciones ya preparadas se lanzan
  como entradas de cola **sin modelo** (`.agent-tmp/director/gate.py` por `lanza.py` + `build_queue.py`), que esperan
  turno sin gastar cuota ni agotar el tiempo de un intento; el autor solo vuelve si algo falla. Lanzada la de R97-11
  (`R97-11-gate`: `run-v2.py build` y luego `gpu`, límite 120 min de ejecución; log `.agent-tmp/R97-11/gate.log`).
  Registrada **Q-011** (reparto de la cola entre proyectos; recomendación: seguir por orden de llegada).
  Avances de otros autores: `a35c3613` (R97-15, sampling), `bdbec359` (R97-13, atención densa).
- 2026-11-17 05:5x UTC · **R97-15 intento 1 (524): primer resultado en GPU del grupo de Ops.** `a35c3613`; compila
  44/44; en la ASUS (HIP 0, bus 6, wave32) pasan **76 de 79 casos**, 0 omitidos; segunda pasada sin ejecutar. Los 3
  fallos son `r9700/accept_greedy/penalised_mixed` (K/B/V 3/5/65, 5/33/257, 1/7/33), fila 0. Causa, comprobada por el
  director en `include/ninfer/ops/speculative_accept.h:37-38`: el test del oráculo pasaba `-1` como target a todas
  las filas del lote mixto, también a las greedy sin penalizaciones, que por contrato consumen el argmax exacto.
  Defecto del oráculo, no del port; el autor no tocó el test. **Corrección del director** (`b86077c3`, 1 línea
  efectiva en `tests/ops/sampling/r9700_edges.cpp`): target exacto para las filas greedy sin penalizaciones y `-1`
  para el resto; esperados, criterios y casos sin cambios; sintaxis HIP comprobada. Validación lanzada sin modelo
  (`R97-15-gate`: compilar y dos pasadas seguidas; log `.agent-tmp/R97-15/gate.log`). Excluidos de `r9700`:
  `accept_sparse`, `candidate_selector`, `gathered_selector`, `vocabulary*`. Único arreglo de producción fuera de
  kernels: incluir `cuda_runtime.h` antes de `random.cuh` en `distribution.cuh`.
- 2026-11-17 05:5x UTC · **R97-13 intento 1 (518): BLOCKED por plazo, casi completa.** Commits `f061bd98` (kernels de
  escritura KV y `convert.cuh`/`fp8.cuh`), `bf4d695c` (copia paginada y grafos en `cuda_compat.h`), `bdbec359`
  (atención densa causal BF16 y FP8). En la ASUS con el ejecutable oficial: **FP8 198/198**; **BF16 136/180**
  (cortado por el plazo; con el arnés de diagnóstico 180/180, L2 relativo máximo 0,0017 con cota 0,0028); 9/9
  rechazos explícitos de INT8/NVFP4/K8V4; conversiones exhaustivas en verde. Continuación lanzada sin modelo
  (`R97-13-gate`: `continue.py` = compilar, BF16 oficial y rechazos; log `.agent-tmp/R97-13/gate.log`).
  **Dos observaciones de máquina** (para la revisión y para el usuario): (1) evento de Windows Display 4101 a las
  03:55:27 UTC, reinicio del driver de AMD durante un lote FP8 de diagnóstico (fallo de lanzamiento HIP); no se
  repitió en tres repeticiones del caso ni en la pasada oficial; causa sin resolver. (2) Durante el BF16 de
  diagnóstico la RAM libre del sistema bajó a 9 MB (proceso de test: 6,4 GB de pico). A las 05:5x UTC hay 10,9 GB
  libres de 31,9 GB.
- 2026-11-17 06:0x UTC · **R97-14 intento 1 (520): entregada completa**, la primera de PLAN_OPS. Commits `d2008b61`
  (aritmética FP32 estricta en `src/ops/kv_cache` para `r9700`) y `acadec71` (enlace de las suites KV a su biblioteca
  de producción). En la ASUS (HIP 0, bus 6, wave32): los dos ejecutables oficiales, **172 casos, 0 omitidos**;
  Hadamard 272 filas, 69 632 valores exactos, 0 discrepancias. Tests: solo registro del oráculo y el arreglo de
  enlace. Revisión lanzada (528, Claude Opus 5.5 high; `R97-14/review-card.md`): incluye los kernels de escritura y
  las conversiones de `f061bd98` (R97-13) en lo que toca a KV; sin recompilar salvo duda concreta (cola saturada).
- 2026-11-17 06:0x UTC · **Relevo 4 del director** escrito por contexto (68,6 %; sesión saliente `8dd8dd4a`,
  `ninfer-e1`). En vuelo, todo adoptable: revisión de R97-14 (528), autores intento 2 de R97-08 (525), R97-10 (527)
  y R97-12 (526), y gates sin modelo de R97-11, R97-13 y R97-15. Cuota a las 06:01 UTC: Claude 43 %, Fable 58 %,
  Codex 48 % (con el 2,3-3,6 % de la semana). Abierta: Q-011.
- 2026-11-17 06:1x UTC · **Relevo 4 aceptado** (sesión `c988ac27`, `ninfer-48`, Fable 5.2 high, contexto inicial
  25,5 %). `deps.py`: 0 dependencias, READY. Adoptados vivos los autores de R97-08/10/12 y los gates de
  R97-11/13/15; vigía `ws://127.0.0.1:8791/` rearmado. INBOX sin entradas nuevas. Cuota: Fable 59 %, Codex 48 %.
  Cola de compilación: las 6 entradas de ninfer esperan (la primera, `R97-08-attempt2-build`, 1 h 52 min); las dos
  ranuras siguen en `quantdesk` y `quantdesk-network` (Q-011 abierta).
- 2026-11-17 06:1x UTC · **R97-14 cerrada** (`d2008b61`, `acadec71`). Revisión 528 (Claude Opus 5.5 high): ACCEPT.
  Contraste del director con el código: (1) `tests/` solo cambia `tests/ops/kv_cache/tests.cmake` (registro del
  oráculo y enlace a `ninfer_kv_cache_r9700`), confirmado con `git show --stat`; (2) hallazgo bajo cierto: el
  rechazo de INT8/NVFP4/K8V4 en `src/ops/kv_cache/kernels/write.cu:155-159` no lo ejercita ningún caso de la suite
  de KV; el fichero es de R97-13, así que va a su ficha de revisión; (3) hallazgo bajo cierto:
  `context_materialize` sigue en `ninfer_ops` sin flags estrictos (`src/ops/kv_cache/sources.cmake:11`), fuera de
  alcance hasta que se porte el drafter. Condición anotada: si R97-13 cambia los kernels de escritura o las
  conversiones tras su revisión, se repiten las dos suites de KV. Fila en PLAN_OPS §5.
- 2026-11-17 06:5x UTC · **R97-08 intento 2 (525): BLOCKED por plazo y cola.** Commits `e9f25fff`, `69e094d3`,
  `034bc7f9` (parches de texto/Jinja autorizados y test de `strftime_now`). Tras ~130 min de cola compiló 102 de 192
  pasos sin errores y se le acabó el plazo; CTest no llegó a arrancar (203 registrados, 73 host, 0 ejecutados).
  Excluidos, ahora 130 por causa: 116 necesitan GPU, 9 por enlace transitivo con Ops, 3 por arnés ELF/Windows, 2 por
  otras dependencias host (`.agent-tmp/R97-08/excluded.md`). Continuación lanzada sin modelo (`R97-08-gate`:
  `attempt2.py build` y `attempt2.py ctest` en una sola entrada; log `.agent-tmp/R97-08/gate.log`). Fichas de
  revisión de R97-11, R97-13 y R97-15 escritas y pre-revisadas (Gemini Flash: READY las tres), a la espera de sus
  gates. Cola a las 06:5x UTC: ninfer tiene una ranura (R97-12 intento 2 en ejecución); detrás, R97-10, los gates
  de R97-11, R97-15 y R97-13, y ahora el de R97-08.
