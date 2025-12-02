import os
import sys
import random
import argparse
import time
import shutil

import pygame

AUDIO_EXTENSIONS = {".mp3", ".wav", ".ogg", ".flac", ".m4a", ".aac"}


class AerialPlayer:
    def __init__(self, tracks):
        if not tracks:
            raise ValueError("No audio files found.")
        self.tracks = tracks
        self.index = 0
        self.is_playing = False
        self.is_paused = False
        self.shuffle = False

        pygame.mixer.init()

    def _load_current(self):
        path = self.tracks[self.index]
        pygame.mixer.music.load(path)

    def play(self, index=None):
        if index is not None:
            self.index = index % len(self.tracks)
        self._load_current()
        pygame.mixer.music.play()
        self.is_playing = True
        self.is_paused = False

    def pause_toggle(self):
        if not self.is_playing:
            return
        if self.is_paused:
            pygame.mixer.music.unpause()
            self.is_paused = False
        else:
            pygame.mixer.music.pause()
            self.is_paused = True

    def stop(self):
        pygame.mixer.music.stop()
        self.is_playing = False
        self.is_paused = False

    def next(self):
        if self.shuffle:
            self.index = random.randrange(len(self.tracks))
        else:
            self.index = (self.index + 1) % len(self.tracks)
        self.play(self.index)

    def prev(self):
        if self.shuffle:
            self.index = random.randrange(len(self.tracks))
        else:
            self.index = (self.index - 1) % len(self.tracks)
        self.play(self.index)

    def toggle_shuffle(self):
        self.shuffle = not self.shuffle

    def get_current_track(self):
        return self.tracks[self.index]

    def is_busy(self):
        return pygame.mixer.music.get_busy()


def scan_directory(path, recursive=True):
    tracks = []
    path = os.path.abspath(path)
    if recursive:
        for root, dirs, files in os.walk(path):
            for f in files:
                _, ext = os.path.splitext(f)
                if ext.lower() in AUDIO_EXTENSIONS:
                    tracks.append(os.path.join(root, f))
    else:
        for f in os.listdir(path):
            full = os.path.join(path, f)
            if os.path.isfile(full):
                _, ext = os.path.splitext(f)
                if ext.lower() in AUDIO_EXTENSIONS:
                    tracks.append(full)

    tracks.sort()
    return tracks


def format_track_name(path, width):
    name = os.path.basename(path)
    if len(name) <= width:
        return name
    if width <= 3:
        return name[:width]
    return name[: width - 3] + "..."


# ---------- WINDOWS UI (no curses) ----------
def run_ui_windows(player: AerialPlayer):
    import msvcrt  # only on Windows

    selected = player.index  # keep selection in sync with current track
    last_busy_state = False

    def clear_screen():
        os.system("cls")

    def redraw():
        clear_screen()
        cols, rows = shutil.get_terminal_size(fallback=(100, 30))

        full_name = os.path.basename(player.get_current_track())
        current = format_track_name(full_name, cols - 25)

        status = (
            "PLAYING" if player.is_playing and not player.is_paused
            else "PAUSED" if player.is_paused
            else "STOPPED"
        )
        shuffle = "ON" if player.shuffle else "OFF"

        # Header + now playing
        print(f"Aerial CLI  |  {status:<7}  |  Shuffle: {shuffle}  |  Track {player.index+1}/{len(player.tracks)}")
        print(f"NOW PLAYING: {current}")
        print()
        print("↑/↓: Navigate  Enter: Play  Space: Pause  n/p: Next/Prev  s: Shuffle  q: Quit")
        print("-" * cols)

        max_rows = rows - 6
        total = len(player.tracks)

        if total <= max_rows:
            start_index = 0
        else:
            half = max_rows // 2
            start_index = max(0, min(selected - half, total - max_rows))
        end_index = min(total, start_index + max_rows)

        for idx in range(start_index, end_index):
            # ▶ indicates currently playing, ▷ just selected
            is_playing = (idx == player.index and player.is_playing and not player.is_paused)
            marker = "▶" if is_playing else " "

            cursor = ">" if idx == selected else " "
            name = format_track_name(player.tracks[idx], cols - 10)
            print(f"{cursor} {marker} {idx + 1:3d}. {name}")

    while True:
        # Auto-advance when track ends
        busy = player.is_busy()
        if player.is_playing and not player.is_paused:
            if last_busy_state and not busy:
                player.next()
                selected = player.index  # keep selection with current track
        last_busy_state = busy

        redraw()

        # Poll for keypress for ~0.2s
        start = time.time()
        key = None
        while time.time() - start < 0.2:
            if msvcrt.kbhit():
                key = msvcrt.getch()
                break
            time.sleep(0.01)

        if key is None:
            continue

        # Arrow keys (extended codes)
        if key in (b"\x00", b"\xe0"):
            key2 = msvcrt.getch()
            if key2 == b"H":  # up
                selected = (selected - 1) % len(player.tracks)
            elif key2 == b"P":  # down
                selected = (selected + 1) % len(player.tracks)
            continue

        # Normal keys
        if key in (b"q", b"Q"):
            player.stop()
            break
        elif key in (b"\r",):  # Enter
            player.play(selected)
            # selection already equals the track we just started
        elif key == b" ":
            player.pause_toggle()
        elif key in (b"n", b"N"):
            player.next()
            selected = player.index
        elif key in (b"p", b"P"):
            player.prev()
            selected = player.index
        elif key in (b"s", b"S"):
            player.toggle_shuffle()


