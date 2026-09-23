# ═══════════════════════════════════════════════════════════════════════
# GEMEINSAME SPERRLISTE — alle vier Mandanten-Blöcke (chat/demo/khj/henne)
# ═══════════════════════════════════════════════════════════════════════
# R-231 Schritt 3 (askvalentinai-kommunikation-Repo,
# readme/strategie/ROUTEN_INVENTAR_R231_2026-09-12.md §12). Ersetzt die
# bis 2026-09-14 in jeder Datei einzeln gepflegten deny-Locations für die
# Pfade, die auf ALLEN VIER Blöcken gleich gesperrt sein sollen.
#
# ENDUNG BEWUSST .inc, NICHT .conf: nginx.conf:61 lädt
# "include /etc/nginx/conf.d/*.conf" — eine .conf-Datei mit location-
# Blöcken AUSSERHALB eines server-Blocks bricht den Test ab. Diese Datei
# liegt im selben, bereits gemounteten conf.d-Verzeichnis, wird vom Glob
# aber NICHT erfasst und deshalb nur über das explizite
# "include /etc/nginx/conf.d/sperrliste.inc;" in jedem der vier
# server-Blöcke geladen. WER DIE ENDUNG AUF .conf ÄNDERT, BRICHT DEN
# PROXY-TEST — nicht "reparieren".
#
# satisfy all VOR jedem deny all, in JEDEM Block — auch dort, wo es heute
# wirkungslos ist (chat/demo/henne): khj setzt server-weit "satisfy any"
# (IP-Allowlist ODER Basic-Auth). satisfy wird in Locations vererbt — ohne
# das eigene "satisfy all" würde ein gültiges Basic-Auth-Passwort einen
# deny hier überstimmen (bei /api/high-level bei khj so gemessen: 401
# statt 403, 2026-09-12). Kein proxy_pass in dieser Datei: die Sperre
# greift, bevor etwas weitergeleitet würde.
#
# Herkunft der einzelnen Sperren (Kurzform — der ausführliche Vorgangstext
# stand bis 2026-09-14 vierfach dupliziert in den einzelnen Dateien):
#   /api/crawler                              SEC-1 (2026-07-27), SEC-2 (2026-08-09) —
#                                              unauthentifizierter Rebuild loeschte Embeddings
#   /api/high-level                           R-230 (2026-09-11) — lesend UND schreibend offen
#   =/api/anthropic-test, =/api/openai-test   R-242 (2026-09-12) — Kostenpfad, ruft den
#                                              Provider mit Produktivschluesseln
#   /api/admin, /debug, /docs, /redoc,
#     =/openapi.json                          R-141 (2026-08-19) — vollstaendige Endpunkt-
#                                              Landkarte und administrative Pfade offen
#   /metrics                                  R-137 (2026-08-19) — Mandantenkennungen im
#                                              Klartext; Prometheus scrapt intern ueber
#                                              app:8000, NICHT hierueber — der deny bricht
#                                              das Scraping nicht
#   /admin/cronjobs                           SEC-2 (2026-08-09) — ACP/MCP-Dashboard-Vorschaltung
#   =/health/detailed, =/api/health           2026-09-23 — Diagnosepfade: /health/detailed
#                                              nennt Version, Redis-Verbindungspool und die
#                                              Namen der Hintergrundaufgaben, /api/health
#                                              zeigt, dass Schluessel fuer OpenAI und
#                                              Anthropic hinterlegt sind. Ohne Passwort auf
#                                              chat, demo, henne gemessen (23.09., 10:39 UTC).
#                                              Keine Kundendaten, kein Schluesselwert — aber
#                                              Vorarbeit fuer gezielte Angriffe und
#                                              Rueckfragen bei der Klinik-Pruefung.
#                                              `location =`: der Praefix-Block /health bleibt
#                                              offen, die Aussenueberwachung haengt daran.
#   /prometheus/                              R-130 (2026-08-19) — dieser Weg hat NIE
#                                              funktioniert (proxy_pass ohne Trailing-Slash,
#                                              lief ins Leere); WER IHN "REPARIERT", OEFFNET
#                                              DIE METRIK-HISTORIE INS NETZ. Messweg ist
#                                              containerintern (docker exec ... http://prometheus:9090)
#                                              oder per SSH-Tunnel.
#
# S3-4 (2026-09-14): für khj eine echte Verhaltensänderung, gewollt —
# /api/admin, /debug, /docs, /redoc, /openapi.json, /metrics waren dort
# bisher mit Passwort erreichbar, sind es ab hier nicht mehr. Interner Weg
# für den Betreiber: docker exec kommunikation-app curl -s
# localhost:8000/docs, oder ein SSH-Tunnel auf den App-Port. Bewusst KEINE
# IP-Allowlist als Ersatz — eine Adresse ist kein Ausweis, ein Schlüssel
# ist einer (R-252-Lehre).
#
# NICHT hier drin, bewusst: /admin (bei chat das Bedienfeld, S3-3),
# henne's chat-ui-Regeln (E-5), = /api/v1/feed/abholen bei khj (eigene
# Absicherung, P-0 (b)).

location /api/crawler {
    satisfy all;
    deny all;
}

location /api/high-level {
    satisfy all;
    deny all;
}

location = /api/anthropic-test {
    satisfy all;
    deny all;
    access_log off;
}

location = /api/openai-test {
    satisfy all;
    deny all;
    access_log off;
}

location /api/admin {
    satisfy all;
    deny all;
    access_log off;
}

location /debug {
    satisfy all;
    deny all;
    access_log off;
}

location /docs {
    satisfy all;
    deny all;
    access_log off;
}

location /redoc {
    satisfy all;
    deny all;
    access_log off;
}

location = /openapi.json {
    satisfy all;
    deny all;
    access_log off;
}

location /metrics {
    satisfy all;
    deny all;
}

# 2026-09-23: Diagnosepfade sperren, Lebenszeichen offen lassen.
# REIHENFOLGE BEACHTET: `location =` schlaegt den Praefix-Block `/health` in
# jeder Mandantendatei — /health antwortet weiter 200, /health/detailed 403.
# Die vier Keyword-Monitore muessen VOR dem Reload auf /health zeigen, sonst
# stehen sie rot (Schluesselwort {"status":"healthy" steht in beiden Antworten).
location = /health/detailed {
    satisfy all;
    deny all;
    access_log off;
}

location = /api/health {
    satisfy all;
    deny all;
    access_log off;
}

location /admin/cronjobs {
    satisfy all;
    deny all;
}

location /prometheus/ {
    satisfy all;
    deny all;
}
