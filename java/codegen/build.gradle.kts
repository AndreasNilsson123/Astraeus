plugins {
    java
    application
}

application {
    mainClass.set("com.astraeus.codegen.CodegenCli")
}

val schemaFile = project.file("../../engine/api/abi_structs_schema.yaml")
val generatedCppDir = project.file("../../engine/generated")
val generatedJavaDir = layout.buildDirectory.dir("generated/sources/astraeusAbi/main")

tasks.register<JavaExec>("generateAbi") {
    group = "codegen"
    description = "Generates ABI code (C++ headers and Java layouts) from schema"

    dependsOn(tasks.named("classes"))

    // Run the CodegenCli
    classpath = sourceSets["main"].runtimeClasspath
    mainClass.set("com.astraeus.codegen.CodegenCli")

    // No args needed - will auto-detect repo and use default paths
    
    // Declare inputs for up-to-date checks
    inputs.file(schemaFile)
    
    // Declare outputs for up-to-date checks
    outputs.dir(generatedCppDir)
    outputs.dir(generatedJavaDir)
    
    doFirst {
        println("Generating ABI code from schema: ${schemaFile.absolutePath}")
    }
}
