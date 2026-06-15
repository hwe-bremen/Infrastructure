import subprocess
from pathlib import Path

BASE = Path("/Users/hans-wernereberhardt/PycharmProjects/askvalentinai-kommunikation")

cmds = [
    ["git", "add", "askvalentin/execution/step_executor.py"],
    ["git", "commit", "-m",
     "fix: booking_id Extraktion bei Pydantic MCPToolResult\n\n"
     "Wurzel: mcp_result ist ein Pydantic-BaseModel (MCPToolResult),\n"
     "kein dict. isinstance(mcp_result, dict) scheiterte → booking_id\n"
     "wurde nie gefunden → track_booking_failed(reason=no_booking_id).\n\n"
     "Fix: Pydantic-Objekt vor dem isinstance-Check zu dict normalisieren\n"
     "(model_dump() für Pydantic v2, .dict() als Fallback für v1).\n\n"
     "Betrifft zwei Stellen:\n"
     "- _format_booking_confirmation (Pass 2, L~1580)\n"
     "- _format_available_slots (MCP-Result-Block, L~1446)\n\n"
     "Symptom im Log: [CONVERSION_METRICS] Buchung fehlgeschlagen\n"
     "reason=no_booking_id obwohl Cal.com ✅ zurückgegeben hatte."],
]

for cmd in cmds:
    r = subprocess.run(cmd, cwd=BASE, capture_output=True, text=True)
    print("CMD:", " ".join(cmd[:3]))
    for line in (r.stdout + r.stderr).strip().splitlines()[:5]:
        print(" ", line)
    if r.returncode != 0:
        print("❌ Exit", r.returncode)
