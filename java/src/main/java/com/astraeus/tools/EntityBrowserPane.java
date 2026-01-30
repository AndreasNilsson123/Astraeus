package com.astraeus.tools;

import com.astraeus.scene.EntityData;
import com.astraeus.scene.SceneManager;
import com.astraeus.ui.SelectionModel;
import com.astraeus.util.FxUtils;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.collections.transformation.FilteredList;
import javafx.collections.transformation.SortedList;
import javafx.geometry.Insets;
import javafx.scene.control.*;
import javafx.scene.layout.*;

/**
 * Entity browser panel for viewing and managing all entities in the scene.
 * 
 * Features:
 * - Table view of all entities with ID, name, position, visibility
 * - Search/filter by name or ID
 * - Sort by any column
 * - Selection integration with other panels
 * - Refresh and clear controls
 * - Entity count display
 * 
 * PERFORMANCE:
 * - Uses ObservableList from SceneManager (no copying)
 * - FilteredList and SortedList wrappers for efficient filtering/sorting
 * - No per-frame allocations
 * 
 * USAGE:
 * <pre>
 * EntityBrowserPane browser = new EntityBrowserPane(sceneManager, selectionModel);
 * // Table automatically updates when entities change
 * </pre>
 */
public class EntityBrowserPane extends BorderPane {
    
    private final SceneManager sceneManager;
    private final SelectionModel selectionModel;
    
    // UI Components
    private final TextField searchField;
    private final Label entityCountLabel;
    private final TableView<EntityData> entityTable;
    private final FilteredList<EntityData> filteredEntities;
    private final SortedList<EntityData> sortedEntities;
    
    public EntityBrowserPane(SceneManager sceneManager, SelectionModel selectionModel) {
        this.sceneManager = sceneManager;
        this.selectionModel = selectionModel;
        
        // === TOP: Toolbar ===
        ToolBar toolbar = new ToolBar();
        
        Label titleLabel = new Label("Entity Browser");
        titleLabel.setStyle("-fx-font-weight: bold;");
        
        Region spacer1 = new Region();
        HBox.setHgrow(spacer1, Priority.ALWAYS);
        
        entityCountLabel = new Label("0 entities");
        entityCountLabel.setStyle("-fx-text-fill: #666666;");
        
        Button refreshButton = new Button("Refresh");
        refreshButton.setOnAction(e -> refresh());
        
        Button clearButton = new Button("Clear All");
        clearButton.setOnAction(e -> clearAll());
        
        toolbar.getItems().addAll(
            titleLabel,
            spacer1,
            entityCountLabel,
            new Separator(),
            refreshButton,
            clearButton
        );
        
        // === TOP: Search Bar ===
        HBox searchBox = new HBox(10);
        searchBox.setPadding(new Insets(5, 10, 5, 10));
        
        Label searchLabel = new Label("Search:");
        searchField = new TextField();
        searchField.setPromptText("Filter by ID or name...");
        searchField.textProperty().addListener((obs, oldVal, newVal) -> updateFilter());
        HBox.setHgrow(searchField, Priority.ALWAYS);
        
        searchBox.getChildren().addAll(searchLabel, searchField);
        
        VBox topSection = new VBox();
        topSection.getChildren().addAll(toolbar, searchBox);
        
        // === CENTER: Entity Table ===
        entityTable = createEntityTable();
        
        // Setup filtering and sorting
        filteredEntities = new FilteredList<>(sceneManager.getEntities(), p -> true);
        sortedEntities = new SortedList<>(filteredEntities);
        sortedEntities.comparatorProperty().bind(entityTable.comparatorProperty());
        
        entityTable.setItems(sortedEntities);
        
        // Selection handling
        entityTable.getSelectionModel().selectedItemProperty().addListener((obs, oldVal, newVal) -> {
            if (newVal != null && selectionModel != null) {
                selectionModel.setSelectedEntity(newVal.getEntityId());
            }
        });
        
        // Listen to external selection changes
        if (selectionModel != null) {
            selectionModel.selectedEntityIdProperty().addListener((obs, oldVal, newVal) -> {
                if (newVal != null && newVal.intValue() > 0) {
                    selectEntityById(newVal.intValue());
                }
            });
        }
        
        // === Layout ===
        setTop(topSection);
        setCenter(entityTable);
        
        // Update count label
        sceneManager.getEntities().addListener((javafx.collections.ListChangeListener.Change<? extends EntityData> c) -> {
            updateEntityCount();
        });
        updateEntityCount();
    }
    
    /**
     * Create the entity table with columns.
     */
    private TableView<EntityData> createEntityTable() {
        TableView<EntityData> table = new TableView<>();
        table.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY);
        table.setPlaceholder(new Label("No entities in scene"));
        
        // ID column
        TableColumn<EntityData, Number> idCol = new TableColumn<>("ID");
        idCol.setCellValueFactory(data -> new SimpleIntegerProperty(data.getValue().getEntityId()));
        idCol.setPrefWidth(60);
        idCol.setStyle("-fx-alignment: CENTER;");
        
        // Name column
        TableColumn<EntityData, String> nameCol = new TableColumn<>("Name");
        nameCol.setCellValueFactory(data -> data.getValue().nameProperty());
        nameCol.setPrefWidth(150);
        
