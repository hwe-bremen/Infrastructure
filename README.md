# 🌐 Infrastructure Configuration - Hetzner Server

Multi-Client Deployment mit nginx-proxy für AskValentinAI

## 📍 Server

- **IP:** 46.224.63.191
- **OS:** Ubuntu 24.04
- **Hosting:** Hetzner Cloud CX43

---

scp -i ~/.ssh/askvalentin_hetzner root@46.224.63.191:"~/infrastructure/proxy/conf.d/*.conf" proxy/conf.d/
scp -i ~/.ssh/askvalentin_hetzner root@46.224.63.191:~/infrastructure/proxy/nginx.conf proxy/nginx.conf

scp -i ~/.ssh/askvalentin_hetzner -r \
  "root@46.224.63.191:/root/infrastructure/proxy/conf.d/" proxy/

ssh -i ~/.ssh/askvalentin_hetzner root@46.224.63.191 \
  "echo '$(cat ~/.ssh/askvalentin_hetzner_new.pub)' >> ~/.ssh/authorized_keys"


ssh -i ~/.ssh/askvalentin_hetzner_new root@46.224.63.191 "
  # Alte Key-Datei aus conf.d löschen
  rm -f ~/infrastructure/proxy/conf.d/askvalentin_hetzner

  # Alten Key aus authorized_keys entfernen
  grep -v 'askvalentin_hetzner' ~/.ssh/authorized_keys > /tmp/auth_keys_new
  mv /tmp/auth_keys_new ~/.ssh/authorized_keys
  chmod 600 ~/.ssh/authorized_keys
  echo 'Alter Key entfernt!'
  cat ~/.ssh/authorized_keys
"

ssh -i ~/.ssh/askvalentin_hetzner_REVOKED root@46.224.63.19 "echo test"

ssh -i ~/.ssh/askvalentin_hetzner root@46.224.63.19 "echo 'Zugang OK'"


## 🎯 Projekte

### 1. AskValentin Core
- **Domain:** askvalentin.duckdns.org / chatp2.duckdns.org
- **HTTPS Port:** 443
- **Deploy Path:** `/root/AskValentinAI`
- **Netzwerk:** `chatp2_network`

### 2. Holzring Client
- **Domain:** askvalentin-holzring.duckdns.org
- **HTTPS Port:** 8443
- **Deploy Path:** `/root/AskValentinAI_Holzring`
- **Netzwerk:** `askvalentinai_holzring_app_network`

---

## 🔧 nginx-proxy Setup

### Architektur
```
Internet (443, 8443)
    ↓
nginx-proxy Container
    ├─→ proxy_net
    ├─→ chatp2_network (AskValentin)
    └─→ askvalentinai_holzring_app_network (Holzring)
```

### Deployment
```bash
# Auf dem Server
cd /root/infrastructure/proxy

# Start nginx-proxy
docker-compose up -d

# Logs prüfen
docker-compose logs -f

# Config testen
docker exec nginx-proxy nginx -t

# Reload (ohne Downtime)
docker exec nginx-proxy nginx -s reload
```

---

## 📋 Port-Mapping

| Projekt | HTTP | HTTPS | App | Redis | Monitor | Grafana | Prometheus |
|---------|------|-------|-----|-------|---------|---------|------------|
| AskValentin | 80 | 443 | 8000 | 6379 | 5050 | 3000 | 9090 |
| Holzring | 8080 | 8443 | 8001 | 6380 | 5051 | 3001 | 9091 |

---

## 🔐 SSL Zertifikate
```bash
# AskValentin
sudo certbot certonly --standalone \
  -d askvalentin.duckdns.org \
  -d chatp2.duckdns.org \
  --email plan2@du-projekt.de \
  --agree-tos

# Holzring
sudo certbot certonly --standalone \
  -d askvalentin-holzring.duckdns.org \
  --email plan2@du-projekt.de \
  --agree-tos

# Auto-Renewal (bereits konfiguriert)
sudo certbot renew --dry-run
```

---

## 🎯 Critical Learnings

### ✅ Non-Standard Ports (8443)
- **PROBLEM:** Flask/Grafana redirects verlieren Port
- **LÖSUNG:** nginx fängt Redirects ab mit explizitem Port
```nginx
# KRITISCH für Port 8443!
location = /grafana {
    return 301 $scheme://$host:8443/grafana/;
}
```

### ✅ Container-Namen
- Jedes Projekt braucht unique Container-Namen
- Prefix mit Projekt-Name (`holzring-app`, `holzring-monitor`, etc.)

### ✅ Docker Networks
- `proxy_net` für nginx-proxy ↔ Projekte
- Projekt-eigene Networks für interne Kommunikation

---

## 🚀 Updates

### nginx-proxy Config aktualisieren
```bash
# Lokal (auf Mac)
cd ~/Desktop/Projekte/infrastructure-hetzner
git add .
git commit -m "feat: Update nginx configs"
git push origin main

# Auf dem Server
cd /root/infrastructure
git pull origin main
docker exec nginx-proxy nginx -s reload
```

### Neues Projekt hinzufügen
1. Neue Config in `proxy/conf.d/neues-projekt.conf`
2. Ports in `docker-compose.yml` hinzufügen
3. Netzwerk in `docker-compose.yml` hinzufügen
4. Committen & Deployen

---

## 📊 Monitoring
```bash
# nginx Status
docker exec nginx-proxy nginx -t
docker exec nginx-proxy nginx -s reload

# Logs
docker logs nginx-proxy --tail 50 -f

# Connections
docker exec nginx-proxy ss -tulpn
```

---

## 🛠️ Troubleshooting

### Config-Test schlägt fehl
```bash
docker exec nginx-proxy nginx -t
# Zeigt Syntax-Fehler

# Fix lokal, dann:
git pull && docker exec nginx-proxy nginx -s reload
```

### SSL-Fehler
```bash
# Zertifikate prüfen
sudo certbot certificates

# Erneuern
sudo certbot renew
docker exec nginx-proxy nginx -s reload
```

### Port bereits belegt
```bash
# Welcher Prozess nutzt Port?
sudo lsof -i :8443
sudo netstat -tulpn | grep 8443

# Container stoppen
docker stop nginx-proxy
# Konflikt beheben
docker start nginx-proxy
```

---

## 📚 Dateien
```
/root/infrastructure/
├── proxy/
│   ├── nginx.conf              # Haupt-Config
│   ├── conf.d/
│   │   ├── askvalentin.conf    # Core Projekt
│   │   └── holzring.conf       # Holzring Projekt
│   ├── ssl/                    # SSL Optionen
│   ├── logs/                   # nginx Logs
│   └── docker-compose.yml      # nginx-proxy Container
├── README.md                   # Diese Datei
└── .gitignore
```

---

**Erstellt:** 20. November 2024  
**Version:** 1.0  
**Maintainer:** Hawe  
**Status:** Production ✅
