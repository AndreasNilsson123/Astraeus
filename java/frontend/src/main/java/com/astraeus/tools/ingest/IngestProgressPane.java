package com.astraeus.tools.ingest;

import javafx.geometry.Insets;
import javafx.scene.control.Label;
import javafx.scene.control.ProgressBar;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.VBox;

/**
 * UI pane for displaying ingestion progress and status.
 * Binds to IngestStatusViewModel for data updates.
 * 
 * <p>Features:</p>
 * <ul>
 *   <li>Job name and status display</li>
 *   <li>Progress bar with percentage</li>
 *   <li>Items processed counter</li>
 *   <li>Error message display</li>
 *   <li>Color-coded status indicators</li>
 * </ul>
 * 
 * <p>This pane is a placeholder for J6 integration. The view model will be populated
 * by wrapper methods that consume native ingest job status.</p>
 * 
 * <p>Usage:</p>
 * <pre>{@code
 * IngestStatusViewModel viewModel = new IngestStatusViewModel();
 * IngestProgressPane progressPane = new IngestProgressPane(viewModel);
 * 
 * // Later, update from wrapper (J6):
 * viewModel.startJob("physics_data.bin", 1000);
 * viewModel.updateJobProgress(500, "Importing entities...");
 * viewModel.completeJob();
 * }</pre>
 */
public class IngestProgressPane extends VBox {
    
    private final IngestStatusViewModel viewModel;
    
    private final Label jobNameLabel;
    private final Label statusLabel;
    private final ProgressBar progressBar;
    private final Label progressLabel;
    private final Label itemsLabel;
    private final Label errorLabel;
    
    public IngestProgressPane(IngestStatusViewModel viewModel) {
        super(10);
        this.viewModel = viewModel;
        
        setPadding(new Insets(10));
        setMinWidth(250);
        setPrefWidth(300);
        setStyle("-fx-background-color: #f5f5f5;");
        
        // Title
        Label titleLabel = new Label("Ingest Progress");
        titleLabel.setStyle("-fx-font-size: 14; -fx-font-weight: bold;");
        
        // Job section
        VBox jobSection = createSection("Current Job");
        
        jobNameLabel = new Label();
        jobNameLabel.setStyle("-fx-font-weight: bold;");
        jobNameLabel.textProperty().bind(viewModel.jobNameProperty());
        
        statusLabel = new Label();
        statusLabel.textProperty().bind(viewModel.statusMessageProperty());
        
        // Update status label style based on state
        viewModel.isActiveProperty().addListener((obs, oldVal, newVal) -> updateStatusStyle());
        viewModel.hasErrorProperty().addListener((obs, oldVal, newVal) -> updateStatusStyle());
        updateStatusStyle();
        
        jobSection.getChildren().addAll(jobNameLabel, statusLabel);
        
        // Progress section
        VBox progressSection = createSection("Progress");
        
        progressBar = new ProgressBar();
        progressBar.setPrefWidth(260);
        progressBar.progressProperty().bind(viewModel.progressProperty());
        
        progressLabel = new Label("0%");
        progressLabel.setStyle("-fx-font-family: monospace;");
        viewModel.progressProperty().addListener((obs, oldVal, newVal) -> {
            int percent = (int) (newVal.doubleValue() * 100);
            progressLabel.setText(percent + "%");
        });
        
        itemsLabel = new Label();
        itemsLabel.setStyle("-fx-font-size: 10; -fx-text-fill: #666666;");
        viewModel.itemsProcessedProperty().addListener((obs, oldVal, newVal) -> updateItemsLabel());
        viewModel.itemsTotalProperty().addListener((obs, oldVal, newVal) -> updateItemsLabel());
        updateItemsLabel();
        
        progressSection.getChildren().addAll(progressBar, progressLabel, itemsLabel);
        
        // Error section (hidden by default)
        VBox errorSection = createSection("Error");
        errorSection.managedProperty().bind(viewModel.hasErrorProperty());
        errorSection.visibleProperty().bind(viewModel.hasErrorProperty());
        
        errorLabel = new Label();
        errorLabel.setStyle("-fx-text-fill: #CC0000; -fx-font-size: 10;");
        errorLabel.setWrapText(true);
        errorLabel.textProperty().bind(viewModel.errorMessageProperty());
        
        errorSection.getChildren().add(errorLabel);
        
        // Info note
        Label noteLabel = new Label("Note: Ingest functionality requires D2/J6 integration");
        noteLabel.setStyle("-fx-font-size: 9; -fx-text-fill: #999999;");
        noteLabel.setWrapText(true);
        
        // Add all sections
        getChildren().addAll(
            titleLabel,
            jobSection,
            progressSection,
            errorSection,
            noteLabel
        );
    }
    
    /**
     * Create a section container.
     */
    private VBox createSection(String title) {
        VBox section = new VBox(5);
        section.setPadding(new Insets(10));
        section.setStyle("-fx-background-color: white; -fx-border-color: #dddddd; -fx-border-width: 1;");
        
        Label titleLabel = new Label(title);
        titleLabel.setStyle("-fx-font-weight: bold; -fx-font-size: 11;");
        section.getChildren().add(titleLabel);
        
        return section;
    }
    
    /**
     * Update status label style based on current state.
     */
    private void updateStatusStyle() {
        if (viewModel.hasError()) {
            statusLabel.setStyle("-fx-text-fill: #CC0000; -fx-font-weight: bold;");
        } else if (viewModel.isActive()) {
            statusLabel.setStyle("-fx-text-fill: #0066CC; -fx-font-weight: bold;");
        } else {
            statusLabel.setStyle("-fx-text-fill: #666666;");
        }
    }
    
    /**
     * Update items label with current progress.
     */
    private void updateItemsLabel() {
        int processed = viewModel.getItemsProcessed();
        int total = viewModel.getItemsTotal();
        
        if (total > 0) {
            itemsLabel.setText(String.format("Items: %d / %d", processed, total));
        } else {
            itemsLabel.setText("Items: 0 / 0");
        }
    }
    
    /**
     * Get the view model bound to this pane.
     * 
     * @return View model
     */
    public IngestStatusViewModel getViewModel() {
        return viewModel;
    }
}
