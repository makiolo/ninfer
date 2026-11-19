# [ISSUE-007] FFmpeg de vcpkg sin PNG en `r9700`: dos tests de medios de GLM5Next

> **Type:** Improvement\
> **Status:** Open\
> **Detected:** 2026-11-17 (R97-08) y 2026-11-19 (R97-33)\
> **Origin:** SPRINT-01 / PLAN_HOST R97-08 y PLAN_INTEGRATION R97-33\
> **Affected component:** dependencia FFmpeg del árbol vcpkg de `r9700` (`G:/vcpkg_root/installed-ninfer-r9700`)\
> **Reference specification:** `.agent-tmp/R97-08/excluded.md` (`ninfer_glm5next_media_test`); `.agent-tmp/R97-33/BLOCKED.md`, `codec-check.log`\
> **Severity:** Low\
> **Destination:** Backlog (medios fuera del objetivo de SPRINT-01, que es Qwen3.8-27B de texto; decisión del director, EXECUTION 2026-11-19)\
> **Priority and reason:** solo afecta a la entrada de imágenes de GLM5Next, que además no se ejecuta en `r9700`\
> **Owner:** por asignar (dueño de las dependencias de vcpkg de `r9700`)\
> **Follow-up:** backlog tras SPRINT-01\

---

## 1. Context and observed behaviour
- El FFmpeg instalado por vcpkg para `r9700` está configurado con `--disable-zlib` y no trae el decodificador PNG.
- `ninfer_glm5next_media_test` (excluido en R97-08) y `ninfer_glm5next_protocol_test` (R97-33: compila y enlaza, pero
  falla al decodificar su imagen PNG) no pasan en `r9700` por ese motivo; los otros tres tests de producto de R97-33
  pasan.
- Esperado: ambos tests pasan en `r9700` sin cambiar lo que comprueban.

## 2. Divergence from the specification
Ninguna invariante del producto de texto rota; falta la decodificación PNG en Windows.

## 3. Minimal reproducible case
`ctest --test-dir build/r9700-dev -R "^ninfer_glm5next_(media|protocol)_test$"`: fallan al decodificar PNG
(`.agent-tmp/R97-33/codec-check.log`).

## 4. Root cause analysis
Configuración del puerto FFmpeg de vcpkg en el árbol `installed-ninfer-r9700` (sin zlib, por tanto sin PNG). Hace falta
la característica que habilita zlib/PNG en el manifiesto de vcpkg de `r9700`, sin dependencias nuevas fuera de vcpkg.

## 5. Acceptance and resolution criteria
- [ ] 1. FFmpeg de `r9700` decodifica PNG (comprobado con el códec, no solo con el test).
- [ ] 2. `ninfer_glm5next_media_test` y `ninfer_glm5next_protocol_test` pasan en `r9700` sin cambiar casos ni esperados.
- [ ] 3. Las dianas CUDA (Linux) siguen igual.
- [ ] 4. No aplica documentación de producto.
