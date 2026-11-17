# Questions to the user — SPRINT-01

Single register of this sprint's questions to the user (skill `concertino`, `references/questions.md`). Every
question is recorded here, with a commit, before or when it is asked; whoever hears the answer writes it here in the
user's own words. Historical entries Q-001..Q-007 stay in [docs/plans/QUESTIONS.md](../QUESTIONS.md).

- States: `open` | `answered` | `expired`. Types: `decision` | `production gate`.
- Open ones: `rg "^### Q-[0-9]+ · open" docs/plans`. Next number: the highest of `rg -o "Q-[0-9]{3,}" docs/plans`
  plus one. An entry is never deleted or renumbered: its state changes.

## Entries

### Q-008 · answered · decision
- Moves: from docs/plans/QUESTIONS.md at the start of SPRINT-01 (2026-11-16); it applies to SPRINT-02 (next/PLAN_TP.md).
- Raised: 2026-11-16, sesión de planificación del port ROCm
- Task: R97-22, R97-24..27 (SPRINT-02)
- Question: El tensor parallel usa las dos R9700. ¿Se libera también la segunda GPU de su ranura de IA local mientras
  duren esas tareas?
- Options: A) Sí, las dos exclusivas durante R97-22 y desde R97-24; B) Turnos: TP solo en ventanas acordadas
- Implications: A: sin IA local en GPU durante esa fase; B: TP más lento de desarrollar y benchmarks en ventanas
- Recommendation: A
- Literal answer: "Sí, exclusivas (Recomendado)"
- Received by: sesión saliente del relevo 2 (7465607f), 2026-11-17 ~03:3x UTC, en lote al entregar el relevo 3
- Confirmation in the executing director: not applicable
- Applied in: next/SPRINT.md §4 y next/PLAN_TP.md §5 (director del relevo 3, 2026-11-17); EXECUTION.md. Pendiente al
  ejecutar SPRINT-02: avisar a los demás proyectos antes de R97-22, como con la ASUS

### Q-009 · answered · decision
- Raised: 2026-11-16 21:2x UTC, sesión que inicia SPRINT-01
- Task: general (GPU dedicada, Q-006)
- Question: ¿Qué R9700 queda solo para ninfer durante SPRINT-01? (ASUS = HIP 0, bus 6, nivel local opus; Sapphire =
  HIP 1, bus 3, niveles sonnet y haiku)
- Options: A) Sapphire (HIP 1); B) ASUS (HIP 0)
- Recommendation: A (conserva el nivel local opus)
- Literal answer: "ASUS (HIP 0)"
- Received by: sesión que inicia SPRINT-01, 2026-11-16
- Confirmation in the executing director: not applicable
- Applied in: CONCERTINO.md §11, current/SPRINT.md §7, current/LAUNCH.md. Pendiente del usuario: descargar el modelo
  de LM Studio de la ASUS antes de la primera tarea con GPU (R97-04).

### Q-010 · answered · decision
- Raised: 2026-11-16, instrucción espontánea del usuario al iniciar SPRINT-01 (no hubo pregunta)
- Task: general (proveedores del sprint)
- Question: Reparto de proveedores con Claude al 96 % (Fable 100 %) y Codex al 94 %.
- Literal answer: "usa ese 4% para orquestar y codex, podria reiniciarlo, usa codex para implementar"
- Received by: sesión que inicia SPRINT-01, 2026-11-16
- Confirmation in the executing director: not applicable
- Applied in: current/LAUNCH.md «Modelos, roles y cupo» y prompt del director: director en Claude (Opus 5.5 high
  hasta el reinicio de Fable), autores en Codex, revisores y oráculos en Claude; si Codex se agota, se pregunta al
  usuario si lo reinicia. Orden temporal: rige hasta que el usuario la retire.

### Q-011 · open · decision
- Raised: 2026-11-17 05:3x UTC, director del relevo 3
- Task: R97-08, R97-10..15 (todas compilan en este checkout)
- Question: La cola de compilación (2 ranuras para todos los proyectos) frena el sprint. Entre las 04:00 y las 05:25
  UTC ninfer ejecutó 45 minutos en total; desde las 04:55 UTC, nada: una ranura la tiene el `cargo test` de cierre de
  `quantdesk-network` (desde las 03:22 UTC, límite 240 min) y la otra `quantdesk`. Cuatro intentos de autor (R97-08,
  10, 11 y 12) han terminado sin validar por esperas de 1-2 h. ¿Cómo se reparte la máquina?
- Options: A) Seguir como está (turno por orden de llegada entre todos los proyectos); B) ninfer va primero durante
  unas horas: los demás directores esperan a que ninfer vacíe su cola de validaciones; C) subir la cola a 3 ranuras
  (solo se puede con la cola vacía; la CPU tiene 4 núcleos, cada compilación iría más lenta)
- Implications: A: SPRINT-01 avanza una entrada cada 45-90 min y los intentos de Codex se agotan esperando (el
  director ya lanza las validaciones como entradas sin modelo para no gastar cuota); B: los cierres y gates de
  quantdesk, quantdesk-network y radiance-win se retrasan esas horas; C: más proyectos a la vez, todos más lentos, y
  riesgo de saturar la máquina
- Recommendation: A, con las validaciones de ninfer lanzadas por el director sin modelo. B solo si quieres ver el
  modelo generando hoy.
- Literal answer: —
- Received by: —
- Confirmation in the executing director: not applicable
- Applied in: —
