#pragma once

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

class Playlist;

namespace fs = std::filesystem;

/**
 * @brief Supported playback modes for the audio engine.
 *
 * NORMAL      → Sequential playback (track 1, 2, 3…)
 * REPEAT_ONE  → Repeat the currently playing track.
 * RANDOM      → Pure randomness; each next track is chosen randomly,
 *               repeats allowed.
 * SHUFFLE     → Smart shuffle; can later be weighted using stats
 *               such as plays/skips and recent history.
 */
enum class PlayMode {
    NORMAL,
    REPEAT_ONE,
    RANDOM,
    SHUFFLE
};

/**
 * @class Player
 * @brief Core audio playback engine for Aerial.
 *
 * Responsibilities:
 *  - Attach and manage a playlist.
 *  - Control playback (play, pause, stop, next, previous).
 *  - Handle seeking and position queries.
 *  - Manage volume level.
 *  - Apply different playback modes (normal, repeat, random, shuffle).
 *
 * This class is UI-agnostic and can be driven by CLI, Qt, mobile, or DJ layers.
 */
class Player {
public:
    Player();
    ~Player();

    /**
     * @brief Initialize the audio backend.
     * @return true on success, false on failure.
     */
    bool init();

    /**
     * @brief Shut down the audio backend and release resources.
     */
    void shutdown();

    /**
     * @brief Attach a Playlist object to this player.
     * @param playlist Shared pointer to a Playlist instance.
     */
    void setPlaylist(std::shared_ptr<Playlist> playlist);

    /**
     * @brief Attach a playlist directly from a list of file paths.
     * @param files Vector of filesystem paths to audio files.
     */
    void setPlaylist(const std::vector<fs::path>& files);

    // ─────────────────────────────────────────────
    // Playback controls
    // ─────────────────────────────────────────────

    /**
     * @brief Start or restart playback of the current track.
     * @return true if playback starts successfully.
     */
    bool playCurrent();

    /**
     * @brief Advance to the next track (respecting play mode) and play it.
     * @return true if successful.
     */
    bool playNext();

    /**
     * @brief Go to the previous track and play it.
     * @return true if successful.
     */
    bool playPrevious();

    /**
     * @brief Pause playback.
     */
    void pause();

    /**
     * @brief Resume playback if it was paused.
     */
    void resume();

    /**
     * @brief Stop playback.
     */
    void stop();

    /**
     * @brief Check if the player is currently playing audio.
     */
    bool isPlaying() const;

    /**
     * @brief Check if playback is currently paused.
     */
    bool isPaused() const;

    /**
     * @brief Get the full path of the currently playing track.
     * @return File path as a string. Empty if nothing is playing.
     */
    std::string nowPlaying() const;

    // ─────────────────────────────────────────────
    // Position / seeking (in seconds)
    // ─────────────────────────────────────────────

    /**
     * @brief Get the current playback position in seconds.
     */
    double getPositionSeconds() const;

    /**
     * @brief Seek to an absolute position in the current track.
     * @param seconds Target position in seconds.
     * @return true if successful.
     */
    bool seekTo(double seconds);

    /**
     * @brief Seek relative to the current position.
     * @param deltaSeconds Offset in seconds (positive or negative).
     * @return true if successful.
     */
    bool seekBy(double deltaSeconds);

    // ─────────────────────────────────────────────
    // Volume controls (0–100%)
    // ─────────────────────────────────────────────

    /**
     * @brief Set the volume level as a percentage (0–100).
     */
    void setVolumePercent(int percent);

    /**
     * @brief Adjust the volume by a delta (e.g., +5, -10).
     */
    void changeVolumePercent(int delta);

    /**
     * @brief Get the current volume level as a percentage.
     */
    int getVolumePercent() const;

    // ─────────────────────────────────────────────
    // Play mode management
    // ─────────────────────────────────────────────

    /**
     * @brief Set the current play mode.
     * @param mode One of NORMAL, REPEAT_ONE, RANDOM, SHUFFLE.
     */
    void setPlayMode(PlayMode mode) { playMode_ = mode; }

    /**
     * @brief Get the current play mode.
     */
    PlayMode getPlayMode() const { return playMode_; }

    /**
     * @brief Move to the next track based on the current play mode.
     *
     * In:
     *  - NORMAL: goes to next sequential track.
     *  - REPEAT_ONE: stays on the same track.
     *  - RANDOM: picks any track at random (repeats allowed).
     *  - SHUFFLE: smart/random logic (skip-aware) to be implemented.
     */
    void nextTrack();

    /**
     * @brief Skip the current track.
     *
     * Intended for user-initiated "next" actions. This can be used
     * to update skip statistics and feed into smart shuffle later.
     */
    void skipCurrent();

private:
    bool initialized_ = false;
    bool paused_      = false;

    std::shared_ptr<Playlist> playlist_;

    int      volumePercent_ = 100;                 ///< Current volume level (0–100).
    PlayMode playMode_      = PlayMode::NORMAL;    ///< Current playback mode.
};
