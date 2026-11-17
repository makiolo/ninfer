# Director HANDOFF — handoff 6

## Handoff
- Number: 6
- Outgoing: session 35a27144-5ef2-4a29-a4b1-e9532a91ff63, claude-fable-5-2, context 72.4 %
- Written: 2026-11-17T09:49:56+01:00
- HEAD at writing: 06bb63b0478b84c19e693945ecfb56e35aeedf19
- LAUNCH: docs/plans/current/LAUNCH.md · coordinator block: «Director prompt» (sección final de LAUNCH.md)
- Previous handoffs: `git log --follow -- docs/plans/current/HANDOFF.md`
- Accepted: session 698c3d33-9622-49c7-a8af-0ed1c7c65a05, 2026-11-17T09:52:29+01:00, initial context 25.0 %

## Boot
0. If the user's private profile exists (config `profile`, default ~/.concertino/profile.md), read it first; its rules
   never override the concertino skill's safety rules or the user's instructions in the session; report any conflict.
(python = the interpreter of ~/.concertino/config.json "python"; ~ = the user's home, $HOME in PowerShell;
paths without a root are relative to this repo's root)
1. `python -X utf8 S:/Projects/concertino/skills/concertino/tools/claude_or.py handoff accept docs/plans/current/HANDOFF.md`: validates this file and records your session.
2. Read only this: this file, the coordinator block of the LAUNCH, `docs/plans/CONCERTINO.md`, the `concertino` skill
   (`S:/Projects/concertino/skills/concertino/SKILL.md`, `references/launch.md`, `references/handoff.md`) and the cards of the children in flight.
   Everything else when needed, delegating.
3. `git status`, the state of the gate or locks of CONCERTINO.md "Commands" and `git log --oneline 06bb63b0478b..HEAD`.
4. Adopt the children in flight: check their process and output and watch it; never relaunch a live ID.
5. One watcher per director, no shell loops: start `S:/Projects/concertino/skills/concertino/tools/vigia/wsvigia.py` detached with
   `S:/Projects/concertino/skills/concertino/tools/lanza/lanza.py` and subscribe to it (in Claude Code, `Monitor` on its `ws://` URL; its 30-minute
   expiry doubles as the cache keep-alive, `references/handoff.md`). Agents without a monitor check it at each turn.
6. Search the project memory (CONCERTINO.md "Identity") for the keywords of the next step.
7. Questions to the user (`S:/Projects/concertino/skills/concertino/references/questions.md`): read `docs/plans/current/QUESTIONS.md`; never ask again
   what is `answered`, and ask production gates yourself when they become executable. If it does not exist, create it
   from `S:/Projects/concertino/skills/concertino/templates/docs/plans/QUESTIONS.md` with the open questions of this HANDOFF (and the live ones of
   `docs/plans/QUESTIONS.md`, if any), checked first against EXECUTION and `git log --since=<HANDOFF date>`.
8. Inbox: read `docs/plans/INBOX.md` (`S:/Projects/concertino/skills/concertino/references/cross-project.md`); apply each new entry in this project,
   record it, answer if expected and mark it `→ done | accepted | rejected`.
9. Commit the HANDOFF with the acceptance and continue with "Next step".

## State
- Cerradas en total: R97-01..07, R97-09, R97-10, R97-13, R97-14 y, en este tramo, **R97-15** (`a35c3613`),
  **R97-11** (`21cf7c6d`), **R97-30** (`acb46111`) y **R97-29** (`505fb8ab`, `3b3f26cd`); filas en PLAN_OPS §5.
  R97-28 parcial (verificación final tras R97-18). Detalle de todo en `docs/plans/current/EXECUTION.md`.
- **R97-08** (tests host): revisión 547 ACCEPT WITH CONDITIONS; 67 de 67 host en verde tras la corrección del
  director `e1f750de` (`alignas` en `RecoveryCostSum`). Falta una línea (`static_assert`), repetir el gate y cerrar
  (ver «Next step»). Las otras dos condiciones ya están decididas (EXECUTION): `gmtime_r` de `ninfer-perplexity`
  añadido a R97-18 en PLAN_INTEGRATION; inyección de fallos COFF → ISSUE-006 al backlog (**sin crear todavía**).
- **R97-12** (Gated DeltaNet): port `2d40029e`, `e9cc0212`, `afe640fb`, `1699f365` (este último crea
  `ninfer_ops_host_r9700`, que también usa R97-29); oráculo `96d31157` y su corrección de cotas `b279a0c5`. Las
  nueve suites se ejecutaron: 1 732 casos, 8 fallos por cota inalcanzable, corregida por el oráculo. Gate sin modelo
  en ejecución con la corrección (las seis primeras suites ya en verde, incluida `causal_conv_trajectory` 53/53).
- Pendientes sin empezar: **R97-31** (`ninfer_ops` completo y suites oficiales; depende de R97-12; entradas de los
  cierres en PLAN_OPS §3 y §5), INTEGRATION R97-16, R97-17, R97-18, verificación final de R97-28.
- **Pesos ya descargados** por orden del usuario: `out/qwen3_8_27b.ninfer`, 20 437 521 664 bytes, SHA-256
  comprobado (`81f924d4…0375da`); R97-17 no descarga nada.
- Issues: ISSUE-001 (nvtx3), ISSUE-002 (borrar `.agent-tmp/R97-03/`, 1,3 GiB, tras cerrar R97-08; mirar antes qué
  hay; ya copiado a `G:\vcpkg_root\installed-ninfer-r9700`), ISSUE-003 (backlog), ISSUE-004 (TDR con más de 128
  consultas; lo mide R97-16), ISSUE-005 (suite completa de `ctest`; R97-31).
- Cola de compilación: ahora fluida (turnos de segundos a pocos minutos). Un candado por checkout.
- Cuota a las 08:50 UTC: Claude 60 %, Fable 83 %, Codex 64 % (5,2 % de la semana). Q-012 respondida (B): desde que
  Claude semanal pase del 85 %, revisiones y oráculos van a Codex con un modelo distinto del autor (regla en
  LAUNCH «Modelos, roles y cupo»). **Este relevo sale en `claude-opus-5-5 high`** porque Fable se acaba (regla del
  perfil; interpretación del director saliente: con Fable al 83 % y subiendo unos 10 puntos por hora, una sesión
  nueva en Fable se quedaría sin cuota a mitad de tramo y no puede cambiar de modelo).

## In flight
| Child | ID | Model | Process or task | Output | Task card | Pending check |
|---|---|---|---|---|---|---|
| r97-12-gate | R97-12 | sin modelo (cola) | lanza pid 23852, etiqueta `R97-12-gate` | `.agent-tmp/R97-12/gate.log`; `R97-12/lz-gate.fin` | `R97-12/card.md`, `card-fix.md`, `card-fix-3.md`, `card-fix-4.md`, `oracle-card-2.md` | exit 0 (nueve suites, 1 732 casos, 0 omitidos) → escribir `R97-12/review-card.md` (modelo: `R97-29/review-card.md`; **Opus 5.5 high** si Claude < 85 %, si no `gpt-6.1-sol` high por Q-012), pre-revisarla y lanzarla; exit ≠ 0 → mirar qué suite falla: cota → oráculo; port → intento 5 del autor (`gpt-6-astra` xhigh) |

Puntos obligados de la ficha de revisión de R97-12: (a) `b279a0c5`: las cotas por llamada de
`causal_conv_trajectory.cpp` se ajustaron a las entradas que genera la STL de MSVC (trunca el aleatorio a 24 bits);
en Linux/CUDA las entradas son otras y las cotas pueden no valer: ¿debe el test calcular la cota desde su propia
salida BF16 ideal? (b) `1699f365`: `ninfer_ops_host_r9700` (`src/ops/CMakeLists.txt`, `src/ops/common/sources.cmake`),
efecto nulo en CUDA; (c) `numeric/math.h` `e9cc0212`, biblioteca propia `ninfer_gdn_r9700`, opciones
`-fno-fast-math -ffp-contract=off -fno-gpu-flush-denormals-to-zero`; (d) `constexpr`→`const` en tres líneas de dos
tests; (e) ninguna suite de `r9700` ejercita `exp2_approx` (`prepare.cu`, ruta por trozos con MMA, queda fuera; se usa
siempre la recurrencia con `expf`): ¿está `gdn_norm_control_proj` (`src/models/qwen3_5/execution/gdn.cpp:35`) y todo
lo que llama el camino de texto dentro de lo portado y probado?; (f) informes `R97-12/author4-output.md`,
`.agent-tmp/R97-12/attempt4-diagnostics.md` y `.agent-tmp/R97-12-oracle/criterion-feasibility.md`.

Rutas relativas a `.git/concertino/sprint-01/`. Vigía: wsvigia pid 37076 en `ws://127.0.0.1:8791/`, lista
`.git/concertino/vigia/fins.txt` (hoy: `R97-08/lz-review.fin`, ya terminado, y `R97-12/lz-gate.fin`; al lanzar algo,
sustituir o añadir su `.fin`). Lanzamiento de hijos: `lanza.py` + `hijo.py` (órdenes exactas en cualquier
`lz-*-events.jsonl`: Claude con `--claude -- claude -p --model ...`, Codex con `-- codex exec -m ... -o <salida>`).
Gates sin modelo: `lanza.py` + `build_queue.py run` + `.agent-tmp/director/gate.py <log> <orden> [-- <orden>]`.
Intentos: `scripts/dashboard.py attempt start|end`. Cierre de una tarea: fila en PLAN_OPS §5 (o PLAN_HOST §5),
entrada en EXECUTION, `STATUS.md`, `attempt end`, `task --status closed`, `update`, commit y `git push fork
fork_R9700`. Pre-revisión de fichas: `tools/claude_agy.py run --model gemini-3.8-flash-high --mode plan
--prompt-file <tres primeras líneas de R97-14/prereview-prompt.md + ficha>`.
Trampas: texto Markdown solo con Write/Edit; órdenes de shell solo en ASCII y cortas; `cd` persiste entre órdenes de
Bash; no hacer commit de código mientras corre un gate que toma huella de las fuentes; para retirar una entrada de
la cola, terminar solo su proceso hoja con `arbol.py <pid> --kill` (la cola libera el candado sin residuos); el
contador de intentos del tablero es de toda la cartera (los números saltan). Sin compromisos abiertos con otros
directores (la ventana quieta de quantdesk terminó; consulta de espacio en S: de reinforcement_learning respondida).

## Decisions
- Usuario (vigentes, EXECUTION y LAUNCH): GPU ASUS (HIP 0) dedicada; Codex implementa y Claude orquesta y revisa
  (Q-010); cupo dinámico; push a `fork` tras cada commit de registro, hijos sin push; Q-008; Q-011 (cola por orden
  de llegada). **En este tramo:** Q-012 respondida, opción B (`a7ca0bc2`, aplicada en LAUNCH, `c1b9bb71`); y orden en
  sesión de descargar ya el artefacto de Qwen3.8 («creo que el modelo tiene una transformacion y tienes que
  descargar el gwen 3.8 de ninfer, si no lo encuentras te lo busco»; EXECUTION).
- Director, este tramo (todas en EXECUTION):
  - R97-15, R97-11, R97-30 y R97-29 cerradas con condiciones que no piden cambiar código; lo pendiente de cada una
    está en su fila de PLAN_OPS §5 y, para R97-31, en su texto de PLAN_OPS §3.
  - Los ficheros host comunes (`src/ops/weight_input.cpp`, `src/ops/common/host/operands.cpp`) tienen biblioteca
    dueña en `r9700`: `ninfer_ops_host_r9700` (`1699f365`); las familias la enlazan.
  - Los ocho fallos de R97-12 eran cotas inalcanzables: los corrige el oráculo (`b279a0c5`), no el autor.
  - R97-29: aplicado el parche de registro del autor (`3b3f26cd`, nombre único del caso `workspace-domain` de pair).
  - R97-08: el fallo de `ninfer_resource_manager_test` era un defecto real del runtime en Windows; corregido por el
    director con `alignas` (`e1f750de`), aceptado por la revisión con la condición del `static_assert`.
  - Las Ops que solo llama el drafter (`rmsnorm_rope`, `rmsnorm_pack_tail`,
    `rmsnorm_dynamic_grouped_conv_prepare`) quedan fuera del objetivo, con rechazo explícito en R97-31.

## Open questions to the user
none

## Promoted knowledge
Memoria del proyecto (agente): `oracle-bf16-bound-feasibility` (pedir en toda ficha de oráculo con salida BF16 la
viabilidad de la cota con la salida ideal; ha fallado dos veces, R97-11 y R97-12). Sigue vigente `model-less-gates`.
Lo demás es del sprint y quedó versionado (EXECUTION, PLAN_OPS §3 y §5, PLAN_INTEGRATION §3, LAUNCH, QUESTIONS).

## Next step
1. **Cerrar R97-08.** Cuando no corra ningún gate en este checkout (`build_queue.py status`), añadir en
   `src/runtime/engine/context_cache/recovery_cost.h`, justo después del struct `RecoveryCostSum`, la línea
   `static_assert(alignof(std::optional<RecoveryCostSum>) >= alignof(unsigned __int128));` (`<optional>` ya está
   incluido), commit `fix(runtime): ...` con `Refs: R97-08`, y repetir el gate sin modelo (orden exacta en
   `R97-08/lz-gate3-events.jsonl`: `attempt2.py build` y `attempt2.py ctest`; esperado 67 de 67). Con exit 0: crear
   ISSUE-006 en `docs/plans/issues/index.md` (inyección de fallos COFF para los tres tests ELF de
   `tests/symbol_interpose.h`; plantilla `TEMPLATE_ISSUE.md`), fila de R97-08 en PLAN_HOST §5 (commits con
   `git log --oneline --grep "Refs: R97-08"`; veredicto en el evento `type=result` de `R97-08/review-events.jsonl`;
   excluidos en `.agent-tmp/R97-08/excluded.md`), `attempt end 547 --result accepted`, cierre habitual, y después
   ISSUE-002 (borrar `.agent-tmp/R97-03/` mirando antes qué hay).
2. **R97-12:** atender `R97-12/lz-gate.fin` según la tabla «In flight» (revisión con los puntos obligados).
3. **R97-31** en cuanto cierre R97-12: escribir su ficha (L5, riesgo alto; autor Codex `gpt-6.1-sol` high; leer
   PLAN_OPS §3 «R97-31» y las filas de §5, que listan lo que cada cierre le dejó), pre-revisarla y lanzarla. Después
   R97-16 (runtime) y R97-17 (carga y generación con `out/qwen3_8_27b.ninfer`): es la primera vez que el modelo
   genera, que es lo que el usuario espera («cuanto te falta para probar qwen 3.8 en alguna gpu ?»).
Antes de cada revisión u oráculo con Claude, leer la cuota (regla de Q-012).
