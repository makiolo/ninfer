# PLAN_TP — Tensor parallel TP2 de Qwen3.8-27B en las dos R9700

Sprint: SPRINT-02 · Grupo: TP · Fuente: petición del usuario en sesión, 2026-11-16 («implementar tensor parallel para
usar las 2 GPUS para un mismo modelo, y esperamos aumentar el prefill y token/s»).
Depende de: R97-18 (SPRINT-01) y R97-19 (línea base, PLAN_PERF). No depende de otros proyectos.

## 1. Contexto y estado de partida (comprobado 2026-11-16)

- **Sin P2P entre las dos R9700 en Windows:** `hipInfo.exe` lista `peers:` vacío y `non-peers: device#0 device#1`
  en ambas, `isLargeBar: 0` (Resizable BAR no activo o no expuesto). Bus PCI 3 y 6. Toda copia entre GPUs pasa por
  memoria del host.
- **Sin RCCL en el HIP SDK de Windows**, y NCCL no existe en AMD.
- El repo ya tiene ejecución distribuida (`src/distributed/`, `docs/maintainer/distributed-execution.md`): un
  coordinador + un worker por rank, arranque con `posix_spawn`/SSH, conexión de control por sockets, backends de
  comunicación `Native` (RDMA) y `Nccl` elegidos por capacidad en `communication`. Todo eso es solo de Linux.
- **Solo GLM5Next y Qwen4Exp tienen `ParallelPlan`.** Qwen3.5 (Qwen3.8-27B) no lo tiene: no hay división de pesos,
  cabezas ni estado por rank para este modelo.
- El cargador ya hace el slicing por rank de los artefactos en la carga (README, despliegue Spark).

## 2. Decisiones fijadas y premisas

- Se sigue la arquitectura existente: el modelo declara su `ParallelPlan` (slicing, collectives y su dtype/count),
  y la comunicación implementa la collective. El modelo no conoce el backend (AGENTS.md, distributed-execution.md).
- La ganancia es una **hipótesis por medir**, no un compromiso:
  - Decode: cada GPU lee la mitad de los pesos (~9.5 GiB), pero son unas 2 allreduce por capa y token, pasando por el
    host. Si cada una cuesta decenas de µs, la comunicación se come una parte notable del ahorro.
  - Prefill: el cómputo se reparte, pero cada allreduce mueve `tokens × hidden × 2 bytes` por PCIe en ambos sentidos.
  - Por eso R97-22 mide antes de construir y su resultado es un punto de decisión del usuario.

## 3. Tareas

### R97-22 — Medición previa y decisión (go/no-go)
- Nivel L5 · Riesgo medio · Autor: strong · Revisor: otro proveedor.
- Microbenchmark en las dos R9700 de: latencia y ancho de banda D2H/H2D con memoria pinned, `hipMemcpyPeerAsync`
  sin P2P, memoria host mapeada (zero-copy) y una allreduce de 2 ranks pasando por el host, para tamaños de decode
  (un vector `hidden`, unos 10 KB en BF16) y de prefill (MB). Proyectar con la línea base de R97-19 la ganancia de TP2
  en tok/s de decode y en prefill.
- Aceptación: informe con condiciones reproducibles (`bench/README.md`), proyección con sus supuestos y una
  recomendación. **Gate:** el usuario decide seguir, cambiar el enfoque (p. ej. activar Resizable BAR, cambiar de
  ranura PCIe) o parar.

### R97-23 — Ejecución distribuida local en Windows
- Nivel L6 · Riesgo alto (procesos, ciclo de vida, fallos) · Autor: strong · Revisor: strong de otro proveedor.
- Portar `src/distributed/{launch,bootstrap,transport}` para workers locales en la misma máquina: `CreateProcess` en
  lugar de `posix_spawn`, conexión de control con Winsock o pipes, `HIP_VISIBLE_DEVICES` por rank. SSH/remoto queda
  fuera.
- Aceptación: el test del grupo de ejecución (`tests/distributed/`) arranca 2 ranks locales, intercambia mensajes y
  limpia todos los procesos tanto si termina bien como si falla.

### R97-24 — Backend de comunicación por el host para 2 GPUs sin P2P
- Nivel L7 · silent-numeric (reducciones) · Autor: pareja fuerte · Revisor: el otro de la pareja.
- Nuevo backend en `src/distributed/communication/` con el mismo contrato que `Native`/`Nccl`: allreduce y allgather
  de BF16/FP32 para los counts declarados, buffers pinned preparados de antemano, sincronización por eventos y
  progreso en el host. La selección por capacidad lo elige cuando no hay ni RDMA ni NCCL.
- Aceptación: tests de collectives contra una referencia en el host (sumas exactas en FP32 y tolerancia declarada en
  BF16); latencia medida comparada con R97-22.

### R97-25 — `ParallelPlan` TP2 de Qwen3.5
- Nivel L8 · silent-numeric · Autor: pareja fuerte a máximo esfuerzo · Revisor: otro proveedor + miembro frontier.
- Para Qwen3.5 Dense: división por columnas y filas de las proyecciones Q4/Q5/Q8 respetando el tamaño de grupo; cabezas
  de atención y KV por rank, con su caché KV local; cabezas de Gated DeltaNet y conv causal con su estado recurrente
  local; embedding y cabeza de salida divididas por vocabulario con su allgather; capa MTP. Se toma el patrón de
  `src/models/qwen4_exp` y `glm5next`.
- Aceptación: tests de modelo TP2 en verde; parámetros, salida y estado salen del mismo plan.

### R97-26 — Equivalencia numérica TP2
- Nivel L7 · silent-numeric · Autor: pareja fuerte · Revisor: el otro de la pareja · Oráculo: fija la tolerancia antes.
- Mismo prompt en 1 GPU y en TP2: logits del primer paso y perplejidad en un corpus fijo dentro de la tolerancia;
  generación con MTP y `--kv-dtype fp8` en TP2.
- Aceptación: informe de diferencias y tests MTP de Qwen3.5 en verde en TP2.

### R97-27 — Rendimiento TP2 frente a 1 GPU
- Nivel L4 · Riesgo bajo · Autor: medium–strong · Revisor: medium de otro proveedor.
- tok/s de decode (con y sin MTP), prefill a varias longitudes y tiempo hasta el primer token, en 1 GPU y en TP2, con
  las mismas condiciones; comparar con la proyección de R97-22.
- Aceptación: tabla con datos reales y explicación de las desviaciones frente a la proyección.

## 4. Orden y paralelismo

R97-22 (gate) → R97-23 ∥ R97-24 → R97-25 → R97-26 → R97-27. **Las dos GPUs exclusivas para ninfer** durante R97-22 y
desde R97-24 (Q-008). Mutex con PLAN_PERF: la GPU se usa de uno en uno, y R97-20/21 (kernels) antes que R97-27 para
medir sobre los kernels finales.

## 5. GAPs abiertos

- Q-008: liberar también la segunda R9700 de su ranura de IA local durante las tareas TP.
- Resizable BAR: `isLargeBar: 0`. Si R97-22 muestra que la latencia del host limita, activarlo en la BIOS es una acción
  del usuario.

## 6. Registro de cierre

| ID | Commit | Tests | Desviaciones |
|---|---|---|---|
