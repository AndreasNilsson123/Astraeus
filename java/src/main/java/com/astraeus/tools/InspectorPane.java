package com.astraeus.tools;

import com.astraeus.scene.EntityData;
import com.astraeus.scene.SceneManager;
import com.astraeus.ui.SelectionModel;
import javafx.geometry.Insets;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.util.StringConverter;

/**
 * Inspector pane for viewing and editing entity properties.
 * Features:
 * - Transform editor (position, rotation, scale)
 * - Renderable/material references (read-only initially)
 * - Live updates when selection changes
 * - Direct property binding with EntityData
 */
public class InspectorPane extends VBox {
    
    private final SceneManager sceneManager;
    private final SelectionModel selectionModel;
    
    private Label entityIdLabel;
    private TextField nameField;
    
    // Transform fields
    private Spinner<Double> posXSpinner;
    private Spinner<Double> posYSpinner;
    private Spinner<Double> posZSpinner;
    
    private Spinner<Double> rotXSpinner;
    private Spinner<Double> rotYSpinner;
    private Spinner<Double> rotZSpinner;
    
    private Spinner<Double> scaleXSpinner;
    private Spinner<Double> scaleYSpinner;
    private Spinner<Double> scaleZSpinner;
    
    // Rendering fields
    private CheckBox visibleCheckBox;
    
    // Color fields (read-only for now)
    private Label colorLabel;
    
    // Trail fields (read-only for now)
    private Label trailLabel;
    
    private EntityData currentEntity;
    private boolean updating = false;
    
    public InspectorPane(SceneManager sceneManager, SelectionModel selectionModel) {
        super(10);
        this.sceneManager = sceneManager;
        this.selectionModel = selectionModel;
        
        setPadding(new Insets(10));
        setMinWidth(250);
        setPrefWidth(300);
        setStyle("-fx-background-color: #f5f5f5;");
        
        buildUI();
        setupBindings();
    }
    
    /**
     * Build the UI components.
     */
    private void buildUI() {
        // === Header ===
        Label titleLabel = new Label("Inspector");
        titleLabel.setStyle("-fx-font-size: 14; -fx-font-weight: bold;");
        
        // === Entity Info Section ===
        VBox entitySection = createSection("Entity");
        
        entityIdLabel = new Label("No selection");
        entityIdLabel.setStyle("-fx-font-weight: bold;");
        
        HBox nameBox = new HBox(5);
        Label nameLabel = new Label("Name:");
        nameLabel.setMinWidth(60);
        nameField = new TextField();
        nameField.setPromptText("Entity name");
        nameField.setDisable(true);
        HBox.setHgrow(nameField, Priority.ALWAYS);
        nameBox.getChildren().addAll(nameLabel, nameField);
        
        entitySection.getChildren().addAll(entityIdLabel, nameBox);
        
        // === Transform Section ===
        VBox transformSection = createSection("Transform");
        
        // Position
        Label posLabel = new Label("Position");
        posLabel.setStyle("-fx-font-weight: bold;");
        
        posXSpinner = createDoubleSpinner(-1000, 1000, 0, 0.1);
        posYSpinner = createDoubleSpinner(-1000, 1000, 0, 0.1);
        posZSpinner = createDoubleSpinner(-1000, 1000, 0, 0.1);
        
        GridPane posGrid = createVector3Grid("X", posXSpinner, "Y", posYSpinner, "Z", posZSpinner);
        
        // Rotation (in degrees for user, stored as radians)
        Label rotLabel = new Label("Rotation (degrees)");
        rotLabel.setStyle("-fx-font-weight: bold;");
        
        rotXSpinner = createDoubleSpinner(-360, 360, 0, 1);
        rotYSpinner = createDoubleSpinner(-360, 360, 0, 1);
        rotZSpinner = createDoubleSpinner(-360, 360, 0, 1);
        
        GridPane rotGrid = createVector3Grid("X", rotXSpinner, "Y", rotYSpinner, "Z", rotZSpinner);
        
        // Scale
        Label scaleLabel = new Label("Scale");
        scaleLabel.setStyle("-fx-font-weight: bold;");
        
        scaleXSpinner = createDoubleSpinner(0.01, 100, 1, 0.1);
        scaleYSpinner = createDoubleSpinner(0.01, 100, 1, 0.1);
        scaleZSpinner = createDoubleSpinner(0.01, 100, 1, 0.1);
        
        GridPane scaleGrid = createVector3Grid("X", scaleXSpinner, "Y", scaleYSpinner, "Z", scaleZSpinner);
        
        transformSection.getChildren().addAll(
            posLabel, posGrid,
            new Separator(),
            rotLabel, rotGrid,
            new Separator(),
            scaleLabel, scaleGrid
        );
        
        // === Rendering Section ===
        VBox renderingSection = createSection("Rendering");
        
        visibleCheckBox = new CheckBox("Visible");
        visibleCheckBox.setDisable(true);
        
        HBox colorBox = new HBox(5);
        Label colorTitleLabel = new Label("Color:");
        colorTitleLabel.setMinWidth(60);
        colorLabel = new Label("1.0, 1.0, 1.0, 1.0");
        colorLabel.setStyle("-fx-font-family: monospace;");
        colorBox.getChildren().addAll(colorTitleLabel, colorLabel);
        
        HBox trailBox = new HBox(5);
        Label trailTitleLabel = new Label("Trail:");
        trailTitleLabel.setMinWidth(60);
        trailLabel = new Label("Disabled");
        trailBox.getChildren().addAll(trailTitleLabel, trailLabel);
        
        renderingSection.getChildren().addAll(visibleCheckBox, colorBox, trailBox);
        
        // === Add all sections ===
        ScrollPane scrollPane = new ScrollPane();
        scrollPane.setFitToWidth(true);
        scrollPane.setStyle("-fx-background-color: transparent;");
        
        VBox content = new VBox(10);
        content.getChildren().addAll(
            titleLabel,
            entitySection,
            transformSection,
            renderingSection
        );
        
        scrollPane.setContent(content);
        VBox.setVgrow(scrollPane, Priority.ALWAYS);
        
        getChildren().add(scrollPane);
    }
    
