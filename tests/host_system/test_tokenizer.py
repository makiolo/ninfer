"""Byte-level tokenizer contracts with explicit independent expectations; no HF package."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unicodedata

# The expected tokens below are byte values,
# one declared merge, and one declared added token, never production probe output.


def resource():
    visible = list(range(33, 127)) + list(range(161, 173)) + list(range(174, 256))
    alphabet = {b: chr(b) for b in visible}
    alphabet.update({b: chr(256 + i) for i, b in enumerate(b for b in range(256) if b not in visible)})
    vocab = {symbol: b for b, symbol in alphabet.items()}
    vocab['ab'] = 256
    pattern = r"(?i:'s|'t|'re|'ve|'m|'ll|'d)|[^\r\n\p{L}\p{N}]?[\p{L}\p{M}]+|\p{N}| ?[^\s\p{L}\p{M}\p{N}]+[\r\n]*|\s*[\r\n]+|\s+(?!\S)|\s+"
    return dict(
        version='1.0', normalizer={'type': 'NFC'},
        pre_tokenizer={'type': 'Sequence', 'pretokenizers': [
            {'type': 'Split', 'pattern': {'Regex': pattern}, 'behavior': 'Isolated', 'invert': False},
            {'type': 'ByteLevel', 'add_prefix_space': False, 'trim_offsets': False, 'use_regex': False}]},
        decoder={'type': 'ByteLevel'}, post_processor=None,
        added_tokens=[dict(id=257, content='<tag>', single_word=False, lstrip=False,
                           rstrip=False, normalized=False, special=True)],
        model=dict(type='BPE', vocab=vocab, merges=[['a', 'b']], ignore_merges=False,
                   dropout=None, unk_token=None, byte_fallback=False))


def main():
    with tempfile.TemporaryDirectory(prefix='tokenizer host ') as folder:
        directory = Path(folder) / 'resources with spaces'
        directory.mkdir()
        # Exercise CRLF JSON fixtures and an absolute Windows drive/backslash path.
        for name, value in [('tokenizer.json', resource()), ('tokenizer_config.json', {})]:
            (directory / name).write_bytes(json.dumps(value, ensure_ascii=False, indent=2)
                                           .replace('\n', '\r\n').encode('utf-8'))
        cases = []
        expected = []
        for text in ['', 'Hello, world!', '你好 🦊', 'e\u0301', 'a\r\nb\r\nc', 'S:\\models\\file.ninfer']:
            normalized = unicodedata.normalize('NFC', text)
            ids = list(normalized.encode('utf-8'))
            cases.append(dict(text=text, limits=[0, 1, 3, 1000]))
            expected.append(dict(ids=ids, decoded=normalized, skip_special=normalized,
                                 boundaries=[], limited=[ids[:n] for n in [0, 1, 3, 1000]]))
        cases.extend([dict(text='ab', boundaries=[0, 1, 2]),
                      dict(text='a<tag>b', boundaries=[0, 1, 3, 6, 7])])
        expected.extend([
            dict(ids=[256], decoded='ab', skip_special='ab', limited=[], boundaries=[
                dict(exact=0, stable=0), dict(exact=None, stable=0), dict(exact=1, stable=1)]),
            dict(ids=[97, 257, 98], decoded='a<tag>b', skip_special='ab', limited=[], boundaries=[
                dict(exact=0, stable=0), dict(exact=1, stable=1), dict(exact=None, stable=1),
                dict(exact=2, stable=2), dict(exact=3, stable=3)])])
        process = subprocess.run([sys.argv[1], str(directory)],
                                 input=''.join(json.dumps(c, ensure_ascii=False) + '\n' for c in cases),
                                 encoding='utf-8', capture_output=True)
        if process.returncode:
            raise RuntimeError(process.stderr)
        actual = [json.loads(line) for line in process.stdout.splitlines()]
        if actual != expected:
            raise AssertionError(json.dumps(dict(actual=actual, expected=expected), ensure_ascii=False, indent=2))
        print(f'{len(cases)} tokenizer host cases passed')


if __name__ == '__main__':
    main()
