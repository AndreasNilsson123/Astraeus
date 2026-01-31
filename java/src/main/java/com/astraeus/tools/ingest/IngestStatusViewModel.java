package com.astraeus.tools.ingest;

import javafx.beans.property.*;

/**
 * View model for ingestion status and progress.
 * Provides observable properties for UI binding without coupling to native implementation.
 * 
 * <p>This model will be populated by wrapper methods (J6) that consume native ingest job status.</p>
 * 
 * <p>Thread Safety: Properties are JavaFX properties and should be updated on the JavaFX Application Thread.</p>
 * 
 * <p>Usage:</p>
 * <pre>{@code
 * IngestStatusViewModel viewModel = new IngestStatusViewModel();
 * 
 * // Bind to UI:
 * statusLabel.textProperty().bind(viewModel.statusMessageProperty());
 * progressBar.progressProperty().bind(viewModel.progressProperty());
 * 
 * // Update from wrapper (J6):
 * IngestJobStatus status = engine.getIngestJobStatus(jobId);
 * viewModel.updateFromNativeStatus(status);
 * }</pre>
 */
public class IngestStatusViewModel {
    
    private final StringProperty jobName;
    private final StringProperty statusMessage;
    private final DoubleProperty progress;  // 0.0 to 1.0
    private final IntegerProperty itemsProcessed;
    private final IntegerProperty itemsTotal;
    private final BooleanProperty isActive;
    private final BooleanProperty hasError;
    private final StringProperty errorMessage;
    
    public IngestStatusViewModel() {
        this.jobName = new SimpleStringProperty("No active job");
        this.statusMessage = new SimpleStringProperty("Idle");
        this.progress = new SimpleDoubleProperty(0.0);
        this.itemsProcessed = new SimpleIntegerProperty(0);
        this.itemsTotal = new SimpleIntegerProperty(0);
        this.isActive = new SimpleBooleanProperty(false);
        this.hasError = new SimpleBooleanProperty(false);
        this.errorMessage = new SimpleStringProperty("");
    }
    
    // ==================== Getters ====================
    
    public String getJobName() {
        return jobName.get();
    }
    
    public String getStatusMessage() {
        return statusMessage.get();
    }
    
    public double getProgress() {
        return progress.get();
    }
    
    public int getItemsProcessed() {
        return itemsProcessed.get();
    }
    
    public int getItemsTotal() {
        return itemsTotal.get();
    }
    
    public boolean isActive() {
        return isActive.get();
    }
    
    public boolean hasError() {
        return hasError.get();
    }
    
    public String getErrorMessage() {
        return errorMessage.get();
    }
    
    // ==================== Setters ====================
    
    public void setJobName(String value) {
        this.jobName.set(value);
    }
    
    public void setStatusMessage(String value) {
        this.statusMessage.set(value);
    }
    
    public void setProgress(double value) {
        this.progress.set(Math.max(0.0, Math.min(1.0, value)));
    }
    
    public void setItemsProcessed(int value) {
        this.itemsProcessed.set(value);
    }
    
    public void setItemsTotal(int value) {
        this.itemsTotal.set(value);
    }
    
    public void setActive(boolean value) {
        this.isActive.set(value);
    }
    
    public void setHasError(boolean value) {
        this.hasError.set(value);
    }
    
    public void setErrorMessage(String value) {
        this.errorMessage.set(value);
    }
    
    // ==================== Properties ====================
    
    public StringProperty jobNameProperty() {
        return jobName;
    }
    
    public StringProperty statusMessageProperty() {
        return statusMessage;
    }
    
    public DoubleProperty progressProperty() {
        return progress;
    }
    
    public IntegerProperty itemsProcessedProperty() {
        return itemsProcessed;
    }
    
    public IntegerProperty itemsTotalProperty() {
        return itemsTotal;
    }
    
    public BooleanProperty isActiveProperty() {
        return isActive;
    }
    
    public BooleanProperty hasErrorProperty() {
        return hasError;
    }
    
    public StringProperty errorMessageProperty() {
        return errorMessage;
    }
    
    // ==================== Helper Methods ====================
    
    /**
     * Update progress based on items processed.
     * Automatically calculates progress as itemsProcessed / itemsTotal.
     */
    public void updateProgress() {
        if (itemsTotal.get() > 0) {
            double calculated = (double) itemsProcessed.get() / (double) itemsTotal.get();
            setProgress(calculated);
        } else {
            setProgress(0.0);
        }
    }
    
    /**
     * Mark job as started.
     * 
     * @param jobName Name of the ingest job
     * @param totalItems Total items to process
     */
    public void startJob(String jobName, int totalItems) {
        setJobName(jobName);
        setItemsTotal(totalItems);
        setItemsProcessed(0);
        setProgress(0.0);
        setActive(true);
        setHasError(false);
        setErrorMessage("");
        setStatusMessage("Processing...");
    }
    
    /**
     * Update progress during job execution.
     * 
     * @param processed Number of items processed so far
     * @param message Optional status message
     */
    public void updateJobProgress(int processed, String message) {
        setItemsProcessed(processed);
        updateProgress();
        if (message != null && !message.isEmpty()) {
            setStatusMessage(message);
        }
    }
    
    /**
     * Mark job as completed successfully.
     */
    public void completeJob() {
        setItemsProcessed(itemsTotal.get());
        setProgress(1.0);
        setActive(false);
        setHasError(false);
        setStatusMessage("Completed");
    }
    
    /**
     * Mark job as failed with error.
     * 
     * @param error Error message
     */
    public void failJob(String error) {
        setActive(false);
        setHasError(true);
        setErrorMessage(error);
        setStatusMessage("Failed");
    }
    
    /**
     * Reset to idle state.
     */
    public void reset() {
        setJobName("No active job");
        setStatusMessage("Idle");
        setProgress(0.0);
        setItemsProcessed(0);
        setItemsTotal(0);
        setActive(false);
        setHasError(false);
        setErrorMessage("");
    }
}
