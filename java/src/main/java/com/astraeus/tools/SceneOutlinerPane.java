package com.astraeus.tools;

import com.astraeus.scene.EntityData;
import com.astraeus.scene.SceneManager;
import com.astraeus.ui.SelectionModel;
import javafx.application.Platform;
import javafx.collections.transformation.FilteredList;
import javafx.geometry.Insets;
import javafx.scene.control.*;
import javafx.scene.layout.*;

/**
 * Scene outliner pane showing hierarchical entity list.
 * Features:
 * - Virtualized TreeView for 50k+ entities without performance issues
 * - Search/filter functionality
 * - Integration with SelectionModel
 * - Refresh mechanism to sync with engine state
 */
public class SceneOutlinerPane extends VBox {
    
    private final SceneManager sceneManager;
    private final SelectionModel selectionModel;
    
    private TreeView<EntityData> treeView;
    private TextField searchField;
    private Label statusLabel;
    private FilteredList<EntityData> filteredEntities;
    
    private boolean autoRefresh = true;
    
    public SceneOutlinerPane(SceneManager sceneManager, SelectionModel selectionModel) {
        super(5);
        this.sceneManager = sceneManager;
        this.selectionModel = selectionModel;
        
        setPadding(new Insets(5));
        setMinWidth(250);
        setPrefWidth(300);
        setStyle("-fx-background-color: #f5f5f5;");
        
        buildUI();
        setupBindings();
        
        // Initial refresh
        refresh();
    }
    
    /**
     * Build the UI components.
     */
    private void buildUI() {
        // === Header ===
        HBox header = new HBox(10);
        header.setPadding(new Insets(5));
        
        Label titleLabel = new Label("Scene Outliner");
        titleLabel.setStyle("-fx-font-size: 14; -fx-font-weight: bold;");
        
        Region spacer = new Region();
        HBox.setHgrow(spacer, Priority.ALWAYS);
        
        Button refreshButton = new Button("⟳");
        refreshButton.setTooltip(new Tooltip("Refresh entity list"));
        refreshButton.setOnAction(e -> refresh());
        refreshButton.setStyle("-fx-font-size: 14;");
        
        header.getChildren().addAll(titleLabel, spacer, refreshButton);
        
        // === Search field ===
        searchField = new TextField();
        searchField.setPromptText("Search entities...");
        searchField.textProperty().addListener((obs, oldVal, newVal) -> applyFilter());
        
        // === Tree View (virtualized for performance) ===
        treeView = new TreeView<>();
        treeView.setShowRoot(false);
        treeView.getSelectionModel().setSelectionMode(SelectionMode.MULTIPLE);
        
        // Use virtualized cell factory for performance
        treeView.setCellFactory(tv -> new EntityTreeCell());
        
        // Handle selection changes
        treeView.getSelectionModel().selectedItemProperty().addListener((obs, oldVal, newVal) -> {
            if (newVal != null && newVal.getValue() != null) {
                EntityData entity = newVal.getValue();
                selectionModel.select(entity.getEntityId());
            }
        });
        
        VBox.setVgrow(treeView, Priority.ALWAYS);
        
        // === Status bar ===
        HBox statusBar = new HBox(5);
        statusBar.setPadding(new Insets(5));
        statusBar.setStyle("-fx-background-color: #e0e0e0; -fx-border-color: #cccccc; -fx-border-width: 1 0 0 0;");
        
        statusLabel = new Label("0 entities");
        statusLabel.setStyle("-fx-font-size: 11;");
        
        statusBar.getChildren().add(statusLabel);
        
        // === Context menu ===
        ContextMenu contextMenu = new ContextMenu();
        
        MenuItem selectItem = new MenuItem("Select");
        selectItem.setOnAction(e -> {
            TreeItem<EntityData> item = treeView.getSelectionModel().getSelectedItem();
            if (item != null && item.getValue() != null) {
                selectionModel.select(item.getValue().getEntityId());
            }
        });
        
        MenuItem deleteItem = new MenuItem("Delete");
        deleteItem.setOnAction(e -> {
            TreeItem<EntityData> item = treeView.getSelectionModel().getSelectedItem();
            if (item != null && item.getValue() != null) {
                sceneManager.destroyEntity(item.getValue().getEntityId());
                refresh();
            }
        });
        
        contextMenu.getItems().addAll(selectItem, deleteItem);
        treeView.setContextMenu(contextMenu);
        
        // === Add all to layout ===
        getChildren().addAll(header, searchField, treeView, statusBar);
    }
    