        // Position column (combined X, Y, Z)
        TableColumn<EntityData, String> posCol = new TableColumn<>("Position");
        posCol.setCellValueFactory(data -> {
            EntityData entity = data.getValue();
            String pos = String.format("%.2f, %.2f, %.2f", 
                entity.getPosX(), entity.getPosY(), entity.getPosZ());
            return new SimpleStringProperty(pos);
        });
        posCol.setPrefWidth(150);
        
        // Visible column
        TableColumn<EntityData, String> visibleCol = new TableColumn<>("Visible");
        visibleCol.setCellValueFactory(data -> {
            boolean visible = data.getValue().isVisible();
            return new SimpleStringProperty(visible ? "✓" : "✗");
        });
        visibleCol.setPrefWidth(60);
        visibleCol.setStyle("-fx-alignment: CENTER;");
        
        // Color column (visual indicator)
        TableColumn<EntityData, String> colorCol = new TableColumn<>("Color");
        colorCol.setCellValueFactory(data -> {
            EntityData entity = data.getValue();
            // Create a simple color indicator
            return new SimpleStringProperty(String.format("R:%.1f G:%.1f B:%.1f", 
                entity.getColorR(), entity.getColorG(), entity.getColorB()));
        });
        colorCol.setPrefWidth(120);
        
        table.getColumns().addAll(idCol, nameCol, posCol, visibleCol, colorCol);
        
        // Enable row selection
        table.getSelectionModel().setSelectionMode(SelectionMode.SINGLE);
        
        // Context menu
        ContextMenu contextMenu = new ContextMenu();
        
        MenuItem selectItem = new MenuItem("Select");
        selectItem.setOnAction(e -> {
            EntityData selected = table.getSelectionModel().getSelectedItem();
            if (selected != null && selectionModel != null) {
                selectionModel.setSelectedEntity(selected.getEntityId());
            }
        });
        
        MenuItem deleteItem = new MenuItem("Delete");
        deleteItem.setOnAction(e -> {
            EntityData selected = table.getSelectionModel().getSelectedItem();
            if (selected != null) {
                deleteEntity(selected);
            }
        });
        
        contextMenu.getItems().addAll(selectItem, new SeparatorMenuItem(), deleteItem);
        table.setContextMenu(contextMenu);
        
        return table;
    }
    
    /**
     * Update the filter predicate based on search text.
     */
    private void updateFilter() {
        String searchText = searchField.getText().toLowerCase().trim();
        
        if (searchText.isEmpty()) {
            filteredEntities.setPredicate(p -> true);
        } else {
            filteredEntities.setPredicate(entity -> {
                // Match by ID
                String idStr = String.valueOf(entity.getEntityId());
                if (idStr.contains(searchText)) {
                    return true;
                }
                
                // Match by name
                String name = entity.getName().toLowerCase();
                if (name.contains(searchText)) {
                    return true;
                }
                
                return false;
            });
        }
    }
    
    /**
     * Update entity count label.
     */
    private void updateEntityCount() {
        int totalCount = sceneManager.getEntityCount();
        int filteredCount = filteredEntities.size();
        
        if (filteredCount < totalCount) {
            entityCountLabel.setText(String.format("%d / %d entities", filteredCount, totalCount));
        } else {
            entityCountLabel.setText(String.format("%d entities", totalCount));
        }
    }
    
    /**
     * Refresh the entity list (force update).
     */
    public void refresh() {
        // The observable list automatically updates, but we can force a refresh if needed
        entityTable.refresh();
        updateEntityCount();
    }
    
    /**
     * Clear all entities from the scene.
     */
    private void clearAll() {
        if (sceneManager.getEntityCount() == 0) {
            return;
        }
        
        // Confirm deletion
        Alert alert = new Alert(Alert.AlertType.CONFIRMATION);
        alert.setTitle("Clear All Entities");
        alert.setHeaderText("Delete all entities from the scene?");
        alert.setContentText(String.format("This will delete %d entities and cannot be undone.", 
            sceneManager.getEntityCount()));
        
        alert.showAndWait().ifPresent(response -> {
            if (response == ButtonType.OK) {
                sceneManager.clearAll();
                if (selectionModel != null) {
                    selectionModel.setSelectedEntity(0);
                }
            }
        });
    }
    
    /**
     * Delete a specific entity.
     */
    private void deleteEntity(EntityData entity) {
        if (entity == null) {
            return;
        }
        
        // Confirm deletion
        Alert alert = new Alert(Alert.AlertType.CONFIRMATION);
        alert.setTitle("Delete Entity");
        alert.setHeaderText(String.format("Delete entity '%s' (ID: %d)?", 
            entity.getName(), entity.getEntityId()));
        alert.setContentText("This action cannot be undone.");
        
        alert.showAndWait().ifPresent(response -> {
            if (response == ButtonType.OK) {
                sceneManager.destroyEntity(entity.getEntityId());
                if (selectionModel != null && 
                    selectionModel.getSelectedEntityId() == entity.getEntityId()) {
                    selectionModel.setSelectedEntity(0);
                }
            }
        });
    }
    
    /**
     * Select an entity by ID in the table.
     */
    private void selectEntityById(int entityId) {
        for (EntityData entity : entityTable.getItems()) {
            if (entity.getEntityId() == entityId) {
                entityTable.getSelectionModel().select(entity);
                entityTable.scrollTo(entity);
                break;
            }
        }
    }
    
    /**
     * Get the currently selected entity in the table.
     */
    public EntityData getSelectedEntity() {
        return entityTable.getSelectionModel().getSelectedItem();
    }
}
