# PLAN_PERF — Rendimiento en RDNA4

Sprint: SPRINT-02 (usuario, 2026-11-16, Q-007) · Grupo: PERF ·
Fuente: `backlog/PLAN_ROCM_R9700.md` (etiqueta `archive/2026-11-16/sprint-01-sources`) §F6. Depende de: R97-18 (SPRINT-01).

## 1. Contexto y estado de partida

- Decode limitado por ancho de banda: ~640 GB/s ÷ 19 GiB ≈ 30 tok/s de techo teórico sin MTP.
- gfx12 ofrece WMMA (`__builtin_amdgcn_wmma_*_gfx12`, rocWMMA) para FP16/BF16/FP8/INT8.

## 2. Decisiones fijadas

- Las conclusiones de rendimiento siguen `bench/README.md` y AGENTS.md: base, carga, condiciones, métricas y criterio
  fijados antes de comparar; un benchmark de Op solo prueba la Op.

## 3. Tareas

### R97-19 — Línea base
- Nivel L4 · Riesgo bajo · Autor: medium–strong · Revisor: medium de otro proveedor.
- tok/s de decode y prefill, tiempo de carga de pesos y ancho de banda de memoria (`tools/hbm_bandwidth_probe.cu`).
- Aceptación: informe con condiciones de medida reproducibles.

### R97-20 — GEMV Q4/Q5 ajustado a RDNA4
- Nivel L7 · silent-numeric · Autor: pareja fuerte · Revisor: el otro de la pareja.
- Cargas de 128 bits, ocupación y desempaquetado para wave32; la corrección la siguen comprobando las suites de R97-10.
- Aceptación: benchmark de Op con mejora medida sobre R97-19 y suites en verde.

### R97-21 — Prefill con WMMA de gfx12
- Nivel L8 · silent-numeric · Autor: pareja fuerte a máximo esfuerzo · Revisor: otro proveedor + miembro frontier.
- Kernels Q4/Q5→BF16 y atención de prefill sobre WMMA de gfx12.
- Aceptación: suites de lineal y atención en verde y prefill medido frente a R97-19.

## 4. Orden y paralelismo

R97-19 → (R97-20 ∥ R97-21); la GPU se usa de uno en uno.

## 5. Registro de cierre

| ID | Commit | Tests | Desviaciones |
|---|---|---|---|
