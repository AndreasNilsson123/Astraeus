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

val codegenGeneratedDir = project(":codegen").layout.buildDirectory.dir("generated/sources/ffm/main")

sourceSets {
    named("main") {
        java.srcDir(codegenGeneratedDir)
    }
}

tasks.named<JavaCompile>("compileJava") {
    dependsOn(":codegen:generateBindings")
}