    /**
     * Create a section container.
     */
    private VBox createSection(String title) {
        VBox section = new VBox(5);
        section.setPadding(new Insets(10));
        section.setStyle("-fx-background-color: white; -fx-border-color: #dddddd; -fx-border-width: 1;");
        return section;
    }
    
    /**
     * Create a double spinner with specified range.
     */
    private Spinner<Double> createDoubleSpinner(double min, double max, double initial, double step) {
        Spinner<Double> spinner = new Spinner<>(min, max, initial, step);
        spinner.setEditable(true);
        spinner.setDisable(true);
        spinner.setPrefWidth(80);
        
        // Format to 2 decimal places
        spinner.setValueFactory(new SpinnerValueFactory.DoubleSpinnerValueFactory(min, max, initial, step));
        spinner.getValueFactory().setConverter(new StringConverter<Double>() {
            @Override
            public String toString(Double value) {
                return value == null ? "0.00" : String.format("%.2f", value);
            }
            
            @Override
            public Double fromString(String string) {
                try {
                    return Double.parseDouble(string);
                } catch (NumberFormatException e) {
                    return 0.0;
                }
            }
        });
        
        return spinner;
    }
    
    /**
     * Create a grid for vector3 input (X, Y, Z).
     */
    private GridPane createVector3Grid(String xLabel, Spinner<Double> xSpinner,
                                       String yLabel, Spinner<Double> ySpinner,
                                       String zLabel, Spinner<Double> zSpinner) {
        GridPane grid = new GridPane();
        grid.setHgap(5);
        grid.setVgap(5);
        
        Label xLbl = new Label(xLabel);
        xLbl.setMinWidth(20);
        Label yLbl = new Label(yLabel);
        yLbl.setMinWidth(20);
        Label zLbl = new Label(zLabel);
        zLbl.setMinWidth(20);
        
        grid.add(xLbl, 0, 0);
        grid.add(xSpinner, 1, 0);
        grid.add(yLbl, 2, 0);
        grid.add(ySpinner, 3, 0);
        grid.add(zLbl, 4, 0);
        grid.add(zSpinner, 5, 0);
        
        return grid;
    }
    
    /**
     * Setup data bindings and listeners.
     */
    private void setupBindings() {
        // Listen to selection changes
        selectionModel.selectedEntityIdProperty().addListener((obs, oldVal, newVal) -> {
            if (newVal != null && newVal != 0) {
                loadEntity(newVal);
            } else {
                clearInspector();
            }
        });
        
        // Setup spinner change listeners
        setupSpinnerListener(posXSpinner, value -> {
            if (currentEntity != null) {
                currentEntity.setPosX(value);
                syncToEngine();
            }
        });
        
        setupSpinnerListener(posYSpinner, value -> {
            if (currentEntity != null) {
                currentEntity.setPosY(value);
                syncToEngine();
            }
        });
        
        setupSpinnerListener(posZSpinner, value -> {
            if (currentEntity != null) {
                currentEntity.setPosZ(value);
                syncToEngine();
            }
        });
        
        setupSpinnerListener(rotXSpinner, value -> {
            if (currentEntity != null) {
                currentEntity.setRotX(Math.toRadians(value));
                syncToEngine();
            }
        });
        
        setupSpinnerListener(rotYSpinner, value -> {
            if (currentEntity != null) {
                currentEntity.setRotY(Math.toRadians(value));
                syncToEngine();
            }
        });
        
        setupSpinnerListener(rotZSpinner, value -> {
            if (currentEntity != null) {
                currentEntity.setRotZ(Math.toRadians(value));
                syncToEngine();
            }
        });
        
        setupSpinnerListener(scaleXSpinner, value -> {
            if (currentEntity != null) {
                currentEntity.setScaleX(value);
                syncToEngine();
            }
        });
        
        setupSpinnerListener(scaleYSpinner, value -> {
            if (currentEntity != null) {
                currentEntity.setScaleY(value);
                syncToEngine();
            }
        });
        
        setupSpinnerListener(scaleZSpinner, value -> {
            if (currentEntity != null) {
                currentEntity.setScaleZ(value);
                syncToEngine();
            }
        });
        
        // Name field
        nameField.textProperty().addListener((obs, oldVal, newVal) -> {
            if (!updating && currentEntity != null) {
                currentEntity.setName(newVal);
            }
        });
        
        // Visibility checkbox
        visibleCheckBox.selectedProperty().addListener((obs, oldVal, newVal) -> {
            if (!updating && currentEntity != null) {
                currentEntity.setVisible(newVal);
                sceneManager.syncVisibilityToEngine(currentEntity);
            }
        });
    }
    
