# [ISSUE-004] Atención densa en `r9700`: prefill de más de 128 consultas recorre todo el contexto en un solo lanzamiento

> **Type:** Defect (riesgo sin medir)\
> **Status:** Open\
> **Detected:** 2026-11-17\
> **Origin:** SPRINT-01 / PLAN_OPS / R97-13 (revisión 530, Opus 5.5 high, hallazgo 4)\
> **Affected component:** `src/ops/softmax_attention/dense/causal_cache/r9700/plan.h:13-16` (`causal_splits`) y el kernel causal de `r9700`\
> **Reference specification:** PLAN_INTEGRATION R97-16 («documentar el límite de TDR observado») y PLAN_PERF\
> **Severity:** Medium\
> **Destination:** Current sprint (R97-16 lo mide; R97-17/18 lo respetan) y PERF (SPRINT-02) para la solución\
> **Priority and reason:** puede tumbar el driver con contexto largo; no afecta a prompts cortos\
> **Owner:** autor de R97-16 para la medida; PERF para el reparto del trabajo\
> **Follow-up:** medir en R97-16; caso o límite en PERF antes de usar contexto largo en `r9700`

## 1. Contexto y comportamiento observado
`causal_splits(width, batch)` devuelve 16 particiones hasta 32 consultas, 4 hasta 128 y **1 por encima de 128**. Con
una sola partición cada bloque recorre todo el contexto. Ningún caso de la suite cubre prefill de más de 128 tokens
con contexto largo: el máximo es T=129 con 162 claves, o T=67 con 131 072 claves y 4 particiones.

Estimación del revisor, **no medida**: con trozos de prefill de 1 024 a 4 096 tokens y más de 100 000 claves, un
lanzamiento tardaría segundos y superaría el límite TDR de 2 s de Windows (WDDM), que se ve como «HIP launch failure»
y el evento Display 4101. Durante el intento 1 de R97-13 hubo un evento 4101 (03:55:27 UTC) en un caso pequeño que
este patrón no explica; su causa sigue abierta.

## 2. Divergencia
No rompe la corrección numérica: las suites pasan (BF16 180/180, FP8 198/198). El riesgo es de ciclo de vida: un
lanzamiento largo reinicia el driver y deja el contexto HIP inservible.

## 3. Caso mínimo
Pendiente. Propuesta: prefill de T=1 024 con 131 072 claves en la ASUS, midiendo la duración del lanzamiento.

## 4. Causa
Topología de lanzamiento fijada por la geometría del tensor, sin reparto por tiempo para anchos grandes.

## 5. Criterios de resolución
- [ ] 1. Duración medida del lanzamiento para T de 256 a 4 096 y contextos de 8 192 a 262 144 claves (R97-16).
- [ ] 2. Límite documentado o reparto del prefill en trozos que queden por debajo del TDR (PERF).
- [ ] 3. Caso de test que cubra más de 128 consultas con contexto largo.
