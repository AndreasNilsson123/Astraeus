# Astraeus Java Test Suite

Comprehensive unit and component tests for the Astraeus Java frontend (FFM + JavaFX).

## Overview

This test suite validates:
- FFM memory layouts and ABI contract correctness
- NativeEngine lifecycle and error handling
- Viewport resize logic and camera propagation
- PixelBuffer management and frame delivery
- Tooling logic (telemetry, inspectors, picking)
- Test pattern generation for visual verification

## Test Structure

```
src/test/java/com/astraeus/
├── native_api/
│   ├── layout/
│   │   ├── LayoutValidatorTest.java      # FFM struct layout validation
│   │   └── ConstantsTest.java            # ABI constants verification
│   ├── lifecycle/
│   │   ├── MemorySegmentLifecycleTest.java  # Arena and segment tests
│   │   └── NativeEngineLifecycleTest.java   # Engine create/destroy tests
│   └── model/
│       └── (model value object tests)
├── rendering/
│   ├── buffers/
│   │   └── PixelBufferManagerTest.java   # Buffer allocation and update regions
│   └── viewport/
│       └── ViewportResizeLogicTest.java  # Size calculations and HiDPI
└── tools/
    ├── telemetry/
    │   └── TelemetryFormattingTest.java  # Frame time, FPS, memory formatting
    ├── inspector/
    │   └── InspectorLogicTest.java       # Entity selection and picking
    └── TestPatternGeneratorTest.java     # Debug pattern validation
```

## Test Categories

### 1. Unit Tests (Fast, Pure Java)
- **No native dependencies required**
- Layout math, sizing logic, wrapper behavior
- Run on all platforms without native library

Tests:
- `LayoutValidatorTest` - FFM layout correctness
- `MemorySegmentLifecycleTest` - Arena/segment lifecycle
- `ViewportResizeLogicTest` - Viewport dimension calculations
- `PixelBufferManagerTest` - Buffer capacity and update regions
- `TelemetryFormattingTest` - Formatting and display logic
- `InspectorLogicTest` - Entity selection and picking logic
- `TestPatternGeneratorTest` - Test pattern generation

### 2. Component Tests (Require Native Library)
- **Require native engine library**
- Use deterministic debug patterns
- Validate native integration

Tests:
- `NativeEngineLifecycleTest` - Engine creation and destruction

Note: Component tests are conditionally enabled via system property:
```
-Dastraeus.native.available=true
```

### 3. JavaFX Tests (Require Toolkit)
- **Require JavaFX toolkit**
- TestFX-based UI validation
- PixelBuffer/WritableImage correctness

(To be added based on specific UI testing needs)

## Running Tests

### All Tests
```bash
cd java
gradle test
```

### Specific Test Class
```bash
gradle test --tests LayoutValidatorTest
gradle test --tests "*.lifecycle.*"
```

### With Native Library
```bash
gradle test -Dastraeus.native.available=true
```

### Headless Mode (CI)
Tests are configured to run in headless mode automatically:
```properties
java.awt.headless=true
testfx.robot=glass
testfx.headless=true
prism.order=sw
```

## Test Output

### Console Output
Tests log key events (passed, skipped, failed) with full stack traces on failure.

### HTML Reports
Located at: `build/reports/tests/test/index.html`

### JUnit XML
Located at: `build/test-results/test/*.xml` (for CI integration)

## Writing New Tests

### Unit Test Template
```java
@DisplayName("Feature X Tests")
class FeatureXTest {
    
    @Test
    @DisplayName("Should do something specific")
    void featureX_shouldDoSomething() {
        // Arrange
        int input = 42;
        
        // Act
        int result = calculateSomething(input);
        
        // Assert
        assertThat(result).isEqualTo(84);
    }
}
```

### Component Test Template (Native Required)
```java
@EnabledIfSystemProperty(named = "astraeus.native.available", matches = "true")
@Test
@DisplayName("Native feature should work")
void nativeFeature_shouldWork() {
    assumeThat(nativeAvailable).isTrue();
    
    try (NativeEngine engine = new NativeEngine(800, 600, false)) {
        // Test native functionality
    } catch (Exception e) {
        assumeThat(false).as("Native library not available").isTrue();
    }
}
```

## Continuous Integration

### GitHub Actions
The test suite is designed to run in CI:

1. **Fast unit tests** run on all platforms (no native lib required)
2. **Component tests** run only when native lib is available
3. **JavaFX tests** run only on compatible agents (with display)

### Test Splitting
- **PR validation**: Fast unit tests only
- **Nightly builds**: Full suite including component tests
- **Release validation**: Full suite with all platforms

## Coverage

### Current Coverage
- FFM layout validation: ✅ Complete
- Memory lifecycle: ✅ Complete
- Viewport resize logic: ✅ Complete
- PixelBuffer management: ✅ Complete
- Telemetry formatting: ✅ Complete
- Inspector logic: ✅ Complete
- Test pattern generation: ✅ Complete

### Planned Coverage
- NativeEngine error handling: 🔄 In progress
- Frame delivery pipeline: 📋 Planned
- JavaFX integration: 📋 Planned
- Golden image comparison: 📋 Planned

## Troubleshooting

### Tests Skip with "Native library not available"
- Component tests require native library to be built and available
- Set system property: `-Dastraeus.native.available=true`
- Ensure native library is in `java.library.path` or `LD_LIBRARY_PATH`

### JavaFX Tests Fail in Headless
- Ensure headless mode is configured (should be automatic)
- Check that Monocle dependency is present
- Some tests may require a display and should be skipped in CI

### Test Timeouts
- Increase timeout for slow systems: `@Timeout(10, unit = TimeUnit.SECONDS)`
- Check that native operations complete within reasonable time

## Dependencies

- **JUnit 5** (Jupiter): Test framework
- **AssertJ**: Fluent assertions
- **Mockito**: Mocking framework
- **TestFX**: JavaFX testing
- **Monocle**: Headless JavaFX runtime

## Maintenance

### Layout Changes
When ABI structs change, update:
1. Expected sizes/alignments in `LayoutValidatorTest`
2. Field offset tests if added
3. Re-run codegen to update generated layouts

### Adding New Tests
1. Place test in appropriate package (native_api/rendering/tools)
2. Follow naming convention: `*Test.java`
3. Use descriptive `@DisplayName` annotations
4. Include both positive and negative test cases
5. Document any special requirements (native lib, JavaFX, etc.)

## Best Practices

1. **Test Isolation**: Each test should be independent
2. **Descriptive Names**: Use `@DisplayName` for human-readable test names
3. **Arrange-Act-Assert**: Follow AAA pattern for clarity
4. **Assume, Not Skip**: Use `assumeThat()` for conditional tests
5. **Fast Tests**: Keep unit tests fast (<100ms each)
6. **No External State**: Don't depend on files, network, or external services
7. **Clean Resources**: Use try-with-resources or `@AfterEach` for cleanup

## See Also

- [Layout Validator Documentation](../../main/java/com/astraeus/native_api/layout/LayoutValidator.java)
- [Test Pattern Generator](../../main/java/com/astraeus/tools/TestPatternGenerator.java)
- [Viewport Documentation](../../main/java/com/astraeus/rendering/VIEWPORT_README.md)
