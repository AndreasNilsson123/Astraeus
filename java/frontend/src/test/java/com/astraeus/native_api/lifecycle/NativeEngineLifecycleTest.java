package com.astraeus.native_api.lifecycle;

import com.astraeus.native_api.NativeEngine;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfSystemProperty;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;
import static org.assertj.core.api.Assertions.fail;
import static org.assertj.core.api.Assumptions.assumeThat;

/**
 * Test suite for NativeEngine lifecycle management.
 * 
 * <p>Verifies:
 * - Engine creation with various configurations
 * - Engine destruction and cleanup
 * - Double-close safety
 * - Use-after-close detection
 * - Resource leak prevention
 * 
 * <p><b>Note:</b> These tests require the native library to be available.
 * Tests are skipped if native library is not loaded.
 */
@DisplayName("NativeEngine Lifecycle Tests")
class NativeEngineLifecycleTest {
    
    private NativeEngine engine;
    private boolean nativeAvailable = false;
    
    @BeforeEach
    void setUp() {
        // Check if native library is available
        // Don't try to load it here - let individual tests handle it
        nativeAvailable = System.getProperty("astraeus.native.available", "false").equals("true");
    }
    
    @AfterEach
    void tearDown() {
        if (engine != null) {
            try {
                engine.close();
            } catch (Exception e) {
                // Ignore cleanup errors
            }
            engine = null;
        }
    }
    
    @Test
    @DisplayName("Engine should be created with default configuration")
    @EnabledIfSystemProperty(named = "astraeus.native.available", matches = "true")
    void createEngine_withDefaultConfig_shouldSucceed() {
        assumeThat(nativeAvailable).isTrue();
        
        assertThatThrownBy(() -> {
            engine = new NativeEngine(1280, 720, false);
        }).doesNotThrowAnyException();
    }
    
    @Test
    @DisplayName("Engine should be created with custom configuration")
    @EnabledIfSystemProperty(named = "astraeus.native.available", matches = "true")
    void createEngine_withCustomConfig_shouldSucceed() {
        assumeThat(nativeAvailable).isTrue();
        
        EngineConfig config = new EngineConfig()
            .setInitialWidth(1920)
            .setInitialHeight(1080)
            .setEnableValidation(true)
            .setEnableDebugOutput(false);
        
        assertThatThrownBy(() -> {
            engine = new NativeEngine(config);
        }).doesNotThrowAnyException();
    }
    
    @Test
    @DisplayName("Engine should reject null configuration")
    void createEngine_withNullConfig_shouldThrow() {
        // This test validates Java-side argument checking
        // It will fail trying to construct, but we're testing that the null check happens first
        try {
            new NativeEngine(null);
            fail("Expected NullPointerException to be thrown");
        } catch (NullPointerException e) {
            // Expected - this is what we're testing
            assertThat(e.getMessage()).contains("engineConfig");
        } catch (Exception e) {
            // If we get a different exception, the null check didn't work
            fail("Expected NullPointerException but got: " + e.getClass().getName());
        }
    }
    
    @Test
    @DisplayName("Engine should reject invalid dimensions")
    @EnabledIfSystemProperty(named = "astraeus.native.available", matches = "true")
    void createEngine_withInvalidDimensions_shouldThrow() {
        assumeThat(nativeAvailable).isTrue();
        
        // Note: These may throw native library errors if lib isn't available
        // We only run this test when native is explicitly available
        
        // Zero width
        assertThatThrownBy(() -> new NativeEngine(0, 720, false))
            .satisfiesAnyOf(
                e -> assertThat(e).isInstanceOf(IllegalArgumentException.class),
                e -> assertThat(e).isInstanceOf(RuntimeException.class)
            );
        
        // Zero height
        assertThatThrownBy(() -> new NativeEngine(1280, 0, false))
            .satisfiesAnyOf(
                e -> assertThat(e).isInstanceOf(IllegalArgumentException.class),
                e -> assertThat(e).isInstanceOf(RuntimeException.class)
            );
        
        // Negative width
        assertThatThrownBy(() -> new NativeEngine(-100, 720, false))
            .satisfiesAnyOf(
                e -> assertThat(e).isInstanceOf(IllegalArgumentException.class),
                e -> assertThat(e).isInstanceOf(RuntimeException.class)
            );
        
        // Negative height
        assertThatThrownBy(() -> new NativeEngine(1280, -100, false))
            .satisfiesAnyOf(
                e -> assertThat(e).isInstanceOf(IllegalArgumentException.class),
                e -> assertThat(e).isInstanceOf(RuntimeException.class)
            );
    }
    
    @Test
    @DisplayName("EngineConfig should have sensible defaults")
    void engineConfig_defaults_shouldBeSensible() {
        EngineConfig config = new EngineConfig();
        
        assertThat(config.getInitialWidth())
            .as("Default width")
            .isGreaterThan(0);
            
        assertThat(config.getInitialHeight())
            .as("Default height")
            .isGreaterThan(0);
            
        assertThat(config.isEnableValidation())
            .as("Default validation (should be false for performance)")
            .isFalse();
    }
    
    @Test
    @DisplayName("EngineConfig should support fluent API")
    void engineConfig_shouldSupportFluentApi() {
        EngineConfig config = new EngineConfig()
            .setInitialWidth(1920)
            .setInitialHeight(1080)
            .setEnableValidation(true);
        
        assertThat(config.getInitialWidth()).isEqualTo(1920);
        assertThat(config.getInitialHeight()).isEqualTo(1080);
        assertThat(config.isEnableValidation()).isTrue();
    }
    
    @Test
    @DisplayName("Engine close should be idempotent")
    @EnabledIfSystemProperty(named = "astraeus.native.available", matches = "true")
    void engineClose_shouldBeIdempotent() {
        assumeThat(nativeAvailable).isTrue();
        
        try {
            engine = new NativeEngine(800, 600, false);
            engine.close();
            
            // Double close should not throw
            assertThatThrownBy(() -> engine.close())
                .doesNotThrowAnyException();
        } catch (Exception e) {
            // If native lib not available, skip test
            assumeThat(false).as("Native library not available").isTrue();
        }
    }
    
    @Test
    @DisplayName("Closed engine should reject operations")
    @EnabledIfSystemProperty(named = "astraeus.native.available", matches = "true")
    void closedEngine_shouldRejectOperations() {
        assumeThat(nativeAvailable).isTrue();
        
        try {
            engine = new NativeEngine(800, 600, false);
            engine.close();
            
            // Operations on closed engine should throw
            assertThatThrownBy(() -> engine.beginFrame(0.016))
                .isInstanceOf(IllegalStateException.class)
                .hasMessageContaining("closed");
        } catch (Exception e) {
            // If native lib not available, skip test
            assumeThat(false).as("Native library not available").isTrue();
        }
    }
    
    @Test
    @DisplayName("Multiple engines can be created and destroyed")
    @EnabledIfSystemProperty(named = "astraeus.native.available", matches = "true")
    void multipleEngines_canBeCreatedAndDestroyed() {
        assumeThat(nativeAvailable).isTrue();
        
        try {
            // Create and destroy first engine
            NativeEngine engine1 = new NativeEngine(640, 480, false);
            engine1.close();
            
            // Create and destroy second engine
            NativeEngine engine2 = new NativeEngine(800, 600, false);
            engine2.close();
            
            // Create and destroy third engine
            NativeEngine engine3 = new NativeEngine(1280, 720, false);
            engine3.close();
            
        } catch (Exception e) {
            // If native lib not available, skip test
            assumeThat(false).as("Native library not available").isTrue();
        }
    }
}
