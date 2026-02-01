package com.astraeus.util;

import java.io.*;
import java.nio.file.*;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * File utility functions for the Astraeus Java application.
 */
public class FileUtils {
    
    /**
     * Read entire file as a string.
     */
    public static String readString(Path path) throws IOException {
        return Files.readString(path);
    }
    
    /**
     * Read all lines from a file.
     */
    public static List<String> readAllLines(Path path) throws IOException {
        return Files.readAllLines(path);
    }
    
    /**
     * Write string to file.
     */
    public static void writeString(Path path, String content) throws IOException {
        Files.writeString(path, content);
    }
    
    /**
     * Check if file exists.
     */
    public static boolean exists(Path path) {
        return Files.exists(path);
    }
    
    /**
     * Create directories if they don't exist.
     */
    public static void createDirectories(Path path) throws IOException {
        Files.createDirectories(path);
    }
    
    /**
     * Get file extension (without dot).
     */
    public static String getExtension(Path path) {
        String fileName = path.getFileName().toString();
        int dotIndex = fileName.lastIndexOf('.');
        if (dotIndex > 0 && dotIndex < fileName.length() - 1) {
            return fileName.substring(dotIndex + 1).toLowerCase();
        }
        return "";
    }
    
    /**
     * List files in directory matching a pattern.
     */
    public static List<Path> listFiles(Path directory, String glob) throws IOException {
        try (Stream<Path> stream = Files.list(directory)) {
            PathMatcher matcher = FileSystems.getDefault().getPathMatcher("glob:" + glob);
            return stream
                .filter(path -> matcher.matches(path.getFileName()))
                .collect(Collectors.toList());
        }
    }
    
    /**
     * Copy file from source to destination.
     */
    public static void copyFile(Path source, Path destination) throws IOException {
        Files.copy(source, destination, StandardCopyOption.REPLACE_EXISTING);
    }
    
    /**
     * Delete file or directory recursively.
     */
    public static void deleteRecursively(Path path) throws IOException {
        if (Files.isDirectory(path)) {
            try (Stream<Path> stream = Files.walk(path)) {
                stream.sorted((a, b) -> -a.compareTo(b)) // Reverse order for deletion
                      .forEach(p -> {
                          try {
                              Files.delete(p);
                          } catch (IOException e) {
                              System.err.println("Failed to delete: " + p);
                          }
                      });
            }
        } else {
            Files.deleteIfExists(path);
        }
    }
}
