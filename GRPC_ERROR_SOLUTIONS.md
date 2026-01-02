# Soluzione Errore gRPC: "h2 protocol error: error reading a body from connection"

## Problema

L'errore intermittente:
```
[2025-12-28T10:48:51Z ERROR re_grpc_server] Error while receiving messages: gRPC error, message: "h2 protocol error: error reading a body from connection"
```

Si verifica quando Rerun non riesce a elaborare i dati abbastanza velocemente, causando un errore nel protocollo HTTP/2 della connessione gRPC.

## Cause Principali

1. **Troppi dati inviati troppo rapidamente**: Il plugin invia fino a 60+ messaggi per frame (18 joints × 3 coordinate + skeleton + bones)
2. **Mancanza di gestione errori**: Le chiamate `_rec->log()` non avevano try-catch
3. **Nessun controllo del flusso**: Non c'era rate limiting tra i frame
4. **Accumulo di dati nel buffer gRPC**: La connessione si sovraccarica

## Soluzioni Implementate

### 1. Gestione Eccezioni (try-catch)

Ora tutte le chiamate a `_rec->log()` sono protette da try-catch. Se si verifica un errore gRPC nelle time series, queste vengono automaticamente disabilitate per evitare crash:

```cpp
try {
  _rec->log("skeleton/keypoints", rerun::Points3D(positions)...);
  logged = true;
} catch (const std::exception& e) {
  // Errore gRPC - disabilita temporaneamente le time series
  _enable_timeseries = false;
  _error = "Rerun gRPC error - disabling timeseries: " + std::string(e.what());
}
```

**Beneficio**: Il plugin continua a funzionare anche se alcune chiamate falliscono.

### 2. Rate Limiting (ABILITATO DI DEFAULT)

Il rate limiting è ora **abilitato di default** con 5ms di delay tra frame:

```cpp
// Default nel codice
_enable_rate_limiting = true;
_min_frame_delay = 5000; // 5ms

// Nel config.json (valori predefiniti)
"enable_rate_limiting": true,
"frame_delay_microseconds": 5000
```

**IMPORTANTE**: Il rate limiting è **essenziale** perché il plugin invia 50+ messaggi per frame (18 joints × 3 coordinate + skeleton + bones).

### 3. Auto-Disabilitazione Time Series

Se si verifica un errore gRPC durante il logging delle time series, queste vengono automaticamente disabilitate per quel frame, permettendo comunque la visualizzazione 3D dello scheletro.

### 3. Counter dei Frame

Aggiunto un contatore `_frame_count` per:
- Tracciare quanti frame sono stati processati
- Limitare i messaggi di errore (1 ogni 100 frame)
- Gestire il rate limiting

## Come Usare

### Configurazione Standard (RACCOMANDATA)

Il rate limiting è ora abilitato di default con 5ms di delay. Questa è la configurazione standard:

```json
{
  "enable_rate_limiting": true,
  "frame_delay_microseconds": 5000,
  "enable_timeseries": true
}
```

**Questo dovrebbe funzionare per la maggior parte dei casi.**

### Se l'Errore Persiste - Aumenta il Delay

Se vedi ancora errori gRPC, aumenta il delay:

```json
{
  "enable_rate_limiting": true,
  "frame_delay_microseconds": 10000  // 10ms invece di 5ms
}
```

### Se Hai Molti Frame al Secondo (>100 Hz)

Per dati ad altissima frequenza, considera di aumentare ulteriormente:

```json
{
  "enable_rate_limiting": true,
  "frame_delay_microseconds": 15000  // 15ms
}
```

**Nota**: Con 15ms di delay, il frame rate massimo è ~66 FPS, sufficiente per la maggior parte delle applicazioni.

### Se Vuoi Disabilitare il Rate Limiting (NON RACCOMANDATO)

Solo per test o se hai una connessione molto veloce:

```json
{
  "enable_rate_limiting": false,
  "frame_delay_microseconds": 0
}
```

**Attenzione**: Senza rate limiting, l'errore gRPC si verificherà molto probabilmente.

### Disabilitare le Time Series per Ridurre il Carico

Se vuoi solo lo scheletro 3D senza i grafici temporali:

```json
{
  "enable_timeseries": false,
  "enable_keypoints": true,
  "enable_bones": true
}
```

Questo riduce il numero di messaggi da ~60 a ~2 per frame.

## Test e Debug

### Controllare se l'errore persiste

1. Avvia il plugin con rate limiting abilitato
2. Monitora i log per vedere se compaiono ancora errori gRPC
3. Se persistono, aumenta `frame_delay_microseconds`

### Log degli errori

Gli errori sono ora loggati solo ogni 100 frame per evitare spam:

```cpp
if (_frame_count % 100 == 0) {
  _error = "Rerun gRPC error: " + std::string(e.what());
}
```

Questo ti permette di vedere quando accadono senza riempire i log.

## Altre Soluzioni (se il problema persiste)

### 1. Aumenta i Buffer di Rerun

Nell'inizializzazione del RecordingStream, puoi aumentare i buffer (richiede modifica al codice):

```cpp
// Da aggiungere dopo _rec->spawn()
_rec->set_global_data_density_filter(0.9); // Riduci densità dati
```

### 2. Batch dei Dati

Invece di loggare ogni coordinata separatamente, raggruppa in batch (richiede refactoring significativo).

### 3. Connessione TCP invece di gRPC

Cambia il metodo di connessione a Rerun (richiede modifica a `_rec->spawn()`):

```cpp
_rec->connect("127.0.0.1:9876").exit_on_failure(); // TCP invece di spawn
```

### 4. Aggiorna Rerun SDK

Verifica di usare l'ultima versione di Rerun SDK - versioni più recenti hanno fix per questi problemi.

## Ricompilazione

Dopo le modifiche, ricompila il plugin:

```powershell
cd build
cmake --build . --config Release
```

## Conclusione

Le modifiche implementate dovrebbero risolvere la maggior parte dei casi di questo errore gRPC:

1. ✅ **Gestione errori robusta**: Il plugin non crasha più
2. ✅ **Rate limiting opzionale**: Controllo del flusso dati
3. ✅ **Log migliorati**: Errori visibili ma non spam
4. ✅ **Contatore frame**: Per diagnostica e monitoraggio

Prova prima senza rate limiting. Se l'errore persiste, abilita rate limiting con 5000 µs e aumenta gradualmente se necessario.
