# AAA Protocol Simulation in NS-3

Ovaj projekt predstavlja simulaciju AAA (Authentication, Authorization, and Accounting) protokola, sličnog RADIUS standardu, implementiranu unutar ns-3 mrežnog simulatora. Fokus simulacije je na mjerenju performansi autentifikacijskog toka između klijenta i servera u kontrolisanim mrežnim uslovima.

## Opis projekta

Simulacija koristi Point-to-Point mrežni model kako bi demonstrirala razmjenu poruka između klijenta i AAA servera. Implementirani su specifični tipovi poruka kao što su Access-Request, Access-Accept, Access-Reject i Accounting-Request. 

Glavni cilj je analiza uticaja mrežnih gubitaka (Error Rate) i kašnjenja procesiranja na serveru na ukupnu pouzdanost i brzinu autentifikacije.

## Funkcionalnosti

- Simulacija kompletnog toka: Klijent šalje zahtjev, server obrađuje i šalje odgovor.
- Modeliranje grešaka: Mogućnost uvođenja gubitaka paketa na linku putem RateErrorModel-a.
- Vjerovatnoća odbijanja: Server može nasumično odbiti zahtjeve na osnovu definisanog parametra (rejectProb).
- Retransmisije: Klijent automatski ponovo šalje paket ako ne dobije odgovor u predviđenom roku, uz praćenje broja pokušaja.
- Praćenje statistike: Detaljan ispis RTT-a, stope uspješnosti (ASR) i broja retransmisija.
- Export u CSV: Automatsko čuvanje svih rezultata u tabelarni format za kasniju analizu.

## Struktura projekta

Projekat se sastoji od sledećih komponenti:

### Aplikacijski sloj (`ns-3/src/applications/model/`)
- `sptm-sink.h` / `sptm-sink.cc` - Server aplikacija koja prima i obrađuje AAA zahtjeve
- `sptm-header.h` / `sptm-header.cc` - Definicija SPTM protokol zaglavlja
- `sptm-client.h` / `sptm-client.cc` - Klijent aplikacija koja šalje AAA zahtjeve

### Helper klase (`ns-3/src/applications/helper/`)
- `sptm-sink-helper.h` / `sptm-sink-helper.cc` - Helper za jednostavno postavljanje server aplikacije
- `sptm-header-helper.h` / `sptm-header-helper.cc` - Helper za rad sa SPTM zaglavljima
- `sptm-client-helper.h` / `sptm-client-helper.cc` - Helper za jednostavno postavljanje klijent aplikacije

### Simulacioni scenario (`ns-3/scratch/`)
- `aaa_sim.cc` - Glavni simulacioni fajl sa topologijom i scenarijima

### Konfiguracioni fajlovi
- `ns-3/src/applications/CMakeLists.txt` - Build konfiguracija za applications modul

## Preduslovi

Za uspješno pokretanje simulacije potrebno je imati instaliran:
- ns-3 simulator (verzija 3.36 ili novija koja podržava CMake build sistem)
- C++ kompajler (GCC ili Clang) sa C++17 podrškom
- Python 3 (za ns-3 build sistem)

## Instalacija i postavljanje

### Korak 1: Priprema direktorijuma

Provjerite da li imate sljedeću strukturu direktorijuma u vašoj ns-3 instalaciji:
```
ns-3/
├── src/
│   └── applications/
│       ├── model/
│       ├── helper/
│       └── CMakeLists.txt
└── scratch/
```

### Korak 2: Kopiranje aplikacijskih fajlova

Kopirajte SPTM aplikacijske fajlove u `model` direktorij:

```bash
cd /putanja/do/ns-3/src/applications/model/

# Kopirajte sljedeće fajlove u ovaj direktorij:
# - sptm-sink.h
# - sptm-sink.cc
# - sptm-header.h
# - sptm-header.cc
# - sptm-client.h
# - sptm-client.cc
```

**Primjer kopiranja (ako su fajlovi u Downloads folderu):**
```bash
cp ~/Downloads/sptm-sink.h .
cp ~/Downloads/sptm-sink.cc .
cp ~/Downloads/sptm-header.h .
cp ~/Downloads/sptm-header.cc .
cp ~/Downloads/sptm-client.h .
cp ~/Downloads/sptm-client.cc .
```

### Korak 3: Kopiranje helper fajlova

Kopirajte SPTM helper fajlove u `helper` direktorij:

```bash
cd /putanja/do/ns-3/src/applications/helper/

# Kopirajte sljedeće fajlove u ovaj direktorij:
# - sptm-sink-helper.h
# - sptm-sink-helper.cc
# - sptm-header-helper.h
# - sptm-header-helper.cc
# - sptm-client-helper.h
# - sptm-client-helper.cc
```

