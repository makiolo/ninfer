# LAUNCH — SPRINT-01 · Port de NInfer a AMD R9700

## Instrucciones de uso

Una sola sesión externa: el director. Se abre **ya**, con
`concertino-launch.cmd --model claude-opus-5-5 --effort high` en la raíz del repo (`S:\Projects\ninfer`): Fable está
al 100 % hasta las 00:00 UTC del 17-11, y el director orquesta con el ~4 % que le queda a Claude hasta su reinicio
(16-11 23:59 UTC). El valor por defecto del lanzador es Opus 5.5 high; tras el reinicio de Fable, el siguiente
relevo sale en **Fable 5.2 high** con `concertino-launch.cmd --model claude-fable-5-2 --effort high`. Relevos y
recuperación: el mismo `concertino-launch.cmd` o `concertino-recover.cmd`. Antes de que el director use la GPU (R97-04), descarga el modelo
de LM Studio de la ASUS (paso de usuario, ver «Guardia de activación»).

## Guardia de activación

- Sprint: [SPRINT.md](SPRINT.md). Dependencias externas: ninguna (`deps.py --repo . --sprint SPRINT-01`, 2026-11-16).
- Rama: `fork_R9700`. Sin push (`origin` es el upstream ajeno, Q-005).
- **GPU dedicada: ASUS, índice HIP 0 (bus PCI 6, subsistema 1043)**, por `HIP_VISIBLE_DEVICES=0` en toda orden con
  GPU (Q-009). La Sapphire (HIP 1) sigue siendo de la IA local: **ningún hijo la usa**. Antes de la primera tarea con
  GPU, el director comprueba que la ASUS está libre de LM Studio (memoria libre con `hipInfo.exe` o `rocm-smi` si
  existe) y que ningún `claude-local` tiene la ranura `asus-*`. Si sigue ocupada, pregunta al usuario; no mata
  procesos ajenos.
- Al arrancar, el director vuelve a ejecutar `deps.py --repo . --sprint SPRINT-01` y para si algo cambió.

## Modelos, roles y cupo

- Esquema de niveles: L1–L10 de la skill `concertino` (`references/levels-and-models.md`).
- Director: arranca en Opus 5.5 high (Fable al 100 %); vuelve a Fable 5.2 high en el primer relevo tras las 00:00 UTC
  del 17-11. HANDOFF en `docs/plans/current/HANDOFF.md`. Regla del perfil: Fable > 95 % → relevo en
  `claude-opus-5-5 high`.
- Suscripciones, por orden de la ruta: Claude, Codex y Antigravity según `claude-or route --level N --risk <r>`
  **antes de cada lanzamiento**. Lectura del 16-11 21:27 UTC: Claude semanal 96 % (reinicio 16-11 23:59 UTC), Fable
  100 % (reinicio 17-11 00:00 UTC), Codex 94 % con el 34 % de su semana (reinicio 21-11 11:38 UTC), Argon 91,8 %,
  Flash 39 %.
- **Orden temporal de proveedores del usuario (2026-11-16, Q-010): Claude orquesta, Codex implementa.** Rige hasta
  que el usuario lo retire; mientras, la ruta no cambia de autor a Claude aunque marque Claude.
  - Autores, todos en Codex: L3–L6 `gpt-6.1-sol` high; L7 (R97-10, R97-12, R97-13, R97-14, numérico-silencioso)
    `gpt-6-astra` xhigh.
  - Revisores en Claude, que es otro proveedor: L3–L6 Sonnet 5.5 high; L7 Opus 5.5 high. Hasta el reinicio de Claude
    (16-11 23:59 UTC) el director no lanza revisiones que no quepan en el ~4 % restante; las deja en cola.
  - **Oráculo distinto del autor** para las tareas numéricas (R97-02, R97-09..15, R97-18): un modelo de Claude
    prepara los casos de referencia antes de lanzar al autor de Codex.
  - Si Codex se agota (94 % usado el 16-11), el director para la implementación, envía un PushNotification y
    pregunta al usuario si reinicia la cuota de Codex (eso solo lo hace el usuario). No pasa los autores a Claude sin
    respuesta.
- Clasificación de las tareas: L3 R97-03, R97-04; L4 R97-06..08; L5 R97-01, R97-05, R97-17; L6 R97-02, R97-09,
  R97-11, R97-15, R97-16, R97-18; L7 R97-10, R97-12, R97-13, R97-14.
- Sin Haiku 5.5 como autor ni revisor. Sin IA local en este sprint (su GPU ASUS está dedicada a ninfer).
- API de pago y modelos gratuitos de OpenRouter: no autorizados. Un error de API o facturación para el plan.
- Cupo **dinámico** (usuario, 2026-11-17: «Haz que este proyecto pueda lanzar mas subagentes que 3, se decide
  dinamicamente»): sin tope fijo de descendientes. Antes de cada lanzamiento el director decide con: tareas listas con
  rutas disjuntas de las que están en curso; cuota del proveedor (`claude-or quota --weekly`); la cola de compilación
  (`build_queue.py status`: un candado por checkout y 2 ranuras globales, así que los autores que compilan en este
  checkout van por turnos y añadir más solo alarga la espera; oráculos y revisiones de lectura no compilan); y la
  GPU. **Solo uno usa la GPU a la vez** (sigue igual).
