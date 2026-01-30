package com.astraeus.scene;

import javafx.beans.property.*;

/**
 * Client-side entity data model.
 * Tracks entity properties that can be edited in the inspector.
 * Changes are synchronized to the engine via NativeEngine API.
 */
public class EntityData {
    
    private final int entityId;
    private final StringProperty name;
    
    // Transform properties
    private final DoubleProperty posX;
    private final DoubleProperty posY;
    private final DoubleProperty posZ;
    
    private final DoubleProperty rotX;
    private final DoubleProperty rotY;
    private final DoubleProperty rotZ;
    
    private final DoubleProperty scaleX;
    private final DoubleProperty scaleY;
    private final DoubleProperty scaleZ;
    
    // Rendering properties
    private final BooleanProperty visible;
    
    // Color properties
    private final DoubleProperty colorR;
    private final DoubleProperty colorG;
    private final DoubleProperty colorB;
    private final DoubleProperty colorA;
    
    // Trail properties
    private final IntegerProperty trailMaxPoints;
    
    public EntityData(int entityId) {
        this.entityId = entityId;
        this.name = new SimpleStringProperty("Entity " + entityId);
        
        // Default transform
        this.posX = new SimpleDoubleProperty(0.0);
        this.posY = new SimpleDoubleProperty(0.0);
        this.posZ = new SimpleDoubleProperty(0.0);
        
        this.rotX = new SimpleDoubleProperty(0.0);
        this.rotY = new SimpleDoubleProperty(0.0);
        this.rotZ = new SimpleDoubleProperty(0.0);
        
        this.scaleX = new SimpleDoubleProperty(1.0);
        this.scaleY = new SimpleDoubleProperty(1.0);
        this.scaleZ = new SimpleDoubleProperty(1.0);
        
        // Default rendering
        this.visible = new SimpleBooleanProperty(true);
        
        // Default color (white)
        this.colorR = new SimpleDoubleProperty(1.0);
        this.colorG = new SimpleDoubleProperty(1.0);
        this.colorB = new SimpleDoubleProperty(1.0);
        this.colorA = new SimpleDoubleProperty(1.0);
        
        // Default trail (disabled)
        this.trailMaxPoints = new SimpleIntegerProperty(0);
    }
    
    // ==================== Getters ====================
    
    public int getEntityId() {
        return entityId;
    }
    
    public String getName() {
        return name.get();
    }
    
    public double getPosX() {
        return posX.get();
    }
    
    public double getPosY() {
        return posY.get();
    }
    
    public double getPosZ() {
        return posZ.get();
    }
    
    public double getRotX() {
        return rotX.get();
    }
    
    public double getRotY() {
        return rotY.get();
    }
    
    public double getRotZ() {
        return rotZ.get();
    }
    
    public double getScaleX() {
        return scaleX.get();
    }
    
    public double getScaleY() {
        return scaleY.get();
    }
    
    public double getScaleZ() {
        return scaleZ.get();
    }
    
    public boolean isVisible() {
        return visible.get();
    }
    
    public double getColorR() {
        return colorR.get();
    }
    
    public double getColorG() {
        return colorG.get();
    }
    
    public double getColorB() {
        return colorB.get();
    }
    
    public double getColorA() {
        return colorA.get();
    }
    
    public int getTrailMaxPoints() {
        return trailMaxPoints.get();
    }
    
    // ==================== Setters ====================
    
    public void setName(String name) {
        this.name.set(name);
    }
    
    public void setPosX(double value) {
        this.posX.set(value);
    }
    
    public void setPosY(double value) {
        this.posY.set(value);
    }
    
    public void setPosZ(double value) {
        this.posZ.set(value);
    }
    
    public void setRotX(double value) {
        this.rotX.set(value);
    }
    
    public void setRotY(double value) {
        this.rotY.set(value);
    }
    
    public void setRotZ(double value) {
        this.rotZ.set(value);
    }
    
    public void setScaleX(double value) {
        this.scaleX.set(value);
    }
    
    public void setScaleY(double value) {
        this.scaleY.set(value);
    }
    
    public void setScaleZ(double value) {
        this.scaleZ.set(value);
    }
    
    public void setVisible(boolean value) {
        this.visible.set(value);
    }
    
    public void setColorR(double value) {
        this.colorR.set(value);
    }
    
    public void setColorG(double value) {
        this.colorG.set(value);
    }
    
    public void setColorB(double value) {
        this.colorB.set(value);
    }
    
    public void setColorA(double value) {
        this.colorA.set(value);
    }
    
    public void setTrailMaxPoints(int value) {
        this.trailMaxPoints.set(value);
    }
    
    // ==================== Properties ====================
    
    public StringProperty nameProperty() {
        return name;
    }
    
    public DoubleProperty posXProperty() {
        return posX;
    }
    
    public DoubleProperty posYProperty() {
        return posY;
    }
    
    public DoubleProperty posZProperty() {
        return posZ;
    }
    
    public DoubleProperty rotXProperty() {
        return rotX;
    }
    
    public DoubleProperty rotYProperty() {
        return rotY;
    }
    
    public DoubleProperty rotZProperty() {
        return rotZ;
    }
    
    public DoubleProperty scaleXProperty() {
        return scaleX;
    }
    
    public DoubleProperty scaleYProperty() {
        return scaleY;
    }
    
    public DoubleProperty scaleZProperty() {
        return scaleZ;
    }
    
    public BooleanProperty visibleProperty() {
        return visible;
    }
    
    public DoubleProperty colorRProperty() {
        return colorR;
    }
    
    public DoubleProperty colorGProperty() {
        return colorG;
    }
    
    public DoubleProperty colorBProperty() {
        return colorB;
    }
    
    public DoubleProperty colorAProperty() {
        return colorA;
    }
    
    public IntegerProperty trailMaxPointsProperty() {
        return trailMaxPoints;
    }
    
    @Override
    public String toString() {
        return String.format("%s (ID: %d)", getName(), entityId);
    }
}
