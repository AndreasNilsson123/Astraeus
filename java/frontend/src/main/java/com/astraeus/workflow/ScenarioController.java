package com.astraeus.workflow;

/**
 * Scenario controller for managing simulation playback and state.
 */
public class ScenarioController {
    
    private boolean playing;
    private double currentTime;
    
    public ScenarioController() {
        this.playing = false;
        this.currentTime = 0.0;
    }
    
    /**
     * Start playback.
     */
    public void play() {
        playing = true;
    }
    
    /**
     * Pause playback.
     */
    public void pause() {
        playing = false;
    }
    
    /**
     * Stop and reset playback.
     */
    public void stop() {
        playing = false;
        currentTime = 0.0;
    }
    
    /**
     * Update simulation time.
     */
    public void update(double deltaTime) {
        if (playing) {
            currentTime += deltaTime;
        }
    }
    
    public boolean isPlaying() {
        return playing;
    }
    
    public double getCurrentTime() {
        return currentTime;
    }
}
