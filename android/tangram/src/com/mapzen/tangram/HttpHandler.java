package com.mapzen.tangram;

import com.squareup.okhttp.Cache;
import com.squareup.okhttp.Callback;
import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;
import com.squareup.okhttp.CacheControl;
import com.squareup.okhttp.ConnectionPool;

import java.io.File;
import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import android.util.Log;

public class HttpHandler {

    private static final int MAX_CONCURRENT_DOWNLOADS = 6;
  private OkHttpClient okClient;
    private Request.Builder okRequestBuilder;
    private CacheControl okCacheControl;
    private ConnectionPool okConnPool;
    private final ExecutorService executor;
    private final ScheduledExecutorService cancelator = Executors.newScheduledThreadPool(1);

    public HttpHandler() {
        okRequestBuilder = new Request.Builder();
        okClient = new OkHttpClient();
        okConnPool = new ConnectionPool(10, 60000);
        okClient.setConnectionPool(okConnPool);
        //okConnPool = okClient.getConnectionPool();

        okClient.setConnectTimeout(500, TimeUnit.MILLISECONDS);
        okClient.setReadTimeout(30, TimeUnit.SECONDS);
        okClient.setWriteTimeout(250, TimeUnit.MILLISECONDS);
        okCacheControl = new CacheControl.Builder().build();
        final int[] id = new int[1];

        executor = Executors.newFixedThreadPool(MAX_CONCURRENT_DOWNLOADS, new ThreadFactory() {
      @Override
      public Thread newThread(Runnable r) {
        Thread thread = new Thread(r, "Tangram Http " + (id[0]++));
        thread.setDaemon(true);
        thread.setPriority(Thread.NORM_PRIORITY - 1);
        return thread;
      }
    });
    }

    /**
     * Begin an HTTP request
     * @param url URL for the requested resource
     * @param cb Callback for handling request result
     * @return true if request was successfully started
     */
    public boolean onRequest(String url, final Callback cb) {

        okRequestBuilder = new Request.Builder();
        final Request request = okRequestBuilder
            .tag(url)
            .url(url)
            .cacheControl(okCacheControl)
            .build();

      try {
      executor.execute(new Runnable() {
        @Override
        public void run() {
           try {
             int count = okConnPool.getConnectionCount();

             long t = System.nanoTime();
            Response response = okClient.newCall(request).execute();

            Log.d("tangram", count + " >>> took: " + (int)((System.nanoTime() - t) / 1e6f));
            cb.onResponse(response);
          } catch (IOException e) {
            cb.onFailure(request, e);
            e.printStackTrace();
          }
        }
      });
    } catch (RejectedExecutionException e) {
      return false;
    }

      //okClient.newCall(request).enqueue(cb);
        return true;
    }

    /**
     * Cancel an HTTP request
     * @param url URL of the request to be cancelled
     */
    public void onCancel(final String url) {
    final long startNanos = System.nanoTime();
    cancelator.schedule(new Runnable() {
      @Override
      public void run() {
        Log.d("tangram", "Canceling call." + (System.nanoTime() - startNanos) / 1e9f);

        okClient.cancel(url);
        Log.d("tangram", "Canceled call" + (System.nanoTime() - startNanos) / 1e9f);
      }
    }, 1, TimeUnit.SECONDS);
    }

    /**
     * Cache map data in a directory with a specified size limit
     * @param directory Directory in which map data will be cached
     * @param maxSize Maximum size of data to cache, in bytes
     * @return true if cache was successfully created
     */
    public boolean setCache(File directory, long maxSize) {
        //  try {
        Cache okTileCache = new Cache(directory, maxSize);
        okClient.setCache(okTileCache);
        // } catch (IOException ignored) { return false; }
        return true;
    }

    public void shutdown() {
    executor.shutdown();
    try {
      executor.awaitTermination(10, TimeUnit.SECONDS);
    } catch (InterruptedException e) {
    }
  }
}
