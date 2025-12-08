# Aerial Player — C++ CLI

Aerial Player is a cross-platform **command-line music player** written in modern C++ using **SDL2** and **SDL2_mixer**.  
It scans a folder of audio files, builds a playlist, and lets you control playback from the terminal or remotely via TCP/HTTP servers.

The CLI serves as the **core engine** behind the Aerial ecosystem — powering desktop apps, DJ tools, mobile clients, and the future Aerial Studio.

---

## 🚀 Features

- Recursive folder scanning for audio files  
- UTF-8 safe path handling (Windows/Linux/macOS)  
- Playlist engine with:
  - Play / pause / resume / stop  
  - Next / previous  
  - Fast-forward / rewind  
  - Volume control  
  - Track search  
- Multiple playback modes:
  - **NORMAL** — sequential  
  - **REPEAT_ONE** — repeat current track  
  - **RANDOM** — random playback (WIP)  
  - **SHUFFLE** — smart shuffle (planned skip/play-aware)  
- SQLite-like logging of plays & skips  
- TCP control server  
- HTTP server for remote now-playing API  
- Fully documented & modular C++ engine  

---

## 📦 Requirements

- C++17 or newer  
- SDL2  
- SDL2_mixer  
- CMake  
- SQLite (optional but recommended)  

---

## 🛠️ Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

With vcpkg:

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg-path]/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

---

## ▶️ Running

```
aerial <music_folder>
```

Example:

```
aerial "E:/Music"
```

On launch Aerial:

1. Loads config  
2. Initializes DB  
3. Scans folder  
4. Builds playlist  
5. Starts playback  
6. Launches TCP + HTTP servers  
7. Starts interactive CLI  

---

## 🎛️ CLI Commands

```
play        - play current track
next        - next track
prev        - previous track
ff          - fast forward 10s
rew         - rewind 10s

search      - search and select a track

volup       - +5%
voldown     - -5%
mute        - 0%
vol         - show volume
vol <n>     - set volume to <n>

mode normal  - sequential playback
mode repeat  - repeat current track
mode random  - random playback (WIP)
mode shuffle - smart shuffle (WIP)

pause       - pause
resume      - resume
stop        - stop
quit/exit   - exit the player
```

---

## 🔎 Search Example

```
> search
Search term: love
Found 3 match(es):
  [0] love_song.mp3
  [1] my_love.wav
  [2] endless_love.flac
Enter number: 1
```

Aerial jumps to the selected track and begins playback.

---

## 🔊 Volume

```
vol         → prints current volume
volup       → +5%
voldown     → -5%
mute        → sets volume to 0%
vol 60      → sets volume to 60%
```

---

## 🔁 Playback Modes

| Mode          | Behavior |
|---------------|----------|
| **NORMAL**    | Sequential (1→2→3→…) |
| **REPEAT_ONE**| Stay on same track |
| **RANDOM**    | Random track each time (WIP) |
| **SHUFFLE**   | Smart random (skip-aware, coming soon) |

---

## 📡 Built-in Servers

### TCP Control Server  
Allows remote commands such as:
- play  
- pause  
- next  
- volume  
- now playing  

### HTTP Server (default: port 8080)  
Provides:
- `/nowplaying`
- `/status`
- Future web remote UI

---

## 🗄 Database Logging

If DB is available:

- When a song begins playing:
  ```
  db.logPlay(track);
  ```

- When a user skips a track:
  ```
  db.logSkip(track);
  ```

This will feed into future **Smart Shuffle** behavior:
- Tracks skipped often are played less  
- Tracks played often get higher weight  

---

## 📁 Folder Scanning

- Recursively parses directories  
- Allowed extensions:
  - `.mp3`
  - `.wav`
  - `.ogg`
  - `.flac`
  - `.m4a`
- Skips unreadable folders silently  
- Stores all paths internally as UTF-8  

---

## 🧠 Planned Enhancements (Full Roadmap)

### 1. True Random Mode
- Full random indexing  
- Optional no-immediate-repeat safeguard  

### 2. Smart Shuffle (Weighted Shuffle)
Based on:
- Skip counts  
- Play counts  
- Recent history  
- Track popularity weighting  

Goal: *Music you like surfaces more. Music you skip disappears.*

### 3. Ratings & Auto-Favorites
- Track becomes a “favorite” after N complete plays  
- Skip-heavy tracks get demoted  
- Favorites boost Smart Shuffle probability  

### 4. Shuffle History Buffer
- Avoid last N played songs  
- DJ-friendly “no-repeat window”  

### 5. Persistent Playlist Metadata
- Last played index  
- Last play mode  
- Resume playback after restart  
- JSON playlist import/export  

### 6. Configurable Servers
- Change ports in `config.json`  
- Disable TCP or HTTP servers  
- HTTPS/WebSocket support (future)  

### 7. Advanced Search
- Fuzzy matching  
- Metadata search (artist/title)  
- Tag-based filters  

### 8. Audio Visualization
- CLI waveform/spectrum view  
- Web UI visualization via WebSocket  

### 9. Playback Error Recovery
- Auto-skip unplayable files  
- Log file errors to DB  

### 10. Aerial Studio Integration
Future integrations:
- Aerial Video Editor  
- Aerial DJ  
- Mobile remote apps  
- Cloud sync  
- Multi-device play history  

---

## 🧰 Project Structure

```
aerial/
 ├── src/
 │    ├── main.cpp
 │    ├── Player.cpp
 │    ├── Playlist.cpp
 │    ├── Server.cpp
 │    ├── UI.cpp
 │    ├── DB.cpp
 │    └── ...
 ├── include/
 │    ├── Player.hpp
 │    ├── Playlist.hpp
 │    ├── Server.hpp
 │    ├── UI.hpp
 │    ├── DB.hpp
 │    └── Config.hpp
 ├── CMakeLists.txt
 └── README.md
```

---

## ❤️ Contributing

Pull requests welcome — especially for:

- Shuffle algorithms  
- Remote UI  
- Audio visualization  
- Platform support  
- DB improvements  

---

## 📜 License

MIT License (or whatever you choose)

---

## 🎵 Part of the Aerial Ecosystem

- **Aerial Player Desktop (Qt)**  
- **Aerial DJ** (Pro Mode)  
- **Aerial Studio** (Video Editor)  
- **Aerial Mobile** (Remote control for player/DJ)  
- **Aerial Cloud** (user stats, favorites, sync)  

Everything starts with this CLI engine.
