package com.astraeus.util;

import java.util.concurrent.*;

/**
 * Threading utility functions for the Astraeus application.
 */
public class ThreadingUtils {
    
    private static final ExecutorService BACKGROUND_EXECUTOR = 
        Executors.newFixedThreadPool(
            Math.max(2, Runtime.getRuntime().availableProcessors() - 1),
            r -> {
                Thread t = new Thread(r);
                t.setDaemon(true);
                t.setName("Astraeus-Background-" + t.getId());
                return t;
            }
        );
    
    private static final ScheduledExecutorService SCHEDULED_EXECUTOR =
        Executors.newScheduledThreadPool(2, r -> {
            Thread t = new Thread(r);
            t.setDaemon(true);
            t.setName("Astraeus-Scheduled-" + t.getId());
            return t;
        });
    
    /**
     * Run task on a background thread.
     */
    public static CompletableFuture<Void> runAsync(Runnable task) {
        return CompletableFuture.runAsync(task, BACKGROUND_EXECUTOR);
    }
    
    /**
     * Run task on a background thread and return result.
     */
    public static <T> CompletableFuture<T> supplyAsync(Callable<T> task) {
        return CompletableFuture.supplyAsync(() -> {
            try {
                return task.call();
            } catch (Exception e) {
                throw new CompletionException(e);
            }
        }, BACKGROUND_EXECUTOR);
    }
    
    /**
     * Schedule a task to run once after a delay.
     */
    public static ScheduledFuture<?> scheduleOnce(Runnable task, long delay, TimeUnit unit) {
        return SCHEDULED_EXECUTOR.schedule(task, delay, unit);
    }
    
    /**
     * Schedule a task to run repeatedly with fixed delay.
     */
    public static ScheduledFuture<?> scheduleWithFixedDelay(
            Runnable task, 
            long initialDelay, 
            long delay, 
            TimeUnit unit) {
        return SCHEDULED_EXECUTOR.scheduleWithFixedDelay(task, initialDelay, delay, unit);
    }
    
    /**
     * Schedule a task to run repeatedly at fixed rate.
     */
    public static ScheduledFuture<?> scheduleAtFixedRate(
            Runnable task,
            long initialDelay,
            long period,
            TimeUnit unit) {
        return SCHEDULED_EXECUTOR.scheduleAtFixedRate(task, initialDelay, period, unit);
    }
    
    /**
     * Sleep for the specified duration, handling interrupts.
     */
    public static void sleep(long millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
    
    /**
     * Get the background executor.
     */
    public static ExecutorService getBackgroundExecutor() {
        return BACKGROUND_EXECUTOR;
    }
    
    /**
     * Get the scheduled executor.
     */
    public static ScheduledExecutorService getScheduledExecutor() {
        return SCHEDULED_EXECUTOR;
    }
    
    /**
     * Shutdown all executors (call on application exit).
     */
    public static void shutdown() {
        BACKGROUND_EXECUTOR.shutdown();
        SCHEDULED_EXECUTOR.shutdown();
    }
}