- Arnés: ficha completa (`references/card.md`) revisada por un modelo barato antes de lanzar; revisor con contexto
  limpio que recibe ficha, diff y log de tests; el director contrasta cada hallazgo con el código.

## Límites

- Prohibido a cualquier hijo: usar la GPU Sapphire (HIP 1); cambiar `CMakeLists.txt`/`cmake/` fuera de su tarea;
  tocar `AGENTS.md` o docs estables de `docs/` sin petición del usuario; modificar los presets o la lógica CUDA más
  allá de lo que la diana `r9700` necesita; descargar pesos (solo R97-17); `git add -A`; push; borrar `out/`.
- Ficheros temporales en `.agent-tmp/<tarea>/` (AGENTS.md); `out/` solo para el artefacto.
- Compilaciones y tests pesados por la cola de compilación de la skill (`tools/build_queue.py`), con la mitad de los
  núcleos como máximo (la CPU es un i7-4790K de 4 núcleos y 8 hilos).
- `git add` por ruta; Conventional Commit en minúscula con `Refs: R97-NN`; commit en `fork_R9700` al aceptar cada
  tarea; sin push.
- Intérprete de las herramientas de la skill: `D:\miniconda312\python.exe`.

## Presupuesto e hitos

Escenario hipotético, sin muestras comparables en este proyecto ([USAGE.md](USAGE.md)): es un port grande
(~126k líneas de CUDA), con 7 tareas L6 y 4 L7. Se espera que ocupe **varias semanas de cuota**, no una. Sin fecha
límite. Hitos:
1. `r9700-ops` configura y el smoke de GPU pasa (R97-01..04).
2. Tests de host en verde en Windows (R97-05..08).
3. Suites de cada Op en verde en `r9700` (R97-09..15).
4. Qwen3.8-27B genera, con MTP y KV FP8, y la perplejidad está dentro de la tolerancia (R97-16..18).

Si la cuota se agota, se espera al reinicio; no se relaja ninguna revisión ni se cambia de proveedor sin la ruta.

## Tablero

Por cada intento de ejecución o de revisión: `dashboard.py attempt start …` al lanzarlo y `attempt end N --result …`
al terminar; `dashboard.py task … --status …` cuando una tarea cambia de estado, y `update` tras cada cierre.

## Director prompt

```text
Eres el director del SPRINT-01 de ninfer (S:\Projects\ninfer, rama fork_R9700). Sigue la skill global `concertino`
(lee primero docs/plans/CONCERTINO.md y el perfil ~/.concertino/profile.md) y AGENTS.md del repo, y ejecuta los
planes con `concertino-run`: docs/plans/current/PLAN_BUILD.md, PLAN_HOST.md, PLAN_OPS.md y PLAN_INTEGRATION.md, en el
orden BUILD → (HOST ∥ OPS) → INTEGRATION que fija docs/plans/current/SPRINT.md §7.

Objetivo: que Qwen3.8-27B (groupwise-int) genere texto con MTP y caché KV FP8 en una AMD R9700 en Windows nativo
(HIP SDK 7.2 en S:\Apps\AMD\ROCm), con cada Op validada contra su referencia independiente y la perplejidad
comprobada. La build CUDA del upstream no se rompe.

1. Al arrancar: anota tu nombre de ListAgents en ~/.concertino/directors.json y en ~/.claude/relevos.json (clave
   ninfer); ejecuta deps.py --repo . --sprint SPRINT-01; toma la lectura de cuota (claude-or quota --weekly) y
   anótala en docs/plans/usage/observations.csv; lee docs/plans/current/QUESTIONS.md e INBOX.md.
2. GPU: solo la ASUS, HIP 0, con HIP_VISIBLE_DEVICES=0. Antes de la primera tarea con GPU comprueba que LM Studio
   la ha liberado; si no, envía un PushNotification y pregunta al usuario (regístralo en QUESTIONS.md). Nunca la
   Sapphire (HIP 1).
3. R97-01 primero. Si CMake 3.28.0-rc5 no soporta el lenguaje HIP en Windows, la tarea vuelve BLOCKED con la
   versión mínima; pregunta al usuario si actualizar CMake antes de seguir.
4. Orden del usuario: tú (Claude) orquestas y revisas; **Codex implementa** (gpt-6.1-sol high en L3–L6, gpt-6-astra
   xhigh en L7), lanzado con tools/lanza/ y eventos JSON a fichero. Las tareas numéricas llevan antes un oráculo de
   Claude. Si Codex se agota, para y pregunta al usuario si reinicia su cuota. Hasta las 23:59 UTC del 16-11 Claude
   solo tiene ~4 %: úsalo para orquestar y deja en cola las revisiones que no quepan.
5. R97-17 descarga out/qwen3_8_27b.ninfer (20,437,521,664 bytes) de neroued/Qwen3.8-27B-NInfer; comprueba el tamaño
   exacto y no lo vuelvas a descargar si ya está.
6. Respeta los límites de docs/plans/current/LAUNCH.md. Commit por tarea aceptada en fork_R9700, sin push. Al cerrar
   todas las tareas, ejecuta `concertino close` por tu cuenta y luego pregunta al usuario por la revisión de planes
   para SPRINT-02.
```
