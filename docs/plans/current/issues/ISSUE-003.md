# [ISSUE-003] `media_acquire`: literal IPv6 en Linux, IPv6 compatible con IPv4 y casos de ruta sin test

> **Type:** Defect\
> **Status:** Open\
> **Detected:** 2026-11-17\
> **Origin:** SPRINT-01 / PLAN_HOST / R97-07 (revisión 504, Sonnet 5.5 high, hallazgos 2-5)\
> **Affected component:** `src/product/media_acquire/acquire.cpp`, `tests/host_serve/test_media_acquire.cpp`\
> **Reference specification:** contrato de `media_acquire` (política de URLs y raíz de medios)\
> **Severity:** Low\
> **Destination:** Backlog (todo preexistente salvo la cobertura de tests; no bloquea el port)\
> **Priority and reason:** frontera de confianza, pero proyecto personal con entradas de confianza (AGENTS.md)\
> **Owner:** dueño de `media_acquire` (fuera del fork R9700)\
> **Follow-up:** backlog tras SPRINT-01\

---

## 1. Context and observed behaviour
1. `acquire.cpp:158-167` (`535693be`): quita los corchetes del host IPv6 antes de `getaddrinfo` solo bajo `_WIN32`.
   CURLU conserva los corchetes en todas las plataformas, así que en Linux un literal `http://[::1]/` probablemente
   falla al resolver (preexistente; no verificado en Linux). R97-07 mantuvo Linux sin cambios por contrato de la ficha.
2. `private_address` (`acquire.cpp:102-118`) no filtra IPv6 compatible con IPv4 (`::127.0.0.1`): `http://[::127.0.0.1]:1/`
   pasa el filtro y llega a curl (preexistente). Otras 25 formas (numéricas IPv4, mapeadas, link-local, ULA, esquemas)
   sí se rechazan.
3. `tests/host_serve/test_media_acquire.cpp:22-32` solo prueba una raíz inexistente; no cubre `..`, letra de unidad,
   UNC ni `\\?\` (el revisor comprobó a mano que se rechazan). `read_path` rechaza por falso positivo un fichero
   `..dots.bin` dentro de la raíz (`starts_with("..")`, falla cerrado).
4. El test IPv6 exige `::1` disponible (`test_media_acquire.cpp:56-57`).

## 2. Expected resolution
Quitar corchetes de forma independiente de la plataforma; tratar `::a.b.c.d` como IPv4 en el filtro; comparar el primer
componente de la ruta relativa en vez del prefijo; tests de ruta y omitir el caso IPv6 si no hay `::1`.
