# Questions to the user

Single register of this project's questions to the user (skill `concertino`, `references/questions.md`). No question
lives only in a HANDOFF, in `ON_CALL.md` or in a conversation: it is recorded here, with a commit, before or when it
is asked, and whoever hears the answer writes it here in the user's own words.

- Where: with an active sprint, `docs/plans/current/QUESTIONS.md`; without a sprint (on-call director),
  `docs/plans/QUESTIONS.md`. At sprint close the `current/` one is archived with the dossier and the live entries move
  back to `docs/plans/QUESTIONS.md`.
- States: `open` | `answered` | `expired`. Types: `decision` | `production gate`.
- Production gates (deploy, recreate or restart services, enable or stop schedules, delete data or caches, touch
  secrets or `.env`, anything that changes production or cannot be undone) are asked and confirmed only in the
  session of the director that will run the action.
- Open ones: `rg "^### Q-[0-9]+ · open" docs/plans`. Next number: the highest of `rg -o "Q-[0-9]{3,}" docs/plans`
  plus one. An entry is never deleted or renumbered: its state changes.

<!-- Entry template: copy it under "Entries" and fill it in.
### Q-NNN · open · decision
- Raised: YYYY-MM-DD HH:MM, <session that raises it>
- Task: <task ID or "general">
- Question: <what is decided, self-contained>
- Options: A) …; B) …
- Implications: A: …; B: …; if not decided now: …
- Recommendation: <option and why>
- Literal answer: —
- Received by: —
- Confirmation in the executing director: not applicable
- Applied in: —
-->

## Entries

### Q-001 · answered · decision
- Raised: 2026-11-16, sesión de planificación del port ROCm
- Task: general (SPRINT-01)
- Question: ¿En qué sistema debe correr el port para la R9700? (el repo hoy solo compila en Linux)
- Options: A) Linux nativo; B) Windows nativo; C) WSL2 + ROCm
- Recommendation: A (menos trabajo de host, RCCL disponible)
- Literal answer: "Windows nativo"
- Received by: sesión de planificación, 2026-11-16
- Confirmation in the executing director: not applicable
- Applied in: next/SPRINT.md §5, next/PLAN_HOST.md

### Q-002 · answered · decision
- Raised: 2026-11-16, sesión de planificación del port ROCm
- Task: general (SPRINT-01)
- Question: ¿Qué alcance para el primer hito funcional?
- Options: A) 27B texto sin extras; B) 27B texto + MTP + KV FP8; C) 27B completo con Vision
- Recommendation: A
- Literal answer: "27B texto + MTP + KV FP8"
- Received by: sesión de planificación, 2026-11-16
- Confirmation in the executing director: not applicable
- Applied in: next/SPRINT.md §1 y §6

### Q-003 · answered · decision
- Raised: 2026-11-16, sesión de planificación del port ROCm
- Task: general (SPRINT-01)
- Question: ¿Cómo se organiza el código del port?
- Options: A) Mismo árbol, diana r9700; B) Rama aparte, solo AMD
- Recommendation: A
- Literal answer: "Mismo árbol, diana r9700 (Recomendado)"
- Received by: sesión de planificación, 2026-11-16
- Confirmation in the executing director: not applicable
- Applied in: next/PLAN_BUILD.md §2

### Q-004 · answered · decision
- Raised: 2026-11-16, sesión de planificación del port ROCm
- Task: R97-17
- Question: ¿Qué hacer con el artefacto qwen3_8_27b.ninfer (19 GiB)?
- Options: A) Descargarlo cuando toque; B) Ya lo tengo
- Recommendation: A
- Literal answer: "Descargarlo cuando toque"
- Received by: sesión de planificación, 2026-11-16
- Confirmation in the executing director: not applicable
- Applied in: next/PLAN_INTEGRATION.md R97-17

### Q-005 · answered · decision
- Raised: 2026-11-16, sesión de planificación del port ROCm
- Task: general (CONCERTINO.md §2)
- Question: `origin` es el repo upstream ajeno (github.com/Neroued/ninfer). ¿Dónde van los commits y el push de cada tarea?
- Options: A) Solo commits locales en master, sin push; B) Push a un fork propio (indicar URL); C) No commitear docs/plans ni launchers
- Implications: A: sin copia remota; B: copia remota, hay que añadir el remoto; C: concertino pierde el registro versionado
- Recommendation: B si ya hay fork; si no, A
- Literal answer: "crea una rama, fork_R9700"
- Received by: sesión de planificación, 2026-11-16
- Confirmation in the executing director: not applicable
- Applied in: rama `fork_R9700` creada; CONCERTINO.md §2 (commits en esa rama, sin push mientras no haya remoto propio)

### Q-006 · answered · decision
- Raised: 2026-11-16, sesión de planificación del port ROCm
- Task: general (mutex de GPU)
- Question: Las dos R9700 alojan ranuras de IA local (claude-local). ¿Qué GPU usa ninfer para compilar, testear y medir?
- Options: A) Una GPU fija y su ranura de IA local se libera durante el sprint; B) Compartir con el lock de ranura por turnos; C) Otra
- Implications: A: sin esperas ni ruido en benchmarks, una ranura de IA local menos; B: esperas y benchmarks menos fiables
- Recommendation: A
- Literal answer: "Una fija, exclusiva (Recomendado)"
- Received by: sesión de planificación, 2026-11-16
- Confirmation in the executing director: not applicable
- Applied in: CONCERTINO.md §11, current/SPRINT.md §7; la GPU concreta se eligió al iniciar (Q-009 en
  current/QUESTIONS.md: ASUS, HIP 0).

### Q-007 · answered · decision
- Raised: 2026-11-16, sesión de planificación del port ROCm
- Task: R97-19..21
- Question: ¿El grupo PERF (línea base, GEMV RDNA4, prefill WMMA) va en SPRINT-01 o en un SPRINT-02?
- Options: A) Dividir: PERF a future/SPRINT-02; B) Todo en SPRINT-01
- Implications: A: SPRINT-01 cierra al funcionar correcto (hito claro); B: sprint más largo
- Recommendation: A
- Literal answer: "Dividir: PERF a SPRINT-02 (Recomendado)"
- Received by: sesión de planificación, 2026-11-16
- Confirmation in the executing director: not applicable
- Applied in: PLAN_PERF movido a future/SPRINT-02/; next/SPRINT.md §3, §5, §6

### Q-008 · moved · decision
- Moves: to docs/plans/current/QUESTIONS.md at the start of SPRINT-01 (2026-11-16); the live entry is there.
- Raised: 2026-11-16, sesión de planificación del port ROCm
- Task: R97-22, R97-24..27 (SPRINT-02)
- Question: El tensor parallel usa las dos R9700. ¿Se libera también la segunda GPU de su ranura de IA local mientras
  duren esas tareas?
- Options: A) Sí, las dos exclusivas durante R97-22 y desde R97-24; B) Turnos: TP solo en ventanas acordadas
- Implications: A: sin IA local en GPU durante esa fase; B: TP más lento de desarrollar y benchmarks en ventanas
- Recommendation: A
- Literal answer: —
- Received by: —
- Confirmation in the executing director: not applicable
- Applied in: —
