/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.3
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.mapzen.tangram;

public class TangramJNI {
  public final static native long new_Properties();
  public final static native void Properties_clear(long jarg1, Properties jarg1_);
  public final static native boolean Properties_contains(long jarg1, Properties jarg1_, String jarg2);
  public final static native float Properties_getNumeric(long jarg1, Properties jarg1_, String jarg2);
  public final static native String Properties_getString(long jarg1, Properties jarg1_, String jarg2);
  public final static native void Properties_add__SWIG_0(long jarg1, Properties jarg1_, String jarg2, String jarg3);
  public final static native void Properties_add__SWIG_1(long jarg1, Properties jarg1_, String jarg2, float jarg3);
  public final static native void delete_Properties(long jarg1);
  public final static native long new_LngLat__SWIG_0();
  public final static native long new_LngLat__SWIG_1(double jarg1, double jarg2);
  public final static native long new_LngLat__SWIG_2(long jarg1, LngLat jarg1_);
  public final static native long LngLat_set__SWIG_0(long jarg1, LngLat jarg1_, long jarg2, LngLat jarg2_);
  public final static native boolean LngLat_equals(long jarg1, LngLat jarg1_, long jarg2, LngLat jarg2_);
  public final static native void LngLat_longitude_set(long jarg1, LngLat jarg1_, double jarg2);
  public final static native double LngLat_longitude_get(long jarg1, LngLat jarg1_);
  public final static native void LngLat_latitude_set(long jarg1, LngLat jarg1_, double jarg2);
  public final static native double LngLat_latitude_get(long jarg1, LngLat jarg1_);
  public final static native void LngLat_setLngLat(long jarg1, LngLat jarg1_, double jarg2, double jarg3);
  public final static native void delete_LngLat(long jarg1);
  public final static native long new_Coordinates__SWIG_0();
  public final static native long new_Coordinates__SWIG_1(long jarg1);
  public final static native long Coordinates_size(long jarg1, Coordinates jarg1_);
  public final static native long Coordinates_capacity(long jarg1, Coordinates jarg1_);
  public final static native void Coordinates_reserve(long jarg1, Coordinates jarg1_, long jarg2);
  public final static native boolean Coordinates_isEmpty(long jarg1, Coordinates jarg1_);
  public final static native void Coordinates_clear(long jarg1, Coordinates jarg1_);
  public final static native void Coordinates_add(long jarg1, Coordinates jarg1_, long jarg2, LngLat jarg2_);
  public final static native long Coordinates_get(long jarg1, Coordinates jarg1_, int jarg2);
  public final static native void Coordinates_set(long jarg1, Coordinates jarg1_, int jarg2, long jarg3, LngLat jarg3_);
  public final static native void Coordinates_append(long jarg1, Coordinates jarg1_, double jarg2, double jarg3);
  public final static native void delete_Coordinates(long jarg1);
  public final static native long new_Tags__SWIG_0();
  public final static native long new_Tags__SWIG_1(long jarg1, Tags jarg1_);
  public final static native long Tags_size(long jarg1, Tags jarg1_);
  public final static native boolean Tags_empty(long jarg1, Tags jarg1_);
  public final static native void Tags_clear(long jarg1, Tags jarg1_);
  public final static native String Tags_get(long jarg1, Tags jarg1_, String jarg2);
  public final static native void Tags_set(long jarg1, Tags jarg1_, String jarg2, String jarg3);
  public final static native void Tags_del(long jarg1, Tags jarg1_, String jarg2);
  public final static native boolean Tags_has_key(long jarg1, Tags jarg1_, String jarg2);
  public final static native void delete_Tags(long jarg1);
  public final static native void DataSource_update(long jarg1, DataSource jarg1_);
  public final static native void DataSource_clear(long jarg1, DataSource jarg1_);
  public final static native String DataSource_name(long jarg1, DataSource jarg1_);
  public final static native void delete_DataSource(long jarg1);
  public final static native long new_MapData(String jarg1, String jarg2);
  public final static native void MapData_addData(long jarg1, MapData jarg1_, String jarg2);
  public final static native void MapData_addPoint(long jarg1, MapData jarg1_, long jarg2, Tags jarg2_, double[] jarg3);
  public final static native void MapData_addLine__SWIG_0(long jarg1, MapData jarg1_, long jarg2, Tags jarg2_, double[] jarg3, int jarg4);
  public final static native void MapData_addLine__SWIG_1(long jarg1, MapData jarg1_, long jarg2, Tags jarg2_, long jarg3, Coordinates jarg3_);
  public final static native void MapData_addPoly(long jarg1, MapData jarg1_, long jarg2, Tags jarg2_, double[] jarg3, int[] jarg4, int jarg5);
  public final static native void delete_MapData(long jarg1);
  public final static native void initialize(String jarg1);
  public final static native void setupGL();
  public final static native void resize(int jarg1, int jarg2);
  public final static native void render();
  public final static native void setPosition(double jarg1, double jarg2);
  public final static native void setZoom(float jarg1);
  public final static native float getZoom();
  public final static native void setRotation(float jarg1);
  public final static native float getRotation();
  public final static native void setTilt(float jarg1);
  public final static native float getTilt();
  public final static native void setPixelScale(float jarg1);
  public final static native void handleTapGesture(float jarg1, float jarg2);
  public final static native void handlePanGesture(float jarg1, float jarg2, float jarg3, float jarg4);
  public final static native void handleDoubleTapGesture(float jarg1, float jarg2);
  public final static native void handlePinchGesture(float jarg1, float jarg2, float jarg3, float jarg4);
  public final static native void handleShoveGesture(float jarg1);
  public final static native int addDataSource(long jarg1, DataSource jarg1_);
  public final static native void clearDataSource(long jarg1, DataSource jarg1_, boolean jarg2, boolean jarg3);
  public final static native long MapData_SWIGSmartPtrUpcast(long jarg1);
}
