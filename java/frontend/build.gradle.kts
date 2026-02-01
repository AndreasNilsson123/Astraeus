plugins {
    application
    id("org.openjfx.javafxplugin") version "0.1.0"
}

description = "JavaFX frontend and FFM bindings for Astraeus 3D visualization engine"

application {
    mainClass.set("com.astraeus.ui.AstraeusApp")
    applicationDefaultJvmArgs = listOf("--enable-preview")
}

javafx {
    version = "25.0.1"
    modules = listOf("javafx.controls", "javafx.graphics", "javafx.fxml")
}

// ---- Wire generated sources from :codegen into this project ----

// Generated sources are placed in frontend's build directory by codegen
val generatedSourcesDir = layout.buildDirectory.dir("generated/sources/astraeusAbi/main")

sourceSets {
    named("main") {
        java.srcDir(generatedSourcesDir)
    }
    
    // Standard test source set
    named("test") {
        java.srcDir("src/test/java")
        resources.srcDir("src/test/resources")
    }
}

// ---- Test dependencies ----

dependencies {
    // JUnit 5 (Jupiter)
    testImplementation("org.junit.jupiter:junit-jupiter-api:5.10.1")
    testRuntimeOnly("org.junit.jupiter:junit-jupiter-engine:5.10.1")
    testImplementation("org.junit.jupiter:junit-jupiter-params:5.10.1")
    testRuntimeOnly("org.junit.platform:junit-platform-launcher:1.10.1")
    
    // AssertJ for fluent assertions
    testImplementation("org.assertj:assertj-core:3.24.2")
    
    // Mockito for mocking
    testImplementation("org.mockito:mockito-core:5.8.0")
    testImplementation("org.mockito:mockito-junit-jupiter:5.8.0")
    
    // TestFX for JavaFX testing
    testImplementation("org.testfx:testfx-core:4.0.18")
    testImplementation("org.testfx:testfx-junit5:4.0.18")
    
    // Monocle for headless JavaFX testing
    testImplementation("org.testfx:openjfx-monocle:jdk-12.0.1+2")
}

// ---- Test configuration ----

tasks.test {
    useJUnitPlatform()
    
    // Enable preview features for tests
    jvmArgs("--enable-preview")
    
    // Configure headless mode for JavaFX tests
    systemProperty("java.awt.headless", "true")
    systemProperty("testfx.robot", "glass")
    systemProperty("testfx.headless", "true")
    systemProperty("prism.order", "sw")
    systemProperty("prism.text", "t2k")
    
    // Generate test reports
    reports {
        html.required.set(true)
        junitXml.required.set(true)
    }
    
    // Test output configuration
    testLogging {
        events("passed", "skipped", "failed")
        exceptionFormat = org.gradle.api.tasks.testing.logging.TestExceptionFormat.FULL
        showStandardStreams = false
    }
}

tasks.named<JavaCompile>("compileJava") {
    dependsOn(":codegen:generateAbi")
}

tasks.named<JavaCompile>("compileTestJava") {
    dependsOn(":codegen:generateAbi")
}
