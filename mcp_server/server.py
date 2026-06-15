#!/usr/bin/env python3
"""
AskValentinAI MCP Server — FastMCP (Claude Desktop kompatibel)
Migriert von Low-Level API auf FastMCP (2026-04-11)
"""

import os
import sys
import json
import subprocess
from pathlib import Path
from mcp.server.fastmcp import FastMCP

PROJECT_ROOT = Path(os.getenv("PROJECT_ROOT", os.getcwd()))

app = FastMCP("askvalentinai-kommunikation")


@app.tool()
def read_file(path: str, start_line: int = None, end_line: int = None) -> str:
    """Datei komplett einlesen (bis 10MB). Optional: start_line und end_line für Ausschnitt."""
    p = Path(path) if Path(path).is_absolute() else PROJECT_ROOT / path
    if not p.exists():
        return f"❌ Datei nicht gefunden: {p}"
    if p.stat().st_size > 10_000_000:
        return f"❌ Datei zu groß (max 10MB): {p.stat().st_size} bytes"
    try:
        content = p.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return f"[Binärdatei: {p.stat().st_size} bytes]"
    if start_line is not None or end_line is not None:
        lines = content.splitlines()
        s = (start_line or 1) - 1
        e = end_line or len(lines)
        content = "\n".join(lines[s:e])
    return f"=== {p} ===\n\n{content}"


@app.tool()
def write_file(path: str, content: str) -> str:
    """Datei erstellen oder überschreiben."""
    p = Path(path) if Path(path).is_absolute() else PROJECT_ROOT / path
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(content, encoding="utf-8")
    return f"✅ Datei geschrieben: {p}"


@app.tool()
def list_directory(path: str = "", recursive: bool = False, pattern: str = "*") -> str:
    """Verzeichnisinhalt auflisten."""
    p = PROJECT_ROOT / path if path else PROJECT_ROOT
    if not p.exists():
        return f"❌ Verzeichnis nicht gefunden: {p}"
    files = list(p.rglob(pattern)) if recursive else list(p.glob(pattern))
    result = []
    for f in sorted(files):
        try:
            rel = f.relative_to(PROJECT_ROOT)
            result.append(f"{rel}: {f.stat().st_size} bytes")
        except Exception:
            result.append(str(f))
    return "\n".join(result) if result else "Keine Dateien"


@app.tool()
def search_files(pattern: str, path: str = "", file_pattern: str = "*") -> str:
    """Text in Dateien suchen (grep)."""
    import re
    search_path = (PROJECT_ROOT / path) if path else PROJECT_ROOT
    regex = re.compile(pattern)
    results = []
    for fp in search_path.rglob(file_pattern):
        if fp.is_file():
            try:
                for i, line in enumerate(fp.read_text(encoding="utf-8").splitlines(), 1):
                    if regex.search(line):
                        results.append(f"{fp.relative_to(PROJECT_ROOT)}:{i}: {line.strip()}")
            except Exception:
                pass
    return "\n".join(results) if results else "Keine Treffer"


@app.tool()
def delete_file(path: str) -> str:
    """Datei löschen."""
    p = Path(path) if Path(path).is_absolute() else PROJECT_ROOT / path
    if not p.exists():
        return f"❌ Datei nicht gefunden: {p}"
    p.unlink()
    return f"✅ Gelöscht: {p}"


@app.tool()
def git_status() -> str:
    """Git Status anzeigen."""
    result = subprocess.run(
        ["git", "status", "--short"],
        cwd=PROJECT_ROOT, capture_output=True, text=True, timeout=30
    )
    return result.stdout or "Keine Änderungen"


@app.tool()
def run_python(script_path: str, args: list[str] = None) -> str:
    """Python-Script ausführen."""
    p = Path(script_path) if Path(script_path).is_absolute() else PROJECT_ROOT / script_path
    cmd = [sys.executable, str(p)] + (args or [])
    result = subprocess.run(
        cmd, cwd=PROJECT_ROOT, capture_output=True, text=True, timeout=60
    )
    return f"STDOUT:\n{result.stdout}\nSTDERR:\n{result.stderr}\nReturn: {result.returncode}"


if __name__ == "__main__":
    app.run()