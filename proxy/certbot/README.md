# certbot — Sollzustand und Wiederherstellung

> **Stand:** 2026-09-04, nach R-188/R-190/R-193
> **Host:** `root@46.224.63.191` (Hetzner), Verzeichnis `/etc/letsencrypt/`
> **Warum es dieses Verzeichnis gibt:** Die gesamte certbot-Konfiguration
> lebte bis heute ausschliesslich auf dem Host, in keinem Repo (R-194).
> Ein Serververlust haette die Reparatur von R-188 und den Deploy-Hook aus
> R-193 lautlos mitgenommen.

---

## 1 · Was hier liegt — und was ausdruecklich nicht

**Hier liegt die Absicht, nicht der Zustand.**

| | |
|---|---|
| ✅ `renewal-hooks/deploy/10-reload-nginx-proxy.sh` | Skript, kein Geheimnis. Genau das Stueck, dessen Fehlen vier Wochen unbemerkt blieb. |
| ✅ Die Tabelle unten | Welche Domain, welcher Authenticator, welcher Port, welche Conf sie nutzt. |
| ✅ Die Ausstellungsbefehle | Damit eine Neu-Ausstellung ohne Raten moeglich ist. |
| ❌ `accounts/` | Privater ACME-Kontoschluessel. |
| ❌ `live/`, `archive/` | Private Schluessel der Zertifikate (`privkey.pem`). |
| ❌ `renewal/*.conf` als Kopie | Waere nutzlos: sie verweisen auf `account = <hash>` und absolute `archive/`-Pfade. Ohne Kontoschluessel und Archiv laesst sich daraus nichts wiederherstellen. |

**Der Wiederherstellungsweg ist deshalb nicht „Dateien zurueckkopieren",
sondern „neu ausstellen mit den dokumentierten Parametern".** Das ist kein
Mangel — bei Zertifikaten ist Neu-Ausstellung ohnehin der normale Weg.

---

## 2 · Sollzustand (geprueft 2026-09-04)

> **Ueberholt seit 15.09. (Portumstellung S4-4, siehe §7):** Port 80 haelt
> nginx-proxy, erneuert wird ueber `webroot` (`webroot_path = /var/www/certbot`).
> Fuer docs am 22.09. gemessen; die uebrigen vor jeder Zertifikatsaenderung mit
> den Befehlen aus §5 pruefen. Die Tabelle beschreibt sonst den Stand 04.09.

Alle neun Zertifikate: `authenticator = standalone`,
`renew_before_expiry = 30 days`, ECDSA, Server
`https://acme-v02.api.letsencrypt.org/directory`.

| Domain | Proxy-Port | Conf in `conf.d/` | aktiv? |
|---|---|---|---|
| `chat.askvalentinai.com` | 10443 | `chat-askvalentinai.conf` | ✅ |
| `askvalentinai-chat.duckdns.org` | — | — | ❌ Block entfernt 2026-09-14 (R-257); Zertifikat wird nicht mehr ausgeliefert, Erneuerung kann entfallen |
| `demo.askvalentinai.com` | 11443 | `demo-askvalentinai.conf` | ✅ |
| `khj.askvalentinai.com` | 12443 | `khj-askvalentinai.conf` * | ✅ |
| `docs.askvalentinai.com` | 443 (seit 22.09.), 13443 bis Firewall Stufe 2b | `docs-askvalentinai.conf` ** | ✅ |
| `henne.askvalentinai.com` | 443 (14443 bis Firewall Stufe 2) | `henne-askvalentinai.conf` | ✅ |
| `askvalentin.duckdns.org` | — | — | ❌ ungenutzt |
| `askvalentin-hofmann.duckdns.org` | — | — | ❌ ungenutzt |
| `energiekonsens.askvalentinai.com` | — | `bek-energiekonsens.conf.bak` | ❌ ungenutzt |
| `hofmann-usa.askvalentinai.com` | — | — | ❌ ungenutzt |

\* **Ehemalige Namensfalle, behoben mit R-267 (2026-09-15):** Die Datei
hiess bis dahin `ikh-…`, obwohl der `server_name` darin immer
`khj.askvalentinai.com` war — "ikh" war schlicht die falsche Bezeichnung
(Betreiberauskunft 2026-09-15), keine Ableitung aus der internen
`client_id` (`israelitisches_krankenhaus_hh`, die bewusst unveraendert
bleibt, siehe Kopf von `khj-askvalentinai.conf`). Dateiname jetzt
`khj-askvalentinai.conf`, konsistent mit `server_name`. Das frueher
vorhandene `ikh.askvalentinai.com`-Zertifikat wurde am 04.09. geloescht —
es war hinter dem Cloudflare-Proxy auf einem fremden Origin und wurde von
keinem aktiven Block genutzt.

\*\* **Erledigt 04.09.:** `docs-askvalentinai.conf` liegt im Repo (byte-identisch
vom Server uebernommen); seit 22.09. auch auf 443.

**Vier Zertifikate bedienen keinen aktiven `server_name`** und werden
trotzdem alle 60 Tage erneuert. Kein Fehler, aber unnoetige Last und
unnoetiges Rauschen im Erneuerungslauf — dieselbe Klasse wie
`askvalentin-holzring` (R-190, am 04.09. geloescht). Vor dem Loeschen
jeweils pruefen, ob wirklich kein Block sie referenziert.

---

## 3 · Neue Domain ausstellen

> **Ueberholt seit 15.09.:** Port 80 haelt nginx-proxy, `--standalone` kann
> nicht mehr binden. Neue Domain ueber den gemeinsamen Port-80-Block:
> `certbot certonly --webroot -w /var/www/certbot -d <domain>` — Pfad vorher mit
> einer bestehenden `/etc/letsencrypt/renewal/*.conf` abgleichen, danach
> `certbot renew --dry-run`. Der Text darunter beschreibt den Stand davor.

