plugins {
    java
    application
    id("org.openjfx.javafxplugin") version "0.1.0"
}

group = "com.astraeus"
version = "0.1.0"
description = "JavaFX frontend and FFM bindings for Astraeus 3D visualization engine"

repositories {
    mavenCentral()
}

java {
    toolchain {
        languageVersion.set(JavaLanguageVersion.of(25))
    }
}

// Match your Maven <sourceDirectory>java/src/main/java</sourceDirectory>
sourceSets {
    named("main") {
        java.setSrcDirs(listOf("src/main/java"))
        resources.setSrcDirs(listOf("src/main/resources"))
    }
    named("test") {
        java.setSrcDirs(listOf("src/test/java"))
        resources.setSrcDirs(listOf("src/test/resources"))
    }
}

application {
    mainClass.set("com.astraeus.ui.AstraeusApp")
    applicationDefaultJvmArgs = listOf("--enable-preview")
}

javafx {
    // For JDK 25, JavaFX 25.x is the typical pairing. :contentReference[oaicite:2]{index=2}
    version = "25.0.1"
    modules = listOf("javafx.controls", "javafx.graphics", "javafx.fxml")
}

// Enable preview features for compile + run (+ tests if you add them)
tasks.withType<JavaCompile>().configureEach {
    options.encoding = "UTF-8"
    options.compilerArgs.add("--enable-preview")
}

tasks.withType<Test>().configureEach {
    jvmArgs("--enable-preview")
}

tasks.withType<JavaExec>().configureEach {
    jvmArgs("--enable-preview")
}