    /**
     * Setup data bindings.
     */
    private void setupBindings() {
        // Create filtered list
        filteredEntities = new FilteredList<>(sceneManager.getEntities(), entity -> true);
        
        // Listen to entity list changes for auto-refresh
        sceneManager.getEntities().addListener((javafx.collections.ListChangeListener.Change<? extends EntityData> change) -> {
            if (autoRefresh) {
                Platform.runLater(this::refresh);
            }
        });
        
        // Listen to selection model changes
        selectionModel.selectedEntityIdProperty().addListener((obs, oldVal, newVal) -> {
            if (newVal != null && newVal != 0) {
                selectEntityInTree(newVal);
            }
        });
    }
    
    /**
     * Refresh the tree view with current entities.
     */
    public void refresh() {
        TreeItem<EntityData> root = new TreeItem<>();
        
        // Add all entities (flat list for now; could be hierarchical later)
        for (EntityData entity : filteredEntities) {
            TreeItem<EntityData> item = new TreeItem<>(entity);
            root.getChildren().add(item);
        }
        
        treeView.setRoot(root);
        
        // Update status
        int totalCount = sceneManager.getEntityCount();
        int visibleCount = filteredEntities.size();
        
        if (totalCount == visibleCount) {
            statusLabel.setText(totalCount + " entities");
        } else {
            statusLabel.setText(visibleCount + " of " + totalCount + " entities");
        }
    }
    
    /**
     * Apply search filter.
     */
    private void applyFilter() {
        String searchText = searchField.getText().toLowerCase().trim();
        
        if (searchText.isEmpty()) {
            filteredEntities.setPredicate(entity -> true);
        } else {
            filteredEntities.setPredicate(entity -> {
                // Search by name or ID
                return entity.getName().toLowerCase().contains(searchText) ||
                       String.valueOf(entity.getEntityId()).contains(searchText);
            });
        }
        
        refresh();
    }
    
    /**
     * Select an entity in the tree view.
     */
    private void selectEntityInTree(int entityId) {
        // Find the tree item with matching entity ID
        TreeItem<EntityData> root = treeView.getRoot();
        if (root == null) {
            return;
        }
        
        for (TreeItem<EntityData> item : root.getChildren()) {
            if (item.getValue() != null && item.getValue().getEntityId() == entityId) {
                treeView.getSelectionModel().select(item);
                treeView.scrollTo(treeView.getRow(item));
                return;
            }
        }
    }
    
    /**
     * Set auto-refresh mode.
     */
    public void setAutoRefresh(boolean autoRefresh) {
        this.autoRefresh = autoRefresh;
    }
    
    /**
     * Custom tree cell for entity display.
     */
    private static class EntityTreeCell extends TreeCell<EntityData> {
        @Override
        protected void updateItem(EntityData entity, boolean empty) {
            super.updateItem(entity, empty);
            
            if (empty || entity == null) {
                setText(null);
                setGraphic(null);
                setStyle("");
            } else {
                // Display entity name and ID
                setText(entity.getName());
                
                // Add visibility indicator
                if (!entity.isVisible()) {
                    setStyle("-fx-text-fill: #999999; -fx-font-style: italic;");
                } else {
                    setStyle("");
                }
            }
        }
    }
}