**Primjer kopiranja:**
```bash
cp ~/Downloads/sptm-sink-helper.h .
cp ~/Downloads/sptm-sink-helper.cc .
cp ~/Downloads/sptm-header-helper.h .
cp ~/Downloads/sptm-header-helper.cc .
cp ~/Downloads/sptm-client-helper.h .
cp ~/Downloads/sptm-client-helper.cc .
```

### Korak 4: Ažuriranje CMakeLists.txt

Otvorite `ns-3/src/applications/CMakeLists.txt` fajl u text editoru:

```bash
cd /putanja/do/ns-3/src/applications/
nano CMakeLists.txt
# ili
gedit CMakeLists.txt
```

Pronađite sekciju gdje su navedeni source fajlovi (obično počinje sa `set(source_files` ili slično) i dodajte:

```cmake
model/sptm-sink.cc
model/sptm-header.cc
model/sptm-client.cc
helper/sptm-sink-helper.cc
helper/sptm-header-helper.cc
helper/sptm-client-helper.cc
```

Zatim pronađite sekciju gdje su navedeni header fajlovi (obično `set(header_files` ili slično) i dodajte:

```cmake
model/sptm-sink.h
model/sptm-header.h
model/sptm-client.h
helper/sptm-sink-helper.h
helper/sptm-header-helper.h
helper/sptm-client-helper.h
```

Sačuvajte izmjene i zatvorite editor.

### Korak 5: Kopiranje simulacionog fajla

Kopirajte glavni simulacioni fajl u `scratch` direktorij:

```bash
cd /putanja/do/ns-3/scratch/
cp ~/Downloads/aaa_sim.cc .
```

### Korak 6: Build projekta

Pozicionirajte se u glavni ns-3 direktorij i izvršite build:

```bash
cd /putanja/do/ns-3/

# Konfigurišite build
./ns3 configure --enable-examples --enable-tests

# Izgradite projekat
./ns3 build
```

# Pokretanje simulacije

Simulaciju pokrećete standardnom **ns-3** komandom. Možete koristiti podrazumijevane parametre ili unijeti vlastite putem komandne linije.

## Primer osnovnog pokretanja
```bash
./ns3 run "scratch/aaa_sim"

# Parametri komandne linije

Simulacija podržava sljedeće argumente za prilagođavanje scenarija:

| Argument      | Opis                                           | Podrazumijevano                |
|---------------|-----------------------------------------------|--------------------------------|
| `--clients`   | Broj AAA klijenata                            | 1                              |
| `--packets`   | Broj sesija po klijentu                       | 10                             |
| `--interval`  | Razmak između sesija (sekunde)                | 1.0                            |
| `--errorRate` | Stopa gubitka paketa na mreži (0.0 - 1.0)     | 0.0                            |
| `--rejectProb`| Vjerovatnoća odbijanja zahtjeva na serveru    | 0.0                            |
| `--procDelay` | Kašnjenje obrade na serveru (ms)              | 10                             |
| `--csvFile`   | Ime izlazne CSV datoteke sa rezultatima       | aaa_simulation_results.csv     |
| `--enableCsv` | Omogući ili onemogući CSV izvoz (true/false)  | true                           |

# Primer pokretanja sa 5% gubitka paketa i 20% šanse za odbijanje zahtjeva

```bash
./ns3 run "scratch/aaa_sim --errorRate=0.05 --rejectProb=0.2"

# Analiza rezultata

Nakon završetka simulacije, u konzoli će se pojaviti rezime koji uključuje:

- Ukupan broj poslanih paketa i jedinstvenih sesija  
- Broj primljenih paketa na strani klijenta i servera  
- Broj prihvaćenih (**Accepted**) i odbijenih (**Rejected**) zahtjeva  
- **ASR (Authentication Success Rate)** procenat  
- Prosječni **RTT (Round Trip Time)**  
- Ukupan broj retransmisija razvrstan po klijentu i serveru  

Svi ovi podaci se automatski dopisuju u CSV fajl, što omogućava hronološko praćenje različitih testova.

# Generisani fajlovi

- `proba-0-0.pcap` → Zapis mrežnog saobraćaja na mrežnom interfejsu klijenta  
- `proba-1-0.pcap` → Zapis mrežnog saobraćaja na mrežnom interfejsu servera  
- `aaa_simulation_results.csv` → Tabela sa svim statističkim podacima (RTT, retransmisije, bajtovi)  

# Licenca

Ovaj kod je razvijen u istraživačke i edukativne svrhe za analizu AAA protokola u simuliranim mrežnim okruženjima.
