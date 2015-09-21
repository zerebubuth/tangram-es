/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.3
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.mapzen.tangram;

public class DataSource {
  private long swigCPtr;
  private boolean swigCMemOwn;

  protected DataSource(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(DataSource obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        TangramJNI.delete_DataSource(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

    /**
     * Get the name of this data source
     * @return The name
     */
    public String getName() {
        return name();
    }

  public void update() {
    TangramJNI.DataSource_update(swigCPtr, this);
  }

  public void clear() {
    TangramJNI.DataSource_clear(swigCPtr, this);
  }

  public String name() {
    return TangramJNI.DataSource_name(swigCPtr, this);
  }

}