**Immer `--standalone`. Nicht `--manual`, nicht `--webroot`.**

    certbot certonly --standalone -d <domain>

`standalone` bindet Port 80 kurzzeitig auf dem Host. Das funktioniert, weil
dort kein Dienst dauerhaft lauscht (der Proxy nutzt 1x080/1x443, siehe
R-189). Danach zwingend:

    certbot renew --dry-run 2>&1 | tail -25

Die neue Domain muss in der Erfolgsliste stehen.

**Warum nicht `manual`:** `ikh.` und `khj.` standen auf `manual` und
konnten sich nicht automatisch erneuern — Certbot bricht ohne
`--manual-auth-hook` ab. Das Ausstellungslog zeigt `--manual
--preferred-challenges dns` als direkten Aufruf ohne vorherigen
gescheiterten `standalone`-Versuch: es gab keine technische Notwendigkeit.
`demo.` und `chat.` standen bis August im selben Zustand und wurden am
19.08. per `--standalone` repariert. Belege:
`askvalentinai-kommunikation/readme/_active/MESSBERICHT_R188_ZERTIFIKATE_2026-09-04.md`.

**Warum nicht `webroot`:** `askvalentin-holzring` stand darauf; das
Webroot-Verzeichnis verschwand mit der Anwendung, die Erneuerung schlug
seither fehl (R-190).

---

## 4 · Deploy-Hook anwenden

Nach einem Serverneuaufbau oder wenn der Hook fehlt:

    install -m 755 proxy/certbot/renewal-hooks/deploy/10-reload-nginx-proxy.sh \
      /etc/letsencrypt/renewal-hooks/deploy/10-reload-nginx-proxy.sh

Pruefen, dass er greift:

    ls -la /etc/letsencrypt/renewal-hooks/deploy/

`pre/` und `post/` sind bewusst leer.

---

## 5 · Abgleich — sonst ist diese Datei schaedlich

Eine versionierte Datei, die niemand anwendet, ist nicht nur wertlos: Sie
weicht mit der Zeit von der Wirklichkeit ab und wird trotzdem geglaubt.
Genau das ist am 04.09. bei `proxy/docker-compose.yml` passiert — die Ports
13080/13443 fehlten im Repo, obwohl der laufende Container sie hatte.

**Vor jeder Zertifikatsaenderung die Tabelle in §2 gegenpruefen:**

    certbot certificates 2>&1 | grep -E 'Certificate Name|Expiry'
    grep -H 'authenticator' /etc/letsencrypt/renewal/*.conf
    docker exec nginx-proxy sh -c 'grep -h server_name /etc/nginx/conf.d/*.conf'

Der letzte Befehl liest bewusst nur `*.conf` — `conf.d/` enthaelt daneben
16 `.bak`/`.disabled`-Dateien, die nginx nicht laedt. Ein `grep -r` ueber
das ganze Verzeichnis hat am 04.09. einen Befund erzeugt, den es nicht gab.

---

## 6 · Naechster Pruefpunkt

**ca. 19.09.2026** — dann erneuert `khj.askvalentinai.com` zum ersten Mal
wirklich. Das ist der Lauf, der die Reparatur produktiv beweist und
zugleich der erste, bei dem der Deploy-Hook von selbst greift.

---

## 7 · Rueckweg — Portumstellung auf 80/443 (Q-5, §13 im Routen-Inventar)

**Festgehalten vor dem ersten Commit dieses Vorgangs (2026-09-15), wie von
Auflage Q-5 verlangt.** Die zehn Zertifikate laufen zwischen 34 und 79
Tagen — solange dieser Weg besteht, ist ein Fehlversuch der Umstellung auf
`webroot` folgenlos, kein Zeitdruck.

**Ausgangszustand (Sollzustand §2 oben):** `authenticator = standalone`,
Port 80/443 auf dem Host ungemappt, `proxy/docker-compose.yml` bildet nur
1x080…14443 ab.

**Rueckweg, in dieser Reihenfolge:**

1. `"80:80"` und `"443:443"` wieder aus der `ports:`-Liste in
   `proxy/docker-compose.yml` entfernen. **Neustart** des Proxy-Containers
   (nicht Reload) — derselbe Schritttyp wie die Umstellung selbst (S4-3).
2. **Nur falls S4-4 schon gelaufen ist:** die zehn
   `/etc/letsencrypt/renewal/<name>.conf` zurueck auf
   `authenticator = standalone` und `webroot_path` entfernen — Vorlage ist
   der in §2 dieser Datei dokumentierte Zustand vor diesem Vorgang.
   Solange S4-4 nicht gelaufen ist (Auflage Q-3 vor S4-4 noch offen),
   entfaellt dieser Punkt ersatzlos.
3. Die vier `listen 443 ssl;`-Zeilen (S4-2) und
   `proxy/conf.d/00-port80-acme-und-redirect.conf` (S4-1) koennen stehen
   bleiben — sie sind wirkungslos, sobald Schritt 1 rueckgaengig ist
   (Port 443/80 wieder ungemappt). Entfernen ist optional, nicht Teil des
   eigentlichen Rueckwegs.
4. `nginx -t`, dann Reload/Neustart durch Hawe (Q-2).

**Nach Schritt 1 ist der Rueckweg vollstaendig:** Port 80 ist wieder
ungemappt, `standalone` kann wieder binden, die Erneuerung laeuft wie vor
diesem Vorgang.