# ---------- POSIX UI (Linux/macOS with curses) ----------
def run_ui_curses(stdscr, player: AerialPlayer):
    import curses

    curses.curs_set(0)
    stdscr.nodelay(True)
    stdscr.timeout(200)

    selected = 0
    last_busy_state = False

    while True:
        stdscr.erase()
        height, width = stdscr.getmaxyx()

        current = format_track_name(player.get_current_track(), width - 20)
        status = (
            "PLAYING" if player.is_playing and not player.is_paused
            else "PAUSED" if player.is_paused
            else "STOPPED"
        )
        shuffle = "ON" if player.shuffle else "OFF"

        stdscr.addstr(0, 0, f"Aerial CLI  |  {status:<7}  |  Shuffle: {shuffle}  ", curses.A_BOLD)
        stdscr.addstr(1, 0, f"Now: {current}")
        stdscr.addstr(
            3,
            0,
            "↑/↓: Navigate  Enter: Play  Space: Pause  n/p: Next/Prev  s: Shuffle  q: Quit",
            curses.A_DIM,
        )

        start_row = 5
        max_rows = height - start_row - 1
        total_tracks = len(player.tracks)

        if total_tracks <= max_rows:
            start_index = 0
        else:
            half = max_rows // 2
            start_index = max(0, min(selected - half, total_tracks - max_rows))
        end_index = min(total_tracks, start_index + max_rows)

        for i, idx in enumerate(range(start_index, end_index)):
            track = player.tracks[idx]
            line = f"{idx + 1:3d}. {format_track_name(track, width - 7)}"
            row = start_row + i

            attr = curses.A_BOLD if idx == player.index else curses.A_NORMAL
            if idx == selected:
                attr |= curses.A_REVERSE

            stdscr.addstr(row, 0, line[: width - 1], attr)

        stdscr.refresh()

        # Auto-advance when track ends
        busy = player.is_busy()
        if player.is_playing and not player.is_paused:
            if last_busy_state and not busy:
                player.next()
        last_busy_state = busy

        try:
            key = stdscr.getch()
        except KeyboardInterrupt:
            break

        if key == -1:
            continue

        if key in (ord("q"), ord("Q")):
            player.stop()
            break
        elif key == curses.KEY_UP:
            selected = (selected - 1) % len(player.tracks)
        elif key == curses.KEY_DOWN:
            selected = (selected + 1) % len(player.tracks)
        elif key in (curses.KEY_ENTER, 10, 13):
            player.play(selected)
        elif key == ord(" "):
            player.pause_toggle()
        elif key in (ord("n"), ord("N")):
            player.next()
        elif key in (ord("p"), ord("P")):
            player.prev()
        elif key in (ord("s"), ord("S")):
            player.toggle_shuffle()

        time.sleep(0.01)


def main():
    parser = argparse.ArgumentParser(description="Aerial CLI music player")
    parser.add_argument(
        "path",
        nargs="?",
        default=".",
        help="Path to music folder (default: current directory)",
    )
    parser.add_argument(
        "-n", "--no-recursive",
        action="store_true",
        help="Do not scan directories recursively",
    )
    args = parser.parse_args()

    tracks = scan_directory(args.path, recursive=not args.no_recursive)
    if not tracks:
        print(f"No audio files found in {os.path.abspath(args.path)}")
        sys.exit(1)

    player = AerialPlayer(tracks)

    if os.name == "nt":
        # Windows path (no curses)
        run_ui_windows(player)
    else:
        # Linux/macOS with curses
        import curses
        curses.wrapper(run_ui_curses, player)


if __name__ == "__main__":
    main()
