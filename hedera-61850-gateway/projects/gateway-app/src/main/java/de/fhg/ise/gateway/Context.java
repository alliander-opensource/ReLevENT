package de.fhg.ise.gateway;

import com.google.gson.ExclusionStrategy;
import com.google.gson.FieldAttributes;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.annotations.Expose;

import java.lang.annotation.Annotation;

/**
 * Holds shared objects
 */
public class Context {

    public static final Gson GSON = new GsonBuilder().addSerializationExclusionStrategy(new ExclusionStrategy() {
        @Override
        public boolean shouldSkipField(FieldAttributes f) {
            for (Annotation a : f.getAnnotations()) {
                if (a instanceof Expose && ((Expose) a).serialize() == false) {
                    return true;
                }
            }
            return false;
        }

        @Override
        public boolean shouldSkipClass(Class<?> clazz) {
            return false;
        }
    }).create();
}
