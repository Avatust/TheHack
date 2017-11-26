package com.example.thehack;

import com.google.gson.annotations.SerializedName;

/**
 * Created by martin on 11/26/17.
 */

public class Impact {
    @SerializedName("Body")
    public final Impact.Body body;

    public Impact(Impact.Body body) {
        this.body = body;
    }

    public static class Body {
        @SerializedName("Timestamp")
        public final long timestamp;

        @SerializedName("Headers")
        public final Impact.Headers header;

        public Body(long timestamp, Impact.Headers header) {
            this.timestamp = timestamp;
           // this.array = array;
            this.header = header;
        }
    }



    public static class Headers {
        @SerializedName("Param0")
        public final int param0;

        public Headers(int param0) {
            this.param0 = param0;
        }
    }
}
