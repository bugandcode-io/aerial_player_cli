#pragma once

#include <string>
#include <vector>
#include <functional>

struct TrackInfo {
    std::string id;         // internal ID or path
    std::string title;
    std::string artist;
    double      durationSec = 0.0;
    double      bpm = 0.0;
};

enum class DeckId {
    A,
    B
};

struct DeckState {
    bool   isPlaying = false;
    double positionSec = 0.0;
    double durationSec = 0.0;
    double pitchPercent = 0.0;
    TrackInfo currentTrack;
};

struct MixerState {
    float crossfader = 0.5f;   // 0 = full A, 1 = full B
    float masterVolume = 0.8f;
};

class AerialEngine {
public:
    virtual ~AerialEngine() = default;

    // Library
    virtual const std::vector<TrackInfo>& getLibrary() const = 0;
    virtual bool loadLibraryFromPath(const std::string& rootPath) = 0;

    // Deck control
    virtual bool loadTrackToDeck(DeckId deck, const std::string& trackId) = 0;
    virtual void play(DeckId deck) = 0;
    virtual void pause(DeckId deck) = 0;
    virtual void seek(DeckId deck, double positionSec) = 0;
    virtual void setPitch(DeckId deck, double pitchPercent) = 0;
    virtual DeckState getDeckState(DeckId deck) const = 0;

    // Mixer
    virtual void setCrossfader(float value01) = 0;
    virtual MixerState getMixerState() const = 0;
    virtual void setMasterVolume(float value01) = 0;

    // Update (to be called periodically by UI loop or audio thread)
    virtual void update(double dtSeconds) = 0;

    // Simple callbacks for UI to subscribe (optional for later)
    std::function<void(const DeckState&)> onDeckStateChangedA;
    std::function<void(const DeckState&)> onDeckStateChangedB;
    std::function<void(const MixerState&)> onMixerStateChanged;
};