    /**
     * Setup spinner value change listener.
     */
    private void setupSpinnerListener(Spinner<Double> spinner, java.util.function.Consumer<Double> onChange) {
        spinner.valueProperty().addListener((obs, oldVal, newVal) -> {
            if (!updating && newVal != null) {
                onChange.accept(newVal);
            }
        });
    }
    
    /**
     * Load entity data into inspector.
     */
    private void loadEntity(int entityId) {
        EntityData entity = sceneManager.getEntity(entityId);
        if (entity == null) {
            clearInspector();
            return;
        }
        
        currentEntity = entity;
        updating = true;
        
        // Update entity info
        entityIdLabel.setText("Entity ID: " + entityId);
        nameField.setText(entity.getName());
        nameField.setDisable(false);
        
        // Update transform
        posXSpinner.getValueFactory().setValue(entity.getPosX());
        posYSpinner.getValueFactory().setValue(entity.getPosY());
        posZSpinner.getValueFactory().setValue(entity.getPosZ());
        
        rotXSpinner.getValueFactory().setValue(Math.toDegrees(entity.getRotX()));
        rotYSpinner.getValueFactory().setValue(Math.toDegrees(entity.getRotY()));
        rotZSpinner.getValueFactory().setValue(Math.toDegrees(entity.getRotZ()));
        
        scaleXSpinner.getValueFactory().setValue(entity.getScaleX());
        scaleYSpinner.getValueFactory().setValue(entity.getScaleY());
        scaleZSpinner.getValueFactory().setValue(entity.getScaleZ());
        
        // Enable spinners
        posXSpinner.setDisable(false);
        posYSpinner.setDisable(false);
        posZSpinner.setDisable(false);
        rotXSpinner.setDisable(false);
        rotYSpinner.setDisable(false);
        rotZSpinner.setDisable(false);
        scaleXSpinner.setDisable(false);
        scaleYSpinner.setDisable(false);
        scaleZSpinner.setDisable(false);
        
        // Update rendering
        visibleCheckBox.setSelected(entity.isVisible());
        visibleCheckBox.setDisable(false);
        
        colorLabel.setText(String.format("%.2f, %.2f, %.2f, %.2f",
            entity.getColorR(), entity.getColorG(), entity.getColorB(), entity.getColorA()));
        
        if (entity.getTrailMaxPoints() > 0) {
            trailLabel.setText(entity.getTrailMaxPoints() + " points");
        } else {
            trailLabel.setText("Disabled");
        }
        
        updating = false;
    }
    
    /**
     * Clear inspector (no selection).
     */
    private void clearInspector() {
        currentEntity = null;
        updating = true;
        
        entityIdLabel.setText("No selection");
        nameField.clear();
        nameField.setDisable(true);
        
        // Reset and disable all spinners
        posXSpinner.getValueFactory().setValue(0.0);
        posYSpinner.getValueFactory().setValue(0.0);
        posZSpinner.getValueFactory().setValue(0.0);
        rotXSpinner.getValueFactory().setValue(0.0);
        rotYSpinner.getValueFactory().setValue(0.0);
        rotZSpinner.getValueFactory().setValue(0.0);
        scaleXSpinner.getValueFactory().setValue(1.0);
        scaleYSpinner.getValueFactory().setValue(1.0);
        scaleZSpinner.getValueFactory().setValue(1.0);
        
        posXSpinner.setDisable(true);
        posYSpinner.setDisable(true);
        posZSpinner.setDisable(true);
        rotXSpinner.setDisable(true);
        rotYSpinner.setDisable(true);
        rotZSpinner.setDisable(true);
        scaleXSpinner.setDisable(true);
        scaleYSpinner.setDisable(true);
        scaleZSpinner.setDisable(true);
        
        visibleCheckBox.setSelected(false);
        visibleCheckBox.setDisable(true);
        
        colorLabel.setText("-");
        trailLabel.setText("-");
        
        updating = false;
    }
    
    /**
     * Synchronize current entity transform to engine.
     */
    private void syncToEngine() {
        if (currentEntity != null) {
            sceneManager.syncTransformToEngine(currentEntity);
        }
    }
}
