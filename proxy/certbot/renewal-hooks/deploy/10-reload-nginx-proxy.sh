#!/bin/sh
# Laedt nginx-proxy nach einer Zertifikats-Erneuerung neu.
# Ohne diesen Hook haelt nginx das alte Zertifikat im Speicher und liefert es
# weiter aus, bis jemand von Hand reloadet - eine Erneuerung ohne Reload ist
# keine wirksame Erneuerung.
# Angelegt 2026-09-04 im Zuge von R-188.
set -e
DOCKER=/usr/bin/docker
if "$DOCKER" ps --format "{{.Names}}" | grep -qx nginx-proxy; then
    if "$DOCKER" exec nginx-proxy nginx -t >/dev/null 2>&1; then
        "$DOCKER" exec nginx-proxy nginx -s reload
        echo "[certbot-deploy] nginx-proxy neu geladen (Lineage: ${RENEWED_LINEAGE:-unbekannt})"
    else
        echo "[certbot-deploy] nginx -t fehlgeschlagen - KEIN Reload" >&2
        exit 1
    fi
else
    echo "[certbot-deploy] Container nginx-proxy laeuft nicht - kein Reload" >&2
fi
